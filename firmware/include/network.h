#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <time.h>
#include "config.h"

enum NetStatus {
  NET_DISCONNECTED,
  NET_CONNECTING,
  NET_PORTAL_ACTIVE,
  NET_CONNECTED,
  NET_NTP_SYNCED
};

static NetStatus netStatus = NET_DISCONNECTED;
static WiFiManager wm;
static unsigned long lastReconnectAttempt = 0;
static const unsigned long RECONNECT_INTERVAL_MS = 30000;

// Called from setup(). Blocks until credentials are entered
// on first boot, then auto-connects on subsequent boots.
inline void networkSetup() {
  wm.setConfigPortalTimeout(180);  // portal times out after 3 min
  wm.setConnectTimeout(15);

  netStatus = NET_PORTAL_ACTIVE;

  // Tries saved credentials first.
  // If none exist (or fail), launches AP "NTP-Clock-Setup".
  if (wm.autoConnect("NTP-Clock-Setup")) {
    netStatus = NET_CONNECTED;
    configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);
    netStatus = NET_NTP_SYNCED;
  } else {
    netStatus = NET_DISCONNECTED;
  }
}

inline void networkLoop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (netStatus < NET_NTP_SYNCED) {
      configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);
      netStatus = NET_NTP_SYNCED;
    }
    return;
  }

  netStatus = NET_DISCONNECTED;
  if (millis() - lastReconnectAttempt > RECONNECT_INTERVAL_MS) {
    lastReconnectAttempt = millis();
    WiFi.disconnect();
    WiFi.begin();  // retries saved credentials
    netStatus = NET_CONNECTING;
  }
}

// Call this (e.g. on a button press) to wipe saved
// credentials and re-launch the config portal.
inline void networkReset() {
  wm.resetSettings();
  ESP.restart();
}

inline bool getTime(struct tm &timeinfo) {
  return getLocalTime(&timeinfo, 10);
}

inline const char* networkStatusStr() {
  switch (netStatus) {
    case NET_DISCONNECTED:   return "WiFi disconnected";
    case NET_CONNECTING:     return "Connecting...";
    case NET_PORTAL_ACTIVE:  return "Connect to NTP-Clock-Setup";
    case NET_CONNECTED:      return "WiFi OK, syncing...";
    case NET_NTP_SYNCED:     return "Synced";
    default:                 return "Unknown";
  }
}

#endif // NETWORK_H