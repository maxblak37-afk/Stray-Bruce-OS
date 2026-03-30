#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Wire.h>
#include <Adafruit_FT6206.h>

#define TFT_CS 42
#define TFT_DC 40
#define TFT_RST 41
#define TFT_MOSI 39
#define TFT_SCLK 38

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Adafruit_FT6206 ts = Adafruit_FT6206();

int currentApp = 0; // 0-Lock, 1-Menu, 2-VPN, 3-Snake
bool vpnConn = false;

// Змейка
int sX[100], sY[100], sLen = 5, sDir = 1, fX, fY;
bool gOver = false;

void setup() {
  tft.begin();
  tft.setRotation(0);
  Wire.begin(2, 1);
  ts.begin(40);
  drawLockScreen();
}

void drawLockScreen() {
  currentApp = 0;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(4);
  tft.setCursor(60, 80); tft.print("12:00");
  tft.fillRoundRect(85, 150, 70, 70, 15, ILI9341_ORANGE);
}

void drawMenu() {
  currentApp = 1;
  tft.fillScreen(0x18E3);
  tft.fillRect(0, 0, 240, 25, ILI9341_BLACK);
  // Иконки
  drawIcon(30, 50, "VPN", 0xFD20);
  drawIcon(135, 50, "SNAKE", ILI9341_GREEN);
}

void drawIcon(int x, int y, const char* name, uint16_t color) {
  tft.fillRoundRect(x, y, 75, 75, 15, color);
  tft.setCursor(x+10, y+85); tft.setTextColor(ILI9341_WHITE); tft.setTextSize(1);
  tft.print(name);
}

void drawVPN() {
  currentApp = 2;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE); tft.setCursor(10, 10); tft.print("Amnezia VPN");
  uint16_t col = vpnConn ? 0xFD20 : 0x7BEF;
  tft.drawCircle(120, 150, 70, col);
  tft.drawCircle(120, 150, 69, col);
  tft.setCursor(vpnConn ? 75 : 65, 145);
  tft.setTextColor(col); tft.setTextSize(2);
  tft.print(vpnConn ? "ON" : "OFF");
  tft.setTextSize(1); tft.setCursor(60, 305); tft.setTextColor(0x7BEF);
  tft.print("EXIT TO HOME");
}

void startSnake() {
  currentApp = 3; gOver = false; sLen = 5; sDir = 1;
  for(int i=0; i<sLen; i++) { sX[i] = 120 - (i*10); sY[i] = 160; }
  fX = random(2, 22) * 10; fY = random(5, 27) * 10;
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRect(0, 30, 240, 260, ILI9341_WHITE);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, 0, 240, 240, 0);
    int y = map(p.y, 0, 320, 320, 0);

    if (currentApp == 0) { drawMenu(); delay(300); }
    else if (currentApp == 1) {
      if (x > 30 && x < 105 && y > 50 && y < 125) { drawVPN(); delay(300); }
      if (x > 135 && x < 210 && y > 50 && y < 125) { startSnake(); delay(300); }
    }
    else if (currentApp == 2) { // VPN Toggle
      if (x > 50 && x < 190 && y > 80 && y < 220) { vpnConn = !vpnConn; drawVPN(); delay(300); }
    }
    else if (currentApp == 3) { // Snake Control
      if (y < 100) sDir = 0; else if (y > 220 && y < 290) sDir = 2;
      else if (x < 100) sDir = 3; else if (x > 140) sDir = 1;
    }
    if (y > 295 && currentApp != 0) { drawMenu(); delay(300); } // HOME
  }

  if (currentApp == 3 && !gOver) {
    tft.fillRect(sX[sLen-1], sY[sLen-1], 9, 9, ILI9341_BLACK);
    for(int i = sLen-1; i > 0; i--) { sX[i] = sX[i-1]; sY[i] = sY[i-1]; }
    if(sDir == 0) sY[0] -= 10; else if(sDir == 1) sX[0] += 10;
    else if(sDir == 2) sY[0] += 10; else if(sDir == 3) sX[0] -= 10;
    if(sX[0] < 5 || sX[0] > 230 || sY[0] < 35 || sY[0] > 280) gOver = true;
    if(sX[0] == fX && sY[0] == fY) { sLen++; fX = random(2, 22) * 10; fY = random(5, 27) * 10; }
    tft.fillRect(sX[0], sY[0], 9, 9, ILI9341_GREEN);
    tft.fillRect(fX, fY, 9, 9, ILI9341_RED);
    delay(100);
  }
}
