#include "face.h"
#include "motor.h"
#include "movement.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Rob";
const char* password = "Robby123";

WebServer server(80);

// =====================================================================
// INTERFACE
// A small console-style control panel: a D-pad for the motors and a
// separate panel for the face, since those are two independent systems
// now (face.cpp animates on its own via faceUpdate(), movement is
// triggered on demand). "%SSID%" is swapped for the real network name
// when the page is served.
// =====================================================================
const char webpage[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ROB</title>
<style>

:root{
  --bg-0:#10141a;
  --panel:#1b2229;
  --border:#2a323c;
  --text:#eef1f4;
  --text-dim:#8b96a3;
  --cyan:#49d3c8;
  --cyan-dim:#2a7a72;
  --amber:#ffab5e;
  --amber-dim:#8a5a2e;
  --danger:#ff5f6d;
  --radius:16px;
}

*{ box-sizing:border-box; }

body{
  margin:0;
  min-height:100vh;
  background:
    radial-gradient(circle at 20% -10%, rgba(73,211,200,0.10), transparent 40%),
    radial-gradient(circle at 100% 110%, rgba(255,171,94,0.08), transparent 45%),
    var(--bg-0);
  font-family:-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
  color:var(--text);
  display:flex;
  justify-content:center;
  padding:28px 16px 48px;
}

.app{ width:100%; max-width:420px; }

header{
  display:flex;
  justify-content:space-between;
  align-items:flex-start;
  margin-bottom:22px;
}
.eyebrow{
  font-size:11px;
  letter-spacing:.14em;
  text-transform:uppercase;
  color:var(--text-dim);
  margin:0 0 6px;
}
h1{
  margin:0;
  font-size:34px;
  letter-spacing:.06em;
  font-weight:700;
}
.net{
  font-family:ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
  font-size:11px;
  color:var(--text-dim);
  margin-top:6px;
}
.status{
  display:flex;
  align-items:center;
  gap:6px;
  font-size:12px;
  color:var(--text-dim);
  margin-top:4px;
  white-space:nowrap;
}
.dot{
  width:8px; height:8px; border-radius:50%;
  background:#3ddc84;
  animation:pulse 2s infinite;
}
.dot.lost{ background:var(--danger); animation:none; }
@keyframes pulse{
  0%{ box-shadow:0 0 0 0 rgba(61,220,132,.5); }
  70%{ box-shadow:0 0 0 8px rgba(61,220,132,0); }
  100%{ box-shadow:0 0 0 0 rgba(61,220,132,0); }
}

.panel{
  background:var(--panel);
  border:1px solid var(--border);
  border-radius:var(--radius);
  padding:18px;
  margin-bottom:18px;
}
.panel h2{
  margin:0 0 14px;
  font-size:12px;
  letter-spacing:.14em;
  text-transform:uppercase;
  color:var(--text-dim);
  font-weight:600;
}

.dpad{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:10px;
}
.dpad button, .facegrid button{
  border:1px solid var(--border);
  border-radius:12px;
  cursor:pointer;
  font-size:16px;
  font-weight:600;
  color:var(--text);
  padding:18px 6px;
  background:#20272f;
  transition:transform .12s ease, opacity .12s ease;
}
.dpad button:active, .facegrid button:active{ transform:scale(.93); }

.dpad .drive{
  background:linear-gradient(160deg, rgba(73,211,200,.18), rgba(73,211,200,.05));
  border-color:var(--cyan-dim);
  color:var(--cyan);
}
.dpad .ghost{ visibility:hidden; }
.dpad .stop{
  background:linear-gradient(160deg, rgba(255,95,109,.22), rgba(255,95,109,.06));
  border-color:#7a3038;
  color:var(--danger);
}

.explore{
  width:100%;
  margin-top:12px;
  padding:16px;
  border-radius:12px;
  background:transparent;
  border:1.5px dashed var(--cyan-dim);
  color:var(--cyan);
  font-weight:700;
  letter-spacing:.04em;
  cursor:pointer;
}
.explore:active{ transform:scale(.97); }

.facegrid{
  display:grid;
  grid-template-columns:repeat(3,1fr);
  gap:10px;
}
.facegrid .amber{
  background:linear-gradient(160deg, rgba(255,171,94,.18), rgba(255,171,94,.05));
  border-color:var(--amber-dim);
  color:var(--amber);
}
.facegrid .ghost{ visibility:hidden; }

.toggles{
  display:flex;
  flex-direction:column;
  gap:12px;
  margin-top:16px;
  padding-top:14px;
  border-top:1px solid var(--border);
}
.toggle-row{
  display:flex;
  align-items:center;
  justify-content:space-between;
}
.toggle-row span{ font-size:14px; }
.switch{ position:relative; width:44px; height:26px; flex-shrink:0; }
.switch input{ opacity:0; width:0; height:0; }
.slider{
  position:absolute; inset:0;
  background:#2a323c;
  border-radius:999px;
  cursor:pointer;
  transition:.2s;
}
.slider:before{
  content:"";
  position:absolute;
  height:20px; width:20px;
  left:3px; top:3px;
  background:#cfd6dd;
  border-radius:50%;
  transition:.2s;
}
input:checked + .slider{ background:var(--amber-dim); }
input:checked + .slider:before{ transform:translateX(18px); background:var(--amber); }

button.busy{ opacity:.45; pointer-events:none; }

footer{
  text-align:center;
  font-size:11px;
  color:var(--text-dim);
  margin-top:8px;
}

</style>
</head>

<body>

<div class="app">

  <header>
    <div>
      <p class="eyebrow">ESP32 &middot; Local Control</p>
      <h1>ROB</h1>
      <p class="net">NETWORK %SSID%</p>
    </div>
    <div class="status">
      <span class="dot" id="statusDot"></span>
      <span id="statusText">online</span>
    </div>
  </header>

  <section class="panel">
    <h2>Movement</h2>
    <div class="dpad">
      <button class="drive" onclick="send('SpinCC', this)">&#8634;</button>
      <button class="drive" onclick="send('Forward', this)">&#9650;</button>
      <button class="drive" onclick="send('SpinC', this)">&#8635;</button>

      <button class="drive" onclick="send('Left', this)">&#9664;</button>
      <button class="stop" onclick="send('Stop', this)">&#9632;</button>
      <button class="drive" onclick="send('Right', this)">&#9654;</button>

      <div class="ghost"></div>
      <button class="drive" onclick="send('Reverse', this)">&#9660;</button>
      <div class="ghost"></div>
    </div>

    <button class="explore" onclick="send('Explore', this)">Explore (auto)</button>
  </section>

  <section class="panel">
    <h2>Face</h2>
    <div class="facegrid">
      <button class="amber" onclick="send('blink', this)">Blink</button>
      <button class="amber" onclick="send('WinkLeft', this)">Wink L</button>
      <button class="amber" onclick="send('WinkRight', this)">Wink R</button>

      <button class="amber" onclick="send('Happy', this)">Happy</button>
      <button class="amber" onclick="send('Neutral', this)">Neutral</button>
      <button class="amber" onclick="send('LookCenter', this)">Center</button>

      <button class="amber" onclick="send('LookUp', this)">Look &#8593;</button>
      <button class="amber" onclick="send('LookDown', this)">Look &#8595;</button>
      <div class="ghost"></div>
    </div>

    <div class="toggles">
      <div class="toggle-row">
        <span>Auto-blink</span>
        <label class="switch">
          <input type="checkbox" id="autoBlinkToggle" onchange="toggleMode('AutoBlink', this.checked)">
          <span class="slider"></span>
        </label>
      </div>
      <div class="toggle-row">
        <span>Idle drift</span>
        <label class="switch">
          <input type="checkbox" id="idleToggle" onchange="toggleMode('Idle', this.checked)">
          <span class="slider"></span>
        </label>
      </div>
    </div>
  </section>

  <footer>Auto-blink and idle drift run continuously once switched on</footer>

</div>

<script>

function send(cmd, btn){
  if(btn){ btn.classList.add('busy'); }
  fetch("/" + cmd)
    .catch(function(){})
    .finally(function(){ if(btn){ btn.classList.remove('busy'); } });
}

function toggleMode(mode, on){
  fetch("/" + mode + (on ? "On" : "Off")).catch(function(){});
}

function setStatus(ok){
  document.getElementById('statusDot').classList.toggle('lost', !ok);
  document.getElementById('statusText').textContent = ok ? 'online' : 'lost';
}

function ping(){
  fetch("/ping", { cache: "no-store" })
    .then(function(){ setStatus(true); })
    .catch(function(){ setStatus(false); });
}

setInterval(ping, 4000);
ping();

</script>

</body>
</html>
)rawliteral";

// =====================================================================
// PAGE
// =====================================================================
void handleRoot()
{
  Serial.println("Root page requested");
  String page = webpage;
  page.replace("%SSID%", ssid);
  server.send(200, "text/html", page);
}

void handlePing()
{
  server.send(200, "text/plain", "pong");
}

// =====================================================================
// MOVEMENT HANDLERS (unchanged — still call the same Movement.cpp
// functions exactly as before)
// =====================================================================
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

// =====================================================================
// NEW FACE-ONLY HANDLERS
// These call straight into face.h and never touch the motors, so they
// can be triggered independently of Movement.cpp's driving logic.
// =====================================================================
void handleWinkLeft()
{
    winkLeft();
    server.send(200,"text/plain","OK");
}

void handleWinkRight()
{
    winkRight();
    server.send(200,"text/plain","OK");
}

void handleLookUp()
{
    lookUp();
    server.send(200,"text/plain","OK");
}

void handleLookDown()
{
    lookDown();
    server.send(200,"text/plain","OK");
}

void handleLookCenter()
{
    lookCenter();
    server.send(200,"text/plain","OK");
}

void handleHappy()
{
    faceHappy();
    server.send(200,"text/plain","OK");
}

void handleNeutral()
{
    faceNoEmo();
    server.send(200,"text/plain","OK");
}

void handleAutoBlinkOn()
{
    setBlinkMode(true);
    server.send(200,"text/plain","OK");
}

void handleAutoBlinkOff()
{
    setBlinkMode(false);
    server.send(200,"text/plain","OK");
}

void handleIdleOn()
{
    setIdleMode(true);
    server.send(200,"text/plain","OK");
}

void handleIdleOff()
{
    setIdleMode(false);
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
    server.on("/ping",handlePing);

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

    server.on("/WinkLeft",handleWinkLeft);
    server.on("/WinkRight",handleWinkRight);
    server.on("/LookUp",handleLookUp);
    server.on("/LookDown",handleLookDown);
    server.on("/LookCenter",handleLookCenter);
    server.on("/Happy",handleHappy);
    server.on("/Neutral",handleNeutral);

    server.on("/AutoBlinkOn",handleAutoBlinkOn);
    server.on("/AutoBlinkOff",handleAutoBlinkOff);
    server.on("/IdleOn",handleIdleOn);
    server.on("/IdleOff",handleIdleOff);

    server.begin();
}

void loop()
{
  server.handleClient();
  faceUpdate();   // non-blocking — advances blinks, winks, looks, idle drift
}
