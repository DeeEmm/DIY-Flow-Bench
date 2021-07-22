var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onLoad);


function initialiseWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage; // <-- add this line
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
    
    case "CONF_WIFI_AP_SSID":
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

function onLoad(event) {
  initialiseWebSocket();
  initialiseButtons();
  document.getElementById("defaultOpen").click();
  console.log('Page Loaded');
}

function initialiseButtons() {
  document.getElementById('refresh-button').addEventListener('click', function(){refresh();});
  document.getElementById('load-config-button').addEventListener('click', function(){socketSend('loadConfig');});
  document.getElementById('save-config-button').addEventListener('click', function(){socketSend('saveConfig');});
}

function serializeJSON (form) {
  // Create a new FormData object
  const formData = new FormData(form);

  // Create an object to hold the name/value pairs
  const pairs = {};

  // Add each name/value pair to the object
  for (const [name, value] of formData) {
    pairs[name] = value;
  }

  // Return the JSON string
  return JSON.stringify(pairs, null, 2);
}

function socketSend(message){
 
  
  switch (message) {
    
    case "loadConfig":
      websocket.send(message);
      console.log('Web Socket Message: ' + message);
    break;

    case "saveConfig":
      const form = document.querySelector('form');
      //serialise web form data into JSON
      //Send JSON to server to update config.txt

      websocket.send(serializeJSON(form));
    break;
  }

}

function openPage(pageName, elmnt, color) {
  // Hide all elements with class="tabcontent" by default */
  var i, tabcontent, tablinks;
  tabcontent = document.getElementsByClassName("tabcontent");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }

  // Show the specific tab content
  document.getElementById(pageName).style.display = "block";

  // Add the specific color to the button used to open the tab content
//  elmnt.style.backgroundColor = color;
}
