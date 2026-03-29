#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Wire.h>
#include <Adafruit_FT6206.h>
#include <WiFi.h>

// --- КОНФИГУРАЦИЯ ПИНОВ (Твоя база) ---
#define TFT_CS 42
#define TFT_DC 40
#define TFT_RST 41
#define TFT_MOSI 39
#define TFT_SCLK 38
#define STRAY_YELLOW 0xFFE0 
#define STRAY_ORANGE 0xFD20

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_FT6206 ts = Adafruit_FT6206();

// Глобальные переменные
int current = 0;
int state = 0; // 0-Main, 1-App
const char* menu[] = {"WIFI", "BLUETOOTH", "IR REMOTE", "BAD USB", "SYSTEM"};

// Переменные для графика (Осциллограф)
int graphX = 11;
int lastY = 180;
unsigned long lastGraphUpdate = 0;

void setup() {
  tft.begin();
  tft.setRotation(1); // ГОРИЗОНТАЛЬНО (320x240)
  Wire.begin(2, 1);   // Твои пины I2C для тача
  ts.begin(40);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  drawMain();
}

// --- ГЛАВНОЕ МЕНЮ ---
void drawMain() {
  state = 0; 
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 320, 35, STRAY_YELLOW);
  tft.setTextColor(ILI9341_BLACK); tft.setTextSize(2);
  tft.setCursor(65, 10); tft.print("STRAY-BRUCE STICK");
  
  tft.drawRect(60, 80, 200, 80, STRAY_YELLOW); 
  drawMainText();
  
  // Стрелки
  tft.fillTriangle(15, 120, 45, 90, 45, 150, STRAY_YELLOW);   
  tft.fillTriangle(305, 120, 275, 90, 275, 150, STRAY_YELLOW); 
}

void drawMainText() {
  tft.fillRect(62, 82, 196, 76, ILI9341_BLACK);
  tft.setTextColor(STRAY_YELLOW); tft.setTextSize(3);
  int textLen = strlen(menu[current]);
  tft.setCursor(160 - (textLen * 9), 110); 
  tft.print(menu[current]);
}

// --- ПРИЛОЖЕНИЯ ---

void runScanner() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setCursor(10, 50); tft.setTextColor(STRAY_YELLOW); tft.setTextSize(1);
  tft.println("SCANNING WIFI AIRWAVES...");
  
  int n = WiFi.scanNetworks();
  if (n == 0) tft.println("No networks found.");
  else {
    tft.println("Found: " + String(n));
    tft.println("-------------------------");
    for (int i = 0; i < n && i < 8; i++) {
      tft.setTextColor(STRAY_YELLOW); tft.print(i+1); tft.print(": ");
      tft.setTextColor(ILI9341_WHITE); tft.println(WiFi.SSID(i));
    }
  }
}

void runBTSpam() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  tft.setTextColor(STRAY_ORANGE); tft.setTextSize(2);
  tft.setCursor(10, 10); tft.println("APPLE BT SPAM");
  tft.drawFastHLine(0, 35, 320, STRAY_ORANGE);
  
  tft.setTextSize(1);
  tft.setCursor(10, 50); tft.println("STATUS: READY TO ATTACK");
  tft.setCursor(10, 70); tft.println("TARGET: iOS / MacOS DEVICES");
  
  // Радар
  tft.drawCircle(160, 150, 50, STRAY_ORANGE);
  tft.drawCircle(160, 150, 30, STRAY_ORANGE);
  tft.fillCircle(160, 150, 5, ILI9341_RED);
  
  tft.setTextColor(STRAY_YELLOW);
  tft.setCursor(70, 220); tft.println("HOLD CENTER TO ATTACK");
}

void runSystemInfo() {
  state = 1; tft.fillScreen(ILI9341_BLACK);
  drawExitButton();
  
  tft.setTextColor(STRAY_YELLOW); tft.setTextSize(2);
  tft.setCursor(10, 10); tft.println("SYSTEM STATUS:");
  tft.drawFastHLine(0, 35, 320, STRAY_YELLOW);
  
  tft.setTextSize(1);
  tft.setCursor(10, 45); tft.print("USER: STRAY_2025");
  tft.setCursor(10, 65); tft.print("CHIP: ESP32-S3 N16R8");
  tft.setCursor(10, 85); tft.print("BDAY: APRIL 20");
  tft.setCursor(10, 105); tft.print("SPEED: 100% TURBO");
  
  // Рамка осциллографа
  tft.drawRect(10, 130, 300, 80, STRAY_YELLOW);
  tft.setCursor(15, 120); tft.print("CPU LOAD OSCILLOSCOPE");
  graphX = 11;
  
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(10, 220); tft.println("STRAY-BRUCE OS v8.0 ONLINE");
}

void updateGraph() {
  if (millis() - lastGraphUpdate > 50) { // Обновляем каждые 50мс
    if (graphX > 305) { 
      tft.fillRect(11, 131, 298, 78, ILI9341_BLACK);
      graphX = 11;
    }
    int val = random(140, 205); // Эмуляция данных
    tft.drawLine(graphX, lastY, graphX + 2, val, ILI9341_GREEN);
    lastY = val;
    graphX += 2;
    lastGraphUpdate = millis();
  }
}

void drawExitButton() {
  tft.fillRect(280, 0, 40, 35, ILI9341_RED); 
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
  tft.setCursor(292, 10); tft.print("X");
}

// --- ОСНОВНОЙ ЦИКЛ ---
void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int y = map(p.x, 0, 240, 0, 240);
    int x = map(p.y, 0, 320, 0, 320);

    if (state == 0) { // Навигация в меню
      if (x < 60) { 
        current = (current > 0) ? current - 1 : 4; 
        drawMainText(); delay(250); 
      }
      else if (x > 260) { 
        current = (current + 1) % 5; 
        drawMainText(); delay(250); 
      }
      else if (x > 60 && x < 260 && y > 80 && y < 160) { 
        if (current == 0) runScanner(); 
        else if (current == 1) runBTSpam();
        else if (current == 4) runSystemInfo();
        else {
          tft.fillScreen(ILI9341_BLACK);
          tft.setCursor(100, 110); tft.setTextColor(STRAY_YELLOW); tft.print("NOT READY");
          delay(800); drawMain();
        }
        delay(300); 
      }
    } 
    else if (state == 1 && x > 270 && y < 45) { // Выход [X]
      drawMain();
      delay(300);
    }
  }

  // Работа фоновых процессов приложений
  if (state == 1 && current == 4) {
    updateGraph(); // Рисуем график только в систем инфо
  }
}
