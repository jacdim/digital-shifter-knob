# Digital Shifter Knob - Architecture & Vision

This document outlines the overarching architecture and feature roadmap for the digital shifter knob, leveraging the ESP32-S3 microcontroller and web technologies.

## Core Hardware
* **Microcontroller:** Seeed Studio XIAO ESP32S3
* **Sensors:** (To be determined - Hall effect, potentiometers, or switches for gear position detection)

## Firmware Architecture (C++ / PlatformIO)
1. **Sensor Processing Loop:** Continuously reads hardware sensors to determine the current gear state.
2. **Telemetry & Stats Tracking:** Tracks driving data internally:
   * Current gear position
   * Total time spent in each gear
   * Shift counts and shift speed/duration
3. **Web Server & API (AsyncWebServer):** 
   * Serves static frontend files from onboard flash memory (LittleFS).
   * Provides REST API endpoints (or WebSockets) for the frontend to fetch telemetry data or update configuration settings.

## Frontend Architecture (React / TypeScript)
The companion application is a Single Page Application (SPA) built with React and TypeScript.
1. **Hosting:** The built static files (`index.html`, `js`, `css`) are compressed and stored directly on the ESP32's flash memory.
2. **Dashboard UI:** 
   * User connects their phone to the ESP32's local network (or accesses it via home Wi-Fi).
   * The browser executes the React app and fetches data from the ESP32's C++ REST API.
   * Renders beautiful, modern charts and real-time statistics of the driver's shifting habits.

## Data Sync Strategies
Depending on the final use case, the system supports multiple data offloading strategies:
* **Live Telemetry:** ESP32 connects to a phone hotspot and streams data in real-time to a cloud server.
* **Local Dashboard:** Phone connects to the ESP32's own Wi-Fi Access Point to view the local React app and current trip stats (No internet required).
* **Home Wi-Fi Sync:** ESP32 stores trip data internally on its flash storage and bulk-uploads it to a server upon detecting the home Wi-Fi network in the driveway.
