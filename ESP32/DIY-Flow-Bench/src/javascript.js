/***********************************************************
* javascript.js
*
* Browser control code for DIY-Flow-bench project
* Handles data transmission and display update tasks
* Provides UI control to browser
*
* TODO: look at implimenting browser based file downloading for data recording...
* https://stackoverflow.com/questions/3665115/how-to-create-a-file-in-memory-for-user-to-download-but-not-through-server
*
***/




/***********************************************************
* Constants
* Websocket message encoding - we need to send intVal to ESP32 Select(Case) 
***/
const GET_FLOW_DATA = 1;
const REC_FLOW_DATA = 2;
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
const LEAK_CAL = 13;
const GET_CAL = 14;

var leakCalVal;
var flowCalVal;
var leakCalTolerance;
var leakTestThreshold;

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

var fileModal = document.getElementById("fileModal");
var aboutModal = document.getElementById("aboutModal");
var spanCloseFileModal = document.getElementsByClassName("closeFileModal")[0];
var spanCloseAboutModal = document.getElementsByClassName("closeAboutModal")[0];

/***********************************************************
* Initialise Web Socket connection
***/
function initialiseWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onSocketMessageReceive;
}

/***********************************************************
* OnOpen event handler
*
* Runs after web socket successfully created
***/
function onOpen(event) {
  console.log('Connection opened');
  socketMessageSend(GET_FLOW_DATA);
  socketMessageSend(LOAD_CONFIG);
  socketMessageSend(FILE_LIST);
  socketMessageSend(GET_CAL);
}

/***********************************************************
* onClose event handler
*
* Runs when web socket closed
***/
function onClose(event) {
  console.log('Connection closed');
  setTimeout(initialiseWebSocket, 2000);
}


/***********************************************************
* Websocket message event handler
*
* Runs when web socket message received
* Pulls header value from JSON data and processes action accordingly
***/
function onSocketMessageReceive(event) {
  
  try {
    var myObj = JSON.parse(event.data);
    console.log("Socket Message Event: " + parseInt(myObj.HEADER));
  } catch(err) {
    console.log("Malformed JSON data");
  }
  
  key = null;

  switch (parseInt(myObj.HEADER)) {    

    case LOAD_CONFIG:      
      for (key in myObj) {
        try {
          if (key != "HEADER") {
            document.getElementById(key).value = myObj[key];
             // console.log(key + ' : ' + myObj[key]);
           }
        } catch (error) {
          console.log('Missing or incorrect configuration parameter(s)');
        }
      }
    break;
    
    case GET_FLOW_DATA:
      for (key in myObj) {
        try {
          if (key != "HEADER") {
            document.getElementById(key).innerHTML = myObj[key];

             // console.log(key + ' : ' + myObj[key]);
             
             // leak test
             if (key == "PREF"){
                if (myObj[key] > (leakCalVal - leakCalTolerance)) {
                  // change pref colour
                  document.getElementById("PREF").className = "prefTestOK";
                } else if (myObj[key] > leakTestThreshold) {
                  // change pref colour back
                  document.getElementById("PREF").className = "prefTestNOK";
                } else {
                  document.getElementById("PREF").className = "tile-value";
                }
             }
           }
        } catch (error) {
          console.log('Missing or incorrect data parameter(s)');
        }
      }    
    break;
    
    case GET_CAL:
      console.log("get-cal");
      for (key in myObj) {
        console.log(key);
        try {
          if (key != "HEADER") {
            document.getElementById(key).value = myObj[key];
             console.log(key + ' : ' + myObj[key]);
           } 
           if (key == "FLOW_OFFSET") flowCalVal = myObj[key];
           if (key == "LEAK_CAL_VAL") leakCalVal = myObj[key];
        } catch (error) {
          console.log('Missing or incorrect calibration parameter(s)');
        }
      }    
    break;
    
    case FILE_LIST: 
      var fileList = '';
      for (key in myObj) {
        try {
          if (key != "HEADER") {
            // TODO: Need to rewrite this...
            // We are breaking a cardinal rule here and creating view (GUI) code within the controller. 
            // We could (should!) send out JSON to the browser and then create the file list HTML using Javascript on the client side
            fileList += '<div class="fileListRow"><span class="column left"><a href="/download' + key + '" download class="file-link" onclick="downloadFile("' + key + '")">' + key + '</a></span><span class="column middle"><span class="fileSizeTxt">' + myObj[key] + ' bytes</span></span><span class="column right"><button id="delete-button"  onclick="socketMessageSend(\'' + FILE_DELETE + ':' + key + '\')" class="button-sml">Del</button></span>';
          }
          // console.log(key + ' : ' + myObj[key]);
        } catch (error) {
          console.log(error);
          console.log('Missing or incorrect system status');
        }
        document.getElementById('file_list').innerHTML = fileList;
      } 
    break;


    case SYS_STATUS: 
      for (key in myObj) {
        try {
          if (key != "HEADER") {
            document.getElementById(key).innerHTML = myObj[key];
            //    console.log(key + ' : ' + myObj[key]);
          }
        } catch (error) {
          console.log('Missing or incorrect status parameter(s)');
        }
      } 
    break;
    
  }
  
}


/***********************************************************
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




/***********************************************************
* onLoad handler
***/
function onLoad(event) {

  initialiseWebSocket();
  initialiseButtons();
  const urlParams = new URLSearchParams(window.location.search);
  const modal = urlParams.get('modal');  
  
  switch (modal) {
  
    case "upload":
      document.getElementById("load-config-button").click();
      document.getElementById('fileModal').style.display='block';
    break;
    
    default:
      document.getElementById("load-dashboard-button").click();
    
  }
  console.log('Page Loaded');
  
}


/***********************************************************
* Initialise buttons
***/
function initialiseButtons() {
  
  document.getElementById('refresh-button').addEventListener('click', function(){socketMessageSend(GET_FLOW_DATA);});
  document.getElementById('load-config-button').addEventListener('click', function(){
    socketMessageSend(LOAD_CONFIG);
    socketMessageSend(GET_CAL);
  });
  document.getElementById('calibrate-button').addEventListener('click', function(){socketMessageSend(CALIBRATE);});
  document.getElementById('leak-cal-button').addEventListener('click', function(){socketMessageSend(LEAK_CAL);});
  document.getElementById('save-config-button').addEventListener('click', function(){socketMessageSend(SAVE_CONFIG);});
  document.getElementById('file-manager-button').addEventListener('click', function(){
    socketMessageSend(FILE_LIST);
    document.getElementById('fileModal').style.display='block';
  });
  document.getElementById('about-button').addEventListener('click', function(){
    socketMessageSend(SYS_STATUS);
    document.getElementById('aboutModal').style.display='block';
  });
  document.getElementById('on-button').addEventListener('click', function(){socketMessageSend(START_BENCH);});
  document.getElementById('off-button').addEventListener('click', function(){socketMessageSend(STOP_BENCH);});
  
}




/***********************************************************
* configFormArray
*
* Create Array object from Configuration form data
***/
function configFormArray () {
  const form = document.querySelectorAll('form')[1];
  //const form = document.querySelector('#config-form');
  const formData = new FormData(form);
  const assocArray = {};
  for (const [name, value] of formData) {
    assocArray[name] = value;
  }
  //return JSON.stringify(assocArray, null, 2);
  return assocArray; 
}

/***********************************************************
* socketMessageSend
* 
* Construct message: header + content & send to ESP32
***/
function socketMessageSend(message){
  
  console.log('Web Socket Message: ' + message);
  
  var jsonMessage;
  var configArray;
  var header;
  var action;
  var actionData;
  
  // check if data has been passed to the function with the action message
  var searchResult = String(message).search(":");
  if (searchResult != -1) {
      header = message.split(":");
      action = Number(header[0]);
      actionData = header[1];
  } else {
      action = message;
  }
  
  switch (action) {
    
    case GET_FLOW_DATA: // HEADER:1
      jsonMessage = '{\"HEADER\":\"' + GET_FLOW_DATA + '\"}';
    break;

    case REC_FLOW_DATA: // HEADER:2
      jsonMessage = '{\"HEADER\":\"' + REC_FLOW_DATA + '\"}';
    break;

    case CALIBRATE: // HEADER:3
      jsonMessage ='{\"HEADER\":\"' + CALIBRATE + '\"}';
    break;
    
    case FILE_LIST: // HEADER:4
      jsonMessage ='{\"HEADER\":\"' + FILE_LIST + '\"}';      
    break;
    
    case SYS_STATUS: // HEADER:5
      jsonMessage ='{\"HEADER\":\"' + SYS_STATUS + '\"}';      
    break;
  
    case SAVE_CONFIG:// HEADER:6
      configArray = configFormArray();
      configArray['HEADER'] = SAVE_CONFIG;
      jsonMessage = JSON.stringify(configArray, null, 2);
    break;
    
    case LOAD_CONFIG: // HEADER:7
      jsonMessage = '{\"HEADER\":\"' + LOAD_CONFIG + '\"}';
    break;
    
    case FILE_DOWNLOAD: // HEADER:8
      // NOTE: File sent by HTTP FORM POST Data and handled by server.on(...) request handler in Webserver::Initialise
    break;
    
    case FILE_DELETE: // HEADER:9
      jsonMessage ='{\"HEADER\":\"' + FILE_DELETE + '\",\"FILENAME\":\"' + actionData + '\"}';

    break;
    
    case FILE_UPLOAD: // HEADER:10
      // NOTE: Handled by onFileUpload(event) and sent by HTTP FORM POST Data to server.on(...) request handler in Webserver::Initialise
    break;
    
    case START_BENCH: // HEADER:11
      jsonMessage ='{\"HEADER\":\"' + START_BENCH + '\"}';
    break;
    
    case STOP_BENCH: // HEADER:12
      jsonMessage ='{\"HEADER\":\"' + STOP_BENCH + '\"}';
    break;
    
    case LEAK_CAL: // HEADER:13
      jsonMessage ='{\"HEADER\":\"' + LEAK_CAL + '\"}';
    break;

    case GET_CAL: // HEADER:14
      jsonMessage ='{\"HEADER\":\"' + GET_CAL + '\"}';
    break;


  }
  console.log(jsonMessage);
  websocket.send(jsonMessage);

}


/***********************************************************
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


/***********************************************************
* Close modal file dialog
***/
spanCloseFileModal.onclick = function() {
  
  fileModal.style.display = "none";
  
}

/***********************************************************
* Close modal about dialog
***/
spanCloseAboutModal.onclick = function() {
  
  aboutModal.style.display = "none";
  
}


/***********************************************************
* Close modal dialogs (lose focus)
***/
window.onclick = function(event) {
  
  if (event.target == fileModal || event.target == aboutModal ) {
    fileModal.style.display = "none";
    aboutModal.style.display = "none";
  }
  
}

/***********************************************************
* Close modal dialog on esc button
***/
document.addEventListener("keydown", ({key}) => {
  if (key === "Escape") {
    fileModal.style.display = "none";
    aboutModal.style.display = "none";
  }
})
