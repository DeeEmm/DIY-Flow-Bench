window.addEventListener('load', onLoad);



/***********************************************************
* onLoad event handler
***/
function onLoad(event) {

    initialiseButtons();

    console.log('Page Loaded');
    
}




/***********************************************************
* Initialise buttons
***/
function initialiseButtons() {
  
    var xhr = new XMLHttpRequest();

      
    document.getElementById('calibrate-button').addEventListener('click', function(event){
        console.log('Calibrate Flow Offset');
        xhr.open('GET', '/api/bench/calibrate');
        xhr.onload = function() {
          if (xhr.status === 200) window.location.href = '/calibration';
        };
        xhr.send();
        event.preventDefault();
      });
    
      document.getElementById('leak-cal-button').addEventListener('click', function(event){
        console.log('Leak Test Calibration');
        xhr.open('GET', '/api/bench/leakcal');
        xhr.onload = function() {
          if (xhr.status === 200) window.location.href = '/calibration';
        };
        xhr.send();
        event.preventDefault();
      });

}