#pragma once

class ggWiFiManager {

public:

  typedef std::function<void()> tFunc;

  ggWiFiManager(const String& aHostName = "")
  : mHostName(aHostName),
    mConnected(false),
    mConnectFunc(nullptr),
    mDisconnectFunc(nullptr) {
  }

  void OnConnect(tFunc aConnectFunc) {
    mConnectFunc = aConnectFunc;
  }

  void OnDisconnect(tFunc aDisconnectFunc) {
    mDisconnectFunc = aDisconnectFunc;
  }

  bool IsWifiConnected() const {
    return WiFi.isConnected();
  }

  void Begin() {
    if (mHostName != "") WiFi.hostname(mHostName.c_str());
  }
  
  void Run() {
    bool vConnected = IsWifiConnected();
    if (mConnected != vConnected) {
      mConnected = vConnected;
      if (mConnected && mConnectFunc != nullptr) mConnectFunc();
      if (!mConnected && mDisconnectFunc != nullptr) mDisconnectFunc();
    }
  }

  int GetSignalQuality() const {
    return IsWifiConnected() ? ConvertRSSItoQuality(WiFi.RSSI()) : 0;
  }

  void Print(Stream& aStream) const {
    aStream.print("Connected to: ");
    aStream.println(WiFi.SSID());
    aStream.print("IP address: ");
    aStream.println(WiFi.localIP());
  }

private:

  static int ConvertRSSItoQuality(int aRSSI) {
    int vQuality = 0;
    if (aRSSI <= -100) {
      vQuality = 0;
    } else if (aRSSI >= -50) {
      vQuality = 100;
    } else {
      vQuality = 2 * (aRSSI + 100);
    }
    return vQuality;
  }

  String mHostName;
  
  bool mConnected;

  tFunc mConnectFunc;
  tFunc mDisconnectFunc;
    
};
