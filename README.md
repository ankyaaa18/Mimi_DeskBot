# Mimi - Desktop Buddy
An expressive ESP32-powered desktop robot with OLED facial animations, Wi-Fi control, and autonomous exploration.

---

## Project Overview
This ESP32-powered robot is designed to create personality using animated OLED eyes while providing multiple movement modes controlled through a built-in Wi-Fi web interface. Unlike Bluetooth-controlled robots, Desk Bot hosts its own Wi-Fi Access Point, allowing any nearby phone or computer to connect directly without requiring an internet connection. It features
- Animated OLED facial expressions
- Wi-Fi based remote control through any web browser
- Differential drive using dual N20 motors
- Autonomous Explore mode
- Battery powered
- Modular firmware architecture

---

## Hardware

| Component | Specification | Qty | Purpose |
|-----------|---------------|:---:|---------|
| ESP32  | ESP32 DevKit V1 | 1 | Main microcontroller responsible for motor control, OLED animations, and hosting the Wi-Fi web interface. |
| OLED Display | SSD1306, 0.96", 128×64, I2C | 1 | Displays expressive facial animations and robot status. |
| Motor Driver | DRV8833 Dual H-Bridge | 1 | Controls the speed and direction of both DC motors. |
| DC Gear Motors | N20 Metal Gear Motors, 100 RPM, 6V | 2 | Provide differential drive for robot movement. |
| Wheels | Compatible with N20 motors | 2 | Enable locomotion and traction. |
| Castor Wheel | Free-rotating swivel wheel | 1 | Supports the robot and provides balance during movement. |
| Battery Pack | 2S 18650 Li-ion (7.4V) | 1 | Primary rechargeable power source for the robot. |
| Buck Converter | LM2596 Adjustable DC-DC Converter | 1 | Steps down the battery voltage to a stable supply for the electronics. |
| Power Switch | SPDT Slide Switch | 1 | Safely turns the robot on and off. |

---

## Hardware Connections

### Wiring
                    ┌──────────────────────────┐
                    │    2S Li-ion Battery     │
                    │           7.4V           │
                    └────────────┬─────────────┘
                                 │
                          Power Switch
                                 │                      
                   |─── ── ── ── ─── ── ──|
                   |                      │
      ┌────────────▼────────────┐         │
      │ LM2596 Buck Converter   │         │
      └───────────────────┬─────┘         │ 
                          │               │ 
                          │               │
                          │               │
                    ┌─────▼─────┐─────────▼──────────────────────────┐
                    │   ESP32   │           DRV8833                  │
                    │           │                                    │
                    │           │                                    │
                    │  GPIO 25  │  --->   IN1      OUT1  ---> Motor1 │
                    │  GPIO 26  │  --->   IN2      OUT2  ---> Motor1 │
                    │  GPIO 27  │  --->   IN1      OUT3  ---> Motor1 │
                    │  GPIO 14  │  --->   IN2      OUT4  ---> Motor1 │
                    │           │────────────────────────────────────┘
                    │           │───────────────────┐                   
                    │  GPIO 21  │  --->  OLED (SCA) │
                    │  GPIO 22  │  --->  OLED (SDA) │
                    │  GND      │  --->  OLED GND   │
                    │  3V3      │  --->  OLED VCC   │
                    └───────────┘───────────────────┘

## Software

- **Expressive OLED Face:** Animated facial expressions including neutral, happy, blink, wink, and other interactive emotions.
- **Wi-Fi Control:** Creates a Wi-Fi Access Point with a browser-based dashboard for wireless robot control.
- **Differential Drive:** Smooth movement using dual N20 gear motors with independent speed and direction control.
- **Modular Architecture:** Firmware is divided into independent modules for movement, facial animations, and web interface.
- Software Stack
  - Programming Language -> C++
  - Development Environment ->  Arduino IDE
  - Microcontroller Framework -> Arduino-ESP32
  - Communication -> Wi-Fi (HTTP)    

---    

## QuickStart
### Setup : 

- **Arduino IDE** or **PlatformIO**
- **ESP32 Board Package**
- Required **Libraries**:
  - `WiFi` (Built-in with ESP32)
  - `WebServer` (Built-in with ESP32)
  - `Wire` (Built-in)

    Go to **Sketch** -> **Include Library** -> **Manage Libraries** -> **Search and Install** :
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`

### PlatformIO Setup :
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps =
    adafruit/Adafruit SSD1306
    adafruit/Adafruit GFX Library
```

1. Upload the code using Arduino IDE (Select correct Board and COM Port) or PlatformIO.
2. Connect to WiFi
3. Access Web Interface -> Open browser to *192.168.4.1*
4. Controls :

| Button | Action |
|---------|--------|
| Forward | Move Forward |
| Reverse | Move Backward |
| Left | Turn Left |
| Right | Turn Right |
| Stop | Stop Motors |
| Spin CW | Rotate Clockwise |
| Spin CCW | Rotate Counter Clockwise |
| Explore | Autonomous Movement |
| Blink | Blink Animation |
| Wink | Wink Animation |

# Project Structure

```
Mimi_DeskBot/
│
├── Mimi/               *Main Arduino Sketch
│   ├── Mimi.ino
│   ├── face.cpp
│   ├── face.h
│   ├── motor.cpp
│   ├── motor.h
│   └── movement.cpp
│   └── movement.h
│
├── images/             *Images
│ 
├── docs/               
│
│
├── README.md           *description
└── LICENSE             *MIT License
``` 

---


# Roadmap

## Version 1.0.0

-  OLED facial expressions
-  Wi-Fi control
-  Differential drive

## Planned Features

- [ ] PC Stat Monitor Mode
- [ ] Battery Percentage Display
- [ ] Voice Interaction
- [ ] Better Facial Animations
- [ ] OTA Firmware Updates
- [ ] AI Assistant Integration
- [ ] Mobile App

---


# License

This project is licensed under the MIT License.

---

# Support

If you enjoyed this project, consider leaving a ⭐ on the repository.
