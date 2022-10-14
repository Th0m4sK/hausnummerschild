let ELEMENTS=["ON","OFF","r","g","b","Helligkeit"]
let aktLight=0
setInterval(function () {
     
  let xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    console.log(this.readyState);
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("akt").innerText = "aktuell: "+ this.responseText+ " Lux";
      aktLight=parseFloat(this.responseText)
    }
  };
  xhttp.open("GET", "/light", true);
  xhttp.send();
}, 1000);


function allGreen() {
  var i;
  for (i = 0; i < ELEMENTS.length; i++){
    document.getElementById(ELEMENTS[i]).style.border = "20px solid #22ca2a";
  }
  document.getElementById("SAVEPAR").style.background = "#22ca2a";
}
 
function getValue(Name) {
 
  return document.getElementById(Name).value

}
function setValue(Name,val) {
  document.getElementById(Name).value = val;
}

 function save() {
   console.log("SavePar");
   let sendtext = [];
   var i;
   for (i = 0; i < ELEMENTS.length; i++){
       sendtext.push(getValue(ELEMENTS[i]))
 }
  post(sendtext);
  allGreen();
   
}

function fkt_boarder(ev) {
  console.log("input");
  document.getElementById(ev.target.id).style.border = "20px solid #ff0202";
  document.getElementById("SAVEPAR").style.backgroundColor = "#ff0202";
  
}

function post(text) {
  var i;
  let sendtext = "";
  for (i = 0; i < text.length; i++) {
    
    sendtext += text[i]+"/";
  }
 
     
  console.log(sendtext);
  let xhr = new XMLHttpRequest();
  xhr.open("POST", "/data", true);
  xhr.setRequestHeader("Content-Type", "text/plain");
  xhr.send(sendtext);
}

function componentToHex(c) {
  var hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;
}

function rgbToHex(r, g, b) {
  return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

function fkt_load() {
  let xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      const DATA = this.responseText.split("/");
      var i;
      for (i = 0; i < ELEMENTS.length; i++){
        
          setValue(ELEMENTS[i], parseFloat(DATA[i]));
        
        }
       
    
      allGreen();
      let col =rgbToHex(getValue("r"),getValue("g"),getValue("b"))
      console.log(col)
      setValue("color1",col)
       }
  };
  xhttp.open("GET", "/ParM", true);
  xhttp.send();
}
function hexToRgb(hex) {
  var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
  return result ? {
    r: parseInt(result[1], 16),
    g: parseInt(result[2], 16),
    b: parseInt(result[3], 16)
  } : null;
}



function fkt_changeColor(ev){
 
  let col=hexToRgb(ev.target.value)
  setValue("r",col.r)
  setValue("g",col.g)
  setValue("b",col.b)


}
window.addEventListener("load", fkt_load);
document.getElementById("ON").addEventListener("input", fkt_boarder);
document.getElementById("OFF").addEventListener("input", fkt_boarder);
document.getElementById("Helligkeit").addEventListener("input", fkt_boarder);
document.getElementById("Color1").addEventListener("input", fkt_changeColor);


