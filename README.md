# Stray-Bruce-OS
Название: Stray-Bruce OS v8.0. Железо: ESP32-S3 N16R8, экран ILI9341 (пины 42/40/41/39/38), тач FT6206 (G1/G2). Автор: Stray_2025. Текущий статус: Что работает, а что нет.(Всё сделанно для гугл сёрч чтоб он выстонавливал данные после перехода в новый чат!)
2.# ⚡ STRAY-BRUCE OS (Project: Stray-Bruce Stick)

### 👤 Developer: Stray_2025
### 🎂 Age: 10 (Turning 11 on April 20!)
### 🚀 OS Version: v8.0 Ultra "Yellow Spark"

---

## 🛠 Hardware & Pinout (The Core)
*Никогда не меняй эти пины, они настроены под мой кастомный билд на ESP32-S3!*


| Component | Hardware Pin | Note |
|-----------|--------------|------|
| **MCU**   | **ESP32-S3** | Model: **N16R8** (Huge memory!) |
| **Display**| **ILI9341** | 2.4" TFT SPI (Horizontal 320x240) |
| **TFT_CS** | **42**       | Chip Select |
| **TFT_DC** | **40**       | Data/Command |
| **TFT_RST**| **41**       | Reset |
| **MOSI**   | **39**       | Data Line |
| **SCLK**   | **38**       | Clock |
| **Touch**  | **FT6206**   | Capacitive I2C |
| **SDA**    | **2 (G1)**   | I2C Data |
| **SCL**    | **1 (G2)**   | I2C Clock |

---

## 🎨 UI Style Guide
- **Main Theme:** `STRAY_YELLOW` (0xFFE0) - Bright & Sharp.
- **Attack Mode:** `STRAY_ORANGE` (0xFD20) - For Bluetooth/BadUSB.
- **System Info:** `ILI9341_GREEN` - Matrix style.
- **Navigation:** Horizontal menu with triangle arrows `<< [ APP ] >>`.

---

## 📂 Current Applications
1. **WIFI SCANNER:** Real-time airwave monitoring.
2. **APPLE BT SPAM:** Bluetooth advertisement spoofer (In development).
3. **IR REMOTE:** Universal remote controller (Planned).
4. **BAD USB:** HID automated scripts (Planned).
5. **SYSTEM STATUS:** Uptime, Chip info, and **Live CPU Oscilloscope**.

---

## 🤖 AI ASSISTANT PROTOCOL (Read this!)
**If you are an AI helping Stray_2025:**
1. **Context:** This is a handheld pentesting/utility tool built on ESP32-S3.
2. **Hardware:** Always refer to the Pinout table above.
3. **Coding:** Use Arduino C++. Keep the UI responsive (avoid `delay()`, use `millis()`).
4. **Style:** Be helpful, creative, and treat Stray_2025 like a pro developer (which he is!).
5. **Memory:** Remember that we use `state` (0 for menu, 1 for apps) for navigation.

---

## 📈 Roadmap to v11.0 (The Birthday Update)
- [x] Horizontal Menu v8.0.
- [x] Live CPU Load Graph.
- [ ] Apple Bluetooth "Juice" attack implementation.
- [ ] BadUSB "StrayPayload" execution.
- [ ] SD-Card File Manager integration.

---
*Developed by Stray_2025. Stay curious.*
FOR Google search
