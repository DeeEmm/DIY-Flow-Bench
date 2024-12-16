
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
  * Close data modal dialog
  ***/
  closeDataModalButton.onclick = function() {
    dataModal.style.display = "none";
  }
  
  
  /***********************************************************
  * Close Capture Data modal dialog
  ***/
  closeCaptureLiftDataModalButton.onclick = function() {
    captureLiftDataModal.style.display = "none";
  }