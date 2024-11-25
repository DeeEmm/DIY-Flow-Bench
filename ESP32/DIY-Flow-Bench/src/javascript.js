/***********************************************************
* javascript.js
*
* Browser control code for DIY-Flow-bench project: diyflowbench.com
* Handles data transmission and display update tasks
* Provides UI control to browser
*
***/

var leakCalVal;
var flowCalVal;
var leakCalTolerance;
var leakTestThreshold;
var updateSSE = true;
var active_orifice;
var runOnce = false;

window.addEventListener('load', onLoad);

var fileModal = document.getElementById("fileModal");
var infoModal = document.getElementById("infoModal");
var captureLiftDataModal = document.getElementById("captureLiftDataModal");
var loadGraphDataModal = document.getElementById("loadGraphDataModal");
var saveGraphDataModal = document.getElementById("saveGraphDataModal");
var calibrationModal = document.getElementById("calibrationModal");
var updateModal = document.getElementById("updateModal");
var flowTargetModal = document.getElementById("flowTargetModal");


var closeFlowTargetModalButton = document.getElementsByClassName("closeFlowTargetModalButton")[0];
var closeCalibrationModalButton = document.getElementsByClassName("closeCalibrationModalButton")[0];
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
            if (key === 'PITOT_COLOUR' || key === 'PDIFF_COLOUR' ){
            } else {
              document.getElementById(key).innerHTML = myObj[key];
            }
 
          } else {
           // we've not got a string... 
           if (key === 'FLOW' || key === 'AFLOW' || key === 'MFLOW' || key === 'SFLOW' || key === 'FDIFF') {
              // HACK: template vars - replaced before page load
              document.getElementById(key).innerHTML = myObj[key].toFixed(~FLOW_DECIMAL_LENGTH~);  
            } else if (key === 'PREF' || key === 'PDIFF' || key === 'PITOT' || key === 'SWIRL' || key === 'TEMP' || key === 'BARO' || key === 'RELH') {
              document.getElementById(key).innerHTML = myObj[key].toFixed(~GEN_DECIMAL_LENGTH~); 
            //} else if (key === '') {
            } else {
              document.getElementById(key).innerHTML = myObj[key];
            }
          }
        } catch (error) {
          console.log('Missing or incorrect JSON data');
          console.log(error);
          console.log(key);
        }
      } 

      // get bench type and set up GUI accoordingly
      var benchType = myObj["BENCH_TYPE"];

      // get active orifice and set up GUI accoordingly
      active_orifice = myObj["ACTIVE_ORIFICE"];
      if (runOnce == false ){
        radioButton = document.getElementById("orifice" + active_orifice);
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
      var dataFilterType = myObj["DATA_FILTER_TYPE"];

      // Get Pitot Tile status colours
      var pitotTileColour = myObj["PITOT_COLOUR"];
      document.getElementById('PITOT').style.color=pitotTileColour;

      // Get pDiff tile tatus colours
      var pDiffTileColour = myObj["PDIFF_COLOUR"];
      document.getElementById('PDIFF').style.color=pDiffTileColour;
    }

  }, false);



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
* onLoad event handler
***/
function onLoad(event) {

  initialiseButtons();
  const urlParams = new URLSearchParams(window.location.search);
  const view = urlParams.get('view');  
  
  switch (view) {
  
    case "upload":
      document.getElementById("load-config-button").click();
      document.getElementById('fileModal').style.display='block';
    break;

    case "graph":
      document.getElementById("load-datalog-button").click();
      document.getElementById('datalog').style.display='block';
    break;
      
    case "config":
      document.getElementById("load-config-button").click();
    break;
      
    
  }

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

  document.getElementById('clear-graph-data-button').addEventListener('click', function(){
        
    // clear data points from graph
    // var p = document.getElementById('dataPlot');
    // var child = p.lastElementChild; 
    // while (child) {
    //     p.removeChild(child);
    //     child = p.lastElementChild;
    // }
    // clear line data from graph
    var l = document.getElementById('lineData');
    var child = l.lastElementChild; 
    while (child) {
        l.removeChild(child);
        child = l.lastElementChild;
    }

    xhr.open('POST', '/api/clearLiftData');
    xhr.onload = function() {
      if (xhr.status === 200) window.location.href = '/?view=graph';
    };
    xhr.send();

  });
  
  document.getElementById('export-graph-data-button').addEventListener('click', function(){
    // initiate JSON Data download from browser
    document.getElementById('file-data-download').click();
  });

  document.getElementById('capture-graph-data-button').addEventListener('click', function(){
    // initiate datagraph image download from browser
    exportSVGAsPNG();
  });

  document.getElementById('STATUS_MESSAGE').addEventListener('dblclick', function(){
    document.getElementById('calibrationModal').style.display='block';
  });

  document.getElementById('file-manager-button').addEventListener('click', function(){
    document.getElementById('fileModal').style.display='block';
  });

  document.getElementById('info-button').addEventListener('click', function(){
    document.getElementById('infoModal').style.display='block';
  });

  document.getElementById('update-button').addEventListener('click', function(){
    document.getElementById('updateModal').style.display='block';
  });

  // Pressure tile
  document.getElementById('tile-pref-title').addEventListener('click', function(){
    document.getElementById('tile-pref').style.display='none';
    document.getElementById('tile-pdiff').style.display='block';
    setCookie("pressure-tile","pdiff","365")    
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
    setCookie("flow-tile","aflow","365")    
  });

  document.getElementById('aflow-tile-title').addEventListener('click', function(){
    document.getElementById('aflow-tile').style.display='none';
    document.getElementById('sflow-tile').style.display='block';
    setCookie("flow-tile","sflow","365")    
  });

  document.getElementById('sflow-tile-title').addEventListener('click', function(){
    document.getElementById('sflow-tile').style.display='none';
    document.getElementById('maf-tile').style.display='block';
    setCookie("flow-tile","maf","365")    
  });

  document.getElementById('maf-tile-title').addEventListener('click', function(){
    document.getElementById('maf-tile').style.display='none';
    document.getElementById('flow-tile').style.display='block';
    setCookie("flow-tile","flow","365")    
  });

  // Tool tile
  document.getElementById('tile-pitot-title').addEventListener('click', function(){
    document.getElementById('tile-pitot').style.display='none';
    document.getElementById('tile-swirl').style.display='block';
    setCookie("tool-tile","swirl","365")    
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
    xhr.onload = function() {
      if (xhr.status === 200) window.location.href = '/';
    };
    xhr.send();
  });

  document.getElementById('off-button').addEventListener('click', function(){
    console.log('Bench Off');
    xhr.open('GET', '/api/bench/off');
    xhr.onload = function() {
      if (xhr.status === 200) window.location.href = '/';
    };
    xhr.send();
  });

  document.getElementById('calibrate-button').addEventListener('click', function(){
    console.log('Calibrate FLow Offset');
    xhr.open('GET', '/api/bench/calibrate');
    xhr.onload = function() {
      if (xhr.status === 200) window.location.href = '/?view=config';
    };
    xhr.send();
  });

  document.getElementById('leak-cal-button').addEventListener('click', function(){
    console.log('Leak Test Calibration');
    xhr.open('GET', '/api/bench/leakcal');
    xhr.onload = function() {
      if (xhr.status === 200) window.location.href = '/?view=config';
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

  document.getElementById('restart-button').addEventListener('click', function(){
    console.log('System Reboot');
    xhr.open('GET', '/api/bench/reboot');
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
* Page tab control
***/
function openTab(tabName, elmnt) {
  var i, tabcontent, tablinks;
  tabcontent = document.getElementsByClassName("tabcontent");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }
  document.getElementById(tabName).style.display = "block";
  // Pause SSE update when on data tab
  if (tabName === "datalog") {
    updateSSE = false;
  } else {
    updateSSE = true;
  }
}

/***********************************************************
* Close update modal dialog
***/
closeUpdateModalButton.onclick = function() {
  updateModal.style.display = "none";
}

/***********************************************************
* Close calibration modal dialog
***/
closeCalibrationModalButton.onclick = function() {
  calibrationModal.style.display = "none";
}

/***********************************************************
* Close file modal dialog
***/
closeFileModalButton.onclick = function() {
  fileModal.style.display = "none";
}

/***********************************************************
* Close info modal dialog
***/
closeInfoModalButton.onclick = function() {
  infoModal.style.display = "none";
}


/***********************************************************
* Close Capture Data modal dialog
***/
closeCaptureLiftDataModalButton.onclick = function() {
  captureLiftDataModal.style.display = "none";
}


/***********************************************************
* Close Load Data modal dialog
***/
closeLoadGraphDataModalButton.onclick = function() {
  loadGraphDataModal.style.display = "none";
}


/***********************************************************
* Close Save Data modal dialog
***/
closeSaveGraphDataModalButton.onclick = function() {
  saveGraphDataModal.style.display = "none";
}

/***********************************************************
* Close Flow Target Data modal dialog
***/
closeFlowTargetModalButton.onclick = function() {
  flowTargetModal.style.display = "none";
}



/***********************************************************
* Close modal dialogs on lose focus
***/
window.onclick = function(event) {
  if (event.target == fileModal || event.target == infoModal || event.target == captureLiftDataModal || event.target == loadGraphDataModal || event.target == saveGraphDataModal || event.target == calibrationModal || event.target == updateModal || event.target == flowTargetModal  ){
    fileModal.style.display = "none";
    infoModal.style.display = "none";
    captureLiftDataModal.style.display = "none";
    loadGraphDataModal.style.display = "none";
    saveGraphDataModal.style.display = "none";
    calibrationModal.style.display = "none";
    updateModal.style.display = "none";
    flowTargetModal.style.display = "none";
  }
}


/***********************************************************
* Close modal dialogs on esc button
***/
document.addEventListener("keydown", ({key}) => {
  if (key === "Escape") {
    fileModal.style.display = "none";
    infoModal.style.display = "none";
    captureLiftDataModal.style.display = "none";
    loadGraphDataModal.style.display = "none";
    saveGraphDataModal.style.display = "none";
    calibrationModal.style.display = "none";
    updateModal.style.display = "none";
    flowTargetModal.style.display = "none";
  }
})



/***********************************************************
* Export Data Graph as PNG Image
* Source: https://takuti.me/note/javascript-save-svg-as-image/
***/
function exportSVGAsPNG() {

  const svg = document.querySelector('svg');

  const data = (new XMLSerializer()).serializeToString(svg);
  const svgBlob = new Blob([data], {
      type: 'image/svg+xml;charset=utf-8'
  });

  // convert the blob object to a dedicated URL
  const url = URL.createObjectURL(svgBlob);

  // load the SVG blob to a fresh image object
  const img = new Image();
  img.addEventListener('load', () => {
    
    // draw the image on an ad-hoc canvas
    const bbox = svg.getBBox();

    const canvas = document.createElement('canvas');
    canvas.width = 800;
    canvas.height = 750;

    const context = canvas.getContext('2d');
    context.drawImage(img, 0, 0, canvas.width, canvas.height);

    URL.revokeObjectURL(url);

    // trigger a synthetic download operation with a temporary link
    const a = document.createElement('a');
    a.download = 'LiftGraph.png';
    document.body.appendChild(a);
    a.href = canvas.toDataURL('image/jpeg');
    a.click();
    a.remove();

  });
  img.src = url;

}


// three cookies to remamber state of top three tiles - pressure / flow / tools

// http://www.quirksmode.org/js/cookies.html
// setCookie('ppkcookie','testcookie',7);

// var x = getCookie('ppkcookie');
// if (x) {
//     [do something with x]
// }
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

// https://www.geeksforgeeks.org/javascript-cookies/
function changeCookieValue(cookieName, newValue) {
  document.cookie = 
      `${cookieName}=${newValue}; 
          expires=Thu, 5 March 2030 12:00:00 UTC; path=/`;
}


