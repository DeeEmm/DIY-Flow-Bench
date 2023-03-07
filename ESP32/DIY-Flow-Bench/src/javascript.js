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
* TODO: Pretty graphical data representation
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
var saveDataModal = document.getElementById("saveDataModal");
var viewDataModal = document.getElementById("viewDataModal");
var closeFileModalButton = document.getElementsByClassName("closeFileModalButton")[0];
var closeInfoModalButton = document.getElementsByClassName("closeInfoModalButton")[0];
var closeSaveDataModalButton = document.getElementsByClassName("closeSaveDataModalButton")[0];
var closeViewDataModalButton = document.getElementsByClassName("closeViewDataModalButton")[0];

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

  document.getElementById('saveData-button').addEventListener('click', function(){
    document.getElementById('saveDataModal').style.display='block';
  });

  document.getElementById('viewData-button').addEventListener('click', function(){
    document.getElementById('viewDataModal').style.display='block';
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
* Close Save Data modal dialog
***/
closeSaveDataModalButton.onclick = function() {
  saveDataModal.style.display = "none";
}


/***********************************************************
* Close View Data modal dialog
***/
closeViewDataModalButton.onclick = function() {
  viewDataModal.style.display = "none";
}


/***********************************************************
* Close modal dialogs on lose focus
***/
window.onclick = function(event) {
  if (event.target == fileModal || event.target == infoModal || event.target == saveDataModal || event.target == viewDataModal ) {
    fileModal.style.display = "none";
    infoModal.style.display = "none";
    saveDataModal.style.display = "none";
    viewDataModal.style.display = "none";
  }
}

/***********************************************************
* Close modal dialogs on esc button
***/
document.addEventListener("keydown", ({key}) => {
  if (key === "Escape") {
    fileModal.style.display = "none";
    infoModal.style.display = "none";
    saveDataModal.style.display = "none";
    viewDataModal.style.display = "none";
  }
})


