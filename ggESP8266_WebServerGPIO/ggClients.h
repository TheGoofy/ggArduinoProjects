#pragma once

#include <WebSocketsServer.h>
#include "ggOutputPins.h"

class ggClients {
  
public:

  ggClients(WebSocketsServer& aWebSockets)
  : mWebSockets(aWebSockets) {
  }

  // update on selected client selected pin
  void UpdateDisplay(uint8_t aClientNumber, int aIndex, bool aValue) {
    mWebSockets.sendTXT(aClientNumber, String("UpdateDisplay(") + aIndex + ", " + aValue + ")");
  }

  // update on all client selected pin
  void UpdateDisplay(int aIndex, bool aValue) {
    mWebSockets.broadcastTXT(String("UpdateDisplay(") + aIndex + ", " + aValue + ")");
  }

  // update on selected clients all pins
  void UpdateDisplay(uint8_t aClientNumber) {
    ggOutputPins::ForEach([&] (int aIndex) {
      UpdateDisplay(aClientNumber, aIndex, ggOutputPins::Get(aIndex));
    });
  }

  // update on all clients all pins
  void UpdateDisplay() {
    ggOutputPins::ForEach([&] (int aIndex) {
      UpdateDisplay(aIndex, ggOutputPins::Get(aIndex));
    });
  }

private:

  WebSocketsServer& mWebSockets;
  
};

