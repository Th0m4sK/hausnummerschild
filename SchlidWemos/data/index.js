let ELEMENTS=["PaSpeed","PaAccer","PaResul","PaMax","PaBacksl"]


function myFunction() {
  var x = document.getElementById("myLinks");
  if (x.style.display === "block") {
    x.style.display = "none";
  } else {
    x.style.display = "block";
  }
}
 
function allGreen() {
  var i;
  for (i = 0; i < ELEMENTS.length; i++){
    document.getElementById(ELEMENTS[i]).style.border = "20px solid #22ca2a";
  }
  document.getElementById("SAVEPAR").style.background = "#22ca2a";
}
 
function getValue(Name,mul) {
  return document.getElementById(Name).value*mul
}
function setValue(Name,val) {
  document.getElementById(Name).value = val;
}

 function save() {
   console.log("SavePar");
   let sendtext = ["ParM"];
   var i;
   for (i = 0; i < ELEMENTS.length; i++){
       sendtext.push(getValue(ELEMENTS[i],1))
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
       }
  };
  xhttp.open("GET", "/ParM", true);
  xhttp.send();
}


window.addEventListener("load", fkt_load);
document.getElementById("PaSpeed").addEventListener("input", fkt_boarder);
document.getElementById("PaAccer").addEventListener("input", fkt_boarder);
document.getElementById("PaResul").addEventListener("input", fkt_boarder);
document.getElementById("PaMax").addEventListener("input", fkt_boarder);
document.getElementById("PaSpeed").addEventListener("input", fkt_boarder);
document.getElementById("PaBacksl").addEventListener("input", fkt_boarder);

