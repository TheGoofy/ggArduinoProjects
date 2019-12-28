#pragma once

#include <functional>

class ggWiFiConnection {

public:

  typedef std::function<void()> tFunc;

  ggWiFiConnection()
  : mConnected(false),
    mConnectFunc(nullptr),
    mDisconnectFunc(nullptr) {
  }

  void OnConnect(tFunc aConnectFunc) {
    mConnectFunc = aConnectFunc;
  }

  void OnDisconnect(tFunc aDisconnectFunc) {
    mDisconnectFunc = aDisconnectFunc;
  }

  void Begin() {
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);
  }

  void Run() {
    bool vConnected = (WiFi.status() == WL_CONNECTED);
    if (mConnected != vConnected) {
      mConnected = vConnected;
      if (mConnected && mConnectFunc != nullptr) mConnectFunc();
      if (!mConnected && mDisconnectFunc != nullptr) mDisconnectFunc();
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

  tFunc mConnectFunc;
  tFunc mDisconnectFunc;

};
