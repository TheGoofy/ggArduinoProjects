
function CreateWebSocket(
  aParent,
  aWebSocketObjectName,
  aHostAddress,
  aMessageFunc,
  aReconnectTimeMS,
  aStatusDiv,
  aDefaultAddress)
{
  if (aHostAddress == '') {
    if (!aDefaultAddress) aDefaultAddress = "192.168.0.150";
    aHostAddress = window.prompt('Please enter address of WebSocket server', aDefaultAddress);
  }

  let vWebSocket = aParent[aWebSocketObjectName];
  if (!vWebSocket || vWebSocket.readyState == WebSocket.CLOSED) {

    aParent[aWebSocketObjectName] = new WebSocket('ws://' + aHostAddress +':81/', ['arduino']);
    let vWebSocket = aParent[aWebSocketObjectName];
    
    vWebSocket.onopen = function (aEvent) {
      console.info('vWebSocket.onopen()');
      if (aStatusDiv) aStatusDiv.innerHTML = 'connected';
      if (aParent.mWebSocketReConnectTimerID) {
        window.clearInterval(aParent.mWebSocketReConnectTimerID);
        aParent.mWebSocketReConnectTimerID = null;
      }
    };
    
    vWebSocket.onclose = function (aEvent) {
      console.warn('vWebSocket.onclose()');
      if (aStatusDiv) aStatusDiv.innerHTML = 'disconnected';
      if (!aParent.mWebSocketReConnectTimerID) {
        aParent.mWebSocketReConnectTimerID = window.setInterval(function () {
          CreateWebSocket(aParent, aWebSocketObjectName, aHostAddress, aMessageFunc, aReconnectTimeMS, aStatusDiv);
        }, aReconnectTimeMS);
      }
    };
    
    vWebSocket.onerror = function (aEvent) {
      console.error('vWebSocket.onerror()');
      if (aStatusDiv) aStatusDiv.innerHTML = 'error';
    };
    
    vWebSocket.onmessage = function (aEvent) {
      try {
        // console.log('OnWebSocketMessage() - evaluating: ' + aEvent.data);
        aMessageFunc(aEvent.data);
      }
      catch (vError) {
        console.error('vWebSocket.onmessage() - unable to evaluate: ' + aEvent.data);
      }
    };
  }
}
