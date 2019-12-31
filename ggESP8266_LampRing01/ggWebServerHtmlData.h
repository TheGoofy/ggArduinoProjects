#pragma once

char mWebServerHtmlRoot[] PROGMEM = R"=====(
<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>
<html>

<head>
  <meta http-equiv='Content-Type' content='text/html; charset=utf-8'>
  <meta http-equiv='Content-Style-Type' content='text/css'>
  <meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no"/>
  <title>ESP8266 Lamp</title>
  <style type='text/css'>

.selectnone {
  -webkit-user-select: none;
  -khtml-user-select: none;
  -moz-user-select: none;
  -ms-user-select: none;
  user-select: none;
}

body {
  text-align: center;
  font-family: verdana,sans-serif;
  font-size: 0.8rem;
  background: #444;
}

td {
  text-align: center;
}

.r {
  border-radius: 2.4rem;
  line-height: 2.4rem;
  font-size: 1.2rem;
  vertical-align: middle;
  text-align: center;
}

input[type=button] {
  background-color: #000;
  border: none;
  cursor: pointer;
}

.switch {
  position: relative;
  display: inline-block;
  width: 100%;
  height: 34px;
}

.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

.switch_slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #000;
  -webkit-transition: .4s;
  transition: .4s;
}

.switch_slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .switch_slider {
  background-color: #f90;
}

input:focus + .switch_slider {
  box-shadow: 0 0 0px #f90;
}

input:checked + .switch_slider:before {
  -webkit-transform: translate(302px, -1px);
  -ms-transform: translate(302px, -1px);
  transform: translate(302px, -1px);
}

.switch_slider.round {
  border-radius: 34px;
}

.switch_slider.round:before {
  border-radius: 50%;
}

.range_slider {
  -webkit-appearance: none;
  width: 100%;
  height: 34px;
  border-radius: 17px;
  background-image: linear-gradient(to right, #000 05%, #f90 95%);
  outline: none;
  opacity: 1;
  -webkit-transition: .2s;
  transition: opacity .2s;
}

.range_slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 34px;
  height: 34px;
  border-radius: 50%;
  border: 4px solid #fff;
  background: transparent;
  cursor: pointer;
}

.range_slider::-moz-range-thumb {
  width: 34px;
  height: 34px;
  border-radius: 50%;
  border: 4px solid #fff;
  background: transparent;
  cursor: pointer;
}

  </style>  
  <script language='javascript' type='text/javascript'>

    var mWebSocket;
    var mWebSocketReConnectTimerID = 0;

    function initAll() {
      initHtmlElements();
      initWebSocket();
    }
  
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

    function WebSocketSendRingColorHSV() {
      mWebSocket.send('SetRingColorHSV('
        + mRingColorRangeH.value + ','
        + mRingColorRangeS.value + ','
        + mRingColorRangeV.value + ')');
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
      mRingColorRangeH.onchange = function() {
        WebSocketSendRingColorHSV();
      }
      mRingColorRangeS.onchange = function() {
        WebSocketSendRingColorHSV();
      }
      mRingColorRangeV.onchange = function() {
        WebSocketSendRingColorHSV();
      }
    }

    function ToHex(aValue) {
      vHex = Number(aValue).toString(16);
      if (vHex.length < 2) vHex = '0' + vHex;
      return vHex;
    }    

    function HSVtoRGB(aH, aS, aV) {
      vR = vG = vB = aV;
      if (aS != 0) {
        aH /= 42.501; // scale to 0..6
        aS /= 255; // scale to 0..1
        aV /= 255; // scale to 0..1
        vI = Math.floor(aH); // interval (region)
        vF = aH - vI; // factorial part of h
        vP = aV * (1 - aS           );
        vQ = aV * (1 - aS * (    vF));
        vT = aV * (1 - aS * (1 - vF));
        switch (vI) {
          case 0:  vR = aV; vG = vT; vB = vP; break;
          case 1:  vR = vQ; vG = aV; vB = vP; break;
          case 2:  vR = vP; vG = aV; vB = vT; break;
          case 3:  vR = vP; vG = vQ; vB = aV; break;
          case 4:  vR = vT; vG = vP; vB = aV; break;
          default: vR = aV; vG = vP; vB = vQ; break; // case 5 or 6
        }
        vR = Math.round(255 * vR);
        vG = Math.round(255 * vG);
        vB = Math.round(255 * vB);
      }
      return '#' + ToHex(vR) + ToHex(vG) + ToHex(vB);
    }

    function UpdateName(aName) {
      mName.value = aName;
    }

    function UpdateOn(aOn) {
      mOnValue.innerHTML = aOn ? 'On' : 'Off';
      mOn.checked = aOn;
    }

    function UpdateCenterBrightness(aBrightness) {
      mCenterBrightnessValue.innerHTML = Math.round(100.0 * aBrightness) + '%';
      mCenterBrightnessRange.value = aBrightness;
    }

    function UpdateRingColorHSV(aH, aS, aV) {
      mRingColorValueHSV.innerHTML = aH + '/' + aS + '/' + aV;
      mRingColorRangeH.value = aH;
      mRingColorRangeS.value = aS;
      mRingColorRangeV.value = aV;
      mRingColorRangeH.style.backgroundImage = 'linear-gradient(to right, '
        + HSVtoRGB(  0.0,aS,aV) + ' 5%, '
        + HSVtoRGB( 43.5,aS,aV) + ', '
        + HSVtoRGB( 85.0,aS,aV) + ', '
        + HSVtoRGB(127.5,aS,aV) + ', '
        + HSVtoRGB(170.0,aS,aV) + ', '
        + HSVtoRGB(221.5,aS,aV) + ', '
        + HSVtoRGB(255.0,aS,aV) + ' 95%)';
      mRingColorRangeS.style.backgroundImage = 'linear-gradient(to right, '
        + HSVtoRGB(aH,0,aV) + ' 5%, '
        + HSVtoRGB(aH,255,aV) + ' 95%)';
      mRingColorRangeV.style.backgroundImage = 'linear-gradient(to right, '
        + HSVtoRGB(aH,aS,0) + ' 5%, '
        + HSVtoRGB(aH,aS,255) + ' 95%)';
    }

  </script>
</head>

<body onload='javascript:initAll()'>
  <div style="text-align:left;display:inline-block;color:#aaa;min-width:340px;">
  <table border='0' cellspacing='3' cellpadding='0'width='100%' class='selectnone'>

    <tr><td>
      <br>Lamp Name
    </td></tr>
    <tr><td>
      <input id='mName' type='text' value='(na)' style='width:100%' class='r' maxlength='30'>
    </td></tr>

    <tr><td>
      <br>Power: <span id='mOnValue'>(na)</span>
    </td></tr>
    <tr><td>
      <label class='switch'><input id='mOn' type='checkbox'><span class='switch_slider round'></span></label>
    </td></tr>

    <tr><td>
      <br>Center Brightness: <span id='mCenterBrightnessValue'>(na)</span>
    </td></tr>
    <tr><td>
      <input id='mCenterBrightnessRange' type='range' name='centerbrightnessrange' min='0' max='1' step='0.01' value='0' class='range_slider'>
    </td></tr>

    <tr><td>
      <br>Ring Color: <span id='mRingColorValueHSV'> (na)</span>
    </td></tr>
    <tr><td>
      <input id='mRingColorRangeH' type='range' name='ringcolorrangeh' min='0' max='255' step='1' value='0' class='range_slider'><br>
      <input id='mRingColorRangeS' type='range' name='ringcolorranges' min='0' max='255' step='1' value='0' class='range_slider'><br>
      <input id='mRingColorRangeV' type='range' name='ringcolorrangev' min='0' max='255' step='1' value='0' class='range_slider'>
    </td></tr>

    <tr><td>
      <br><hr noshade>
      Web Socket Status: <span id='mWebSocketStatus'>(na)</span><br>
      (c) 2019, Christoph Laimer
    </td></tr>

  </table>
  </div>
</body>

</html>
)=====";
