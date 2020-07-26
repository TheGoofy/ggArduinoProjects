
function CreateWebSocket(
  aParent,
  aWebSocketObjectName,
  aHostAddress,
  aMessageFunc,
  aReconnectTimeMS,
  aStatusDiv)
{
  if (aHostAddress == '') {
    aHostAddress = window.prompt('Please enter address of WebSocket server', "192.168.0.150");
  }

  let vWebSocket = aParent[aWebSocketObjectName];
  if (!vWebSocket || vWebSocket.readyState == WebSocket.CLOSED) {

    aParent[aWebSocketObjectName] = new WebSocket('ws://' + aHostAddress +':81/', ['arduino']);
    let vWebSocket = aParent[aWebSocketObjectName];
    
    vWebSocket.onopen = function (aEvent) {
      console.info('OnWebSocketOpen()');
      if (aStatusDiv) aStatusDiv.innerHTML = 'connected';
      if (aParent.mWebSocketReConnectTimerID) {
        window.clearInterval(aParent.mWebSocketReConnectTimerID);
        aParent.mWebSocketReConnectTimerID = null;
      }
    };
    
    vWebSocket.onclose = function (aEvent) {
      console.warn('OnWebSocketClose()');
      if (aStatusDiv) aStatusDiv.innerHTML = 'disconnected';
      if (!aParent.mWebSocketReConnectTimerID) {
        aParent.mWebSocketReConnectTimerID = window.setInterval(function () {
          CreateWebSocket(aParent, aWebSocketObjectName, aHostAddress, aMessageFunc, aReconnectTimeMS, aStatusDiv);
        }, aReconnectTimeMS);
      }
    };
    
    vWebSocket.onerror = function (aEvent) {
      console.error('OnWebSocketError()');
      if (aStatusDiv) aStatusDiv.innerHTML = 'error';
    };
    
    vWebSocket.onmessage = function (aEvent) {
      try {
        console.log('OnWebSocketMessage() - evaluating: ' + aEvent.data);
        aMessageFunc(aEvent.data);
      }
      catch (vError) {
        console.error('OnWebSocketMessage() - unable to evaluate: ' + aEvent.data);
      }
    };
  }
}
