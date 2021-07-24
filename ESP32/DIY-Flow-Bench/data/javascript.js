var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

/***
* Web Socket handlers
*/

function initialiseWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage;
}

function refresh (){
  websocket.send('refresh'); 
  websocket.send('loadConfig'); 
}


function onOpen(event) {
  console.log('Connection opened');
  refresh();
}


function onClose(event) {
  console.log('Connection closed');
  setTimeout(initialiseWebSocket, 2000);
}


function onMessage(event) {
  console.log(event.data);
  var myObj = JSON.parse(event.data);
  var keys = Object.keys(myObj);
  
  console.log(keys[0]);
  
  switch (keys[0]) {
    
    // We have received config data (not #st value)
    case "CONF_WIFI_SSID": 
      for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        try {
          document.getElementById(key).value = myObj[key];
          //    console.log(key + ' : ' + myObj[key]);
        } catch (error) {
          console.log('Missing or incorrect configuration parameter(s)');
        }
      }
    break;
    
    //We have received status data
    case "STATUS_MESSAGE":
      for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        try {
          document.getElementById(key).innerHTML = myObj[key];
          //    console.log(key + ' : ' + myObj[key]);
        } catch (error) {
          console.log(error);
          console.log('Missing or incorrect data parameter(s)');
        }
      }    
    break;
    
  }
  
}


/***
* Page Load Handler
*/
function onLoad(event) {
  initialiseWebSocket();
  initialiseButtons();
  document.getElementById("defaultOpen").click();
  console.log('Page Loaded');
}


/***
* Initialise Buttons
*/
function initialiseButtons() {
  document.getElementById('refresh-button').addEventListener('click', function(){refresh();});
  document.getElementById('load-config-button').addEventListener('click', function(){socketSend('loadConfig');});
  document.getElementById('calibrate-button').addEventListener('click', function(){socketSend('calibrate');});
  document.getElementById('save-config-button').addEventListener('click', function(){socketSend('saveConfig');});
}


/***
* Serialise JSON
*/
function serializeJSON (form) {
  const formData = new FormData(form);
  const pairs = {};
  for (const [name, value] of formData) {
    pairs[name] = value;
  }
  return JSON.stringify(pairs, null, 2);
}

/***
* Socket Send
*/
function socketSend(message){

  switch (message) {
    
    case "loadConfig":
      websocket.send(message);
//      console.log('Web Socket Message: ' + message);
    break;

    case "saveConfig":
      const form = document.querySelector('form');
      websocket.send(serializeJSON(form));
    break;
  }

}


/***
* Page tabs
*/
function openPage(pageName, elmnt, color) {
  var i, tabcontent, tablinks;
  tabcontent = document.getElementsByClassName("tabcontent");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }
  document.getElementById(pageName).style.display = "block";
}


/***
* Modal Dialogs
*/

var fileModal = document.getElementById("fileModal");
var statusModal = document.getElementById("statusModal");

var spanCloseFileModal = document.getElementsByClassName("closeFileModal")[0];
var spanCloseStatusModal = document.getElementsByClassName("closeStatusModal")[0];

spanCloseFileModal.onclick = function() {
  fileModal.style.display = "none";
}

spanCloseStatusModal.onclick = function() {
  statusModal.style.display = "none";
}

window.onclick = function(event) {
  if (event.target == fileModal || event.target == statusModal ) {
    fileModal.style.display = "none";
    statusModal.style.display = "none";
  }
}