#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "config.h"

#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// Screen dimensions after rotation (landscape)
#define SCREEN_W  160
#define SCREEN_H  128

static Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ── Helper: center text horizontally ───────────────────────

inline int centerX(const char* text) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return (SCREEN_W - w) / 2 - x1;
}

inline void getTextSize(const char* text, uint16_t &w, uint16_t &h) {
  int16_t x1, y1;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
}

// ── Backlight ──────────────────────────────────────────────

inline void setBacklight(uint8_t brightness) {
  analogWrite(TFT_LITE, brightness);
}

inline void updateBacklight(int hour) {
  bool isNight = (hour >= NIGHT_START_HOUR || hour < NIGHT_END_HOUR);
  setBacklight(isNight ? BRIGHTNESS_NIGHT : BRIGHTNESS_DAY);
}

// ── Init ───────────────────────────────────────────────────

inline void displaySetup() {
  pinMode(TFT_LITE, OUTPUT);
  setBacklight(BRIGHTNESS_DAY);

  tft.initR(TFT_TAB_TYPE);
  tft.setRotation(3);  // landscape: 160x128
  tft.fillScreen(ST77XX_BLACK);
}

// ── Startup / Status Screens ───────────────────────────────
// Status uses built-in font (small, reliable for long strings)

inline void displayShowStatus(const char* line1,
                               const char* line2 = nullptr,
                               const char* line3 = nullptr) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setFont(NULL);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 45);
  tft.print(line1);
  if (line2) { tft.setCursor(10, 60); tft.print(line2); }
  if (line3) { tft.setCursor(10, 75); tft.print(line3); }
}

// ── Main Clock Face ────────────────────────────────────────

inline void displayShowClock(struct tm &t) {
  static bool firstDraw = true;
  if (firstDraw) {
    tft.fillScreen(ST77XX_BLACK);
    firstDraw = false;
  }

  int h = t.tm_hour;
  const char* ampm = "";

  if (!USE_24H_FORMAT) {
    ampm = (h >= 12) ? "PM" : "AM";
    if (h > 12) h -= 12;
    if (h == 0) h = 12;
  }

// ── Row 1: Time + AM/PM (centered as one block) ──
  char timeBuf[6];
  sprintf(timeBuf, "%d:%02d", h, t.tm_min);

  // Measure time
  tft.setFont(&FreeSansBold24pt7b);
  int16_t tx1, ty1;
  uint16_t timeW, timeH;
  tft.getTextBounds(timeBuf, 0, 0, &tx1, &ty1, &timeW, &timeH);

  // Measure AM/PM
  tft.setFont(&FreeSansBold12pt7b);
  int16_t ax1, ay1;
  uint16_t ampmW, ampmH;
  tft.getTextBounds(ampm, 0, 0, &ax1, &ay1, &ampmW, &ampmH);

  int gap = 4;
  int totalW = timeW + gap + ampmW;
  int blockX = (SCREEN_W - totalW) / 2;
  int timeY = 45;

  tft.fillRect(0, 5, SCREEN_W, 48, ST77XX_BLACK);

  tft.setFont(&FreeSansBold24pt7b);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(blockX - tx1, timeY);  // subtract tx1 to compensate
  tft.print(timeBuf);

  if (!USE_24H_FORMAT) {
    tft.setFont(&FreeSansBold12pt7b);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(blockX - tx1 + timeW + gap, timeY);
    tft.print(ampm);
  }

  // ── Row 2: Day name ──
  char dayBuf[10];
  strftime(dayBuf, sizeof(dayBuf), "%A", &t);

  tft.setFont(&FreeSans9pt7b);
  tft.fillRect(0, 58, SCREEN_W, 22, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(centerX(dayBuf), 74);
  tft.print(dayBuf);

  // ── Row 3: Date ──
  char dateBuf[14];
  strftime(dateBuf, sizeof(dateBuf), "%b %d, %Y", &t);

  tft.setFont(&FreeSans9pt7b);
  tft.fillRect(0, 84, SCREEN_W, 22, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(centerX(dateBuf), 100);
  tft.print(dateBuf);

  // Reset to built-in font
  tft.setFont(NULL);

  updateBacklight(t.tm_hour);
}

#endif // DISPLAY_H