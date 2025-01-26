/***********************************************************
* javascript.js
*
* Browser control code for DIY-Flow-bench project: diyflowbench.com
* Handles data transmission and display update tasks
* Provides UI control to browser
*
***/

var updateSSE = true;
var activeOrifice;
var runOnce = false;

window.addEventListener('load', onLoad);

var fileModal = document.getElementById("fileModal");
var infoModal = document.getElementById("infoModal");
var captureLiftDataModal = document.getElementById("captureLiftDataModal");
var loadGraphDataModal = document.getElementById("loadGraphDataModal");
var saveGraphDataModal = document.getElementById("saveGraphDataModal");
var calibrationModal = document.getElementById("calibrationModal");
var dataModal = document.getElementById("dataModal");
var serialModal = document.getElementById("serialModal");
var updateModal = document.getElementById("updateModal");
var flowTargetModal = document.getElementById("flowTargetModal");


var closeFlowTargetModalButton = document.getElementsByClassName("closeFlowTargetModalButton")[0];
var closeCalibrationModalButton = document.getElementsByClassName("closeCalibrationModalButton")[0];
var closeSerialModalButton = document.getElementsByClassName("closeSerialModalButton")[0];
var closeCaptureLiftDataModalButton = document.getElementsByClassName("closeCaptureLiftDataModalButton")[0];
var closeLoadGraphDataModalButton = document.getElementsByClassName("closeLoadGraphDataModalButton")[0];
var closeSaveGraphDataModalButton = document.getElementsByClassName("closeSaveGraphDataModalButton")[0];
var closeCalibrationModalButton = document.getElementsByClassName("closeCalibrationModalButton")[0];

var bSWIRL_ENBLD;
var iPDIFF_SENS_TYP;

// Set up Server Side Events (SSE)
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('JSON_DATA', function(e) {
    var myObj = JSON.parse(e.data);

    var FLOW_DECIMAL_ACCURACY = myObj["FLOW_DECIMAL_ACCURACY"];
    var GEN_DECIMAL_ACCURACY = myObj["GEN_DECIMAL_ACCURACY"];

    if (updateSSE === true){

      for (key in myObj) {
        try {

          if (key === 'FLOW_DECIMAL_ACCURACY' || key === 'GEN_DECIMAL_ACCURACY' ) {
            continue;
          } 


          if (typeof myObj[key] === 'string' || myObj[key] instanceof String) {
          // We've got a string...
            if (key === 'PITOT_COLOUR' || key === 'PDIFF_COLOUR' ) {
            } else {
              // it' a template variable
              document.getElementById(key).innerHTML = myObj[key];
            }
 
          } else {
            // we've not got a string... 
            
           if (key === 'FLOW' || key === 'AFLOW' || key === 'MFLOW' || key === 'SFLOW' || key === 'FDIFF') {
              document.getElementById(key).innerHTML = myObj[key].toFixed(FLOW_DECIMAL_ACCURACY);  
              
            } else if (key === 'PREF' || key === 'PDIFF' || key === 'PITOT' || key === 'PITOT_DELTA' || key === 'SWIRL' || key === 'TEMP' || key === 'BARO' || key === 'RELH') {
              document.getElementById(key).innerHTML = myObj[key].toFixed(GEN_DECIMAL_ACCURACY); 
            //} else if (key === '') {
            } else {
              document.getElementById(key).innerHTML = myObj[key];
            }
          }
        } catch (error) {
          console.log('Missing or incorrect JSON data');
          console.log(" error: " + error + " key: " + key );
          // console.log(key);
        }
      } 

      // get bench type and set up GUI accoordingly
      var benchType = myObj["iBENCH_TYPE"];

      // get active orifice and set up GUI accoordingly
      activeOrifice = myObj["ACTIVE_ORIFICE"];
      if (runOnce == false ){
        radioButton = document.getElementById("orifice" + activeOrifice);
        radioButton.checked = true;
      }

      switch (benchType) {
    
        case "MAF":
          document.getElementById('orificeRadio').style.display='none';
        break;
    
        case "ORIFICE":
          document.getElementById('orificeRadio').style.display='block';
        break;
          
        case "VENTURI":
          document.getElementById('orificeData').style.display='block';
        break;
          
        case "PITOT":
          document.getElementById('orificeData').style.display='block';
        break;
          
      }

      // Get swirl status
      bSWIRL_ENBLD = myObj["bSWIRL_ENBLD"]; 

      iPDIFF_SENS_TYP = myObj["iPDIFF_SENS_TYP"];

      // Get data filter type
      var dataFilterType = myObj["iDATA_FLTR_TYP"];

      // Get Pitot Tile status colours
      var pitotTileColour = myObj["PITOT_COLOUR"];
      document.getElementById('PITOT').style.color=pitotTileColour;

      // Get pDiff tile status colours
      var pDiffTileColour = myObj["PDIFF_COLOUR"];
      document.getElementById('PDIFF').style.color=pDiffTileColour;
    }

  }, false);



}

function setCookie(name,value,days) {
  var expires = "";
  if (days) {
      var date = new Date();
      date.setTime(date.getTime() + (days*24*60*60*1000));
      expires = "; expires=" + date.toUTCString();
  }
  document.cookie = name + "=" + (value || "")  + expires + "; path=/";
}

function getCookie(name) {
  var nameEQ = name + "=";
  var ca = document.cookie.split(';');
  for(var i=0;i < ca.length;i++) {
      var c = ca[i];
      while (c.charAt(0)==' ') c = c.substring(1,c.length);
      if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
  }
  return null;
}

// https://github.com/Autodrop3d/serialTerminal.com
function changeCookieValue(cookieName, newValue) {
  document.cookie = 
      `${cookieName}=${newValue}; 
          expires=Thu, 5 March 2030 12:00:00 UTC; path=/`;
}


/***********************************************************
* onLoad event handler
***/
function onLoad(event) {

    initialiseButtons();
   
    // Set tile status on page reload from cookie data
    var tileStatus = getCookie('pressure-tile');
    
    switch (tileStatus) {
  
      case "pdiff":
        document.getElementById('tile-pref').style.display='none';
        document.getElementById('tile-pdiff').style.display='block';
      break;
  
      case "pref":
        document.getElementById('tile-pref').style.display='block';
        document.getElementById('tile-pdiff').style.display='none';
      break;
  
    }
  
    var tileStatus = getCookie('flow-tile');
  
    switch (tileStatus){
  
      case "flow":
        document.getElementById('flow-tile').style.display='block';
        document.getElementById('aflow-tile').style.display='none';
        document.getElementById('sflow-tile').style.display='none';
        document.getElementById('maf-tile').style.display='none';
      break;
  
      case "aflow":
        document.getElementById('flow-tile').style.display='none';
        document.getElementById('aflow-tile').style.display='block';
        document.getElementById('sflow-tile').style.display='none';
        document.getElementById('maf-tile').style.display='none';
      break;
  
      case "sflow":
        document.getElementById('flow-tile').style.display='none';
        document.getElementById('aflow-tile').style.display='none';
        document.getElementById('sflow-tile').style.display='block';
        document.getElementById('maf-tile').style.display='none';
      break;
  
      case "maf":
        document.getElementById('flow-tile').style.display='none';
        document.getElementById('aflow-tile').style.display='none';
        document.getElementById('sflow-tile').style.display='none';
        document.getElementById('maf-tile').style.display='block';
      break;
  
    }
  
    var tileStatus = getCookie('tool-tile');
  
    switch (tileStatus){
  
      case "pitot":
        document.getElementById('tile-pitot').style.display='block';
        document.getElementById('tile-swirl').style.display='none';
        document.getElementById('tile-fdiff').style.display='none';
      break;
  
      case "swirl":
        document.getElementById('tile-pitot').style.display='none';
        document.getElementById('tile-swirl').style.display='block';
        document.getElementById('tile-fdiff').style.display='none';
      break;
  
      case "fdiff":
        document.getElementById('tile-pitot').style.display='none';
        document.getElementById('tile-swirl').style.display='none';
        document.getElementById('tile-fdiff').style.display='block';
      break;
    
    }
  
    console.log('Page Loaded');
    
  }
  
  





/***********************************************************
* Initialise buttons
***/
function initialiseButtons() {
  
    var xhr = new XMLHttpRequest();
  
    document.getElementById('show-capture-modal-button').addEventListener('click', function(){
      document.getElementById('captureLiftDataModal').style.display='block';
    });
  
    document.getElementById('FDIFF').addEventListener('click', function(){
      document.getElementById('flowTargetModal').style.display='block';
    });
  
  
    document.getElementById('capture-lift-data-button').addEventListener('click', function(){
      console.log('Capture Lift Data');
      let formData = new FormData(document.forms.lift_data_form);
      xhr.open('POST', '/api/saveliftdata');
      xhr.send(formData)
      xhr.onload = function() {
        if (xhr.status === 200) {
          console.log('Lift data saved');
          // update datagraph
        }
  
      };
    });
    
  
    document.getElementById('STATUS_MESSAGE').addEventListener('dblclick', function(){
      document.getElementById('calibrationModal').style.display='block';
    });

  
    // Pressure tile
    document.getElementById('tile-pref-title').addEventListener('click', function(){
      if (iPDIFF_SENS_TYP > 1){
        document.getElementById('tile-pref').style.display='none';
        document.getElementById('tile-pdiff').style.display='block';
        setCookie("pressure-tile","pdiff","365")      
      }
    });
  
    document.getElementById('tile-pdiff-title').addEventListener('click', function(){
      document.getElementById('tile-pdiff').style.display='none';
      document.getElementById('tile-pref').style.display='block';
      setCookie("pressure-tile","pref","365")
    });
  
    // flow tile
    document.getElementById('flow-tile-title').addEventListener('click', function(){
      document.getElementById('flow-tile').style.display='none';
      document.getElementById('aflow-tile').style.display='block';
      xhr.open('GET', '/api/flow/aflow');
      xhr.send();
      setCookie("flow-tile","aflow","365")    
    });
  
    document.getElementById('aflow-tile-title').addEventListener('click', function(){
      document.getElementById('aflow-tile').style.display='none';
      document.getElementById('sflow-tile').style.display='block';
      xhr.open('GET', '/api/flow/sflow');
      xhr.send();
      setCookie("flow-tile","sflow","365")    
    });
  
    document.getElementById('sflow-tile-title').addEventListener('click', function(){
      document.getElementById('sflow-tile').style.display='none';
      document.getElementById('maf-tile').style.display='block';
      xhr.open('GET', '/api/flow/mafflow');
      xhr.send();
      setCookie("flow-tile","maf","365")    
    });
  
    document.getElementById('maf-tile-title').addEventListener('click', function(){
      document.getElementById('maf-tile').style.display='none';
      document.getElementById('flow-tile').style.display='block';
      xhr.open('GET', '/api/flow/flow');
      xhr.send();
      setCookie("flow-tile","flow","365")    
    });
  
    // Tool tile
    document.getElementById('tile-pitot-title').addEventListener('click', function(){
      if (bSWIRL_ENBLD == true) {
        document.getElementById('tile-pitot').style.display='none';
        document.getElementById('tile-swirl').style.display='block';
        setCookie("tool-tile","swirl","365")      
      } else {
        document.getElementById('tile-pitot').style.display='none';
        document.getElementById('tile-fdiff').style.display='block';
        setCookie("tool-tile","fdiff","365")      
      }
    });
  
    document.getElementById('tile-swirl-title').addEventListener('click', function(){
      document.getElementById('tile-swirl').style.display='none';
      document.getElementById('tile-fdiff').style.display='block';
      setCookie("tool-tile","fdiff","365")    
    });
  
    document.getElementById('tile-fdiff-title').addEventListener('click', function(){
      document.getElementById('tile-fdiff').style.display='none';
      document.getElementById('tile-pitot').style.display='block';
      setCookie("tool-tile","pitot","365")    
    });
  
  
    document.getElementById('FDIFFTYPEDESC').addEventListener('click', function(){
      console.log('Toggle Flow Diff');
      xhr.open('GET', '/api/fdiff/toggle');
      // xhr.onload = function() {
      //   if (xhr.status === 200) window.location.href = '/';
      // };
      xhr.send();
    });
  
    document.getElementById('PDIFF').addEventListener('click', function(){
      console.log('Zero pDiff Value');
      xhr.open('GET', '/api/pdiff/zero');
      // xhr.onload = function() {
      //   if (xhr.status === 200) window.location.href = '/';
      // };
      xhr.send();
    });
  
    
    document.getElementById('PITOT').addEventListener('click', function(){
      console.log('Zero Pitot Value');
      xhr.open('GET', '/api/pitot/zero');
      // xhr.onload = function() {
      //   if (xhr.status === 200) window.location.href = '/';
      // };
      xhr.send();
    });
  
  
  
    document.getElementById('on-button').addEventListener('click', function(){
      console.log('Bench On');
      xhr.open('GET', '/api/bench/on');
      // xhr.onload = function() {
      //   if (xhr.status === 200) window.location.href = '/';
      // };
      xhr.send();
    });
  
    document.getElementById('off-button').addEventListener('click', function(){
      console.log('Bench Off');
      xhr.open('GET', '/api/bench/off');
      // xhr.onload = function() {
      //   if (xhr.status === 200) window.location.href = '/';
      // };
      xhr.send();
    });
  
    document.getElementById('calibrate-button').addEventListener('click', function(){
      console.log('Calibrate FLow Offset');
      xhr.open('GET', '/api/bench/calibrate');
      xhr.onload = function() {
        if (xhr.status === 200) window.location.href = '/';
      };
      xhr.send();
    });
  
    document.getElementById('leak-cal-button').addEventListener('click', function(){
      console.log('Leak Test Calibration');
      xhr.open('GET', '/api/bench/leakcal');
      xhr.onload = function() {
        if (xhr.status === 200) window.location.href = '/';
      };
      xhr.send();
    });
  
    document.getElementById('clear-message-button').addEventListener('click', function(){
      console.log('Clear Message');
      xhr.open('GET', '/api/clear-message');
      xhr.onload = function() {
        if (xhr.status === 200) window.location.href = '/';
      };
      xhr.send();
    });
  
  }
  



  /***********************************************************
* Update selected orifice when orifice-radio button change
***/
function orificeChange(src) {
  
    var xhr = new XMLHttpRequest();
  
    console.log('Orifice ' + src.value + ' Selected');
  
    xhr.open('GET', '/api/orifice-change?orifice=' + src.value);
    xhr.onload = function() {
  //    if (xhr.status === 200) window.location.href = '/';
    };
    xhr.send();
    }
  

/***********************************************************
* Close modal dialogs on lose focus
***/
window.onclick = function(event) {
    if (event.target == captureLiftDataModal || event.target == calibrationModal || event.target == flowTargetModal  ){
      captureLiftDataModal.style.display = "none";
      calibrationModal.style.display = "none";
      flowTargetModal.style.display = "none";
    }
  }
  
  
  /***********************************************************
  * Close modal dialogs on esc button
  ***/
  document.addEventListener("keydown", ({key}) => {
    if (key === "Escape") {
      captureLiftDataModal.style.display = "none";
      calibrationModal.style.display = "none";
      flowTargetModal.style.display = "none";
    }
  })
  
    /***********************************************************
  * Close Flow Target Data modal dialog
  ***/
  closeFlowTargetModalButton.onclick = function() {
    flowTargetModal.style.display = "none";
  }  

  /***********************************************************
  * Close calibration modal dialog
  ***/
  closeCalibrationModalButton.onclick = function() {
    calibrationModal.style.display = "none";
  }
  
  
  
  /***********************************************************
  * Close Capture Data modal dialog
  ***/
  closeCaptureLiftDataModalButton.onclick = function() {
    captureLiftDataModal.style.display = "none";
  }