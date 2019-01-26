#pragma once

char mHtmlRoot[] PROGMEM = R"=====(
﻿<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>
<html>

<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>
  <meta http-equiv='Content-Style-Type' content='text/css'>
  <title>ESP8266 Test GPIO</title>
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
  
    var mWebSocket;
    var mWebSocketReConnectTimerID = 0;
    
    function initWebSocket() {
      mWebSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
      mWebSocket.onopen = onWebSocketOpen;
      mWebSocket.onclose = onWebSocketClose;
      mWebSocket.onmessage = onWebSocketMessage;
    }

    function onWebSocketOpen(aEvent) {
      if (mWebSocketReConnectTimerID) {
        window.clearInterval(mWebSocketTimerID);
        mWebSocketReConnectTimerID = 0;
      }
    }
    
    function onWebSocketClose(aEvent) {
      if (!mWebSocketReConnectTimerID) {
        mWebSocketReConnectTimerID = window.setInterval(initWebSocket, 5000);
      }
    }
    
    function onWebSocketMessage(aEvent) {
      // server broadcasts real function names, which are executed by the client
      eval(aEvent.data);
    }
    
    function UpdateDisplay(aId, aOn) {
      document.getElementById('td' + aId).bgColor = aOn ? 'orange' : 'skyblue';
    }

    function onClick(aId) {
      var vToggleOutput = 'ToggleOutput(' + aId + ')';
      mWebSocket.send(vToggleOutput);
    }
    
  </script>
</head>

<body onload='javascript:initWebSocket()'>
  <table border='0' cellspacing='2' cellpadding='5'>
    <tr><td id='td0' class='selectnone' align='center' valign='middle' onclick='onClick(0)'>toggle output 0</td></tr>
    <tr><td id='td1' class='selectnone' align='center' valign='middle' onclick='onClick(1)'>toggle output 1</td></tr>
    <tr><td id='td2' class='selectnone' align='center' valign='middle' onclick='onClick(2)'>toggle output 2</td></tr>
    <tr><td id='td3' class='selectnone' align='center' valign='middle' onclick='onClick(3)'>toggle output 3</td></tr>
  </table>
</body>

</html>
)=====";

