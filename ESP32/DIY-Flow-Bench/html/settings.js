var closeFileModalButton = document.getElementsByClassName("closeFileModalButton")[0];
var closeInfoModalButton = document.getElementsByClassName("closeInfoModalButton")[0];
var closeUpdateModalButton = document.getElementsByClassName("closeUpdateModalButton")[0];



window.addEventListener('load', onLoad);


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

  document.getElementById('file-manager-button').addEventListener('click', function(){
    document.getElementById('fileModal').style.display='block';
  });

  document.getElementById('info-button').addEventListener('click', function(){
    document.getElementById('infoModal').style.display='block';
  });

  document.getElementById('update-button').addEventListener('click', function(){
    document.getElementById('updateModal').style.display='block';
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
* Close modal dialogs on lose focus
***/
window.onclick = function(event) {
  if (event.target == fileModal || event.target == infoModal || event.target == updateModal ){
    fileModal.style.display = "none";
    infoModal.style.display = "none";
    updateModal.style.display = "none";
  }
}


/***********************************************************
* Close modal dialogs on esc button
***/
document.addEventListener("keydown", ({key}) => {
  if (key === "Escape") {
    fileModal.style.display = "none";
    infoModal.style.display = "none";
    updateModal.style.display = "none";
  }
})




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
* Close update modal dialog
***/
  closeUpdateModalButton.onclick = function() {
    updateModal.style.display = "none";
}

  