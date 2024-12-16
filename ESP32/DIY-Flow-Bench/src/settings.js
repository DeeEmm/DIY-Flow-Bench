
/***********************************************************
* onLoad event handler
***/
function onLoad(event) {

    initialiseButtons();

}


/***********************************************************
* onFileUpload event handler
***/
function onFileUpload(event) {
    this.setState({file: event.target.files[0]});
    const {file} = this.state;
    const data = new FormData;
    data.append('data', file);
    fetch('/api/file/upload', {
        method: 'POST',
        body: data
    })
        .catch(e => {
            console.log('Request failed', e);
        });
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
  
    document.getElementById('export-graph-image-button').addEventListener('click', function(){
      // initiate datagraph image download from browser
      exportSVGAsImage();
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
    if (event.target == fileModal || event.target == dataModal || event.target == serialModal || event.target == infoModal || event.target == captureLiftDataModal || event.target == loadGraphDataModal || event.target == saveGraphDataModal || event.target == calibrationModal || event.target == updateModal || event.target == flowTargetModal  ){
      fileModal.style.display = "none";
      infoModal.style.display = "none";
      captureLiftDataModal.style.display = "none";
      loadGraphDataModal.style.display = "none";
      saveGraphDataModal.style.display = "none";
      calibrationModal.style.display = "none";
      dataModal.style.display = "none";
      serialModal.style.display = "none";
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
      dataModal.style.display = "none";
      serialModal.style.display = "none";
      updateModal.style.display = "none";
      flowTargetModal.style.display = "none";
    }
  })
  
  


    /***********************************************************
    * Close update modal dialog
    ***/
    closeUpdateModalButton.onclick = function() {
        updateModal.style.display = "none";
    }