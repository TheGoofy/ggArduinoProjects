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

  void Print(Stream& aStream) const {
    aStream.print("Connected to: ");
    aStream.println(WiFi.SSID());
    aStream.print("IP address: ");
    aStream.println(WiFi.localIP());
  }

private:

  bool mConnected;

  tConnectionFunc mConnectionFunc;

};
