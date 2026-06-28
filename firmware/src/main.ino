// ============================================================
// NTP Desk Clock
// ESP32-S3 Feather V2 + ST7735 TFT
//
// Syncs time via WiFi/NTP. No RTC module required.
// Displays time, date, and seconds progress bar.
// Auto-dims backlight at night.
// ============================================================

#include "config.h"
#include "network.h"
#include "display.h"

int lastSec = -1;

void setup() {
  Serial.begin(115200);
  delay(500);

  displaySetup();
  displayShowStatus("Starting WiFi...", "If new: connect to", 
                    "NTP-Clock-Setup AP");

  networkSetup();

  if (netStatus == NET_NTP_SYNCED) {
    displayShowStatus("Time synced!");
  } else {
    displayShowStatus("WiFi failed.", "Restarting in 10s...");
    delay(10000);
    ESP.restart();
  }
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  // Maintain WiFi connection
  networkLoop();

  // Get current time
  struct tm timeinfo;
  if (!getTime(timeinfo)) {
    // No time yet — show status
    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate > 2000) {
      lastStatusUpdate = millis();
      displayShowStatus("Waiting for NTP...", networkStatusStr());
    }
    delay(100);
    return;
  }

  // Only redraw when second changes
  if (timeinfo.tm_sec == lastSec) {
    delay(100);
    return;
  }
  lastSec = timeinfo.tm_sec;

  // Draw clock face
  displayShowClock(timeinfo);
}
