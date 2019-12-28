#pragma once

char mWebServerHtmlRoot[] PROGMEM = R"=====(
﻿<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>
<html>

<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>
  <meta http-equiv='Content-Style-Type' content='text/css'>
  <title>ESP8266 Lamp</title>
  <style type='text/css'>
  
    .selectnone {
      -webkit-user-select: none;
      -khtml-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
    }
    
  </style>  
  <script language='javascript' type='text/javascript'>

    function initAll() {
      initWebSocket();
      initHtmlElements();
    }
  
    var mWebSocket;
    var mWebSocketReConnectTimerID = 0;

    function initWebSocket() {
      // init web socket client
      mWebSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
      mWebSocket.onopen = onWebSocketOpen;
      mWebSocket.onclose = onWebSocketClose;
      mWebSocket.onerror = onWebSocketError;
      mWebSocket.onmessage = onWebSocketMessage;
    }

    function onWebSocketOpen(aEvent) {
      mWebSocketStatus.innerHTML = 'connected';
      if (mWebSocketReConnectTimerID) {
        window.clearInterval(mWebSocketTimerID);
        mWebSocketReConnectTimerID = 0;
      }
    }

    function onWebSocketClose(aEvent) {
      mWebSocketStatus.innerHTML = 'disconnected';
      if (!mWebSocketReConnectTimerID) {
        mWebSocketReConnectTimerID = window.setInterval(initWebSocket, 5000);
      }
    }

    function onWebSocketError(aEvent) {
      mWebSocketStatus.innerHTML = aEvent;
    }

    function onWebSocketMessage(aEvent) {
      // server sends actual function names, which can directly be executed by the client
      eval(aEvent.data);
    }

    function initHtmlElements() {
      mName.onchange = function() {
        mWebSocket.send('SetName(' + mName.value + ')');
      }
      mOn.onchange = function() {
        mWebSocket.send('SetOn(' + mOn.checked + ')');
      }
      mCenterBrightnessRange.onchange = function() {
        mWebSocket.send('SetCenterBrightness(' + mCenterBrightnessRange.value + ')');
      }
      mRingColor.onchange = function() {
        mWebSocket.send('SetRingColor(' + mRingColor.value + ')');
      }
    }

    function UpdateName(aName) {
      mName.value = aName;
    }

    function UpdateOn(aOn) {
      mOn.checked = aOn;
    }

    function UpdateCenterBrightness(aBrightness) {
      mCenterBrightness.bgColor = aBrightness > 0.5 ? 'Gold' : 'LightGray';
      mCenterBrightnessValue.innerHTML = Math.round(100.0 * aBrightness) + '%';
      mCenterBrightnessRange.value = aBrightness;
    }

    function UpdateRingColor(aColor) {
      mRingColor.value = aColor;
    }

  </script>
</head>

<body onload='javascript:initAll()'>
  <table border='0' cellspacing='2' cellpadding='5'>

    <tr>
      <td>Web Socket Status</td>
      <td id='mWebSocketStatus'>(na)</td>
    </tr>

    <tr>
      <td>Name</td>
      <td><input id='mName' type='text' value='(na)'></td>
    </tr>

    <tr>
      <td>On/Off</td>
      <td><input id='mOn' type='checkbox'></td>
    </tr>

    <tr>
      <td>Center Brightness</td>
      <td id='mCenterBrightness'><table cellpadding='0' cellspacing='0' border='0'><tr>
        <td><input id='mCenterBrightnessRange' type='range' name='centerbrightnessrange' min='0' max='1' step='0.01' value='0'></td>
        <td>&nbsp;<span id='mCenterBrightnessValue'>(na)</span></td>
      </tr></table></td>
    </tr>

    <tr>
      <td>Ring Color</td>
      <td><input id='mRingColor' type="color" value="#ff0000"></td>
    </tr>

  </table>
</body>

</html>
)=====";
