### Setup : 

- **Arduino IDE** or **PlatformIO**
- **ESP32 Board Package**
- Required **Libraries**:
  - `WiFi` (Built-in with ESP32)
  - `WebServer` (Built-in with ESP32)
  - `Wire` (Built-in)
  - `ESPmDNS`
  - `ArduinoOTA`
  - `HTTPClient`
  - `math`

    Go to **Sketch** -> **Include Library** -> **Manage Libraries** -> **Search and Install** :
   - `Adafruit GFX Library`
   - `Adafruit SH1106G`
   - `ArduinoJson`

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
3. Access Web Interface -> https://mimi.local
4. Controls :

 ![Controls](images/Controls.png)

