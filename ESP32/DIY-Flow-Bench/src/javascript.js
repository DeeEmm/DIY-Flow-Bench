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

window.addEventListener('load', onLoad);

var fileModal = document.getElementById("fileModal");
var infoModal = document.getElementById("infoModal");
var captureLiftDataModal = document.getElementById("captureLiftDataModal");
var loadGraphDataModal = document.getElementById("loadGraphDataModal");
var saveGraphDataModal = document.getElementById("saveGraphDataModal");

var closeFileModalButton = document.getElementsByClassName("closeFileModalButton")[0];
var closeInfoModalButton = document.getElementsByClassName("closeInfoModalButton")[0];
var closeCaptureLiftDataModalButton = document.getElementsByClassName("closeCaptureLiftDataModalButton")[0];
var closeLoadGraphDataModalButton = document.getElementsByClassName("closeLoadGraphDataModalButton")[0];
var closeSaveGraphDataModalButton = document.getElementsByClassName("closeSaveGraphDataModalButton")[0];

// Set up Server Side Events (SSE)
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('JSON_DATA', function(e) {
    var myObj = JSON.parse(e.data);

    if (updateSSE === true){

      for (key in myObj) {
        try {
          if (typeof myObj[key] === 'string' || myObj[key] instanceof String) {
            document.getElementById(key).innerHTML = myObj[key];
          } else {
            document.getElementById(key).innerHTML = myObj[key].toFixed(2); 
          }
        } catch (error) {
          console.log('Missing or incorrect data');
          console.log(key + ' : ' + myObj[key]);
        }
      } 

      // get bench type and set up GUI accoordingly
      var benchType = myObj["BENCH_TYPE"];

      switch (benchType) {
    
        case "MAF":
          document.getElementById('orificeData').style.display='none';
        break;
    
        case "ORIFICE":
          document.getElementById('orificeData').style.display='block';
        break;
          
        case "VENTURI":
          document.getElementById('orificeData').style.display='block';
        break;
          
        case "PITOT":
          document.getElementById('orificeData').style.display='block';
        break;
          
      }

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


  document.getElementById('file-manager-button').addEventListener('click', function(){
    document.getElementById('fileModal').style.display='block';
  });

  document.getElementById('info-button').addEventListener('click', function(){
    document.getElementById('infoModal').style.display='block';
  });

  document.getElementById('tile-pref').addEventListener('click', function(){
    document.getElementById('tile-pref').style.display='none';
    document.getElementById('tile-pdiff').style.display='block';
  });

  document.getElementById('tile-pdiff').addEventListener('click', function(){
    document.getElementById('tile-pdiff').style.display='none';
    document.getElementById('tile-pref').style.display='block';
  });

  document.getElementById('flow-tile').addEventListener('click', function(){
    document.getElementById('flow-tile').style.display='none';
    document.getElementById('aflow-tile').style.display='block';
  });

  document.getElementById('aflow-tile').addEventListener('click', function(){
    document.getElementById('aflow-tile').style.display='none';
    document.getElementById('flow-tile').style.display='block';
  });

  document.getElementById('tile-pitot').addEventListener('click', function(){
    document.getElementById('tile-pitot').style.display='none';
    document.getElementById('tile-swirl').style.display='block';
  });

  document.getElementById('tile-swirl').addEventListener('click', function(){
    document.getElementById('tile-swirl').style.display='none';
    document.getElementById('tile-pitot').style.display='block';
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
* Close modal dialogs on lose focus
***/
window.onclick = function(event) {
  if (event.target == fileModal || event.target == infoModal || event.target == captureLiftDataModal || event.target == loadGraphDataModal || event.target == saveGraphDataModal ) {
    fileModal.style.display = "none";
    infoModal.style.display = "none";
    captureLiftDataModal.style.display = "none";
    loadGraphDataModal.style.display = "none";
    saveGraphDataModal.style.display = "none";
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

  // load the SVG blob to a flesh image object
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
    a.href = canvas.toDataURL();
    a.click();
    a.remove();

  });
  img.src = url;

}
