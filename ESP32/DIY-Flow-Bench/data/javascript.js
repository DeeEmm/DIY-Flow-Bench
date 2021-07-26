var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

var fileModal = document.getElementById("fileModal");
var statusModal = document.getElementById("statusModal");
var spanCloseFileModal = document.getElementsByClassName("closeFileModal")[0];
var spanCloseStatusModal = document.getElementsByClassName("closeStatusModal")[0];

// websocket message encoding - we need to send intVal to ESP32 Select(Case) 
const GET_FLOW_DATA = 1;
const CONFIG = 2;
const CALIBRATE = 3;
const FILE_LIST = 4;
const SYS_STATUS = 5;
const SAVE_CONFIG = 6;
const LOAD_CONFIG = 6;


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

function onOpen(event) {
  console.log('Connection opened');
  socketSend(GET_FLOW_DATA);
  socketSend(CONFIG);
}


function onClose(event) {
  console.log('Connection closed');
  setTimeout(initialiseWebSocket, 2000);
}


function onMessage(event) {
  
  var myObj = JSON.parse(event.data);
  var keys = Object.keys(myObj);
  var values = Object.values(myObj);
  
  console.log("onMessage Event: " + values[0]);
  
  // Lets check the first key value to determine what message we have received
  switch (values[0]) {
    
    // We have received config data
    case LOAD_CONFIG: 
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
    
    //We have received bench data
    case GET_FLOW_DATA:
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
    
    // We have received a file list
    case FILE_LIST: 
      var fileList = '';
      for (var i = 1; i < keys.length; i++){
        var key = keys[i];
        try {
          
          // we need to break a rule here and create some HTML 
          fileList += '<div class="fileListRow"><span class="column left"><span onclick="downloaFile()">' + key + '</span></span><span class="column middle"><span class="fileSizeTxt">' + myObj[key] + ' bytes</span></span><span class="column right"><span class="fileDelLink" onclick="deleteFile(' + key + ')">[X]</span></span>';
                   
          //    console.log(key + ' : ' + myObj[key]);
        } catch (error) {
          console.log(error);
          console.log('Missing or incorrect system status');
        }
        document.getElementById('file_list').innerHTML = fileList;
      } 
    break;

    // We have received system status
    case SYS_STATUS: 
      for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        try {
          document.getElementById(key).innerHTML = myObj[key];
          //    console.log(key + ' : ' + myObj[key]);
        } catch (error) {
          console.log('Missing or incorrect status parameter(s)');
        }
      } 
    break;



  }
  
}


function downloadFile(filename) {
  // TODO send the file to the browser
}

function deleteFile(filename) {
  // TODO Delete the file
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
  
  document.getElementById('refresh-button').addEventListener('click', function(){socketSend(GET_FLOW_DATA);});
  document.getElementById('load-config-button').addEventListener('click', function(){socketSend(CONFIG);});
  document.getElementById('calibrate-button').addEventListener('click', function(){socketSend(CALIBRATE);});
  document.getElementById('save-config-button').addEventListener('click', function(){socketSend(SAVE_CONFIG);});
  document.getElementById('file-manager-button').addEventListener('click', function(){socketSend(FILE_LIST);});
  document.getElementById('status-button').addEventListener('click', function(){socketSend(SYS_STATUS);});
  
}


/***
* Serialise JSON
*/
function configFormJSON () {
  
  const form = document.querySelector('form');
  const formData = new FormData(form);
  const assocArray = {};
  assocArray['SCHEMA'] = 6;
  for (const [name, value] of formData) {
    assocArray[name] = value;
  }
  return JSON.stringify(assocArray, null, 2);
  
}

/***
* Socket Send
*/
function socketSend(message){
  
  console.log('Web Socket Message: ' + message);
  
  var jsonMessage;

  switch (message) {
    
    case GET_FLOW_DATA:
      jsonMessage = "{\"SCHEMA\":\"" + GET_FLOW_DATA + "\"}";
    break;

    case CONFIG:
      jsonMessage = "{\"SCHEMA\":\"" + CONFIG + "\"}";
    break;

    case CALIBRATE:
      jsonMessage ="{\"SCHEMA\":\"" + CALIBRATE + "\"}";
    break;
    
    case FILE_LIST:
      jsonMessage ="{\"SCHEMA\":\"" + FILE_LIST + "\"}";
      document.getElementById('fileModal').style.display='block';
    break;
    
    case SYS_STATUS:
      jsonMessage ="{\"SCHEMA\":\"" + SYS_STATUS + "\"}";
      document.getElementById('statusModal').style.display='block';
    break;
  
    case SAVE_CONFIG:
      jsonMessage = configFormJSON();
    break;

  }
  
  websocket.send(jsonMessage);

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