
window.addEventListener('load', onLoad);

/***********************************************************
* Initialise buttons
***/
function initialiseButtons() {
  
    var xhr = new XMLHttpRequest();
  
    document.getElementById('clear-graph-data-button').addEventListener('click', function() {
          
      // clear line data from graph
      var l = document.getElementById('lineData');
      var child = l.lastElementChild; 
      while (child) {
          l.removeChild(child);
          child = l.lastElementChild;
      }
      
      // console.log('XHR: /api/clearLiftData');

      xhr.open('POST', '/api/clearLiftData');
      xhr.onload = function() {
        if (xhr.status === 200) {
           window.location.href = '/data';
        }
      };
      xhr.send();
  
    });
    
    document.getElementById('export-graph-data-button').addEventListener('click', function(){
      // initiate JSON Data download from browser
      document.getElementById('graph-data-download').click();
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
* onLoad event handler
***/
function onLoad(event) {
  
  initialiseButtons();
  
  console.log('Data page Loaded');
  
}