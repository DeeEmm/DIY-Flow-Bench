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

window.addEventListener('load', onLoad);

var fileModal = document.getElementById("fileModal");
var infoModal = document.getElementById("infoModal");
var closeFileModalButton = document.getElementsByClassName("closeFileModalButton")[0];
var closeInfoModalButton = document.getElementsByClassName("closeInfoModalButton")[0];

// Set up Server Side Events (SSE)
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('JSON_DATA', function(e) {
    var myObj = JSON.parse(e.data);
    for (key in myObj) {
      try {
        if (typeof myObj[key] === 'string' || myObj[key] instanceof String) {
          document.getElementById(key).innerHTML = myObj[key];
        } else {
          document.getElementById(key).innerHTML = myObj[key].toFixed(2); 
        }
        // console.log(key + ' : ' + myObj[key]);
      } catch (error) {
        console.log('Missing or incorrect data');
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

  document.getElementById('file-manager-button').addEventListener('click', function(){
    document.getElementById('fileModal').style.display='block';
  });

  document.getElementById('info-button').addEventListener('click', function(){
    document.getElementById('infoModal').style.display='block';
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
* Close modal dialogs on lose focus
***/
window.onclick = function(event) {
  if (event.target == fileModal || event.target == infoModal ) {
    fileModal.style.display = "none";
    infoModal.style.display = "none";
  }
}

/***********************************************************
* Close modal dialogs on esc button
***/
document.addEventListener("keydown", ({key}) => {
  if (key === "Escape") {
    fileModal.style.display = "none";
    infoModal.style.display = "none";
  }
})


