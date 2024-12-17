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
var closeDataModalButton = document.getElementsByClassName("closeDataModalButton")[0];
var closeSerialModalButton = document.getElementsByClassName("closeSerialModalButton")[0];
var closeFileModalButton = document.getElementsByClassName("closeFileModalButton")[0];
var closeInfoModalButton = document.getElementsByClassName("closeInfoModalButton")[0];
var closeCaptureLiftDataModalButton = document.getElementsByClassName("closeCaptureLiftDataModalButton")[0];
var closeLoadGraphDataModalButton = document.getElementsByClassName("closeLoadGraphDataModalButton")[0];
var closeSaveGraphDataModalButton = document.getElementsByClassName("closeSaveGraphDataModalButton")[0];
var closeCalibrationModalButton = document.getElementsByClassName("closeCalibrationModalButton")[0];
var closeUpdateModalButton = document.getElementsByClassName("closeUpdateModalButton")[0];

// Set up Server Side Events (SSE)
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('JSON_DATA', function(e) {
    var myObj = JSON.parse(e.data);

    if (updateSSE === true){

      for (key in myObj) {
        try {
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
              // HACK: template vars - replaced before page load
              document.getElementById(key).innerHTML = myObj[key].toFixed(~FLOW_DECI_ACC~);  
            } else if (key === 'PREF' || key === 'PDIFF' || key === 'PITOT' || key === 'PITOT_DELTA' || key === 'SWIRL' || key === 'TEMP' || key === 'BARO' || key === 'RELH') {
              document.getElementById(key).innerHTML = myObj[key].toFixed(~GEN_DECI_ACC~); 
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
      var benchType = myObj["BENCH_TYPE"];

      // get active orifice and set up GUI accoordingly
      activeOrifice = myObj["ACTIVE_ORIFICE"];
      if (runOnce == false ){
        radioButton = document.getElementById("orifice" + activeOrifice);
        radioButton.checked = true;
      }

      switch (benchType) {
    
        case "MAF":
          document.getElementById('orificeData').style.display='none';
          document.getElementById('orificeRadio').style.display='none';
        break;
    
        case "ORIFICE":
          document.getElementById('orificeData').style.display='block';
          document.getElementById('orificeRadio').style.display='block';
        break;
          
        case "VENTURI":
          document.getElementById('orificeData').style.display='block';
        break;
          
        case "PITOT":
          document.getElementById('orificeData').style.display='block';
        break;
          
      }

      // Get data filter type
      var dataFilterType = myObj["DATA_FILTER_TYP"];

      // Get Pitot Tile status colours
      var pitotTileColour = myObj["PITOT_COLOUR"];
      document.getElementById('PITOT').style.color=pitotTileColour;

      // Get pDiff tile tatus colours
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
