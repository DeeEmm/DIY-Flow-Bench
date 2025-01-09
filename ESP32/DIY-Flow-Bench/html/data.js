
window.addEventListener('load', onLoad);

/***********************************************************
* Initialise buttons
***/
function initialiseButtons() {
  
    var xhr = new XMLHttpRequest();
  
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
  
}





/***********************************************************
* Export Data Graph as PNG Image (temp change to jpg)
* Source: https://takuti.me/note/javascript-save-svg-as-image/
***/
function exportSVGAsImage() {

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
      context.clearRect(0, 0, canvas.width, canvas.height);
      context.drawImage(img, 0, 0, canvas.width, canvas.height);
  
      URL.revokeObjectURL(url);
  
      // trigger a synthetic download operation with a temporary link
      const a = document.createElement('a');
      document.body.appendChild(a);
      a.download = 'LiftGraph.png';
      // a.href = canvas.toDataURL('image/jpeg');
      a.href = canvas.toDataURL();
      a.click();
      a.remove();
  
    });
    img.src = url;
  
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
* onLoad event handler
***/
function onLoad(event) {
  
  initialiseButtons();
  
  console.log('Data page Loaded');
  
}