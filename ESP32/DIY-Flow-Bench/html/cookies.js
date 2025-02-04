
// three cookies to remamber state of top three tiles - pressure / flow / tools

// http://www.quirksmode.org/js/cookies.html
// setCookie('ppkcookie','testcookie',7);

// var x = getCookie('ppkcookie');
// if (x) {
//     [do something with x]
// }
function setCookie(name,value,days) {
    var expires = "";
    if (days) {
        var date = new Date();
        date.setTime(date.getTime() + (days*24*60*60*1000));
        expires = "; expires=" + date.toUTCString();
    }
    document.cookie = name + "=" + (value || "")  + expires + "; path=/";
  }
  
  function getCookie(name) {
    var nameEQ = name + "=";
    var ca = document.cookie.split(';');
    for(var i=0;i < ca.length;i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1,c.length);
        if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
    }
    return null;
  }
  
  // https://github.com/Autodrop3d/serialTerminal.com
  function changeCookieValue(cookieName, newValue) {
    document.cookie = 
        `${cookieName}=${newValue}; 
            expires=Thu, 5 March 2030 12:00:00 UTC; path=/`;
  }
  