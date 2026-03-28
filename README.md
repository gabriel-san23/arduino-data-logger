# Arduino Data Logger with RTC & EEPROM

![Platform](https://img.shields.io/badge/platform-Arduino-blue)
![Language](https://img.shields.io/badge/language-C++-informational)
![Sensor](https://img.shields.io/badge/sensor-DHT11%2FDHT22-orange)
![Light Sensor](https://img.shields.io/badge/sensor-LDR%20(light)-orange)
![RTC](https://img.shields.io/badge/RTC-module-blueviolet)
![Storage](https://img.shields.io/badge/storage-EEPROM-lightgrey)

For more informations about the use:
[Check the User Guide here (PT-BR)](Manual%20de%20Uso.pdf)

## 🧩 Technical Specifications

### 🔧 Hardware

- **Microcontroller:** ATmega328P (Arduino Uno R3)
- **Temperature & Humidity Sensor:** DHT11
- **Light Sensor:** LDR (photoresistor) with 10kΩ resistor
- **Display:** 16x2 LCD with I2C interface
- **Real-Time Clock (RTC):** RTC module (model not specified)
- **Storage:** Internal EEPROM (ATmega328P)
- **Power Supply:** 9V battery
- **User Input:** Push buttons for navigation and configuration
- **Prototyping Components:** Breadboard, jumper wires, LEDs, resistors

---

### 📊 Measurements & Accuracy

- **Temperature:**
  - Unit: Celsius (°C) / Fahrenheit (°F)
  - Range: 0°C to 50°C
  - Accuracy: ±2°C

- **Humidity:**
  - Unit: Relative Humidity (%)
  - Range: 20% to 90%
  - Accuracy: ±5%

- **Luminosity:**
  - Unit: Relative Luminosity (%)
  - Range: Depends on calibration
  - Accuracy: Not calibrated (relative measurement only)

---

### ⚙️ System Features

- Data logging with timestamp (RTC-based)
- Storage of measurements using internal EEPROM
- LCD interface for real-time data visualization
- User navigation and configuration via buttons
- Temperature unit switching (°C / °F)
- Optional startup animation (can be disabled)

---

### 📖 Libraries Required

 - https://github.com/johnrickman/LiquidCrystal_I2C
 - https://github.com/adafruit/dht-sensor-library
 - https://github.com/adafruit/RTClib
 - https://github.com/adafruit/Adafruit_BusIO/tree/master
 - https://github.com/adafruit/adafruit_sensor
