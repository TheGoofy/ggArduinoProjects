#pragma once

#include <functional>

class ggWiFiConnection {

public:

  typedef std::function<void(bool aConnected)> tConnectionFunc;

  ggWiFiConnection()
  : mConnected(false),
    mConnectionFunc(nullptr) {
  }

  void OnConnection(tConnectionFunc aConnectionFunc) {
    mConnectionFunc = aConnectionFunc;
  }

  void Begin() {
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);
  }

  void Run() {
    bool vConnected = (WiFi.status() == WL_CONNECTED);
    if (mConnected != vConnected) {
      mConnected = vConnected;
      if (mConnectionFunc != nullptr) {
        mConnectionFunc(mConnected);
      }
    }
  }

  void PrintDebug(const String& aName = "") const {
    unsigned long vQuality = 2 * (WiFi.RSSI() + 100);
    if (vQuality > 100) vQuality = 100;
    ggDebug vDebug("ggWiFiConnection", aName);
    vDebug.PrintF("SSID = %s\n", WiFi.SSID().c_str());
    vDebug.PrintF("RSSI = %ddBm\n", WiFi.RSSI());
    vDebug.PrintF("RSSI quality = %d%%\n", vQuality);
    vDebug.PrintF("IP = %s\n", WiFi.localIP().toString().c_str());
    vDebug.PrintF("mConnected = %s\n", mConnected ? "true" : "false");
    vDebug.PrintF("mConnectionFunc = 0x%08X\n", std::addressof(mConnectionFunc));
  }

private:

  bool mConnected;

  tConnectionFunc mConnectionFunc;

};
