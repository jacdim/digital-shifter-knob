import { useState, useEffect, useCallback } from 'react'
import './index.css'

interface LedState {
  on: boolean
  blink: boolean
  blinkRate: number
  temperature?: number
  rssi?: number
  freeHeap?: number
  uptime?: number
}

interface Preset {
  label: string
  value: number
}

const PRESETS: Preset[] = [
  { label: 'Slow', value: 1000 },
  { label: 'Medium', value: 500 },
  { label: 'Fast', value: 200 },
  { label: 'Strobe', value: 80 },
]

function isDevMode(): boolean {
  const host = window.location.hostname
  return host === 'localhost' || host === '127.0.0.1'
}

function getBaseUrl(ip: string): string {
  if (isDevMode()) {
    return ip ? `http://${ip}` : ''
  }
  return ''
}

function PowerIcon() {
  return (
    <svg className="power-btn__icon" viewBox="0 0 24 24">
      <path d="M12 2v10" strokeLinecap="round" />
      <path
        d="M18.36 6.64A9 9 0 1 1 5.64 6.64"
        strokeLinecap="round"
      />
    </svg>
  )
}

function App() {
  const devMode = isDevMode()
  const [ip, setIp] = useState<string>(() => localStorage.getItem('esp32_ip') || '')
  const [ledOn, setLedOn] = useState(false)
  const [blinkMode, setBlinkMode] = useState(false)
  const [blinkRate, setBlinkRate] = useState(500)
  const [temperature, setTemperature] = useState<number | null>(null)
  const [rssi, setRssi] = useState<number | null>(null)
  const [freeHeap, setFreeHeap] = useState<number | null>(null)
  const [uptime, setUptime] = useState<number | null>(null)
  const [connected, setConnected] = useState(false)
  const [sending, setSending] = useState(false)

  useEffect(() => {
    if (devMode) localStorage.setItem('esp32_ip', ip)
  }, [ip, devMode])

  const baseUrl = getBaseUrl(ip)

  const fetchState = useCallback(async () => {
    if (devMode && !ip) return
    try {
      const res = await fetch(`${baseUrl}/api/led`, { signal: AbortSignal.timeout(3000) })
      if (res.ok) {
        const data: LedState = await res.json()
        setLedOn(data.on)
        setBlinkMode(data.blink)
        setBlinkRate(data.blinkRate)
        if (data.temperature !== undefined) setTemperature(data.temperature)
        if (data.rssi !== undefined) setRssi(data.rssi)
        if (data.freeHeap !== undefined) setFreeHeap(data.freeHeap)
        if (data.uptime !== undefined) setUptime(data.uptime)
        setConnected(true)
      }
    } catch {
      setConnected(false)
    }
  }, [baseUrl, ip, devMode])

  useEffect(() => {
    fetchState()
    const interval = setInterval(fetchState, 3000)
    return () => clearInterval(interval)
  }, [fetchState])

  const sendState = useCallback(async (state: Partial<LedState>) => {
    if ((devMode && !ip) || sending) return
    setSending(true)
    try {
      const res = await fetch(`${baseUrl}/api/led`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(state),
        signal: AbortSignal.timeout(3000),
      })
      if (res.ok) {
        const data: LedState = await res.json()
        setLedOn(data.on)
        setBlinkMode(data.blink)
        setBlinkRate(data.blinkRate)
        if (data.temperature !== undefined) setTemperature(data.temperature)
        if (data.rssi !== undefined) setRssi(data.rssi)
        if (data.freeHeap !== undefined) setFreeHeap(data.freeHeap)
        if (data.uptime !== undefined) setUptime(data.uptime)
        setConnected(true)
      }
    } catch {
      setConnected(false)
    } finally {
      setSending(false)
    }
  }, [baseUrl, ip, devMode, sending])

  const togglePower = () => {
    const newState = !ledOn
    setLedOn(newState)
    sendState({ on: newState, blink: blinkMode, blinkRate })
  }

  const toggleBlink = () => {
    const newBlink = !blinkMode
    setBlinkMode(newBlink)
    sendState({ on: ledOn, blink: newBlink, blinkRate })
  }

  const handleBlinkRate = (value: number) => {
    setBlinkRate(value)
    sendState({ on: ledOn, blink: blinkMode, blinkRate: value })
  }

  const ledDotClasses = [
    'led-dot',
    ledOn && !blinkMode && 'led-dot--on',
    ledOn && blinkMode && 'led-dot--on led-dot--blink',
  ].filter(Boolean).join(' ')

  const ledStatusText = !connected
    ? 'Not connected'
    : !ledOn
      ? 'LED Off'
      : blinkMode
        ? `Blinking (${blinkRate}ms)`
        : 'LED On — Solid'

  return (
    <div className="app">
      <header className="header">
        <span className="header__icon">💡</span>
        <h1 className="header__title">LED Controller</h1>
        <p className="header__subtitle">XIAO ESP32-S3</p>
      </header>

      <section className={`card ${connected ? 'card--active' : ''}`}>
        <div className="card__label">Device Connection</div>
        <div className="connection">
          {devMode ? (
            <input
              id="ip-input"
              className="connection__input"
              type="text"
              placeholder="192.168.1.xxx"
              value={ip}
              onChange={(e) => setIp(e.target.value)}
              spellCheck={false}
            />
          ) : (
            <span className="connection__host">
              {window.location.hostname}
            </span>
          )}
          <span
            className={`status-badge ${connected ? 'status-badge--connected' : 'status-badge--disconnected'}`}
          >
            <span className="status-badge__dot" />
            {connected ? 'Live' : 'Offline'}
          </span>
        </div>
      </section>

      <section className="card">
        <div className="card__label">Power</div>
        <div className="power-section">
          <button
            id="power-toggle"
            className={`power-btn ${ledOn ? 'power-btn--on' : ''}`}
            onClick={togglePower}
            disabled={!connected}
            aria-label={ledOn ? 'Turn LED off' : 'Turn LED on'}
          >
            <PowerIcon />
          </button>
          <span className={`power-label ${ledOn ? 'power-label--on' : ''}`}>
            {ledOn ? 'ON' : 'OFF'}
          </span>
        </div>
      </section>

      <section className="card">
        <div className="card__label">Blink Mode</div>
        <div className="blink-controls">
          <div className="blink-toggle">
            <span className="blink-toggle__label">Enable Blinking</span>
            <button
              id="blink-toggle"
              className={`toggle ${blinkMode ? 'toggle--on' : ''}`}
              onClick={toggleBlink}
              disabled={!connected || !ledOn}
              aria-label={blinkMode ? 'Disable blink' : 'Enable blink'}
            >
              <span className="toggle__knob" />
            </button>
          </div>

          {blinkMode && (
            <>
              <div className="slider-group">
                <div className="slider-header">
                  <span className="slider-label">Speed</span>
                  <span className="slider-value">{blinkRate}ms</span>
                </div>
                <input
                  id="blink-rate-slider"
                  className="slider"
                  type="range"
                  min="50"
                  max="2000"
                  step="10"
                  value={blinkRate}
                  onChange={(e) => handleBlinkRate(parseInt(e.target.value))}
                />
              </div>

              <div className="presets">
                {PRESETS.map((preset) => (
                  <button
                    key={preset.label}
                    className={`preset-btn ${blinkRate === preset.value ? 'preset-btn--active' : ''}`}
                    onClick={() => handleBlinkRate(preset.value)}
                  >
                    {preset.label}
                  </button>
                ))}
              </div>
            </>
          )}
        </div>
      </section>

      <section className={`card ${connected ? 'card--active' : ''}`}>
        <div className="card__label">System Metrics</div>
        <div className="metrics-grid">
          <div className="metric">
            <span className="metric__label">Core Temp</span>
            <span className="metric__value">{temperature !== null ? `${temperature.toFixed(1)}°C` : '--'}</span>
          </div>
          <div className="metric">
            <span className="metric__label">WiFi Signal</span>
            <span className="metric__value">{rssi !== null ? `${rssi} dBm` : '--'}</span>
          </div>
          <div className="metric">
            <span className="metric__label">Free Memory</span>
            <span className="metric__value">{freeHeap !== null ? `${(freeHeap / 1024).toFixed(1)} KB` : '--'}</span>
          </div>
          <div className="metric">
            <span className="metric__label">Uptime</span>
            <span className="metric__value">{uptime !== null ? `${uptime} s` : '--'}</span>
          </div>
        </div>
      </section>

      <div
        className="led-indicator"
        style={ledOn && blinkMode ? { '--blink-speed': `${blinkRate}ms` } as React.CSSProperties : undefined}
      >
        <span className={ledDotClasses} />
        <span className="led-text">{ledStatusText}</span>
      </div>
    </div>
  )
}

export default App
