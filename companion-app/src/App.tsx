import { useState } from 'react';
import { Bluetooth, Settings as SettingsIcon, Home, Crosshair, BluetoothOff } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { useBluetooth } from './hooks/useBluetooth';

function App() {
  const { connected, connect, disconnect, gear, rawSensor, calibrate } = useBluetooth();
  const [activeTab, setActiveTab] = useState<'dashboard' | 'calibration' | 'settings'>('dashboard');

  const navItems = [
    { id: 'dashboard', icon: Home, label: 'Dashboard' },
    { id: 'calibration', icon: Crosshair, label: 'Calibration' },
    { id: 'settings', icon: SettingsIcon, label: 'Settings' }
  ] as const;

  return (
    <div className="min-h-screen bg-slate-950 text-slate-50 flex flex-col font-sans overflow-hidden selection:bg-blue-500/30">
      <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_top,_var(--tw-gradient-stops))] from-blue-900/20 via-slate-950 to-slate-950 -z-10" />
      
      <header className="w-full px-6 pt-12 pb-6 flex justify-between items-center z-10 border-b border-white/5 bg-slate-950/50 backdrop-blur-md">
        <div>
          <h1 className="text-3xl font-extrabold tracking-tight bg-gradient-to-r from-blue-400 to-indigo-400 bg-clip-text text-transparent">ShiftControl</h1>
          <p className="text-slate-400 text-sm font-medium mt-1">Premium Digital Interface</p>
        </div>
        <button 
          onClick={connected ? disconnect : connect}
          className={`p-3.5 rounded-2xl transition-all duration-300 shadow-lg flex items-center justify-center ${
            connected 
            ? 'bg-blue-500/20 text-blue-400 shadow-blue-500/20 border border-blue-500/30 hover:bg-blue-500/30' 
            : 'bg-slate-800/80 text-slate-300 hover:bg-slate-700 hover:shadow-slate-800/50 border border-white/5'
          }`}
        >
          {connected ? <Bluetooth size={22} /> : <BluetoothOff size={22} />}
        </button>
      </header>

      <main className="flex-1 w-full max-w-md mx-auto p-6 relative z-10 flex flex-col">
        <AnimatePresence mode="wait">
          {activeTab === 'dashboard' && (
            <motion.div 
              key="dashboard"
              initial={{ opacity: 0, x: -20, filter: 'blur(4px)' }}
              animate={{ opacity: 1, x: 0, filter: 'blur(0px)' }}
              exit={{ opacity: 0, x: 20, filter: 'blur(4px)' }}
              transition={{ duration: 0.3, ease: 'easeOut' }}
              className="flex-1 flex flex-col items-center justify-center"
            >
              <div className="relative w-72 h-72 rounded-full flex items-center justify-center group">
                <div className="absolute inset-0 rounded-full bg-blue-500/5 group-hover:bg-blue-500/10 transition-colors duration-700" />
                <div className="absolute inset-2 rounded-full border border-blue-500/20 backdrop-blur-sm" />
                <div className="absolute inset-6 rounded-full border border-blue-400/30 bg-gradient-to-b from-slate-900 to-slate-950 shadow-[inset_0_0_60px_rgba(0,0,0,0.5)] flex items-center justify-center">
                  <motion.span 
                    key={gear}
                    initial={{ scale: 0.5, opacity: 0 }}
                    animate={{ scale: 1, opacity: 1 }}
                    transition={{ type: 'spring', stiffness: 200, damping: 15 }}
                    className="text-8xl font-black bg-gradient-to-b from-white to-slate-400 bg-clip-text text-transparent drop-shadow-2xl"
                  >
                    {gear}
                  </motion.span>
                </div>
                {/* Decorative outer ring glowing */}
                <div className="absolute inset-0 rounded-full border-[1px] border-blue-500/30 shadow-[0_0_30px_rgba(59,130,246,0.15)] pointer-events-none" />
              </div>
              <motion.p 
                initial={{ opacity: 0 }} 
                animate={{ opacity: 1, transition: { delay: 0.2 } }}
                className="mt-12 text-slate-400 font-semibold tracking-[0.2em] text-sm"
              >
                CURRENT GEAR
              </motion.p>
            </motion.div>
          )}

          {activeTab === 'calibration' && (
            <CalibrationView key="calibration" rawSensor={rawSensor} calibrate={calibrate} connected={connected} />
          )}

          {activeTab === 'settings' && (
            <motion.div 
              key="settings"
              initial={{ opacity: 0, x: -20, filter: 'blur(4px)' }}
              animate={{ opacity: 1, x: 0, filter: 'blur(0px)' }}
              exit={{ opacity: 0, x: 20, filter: 'blur(4px)' }}
              transition={{ duration: 0.3, ease: 'easeOut' }}
              className="flex-1 flex flex-col space-y-6"
            >
              <h2 className="text-xl font-bold text-slate-200">Appearance</h2>
              <div className="bg-slate-900/60 border border-white/5 rounded-2xl p-4 flex items-center justify-between">
                <div>
                  <h3 className="font-medium text-slate-200">Theme</h3>
                  <p className="text-sm text-slate-400">Choose your interface color</p>
                </div>
                <div className="flex gap-2">
                  <div className="w-8 h-8 rounded-full bg-blue-500 ring-2 ring-blue-500/50 cursor-pointer shadow-lg shadow-blue-500/20" />
                  <div className="w-8 h-8 rounded-full bg-emerald-500 cursor-pointer opacity-50 hover:opacity-100 transition-opacity" />
                  <div className="w-8 h-8 rounded-full bg-rose-500 cursor-pointer opacity-50 hover:opacity-100 transition-opacity" />
                </div>
              </div>

              <h2 className="text-xl font-bold text-slate-200 mt-4">Device Info</h2>
              <div className="bg-slate-900/60 border border-white/5 rounded-2xl p-4 flex flex-col gap-3">
                <div className="flex justify-between">
                  <span className="text-slate-400">Status</span>
                  <span className={connected ? "text-emerald-400" : "text-rose-400"}>{connected ? 'Connected' : 'Disconnected'}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-slate-400">Firmware Version</span>
                  <span className="text-slate-200">v1.2.0</span>
                </div>
              </div>
            </motion.div>
          )}
        </AnimatePresence>
      </main>

      <nav className="fixed bottom-0 left-0 right-0 p-6 z-20 pb-10 bg-gradient-to-t from-slate-950 via-slate-950 to-transparent">
        <div className="max-w-md mx-auto bg-slate-900/80 backdrop-blur-xl border border-white/10 rounded-3xl p-2 flex justify-between shadow-2xl">
          {navItems.map((item) => {
            const Icon = item.icon;
            const isActive = activeTab === item.id;
            return (
              <button
                key={item.id}
                onClick={() => setActiveTab(item.id)}
                className={`relative flex-1 py-3 px-4 rounded-2xl flex flex-col items-center gap-1 transition-all duration-300 ${
                  isActive ? 'text-blue-400' : 'text-slate-500 hover:text-slate-300 hover:bg-white/5'
                }`}
              >
                {isActive && (
                  <motion.div 
                    layoutId="nav-pill"
                    className="absolute inset-0 bg-blue-500/10 rounded-2xl border border-blue-500/20"
                    transition={{ type: "spring", bounce: 0.2, duration: 0.6 }}
                  />
                )}
                <Icon size={20} className="relative z-10" />
                <span className="text-[10px] font-semibold uppercase tracking-wider relative z-10">{item.label}</span>
              </button>
            );
          })}
        </div>
      </nav>
    </div>
  );
}

function CalibrationView({ rawSensor, calibrate, connected }: { rawSensor: string, calibrate: (g: string, x: string, y: string) => void, connected: boolean }) {
  const [selectedGear, setSelectedGear] = useState('1');
  const gears = ['R', '1', '2', '3', '4', '5', '6', 'N'];

  const [x, y] = rawSensor.split(',');

  const handleSave = () => {
    calibrate(selectedGear, x, y);
  };

  return (
    <motion.div 
      initial={{ opacity: 0, x: -20, filter: 'blur(4px)' }}
      animate={{ opacity: 1, x: 0, filter: 'blur(0px)' }}
      exit={{ opacity: 0, x: 20, filter: 'blur(4px)' }}
      transition={{ duration: 0.3, ease: 'easeOut' }}
      className="flex-1 flex flex-col"
    >
      <div className="bg-slate-900/50 border border-white/5 rounded-3xl p-6 mb-6 shadow-xl relative overflow-hidden">
        <div className="absolute top-0 left-0 right-0 h-1 bg-gradient-to-r from-blue-500 to-indigo-500 opacity-50" />
        <h2 className="text-slate-400 text-sm font-bold uppercase tracking-widest mb-4">Live Sensor</h2>
        <div className="flex items-center justify-between">
          <div className="flex flex-col">
            <span className="text-xs text-slate-500 uppercase tracking-wider">X Axis</span>
            <span className="text-3xl font-mono text-slate-200">{x || '0'}</span>
          </div>
          <div className="h-12 w-px bg-white/10" />
          <div className="flex flex-col items-end">
            <span className="text-xs text-slate-500 uppercase tracking-wider">Y Axis</span>
            <span className="text-3xl font-mono text-slate-200">{y || '0'}</span>
          </div>
        </div>
      </div>

      <div className="mb-6">
        <h2 className="text-slate-400 text-sm font-bold uppercase tracking-widest mb-4 px-2">Target Gear</h2>
        <div className="grid grid-cols-4 gap-3">
          {gears.map(g => (
            <button
              key={g}
              onClick={() => setSelectedGear(g)}
              className={`py-3 rounded-2xl font-bold text-lg transition-all duration-200 border ${
                selectedGear === g
                  ? 'bg-blue-500 text-white border-blue-400 shadow-[0_0_15px_rgba(59,130,246,0.4)] scale-105'
                  : 'bg-slate-900/50 text-slate-400 border-white/5 hover:bg-slate-800'
              }`}
            >
              {g}
            </button>
          ))}
        </div>
      </div>

      <div className="mt-auto pb-10">
        <button
          onClick={handleSave}
          disabled={!connected}
          className="w-full py-4 rounded-2xl bg-gradient-to-r from-blue-600 to-indigo-600 hover:from-blue-500 hover:to-indigo-500 text-white font-bold tracking-wide text-lg shadow-[0_0_20px_rgba(59,130,246,0.3)] transition-all active:scale-[0.98] disabled:opacity-50 disabled:pointer-events-none disabled:grayscale flex items-center justify-center gap-2"
        >
          <Crosshair size={20} />
          Save Center Point
        </button>
        {!connected && (
          <p className="text-center text-rose-400 text-sm mt-3 font-medium">Connect device to calibrate</p>
        )}
      </div>
    </motion.div>
  );
}

export default App;
