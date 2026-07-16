#include "face.h"
#include "motor.h"
#include "movement.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Rob";
const char* password = "Robby123";

WebServer server(80);

// INTERFACE
const char webpage[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body{
    margin:0;
    padding:20px;
    background:#008080;        /* Teal */
    font-family:Arial, Helvetica, sans-serif;
    text-align:center;
    color:white;
}

h1{
    margin-bottom:35px;
}

button{
    width:140px;
    height:65px;
    margin:8px;
    border:none;
    border-radius:20px;
    background:crimson;
    color:white;
    font-size:18px;
    font-weight:bold;
    cursor:pointer;
    transition:0.2s;
}

button:hover{
    transform:scale(1.05);
    background:#b3002d;
}

button:active{
    transform:scale(0.95);
}

.row{
    margin:10px;
}

</style>

</head>

<body>

<h1> ROB </h1>

<div class="row">
<button onclick="send('Forward')">Forward</button>
</div>

<div class="row">
<button onclick="send('Left')">Left</button>
<button onclick="send('Stop')">Stop</button>
<button onclick="send('Right')">Right</button>
</div>

<div class="row">
<button onclick="send('Reverse')">Reverse</button>
</div>

<br>

<div class="row">
<button onclick="send('SpinC')">Spin CW</button>
<button onclick="send('SpinCC')">Spin CCW</button>
</div>

<div class="row">
<button onclick="send('Explore')">Explore</button>
</div>

<br>

<div class="row">
<button onclick="send('blink')">Blink</button>
<button onclick="send('Wink')">Wink</button>
</div>

<script>

function send(cmd)
{
    fetch("/" + cmd);
}

</script>

</body>
</html>
)rawliteral";

// HOMEPAGE
void handleRoot()
{
  Serial.println("Root page requested");
  server.send(200,"text/html",webpage);
}

// BUTTON HANDLER
void handleForward()
{
    Forward();
    server.send(200,"text/plain","OK");
}

void handleReverse()
{
    Reverse();
    server.send(200,"text/plain","OK");
}

void handleLeft()
{
    Left();
    server.send(200,"text/plain","OK");
}

void handleRight()
{
    Right();
    server.send(200,"text/plain","OK");
}

void handleStop()
{
    Stop();
    server.send(200,"text/plain","OK");
}

void handleSpinC()
{
    SpinC();
    server.send(200,"text/plain","OK");
}

void handleSpinCC()
{
    SpinCC();
    server.send(200,"text/plain","OK");
}

void handleExplore()
{
    Explore();
    server.send(200,"text/plain","OK");
}

void handleblink()
{
    blink();
    server.send(200,"text/plain","OK");
}

void handleWink()
{
    Wink();
    server.send(200,"text/plain","OK");
}

void setup()
{
  Serial.begin(115200);
  randomSeed(micros());
   faceBegin();
   motorBegin();

    WiFi.softAP(ssid, password);

    Serial.println("WiFi Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/",handleRoot);

    server.on("/Forward",handleForward);
    server.on("/Reverse",handleReverse);

    server.on("/Left",handleLeft);
    server.on("/Right",handleRight);

    server.on("/Stop",handleStop);

    server.on("/SpinC",handleSpinC);
    server.on("/SpinCC",handleSpinCC);

    server.on("/Explore",handleExplore);

    server.on("/blink",handleblink);
    server.on("/Wink",handleWink);

    server.begin();
}

void loop()
{
  server.handleClient();
}