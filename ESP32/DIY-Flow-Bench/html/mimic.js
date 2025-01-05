/***********************************************************
* mimic.js
*
* Browser control code for DIY-Flow-bench project: diyflowbench.com
* Handles data transmission and display update tasks
* Provides UI control to browser
*
***/

var updateSSE = true;

window.addEventListener('load', onLoad);


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
              // we've got a number... 
              
                if (key === 'MAF_VOLTS' || key === 'PREF_VOLTS' || key === 'PDIFF_VOLTS' || key === 'PITOT_VOLTS' || key === "FLOW_KG_H" || key === "FLOW_MG_S" || key === "FLOW_CFM" || key === "FLOW_LPM") {
                    document.getElementById(key).innerHTML = myObj[key].toFixed(2);  

                } else if (key === 'MAF_ADC' || key === 'PREF_ADC' || key === 'PDIFF_ADC' || key === 'PITOT_ADC') {
                    document.getElementById(key).innerHTML = myObj[key].toFixed(0); 
      
                } else {
                    document.getElementById(key).innerHTML = myObj[key];
                }
            }
          } catch (error) {
            console.log('Missing or incorrect JSON data');
            console.log(" error: " + error + " key: " + key );
          }
        } 
  
      }
  
    }, false);
  
  }
  




/***********************************************************
* onLoad event handler
***/
function onLoad(event) {

  
    console.log('Mimic page Loaded');
    
  }
  
  