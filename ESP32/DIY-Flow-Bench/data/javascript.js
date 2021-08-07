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
const LOAD_CONFIG = 7;
const FILE_DOWNLOAD = 8;
const FILE_DELETE = 9;
const FILE_UPLOAD = 10;
const START_BENCH = 11;
const STOP_BENCH = 12;

/****************************************
* Web Socket handlers
***/
function initialiseWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onSocketMessageReceive;
}

/****************************************
* OnOpen event handler
***/
function onOpen(event) {
  console.log('Connection opened');
  socketMessageSend(GET_FLOW_DATA);
  socketMessageSend(CONFIG);
}

/****************************************
* onClose event handler
***/
function onClose(event) {
  console.log('Connection closed');
  setTimeout(initialiseWebSocket, 2000);
}


/****************************************
* Websocket message event handler
***/
function onSocketMessageReceive(event) {
  
  try {
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);
    var values = Object.values(myObj);
  } catch(err) {
  }
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
    
    
    case CALIBRATE: 
      /* We have calculated the calibration offset (on the ESP32) and pushed the configuration values to the browser.
       Now we are remotely instigating a save config (from the ESP32) to save the browser values back to the config.json file on the ESP32
       This is not perfect as we are playing message pingpong but it uses our existing functions and negates the need to write a new
       Save config function on the ESP32 just to be able to update the calibration value.
       NOTE: We could save the config values into a dedicated file and keep them out of the browser altogether (perhaps we should) */
       socketMessageSend(SAVE_CONFIG); 
    break;
    
    // We have received a file list
    case FILE_LIST: 
      var fileList = '';
      for (var i = 1; i < keys.length; i++){
        var key = keys[i];
        try {
          
          // we need to break a cardinal rule here and create some HTML 
          fileList += '<div class="fileListRow"><span class="column left"><a href="/download' + key + '" download class="file-link" onclick="downloadFile("' + key + '")">' + key + '</a></span><span class="column middle"><span class="fileSizeTxt">' + myObj[key] + ' bytes</span></span><span class="column right"><button id="delete-button"  onclick="socketMessageSend(\'' + FILE_DELETE + ':' + key + '\')" class="button-sml">Del</button></span>';
          
                   
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


/****************************************
* onFileUpload event handler
***/
function onFileUpload(event) {
  this.setState({file: event.target.files[0]});
  const {file} = this.state;
  const data = new FormData;
  data.append('data', file);
  fetch('/upload', {
      method: 'POST',
      body: data
  })
      .catch(e => {
          console.log('Request failed', e);
      });
}



/****************************************
* onLoad handler
***/
function onLoad(event) {
  
  initialiseWebSocket();
  initialiseButtons();
  document.getElementById("defaultOpen").click();
  console.log('Page Loaded');
  
}


/****************************************
* Initialise buttons
***/
function initialiseButtons() {
  
  document.getElementById('refresh-button').addEventListener('click', function(){socketMessageSend(GET_FLOW_DATA);});
  document.getElementById('load-config-button').addEventListener('click', function(){socketMessageSend(CONFIG);});
  document.getElementById('calibrate-button').addEventListener('click', function(){socketMessageSend(CALIBRATE);});
  document.getElementById('save-config-button').addEventListener('click', function(){socketMessageSend(SAVE_CONFIG);});
  document.getElementById('file-manager-button').addEventListener('click', function(){socketMessageSend(FILE_LIST);});
  document.getElementById('status-button').addEventListener('click', function(){socketMessageSend(SYS_STATUS);});
  document.getElementById('on-button').addEventListener('click', function(){socketMessageSend(START_BENCH);});
  document.getElementById('off-button').addEventListener('click', function(){socketMessageSend(STOP_BENCH);});
  
}


/****************************************
* configForumJSON
***/
function configFormJSON () {
  
  const form = document.querySelectorAll('form')[1];
  //const form = document.querySelector('#config-form');
  const formData = new FormData(form);
  const assocArray = {};
  assocArray['SCHEMA'] = 6;
  for (const [name, value] of formData) {
    assocArray[name] = value;
  }
  return JSON.stringify(assocArray, null, 2);
  
}

/****************************************
* socketMessageSend
***/
function socketMessageSend(message){
  
  console.log('Web Socket Message: ' + message);
  
  var jsonMessage;
  var schema;
  var action;
  var actionData;
  
  // check if data has been passed to the function with the action message
  var searchResult = String(message).search(":");
  if (searchResult != -1) {
      schema = message.split(":");
      action = Number(schema[0]);
      actionData = schema[1];
  } else {
      action = message;
  }
  
  switch (action) {
    
    case GET_FLOW_DATA: // SCHEMA:1
      jsonMessage = '{\"SCHEMA\":\"' + GET_FLOW_DATA + '\"}';
    break;

    case CONFIG: // SCHEMA:2
      jsonMessage = '{\"SCHEMA\":\"' + CONFIG + '\"}';
    break;

    case CALIBRATE: // SCHEMA:3
      jsonMessage ='{\"SCHEMA\":\"' + CALIBRATE + '\"}';
    break;
    
    case FILE_LIST: // SCHEMA:4
      jsonMessage ='{\"SCHEMA\":\"' + FILE_LIST + '\"}';
      document.getElementById('fileModal').style.display='block';
    break;
    
    case SYS_STATUS: // SCHEMA:5
      jsonMessage ='{\"SCHEMA\":\"' + SYS_STATUS + '\"}';
      document.getElementById('statusModal').style.display='block';
    break;
  
    case SAVE_CONFIG:// SCHEMA:6
      jsonMessage = configFormJSON();
    break;
    
    case LOAD_CONFIG: // SCHEMA:7
    
    break;
    
    case FILE_DOWNLOAD: // SCHEMA:8
    
    break;
    
    case FILE_DELETE: // SCHEMA:9
      jsonMessage ='{\"SCHEMA\":\"' + FILE_DELETE + '\",\"FILENAME\":\"' + actionData + '\"}';

    break;
    
    case FILE_UPLOAD: // SCHEMA:10
      
    break;
    
    case START_BENCH: // SCHEMA:11
      jsonMessage ='{\"SCHEMA\":\"' + START_BENCH + '\"}';
    break;
    
    case STOP_BENCH: // SCHEMA:12
      jsonMessage ='{\"SCHEMA\":\"' + STOP_BENCH + '\"}';
    break;

  }
  console.log(jsonMessage);
  websocket.send(jsonMessage);

}


/****************************************
* Page tabs
***/
function openPage(pageName, elmnt) {
  var i, tabcontent, tablinks;
  tabcontent = document.getElementsByClassName("tabcontent");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }
  document.getElementById(pageName).style.display = "block";
}


/****************************************
* Close modal file dialog
***/
spanCloseFileModal.onclick = function() {
  
  fileModal.style.display = "none";
  
}

/****************************************
* Close modal status dialog
***/
spanCloseStatusModal.onclick = function() {
  
  statusModal.style.display = "none";
  
}


/****************************************
* Close modal dialogs (lose focus)
***/
window.onclick = function(event) {
  
  if (event.target == fileModal || event.target == statusModal ) {
    fileModal.style.display = "none";
    statusModal.style.display = "none";
  }
  
}