import { useState } from 'react';

const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CHAR_GEAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const CHAR_RAW_SENSOR_UUID = '8c1df81c-d7d8-4f8e-bd71-c0068dc26a87';
const CHAR_CALIBRATE_UUID = '59c62376-74fc-487b-83ee-bf35a4d468eb';

export function useBluetooth() {
  const [device, setDevice] = useState<BluetoothDevice | null>(null);
  const [connected, setConnected] = useState(false);
  const [gear, setGear] = useState('N');
  const [rawSensor, setRawSensor] = useState('0,0');
  
  const [calibrateChar, setCalibrateChar] = useState<BluetoothRemoteGATTCharacteristic | null>(null);

  const connect = async () => {
    try {
      const dev = await navigator.bluetooth.requestDevice({
        filters: [{ services: [SERVICE_UUID] }],
        optionalServices: [SERVICE_UUID]
      });

      dev.addEventListener('gattserverdisconnected', onDisconnected);
      
      const server = await dev.gatt?.connect();
      if (!server) throw new Error('Failed to connect to GATT server');
      
      const service = await server.getPrimaryService(SERVICE_UUID);

      // Gear Char
      const gearChar = await service.getCharacteristic(CHAR_GEAR_UUID);
      await gearChar.startNotifications();
      gearChar.addEventListener('characteristicvaluechanged', (e: any) => {
        const val = new TextDecoder().decode(e.target.value);
        setGear(val);
      });

      // Raw Sensor Char
      const rawChar = await service.getCharacteristic(CHAR_RAW_SENSOR_UUID);
      await rawChar.startNotifications();
      rawChar.addEventListener('characteristicvaluechanged', (e: any) => {
        const val = new TextDecoder().decode(e.target.value);
        setRawSensor(val);
      });

      // Calibrate Char
      const calibChar = await service.getCharacteristic(CHAR_CALIBRATE_UUID);
      setCalibrateChar(calibChar);

      setDevice(dev);
      setConnected(true);
    } catch (err) {
      console.error('Bluetooth connection failed:', err);
    }
  };

  const disconnect = () => {
    if (device?.gatt?.connected) {
      device.gatt.disconnect();
    }
  };

  const onDisconnected = () => {
    setConnected(false);
    setDevice(null);
    setCalibrateChar(null);
  };

  const calibrate = async (gearIndex: string, x: string, y: string) => {
    if (!calibrateChar) return;
    try {
      // Map string gear to integer index expected by firmware
      let mappedIndex = '0';
      if (gearIndex === 'N') mappedIndex = '0';
      else if (gearIndex === 'R') mappedIndex = '7';
      else mappedIndex = gearIndex; // '1' to '6' maps to '1' to '6'

      const val = `${mappedIndex},${x},${y}`;
      const encoded = new TextEncoder().encode(val);
      await calibrateChar.writeValue(encoded);
      console.log(`Calibrated ${val} (Original: ${gearIndex})`);
    } catch (err) {
      console.error('Calibration failed:', err);
    }
  };

  return { connected, connect, disconnect, gear, rawSensor, calibrate };
}
