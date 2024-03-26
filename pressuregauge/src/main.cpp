#include <ESP8266WiFi.h>
#include <Servo.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

Servo ObjServo;
static const int ServoGPIO = D4;
static const int beepPin = D3;
static const int beepPinGround = D2;

ESP8266WebServer server(80);
String header;
String valueString = String(0);
String pressureValueString = String(0);
int positon1 = 0;
int positon2 = 0;

const char DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 112, 1);

String temp = "";

boolean isIp(String str) {
	for (int i = 0; i < str.length();i++) {
	int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9')) {       
			return false;
     		}   
	}   
	return true;
}

String toStringIp(IPAddress ip) {   
	String res = "";
	for (int i = 0; i < 3; i++) {
		res += String((ip >> (8 * i)) & 0xFF) + ".";
   	}   
	res += String(((ip >> 8 * 3)) & 0xFF);
   	return res;
}

bool captivePortal() {   
	if (!isIp(server.hostHeader())) {
		Serial.println("Request redirected to captive portal");
     		server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
     		server.send ( 302, "text/plain", "");
 		server.client().stop();
 		return true;
	}   
	return false;
}

void handleNotFound() {   
   if (captivePortal())   {
      	   return;
   }   temp = "";
   // HTML Header   
   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
   server.sendHeader("Pragma", "no-cache");
   server.sendHeader("Expires", "-1");
   server.setContentLength(CONTENT_LENGTH_UNKNOWN);
   // HTML Content   
   temp += "<! DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
   temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
   temp += "body {background-color: powderblue;} </style>";
   temp += "<head><title>File not found</title></head>";
   temp += "<h2> 404 File Not Found</h2><br>";
   temp += "<h4>Debug Information:</h4><br>";
   temp += "<body>";
   temp += "URI: ";
   temp += server.uri();
   temp += "\nMethod: ";
   temp += ( server.method() == HTTP_GET ) ? "GET" : "POST";
   temp += "<br>Arguments: ";
   temp += server.args();
   temp += "\n";
   for ( uint8_t i = 0; i < server.args(); i++ ) {     
	temp += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
   }  

   temp += "<br>Server Hostheader: " + server.hostHeader();
   for ( uint8_t i = 0; i < server.headers(); i++ ) {
	   temp += " " + server.headerName ( i ) + ": " + server.header ( i ) + "\n<br>";
   }   
   temp += "</table></form><br><br><table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h2>You may want to browse to:</h2></p></caption>";
   temp += "<tr><th>";
   temp += "<a href='/'>Main Page</a><br>";
   temp += "<a href='/wifi'>WIFI Settings</a><br>";
   temp += "</th></tr></table><br><br>";
   temp += "</body></html>";
   server.send ( 404, "", temp );
   server.client().stop();
   temp = "";
}

void handleRoot() {
   String temp = "";
   // HTML Header
   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
   server.sendHeader("Pragma", "no-cache");
   server.sendHeader("Expires", "-1");
   server.setContentLength(CONTENT_LENGTH_UNKNOWN);
   // HTML Content
   server.send ( 200, "text/html", temp );
   // Speichersparen - Schon mal dem Cleint senden
   temp = "";
   temp += "<!DOCTYPE html><html><head><title>Manometer Beispiel</title><style>canvas { border: 1px solid black; }</style></head>";
   temp += "<body onload=\"init()\"><canvas id=\"manometer\" width=\"400\" height=\"400\"></canvas>";
   temp += "<div></div><div id=\"anzeige\"></div><div id=\"timerDisplay\">Verstrichene Zeit: 00:00</div>";
   temp += "<script>var canvas = document.getElementById('manometer');var context = canvas.getContext('2d');";
   temp += "var x = canvas.width / 2;var y = canvas.height / 2;var radius = canvas.width / 2 - 10;";
   temp += "var minBar = 0;var maxBar = 300;var rotBereich = 50;var timerId = null;var elapsedTime = 0;";
   temp += "var intervalId = null;var paused = false;function drawManometer() {";
   temp += "context.clearRect(0, 0, canvas.width, canvas.height);context.beginPath();";
   temp += "context.arc(x, y, radius + 10, Math.PI, 0, false);context.lineWidth = 2;context.strokeStyle = '#000000';";
   temp += "context.stroke();context.font = \"20px Arial\";context.textAlign = \"center\";context.textBaseline = \"middle\";";
   temp += "for (var i = 0; i <= 6; i++) {var grad = i * 50;var angle = (((grad / (maxBar - minBar)) * (Math.PI)) - (Math.PI));";
   temp += "var xGrad = x + Math.cos(angle) * (radius - 20);var yGrad = y + Math.sin(angle) * (radius - 20);";
   temp += "context.fillText(grad.toString(), xGrad, yGrad);}context.beginPath();context.arc(x, y, radius + 5, Math.PI, 0, false);";
   temp += "context.lineWidth = 5;context.strokeStyle = '#000000';context.stroke();context.beginPath();";
   temp += "context.arc(x, y, radius - 3, (((50 - rotBereich) / (maxBar - minBar)) * (Math.PI)) + Math.PI, ((50 / (maxBar - minBar)) * (Math.PI)) + Math.PI, false);";
   temp += "context.lineWidth = 10;context.strokeStyle = '#FF0000';context.stroke();}var startDruckSlider = document.createElement(\"input\");";
   temp += "startDruckSlider.setAttribute(\"type\", \"range\");startDruckSlider.setAttribute(\"min\", minBar);startDruckSlider.setAttribute(\"max\", maxBar);";
   temp += "startDruckSlider.setAttribute(\"value\", minBar);startDruckSlider.setAttribute(\"step\", \"10\");startDruckSlider.setAttribute(\"style\", \"width: 200px;\");";
   temp += "startDruckSlider.oninput = function () {startDruck = parseInt(this.value);drawZeiger(startDruck);};";
   temp += "var startDruckText = document.createTextNode(\"Startdruck (bar): \");var startDruckDiv = document.createElement(\"div\");";
   temp += "startDruckDiv.appendChild(startDruckText);startDruckDiv.appendChild(startDruckSlider);document.body.appendChild(startDruckDiv);";
   temp += "var druckAbfallSlider = document.createElement(\"input\");druckAbfallSlider.setAttribute(\"type\", \"range\");";
   temp += "druckAbfallSlider.setAttribute(\"min\", \"1\");druckAbfallSlider.setAttribute(\"max\", \"10\");druckAbfallSlider.setAttribute(\"value\", \"1\");";
   temp += "druckAbfallSlider.setAttribute(\"step\", \"1\");druckAbfallSlider.setAttribute(\"style\", \"width: 200px;\");";
   temp += "druckAbfallSlider.oninput = function () {druckAbfallRate = parseInt(this.value);};";
   temp += "var druckAbfallText = document.createTextNode(\"Druckabfall-Rate (bar/s): \");var druckAbfallDiv = document.createElement(\"div\");";
   temp += "druckAbfallDiv.appendChild(druckAbfallText);druckAbfallDiv.appendChild(druckAbfallSlider);document.body.appendChild(druckAbfallDiv);";
   temp += "var startButton = document.createElement(\"button\");startButton.innerHTML = \"Start\";startButton.addEventListener(\"click\", function () {";
   temp += "clearInterval(intervalId);startDruck = parseFloat(startDruckSlider.value);druckAbfallRate = parseFloat(druckAbfallSlider.value);";
   temp += "bar = startDruck;elapsedTime = 0;paused = false;intervalId = setInterval(function () {if (!paused) {elapsedTime++;";
   temp += "bar -= druckAbfallRate / fps;if (bar < minBar) {bar = minBar;clearInterval(intervalId);}drawZeiger(bar);updateElapsedTime();}}, 1000 / fps);});";
   temp += "document.body.appendChild(startButton);var pauseButton = document.createElement(\"button\");pauseButton.innerHTML = \"Pause\";";
   temp += "pauseButton.addEventListener(\"click\", function () {paused = !paused;});document.body.appendChild(pauseButton);";
   temp += "var resetButton = document.createElement(\"button\");resetButton.innerHTML = \"Reset\";resetButton.addEventListener(\"click\", function () {";
   temp += "clearInterval(intervalId);bar = startDruckSlider.value;elapsedTime = 0;drawZeiger(bar);updateElapsedTime();});";
   temp += "document.body.appendChild(resetButton);var fps = 60;var startDruck = 0;var druckAbfallRate = 1;var bar = startDruck;";
   temp += "function drawZeiger(bar) {var angle = (((bar - minBar) / (maxBar - minBar)) * (Math.PI)) + Math.PI;";
   temp += "var xZeiger = x + Math.cos(angle) * (radius - 70);var yZeiger = y + Math.sin(angle) * (radius - 70);";
   temp += "context.beginPath();context.arc(x, y, radius - 65, 0, 2 * Math.PI);context.fillStyle = '#FFFFFF';context.fill();";
   temp += "context.beginPath();context.moveTo(x, y);context.lineTo(xZeiger, yZeiger);context.lineWidth = 5;context.strokeStyle = '#FF0000';";
   temp += "context.stroke();var anzeige = document.getElementById(\"anzeige\");anzeige.innerHTML = bar.toString();}";
   temp += "function updateElapsedTime() {var timeDisplay = document.getElementById(\"timerDisplay\");";
   temp += "var minutes = Math.floor(elapsedTime / 60);var seconds = elapsedTime % 60;";
   temp += "timeDisplay.innerHTML = \"Verstrichene Zeit: \" + (minutes < 10 ? \"0\" : \"\") + minutes + \":\" + (seconds < 10 ? \"0\" : \"\") + seconds;}";
   temp += "function init() {drawManometer();drawZeiger(0);}</script></body></html>";

   server.sendContent(temp);
}


void InitializeHTTPServer(){
   server.on("/", handleRoot);
   server.on("/generate_204", handleRoot);
   server.on("/favicon.ico", handleRoot);
   server.on("/fwlink", handleRoot);
   server.on("/generate_204", handleRoot);
   server.onNotFound(handleNotFound);
   //server.collectHeaders(Headers, sizeof(Headers)/ sizeof(Headers[0]));
   server.begin();
}

void setup() {
  Serial.begin(115200);
  pinMode(beepPinGround, OUTPUT);
  digitalWrite(beepPinGround, LOW);
  pinMode(beepPin, OUTPUT);
  digitalWrite(beepPin, LOW);
  ObjServo.attach(ServoGPIO, 500, 2800);
  Serial.print("Making connection to ");
  
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Drucksensor");
  //WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("");
  //Serial.println("WiFi connected.");
  //Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  dnsServer.start(DNS_PORT, "*", apIP);
  server.begin();
  InitializeHTTPServer();
}



void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
