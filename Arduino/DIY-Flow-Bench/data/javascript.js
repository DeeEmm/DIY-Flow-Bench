var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage; // <-- add this line
}

function onOpen(event) {
  console.log('Connection opened');
  initialiseData();  
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
  console.log(event.data);
  var myObj = JSON.parse(event.data);
  var keys = Object.keys(myObj);

  for (var i = 0; i < keys.length; i++){
    var key = keys[i];
    document.getElementById(key).innerHTML = myObj[key];
  }
}

window.addEventListener('load', onLoad);

function onLoad(event) {
  initWebSocket();
  initButton();
  console.log('Page Loaded');
}

function initButton() {
  document.getElementById('button').addEventListener('click', toggle);
}

function toggle(){
  websocket.send('toggle');
}

function initialiseData(){
  websocket.send('toggle');
}

