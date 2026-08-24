/*
 * ============================================================
 * ESP8266 SMART HOME HUB
 * ============================================================
 *
 * Features:
 * - MQ135 Air Quality Monitoring
 * - Door Open/Closed Detection
 * - Electronic Door Lock Control
 * - Automatic Door Lock after 4 seconds
 * - Auxiliary Relay Control
 * - 16x2 I2C LCD Display
 * - Wi-Fi Web Dashboard
 * - Real-time JSON Status API
 *
 * Hardware:
 * - NodeMCU ESP8266
 * - MQ135 Air Quality Sensor
 * - Door/Magnetic Switch
 * - 2-Channel Relay Module
 * - 16x2 I2C LCD
 *
 * Pin Configuration:
 * MQ135       -> A0
 * Door Switch -> D5 (GPIO14)
 * Lock Relay  -> D6 (GPIO12)
 * Aux Relay   -> D7 (GPIO13)
 * I2C SDA     -> D2 (GPIO4)
 * I2C SCL     -> D1 (GPIO5)
 *
 * Author: Raj B
 * Project: ESP8266 Smart Home Hub
 * ============================================================
 */

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ============================================================
// Wi-Fi Configuration
// ============================================================
// IMPORTANT:
// Do NOT upload your real Wi-Fi credentials to GitHub.
//
// Replace these locally before uploading to ESP8266.
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// Pin Definitions
// ============================================================

#define MQ135_PIN    A0

#define RELAY_LOCK   D6      // GPIO12 - Door lock relay
#define RELAY_AUX    D7      // GPIO13 - Auxiliary relay
#define DOOR_SWITCH  D5      // GPIO14 - Door switch

// ============================================================
// LCD Configuration
// ============================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================
// Web Server
// ============================================================

AsyncWebServer server(80);

// ============================================================
// System Variables
// ============================================================

bool auxState = false;
bool lockRelayActive = false;

unsigned long unlockTime = 0;

const unsigned long autoLockDelay = 4000;

unsigned long lastLCDUpdate = 0;
const unsigned long lcdInterval = 500;

// ============================================================
// Air Quality Functions
// ============================================================

String getAirQualityText(int value)
{
  if (value < 300)
    return "GOOD";

  if (value < 700)
    return "MODERATE";

  return "HAZARDOUS";
}

int getAirValue()
{
  return analogRead(MQ135_PIN);
}

// ============================================================
// Door Functions
// ============================================================

bool isDoorClosed()
{
  return digitalRead(DOOR_SWITCH) == LOW;
}

String getDoorStatus()
{
  return isDoorClosed() ? "CLOSED" : "OPEN";
}

// ============================================================
// Lock Functions
// ============================================================

String getLockStatus()
{
  return lockRelayActive ? "UNLOCKED" : "LOCKED";
}

void lockDoor()
{
  // Relay OFF = Door locked
  digitalWrite(RELAY_LOCK, HIGH);

  lockRelayActive = false;
}

void unlockDoor()
{
  // Relay ON = Door unlocked
  digitalWrite(RELAY_LOCK, LOW);

  lockRelayActive = true;
  unlockTime = millis();
}

// ============================================================
// Auxiliary Relay
// ============================================================

String getAuxStatus()
{
  return auxState ? "ON" : "OFF";
}

void toggleAuxRelay()
{
  auxState = !auxState;

  // Active LOW relay
  digitalWrite(RELAY_AUX, auxState ? LOW : HIGH);
}

// ============================================================
// LCD Display
// ============================================================

void updateLCD()
{
  int airValue = getAirValue();

  String airText = getAirQualityText(airValue);
  String doorText = getDoorStatus();
  String lockText = getLockStatus();

  // First screen
  lcd.clear();

  lcd.setCursor(0, 0);

  if (doorText == "OPEN")
    lcd.print("Door:OPEN");
  else
    lcd.print("Door:CLOSED");

  lcd.setCursor(0, 1);

  if (airText == "GOOD")
    lcd.print("Air:GOOD");

  else if (airText == "MODERATE")
    lcd.print("Air:MODERATE");

  else
    lcd.print("Air:HAZARDOUS");

  delay(5);

  // Second screen
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Lock:");
  lcd.print(lockText);

  lcd.setCursor(0, 1);
  lcd.print("MQ135:");
  lcd.print(airValue);
}

// ============================================================
// Web Dashboard
// ============================================================

String webpage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>ESP8266 Smart Home Hub</title>

<style>

*{
  box-sizing:border-box;
  margin:0;
  padding:0;
  font-family:Arial,sans-serif;
}

body{
  background:linear-gradient(
    135deg,
    #0f172a,
    #1e293b,
    #334155
  );

  color:white;
  min-height:100vh;
  padding:20px;
}

.container{
  max-width:900px;
  margin:auto;
}

.title{
  text-align:center;
  margin-bottom:20px;
}

.title h1{
  font-size:32px;
  margin-bottom:8px;
}

.title p{
  color:#cbd5e1;
}

.grid{
  display:grid;
  grid-template-columns:
    repeat(auto-fit,minmax(240px,1fr));

  gap:18px;
}

.card{
  background:rgba(255,255,255,0.08);

  border:
    1px solid
    rgba(255,255,255,0.12);

  backdrop-filter:blur(10px);

  border-radius:20px;

  padding:20px;

  box-shadow:
    0 8px 25px
    rgba(0,0,0,0.25);

  transition:0.3s;
}

.card:hover{
  transform:translateY(-4px);
}

.card h2{
  font-size:20px;
  margin-bottom:15px;
}

.value{
  font-size:28px;
  font-weight:bold;
  margin:8px 0;
}

.sub{
  color:#cbd5e1;
  font-size:14px;
}

.status{
  display:inline-block;

  padding:8px 14px;

  border-radius:999px;

  font-weight:bold;

  margin-top:10px;

  font-size:14px;
}

.good{
  background:#14532d;
  color:#bbf7d0;
}

.moderate{
  background:#78350f;
  color:#fde68a;
}

.hazard{
  background:#7f1d1d;
  color:#fecaca;
}

.open{
  background:#991b1b;
  color:#fee2e2;
}

.closed{
  background:#166534;
  color:#dcfce7;
}

.btn-group{
  display:flex;
  flex-wrap:wrap;
  gap:10px;
  margin-top:15px;
}

button{
  border:none;
  outline:none;

  padding:12px 18px;

  border-radius:12px;

  font-size:15px;
  font-weight:bold;

  cursor:pointer;

  transition:0.25s;
}

button:hover{
  transform:scale(1.04);
  opacity:0.95;
}

.btn-primary{
  background:#22c55e;
  color:#052e16;
}

.btn-danger{
  background:#ef4444;
  color:white;
}

.btn-secondary{
  background:#38bdf8;
  color:#082f49;
}

.switch-box{
  margin-top:12px;

  display:flex;
  align-items:center;
  justify-content:space-between;

  gap:12px;

  background:
    rgba(255,255,255,0.05);

  border-radius:12px;

  padding:12px 14px;
}

.toggle{
  position:relative;

  width:64px;
  height:34px;
}

.toggle input{
  display:none;
}

.slider{
  position:absolute;
  inset:0;

  background:#64748b;

  border-radius:34px;

  transition:0.3s;
  cursor:pointer;
}

.slider:before{
  content:"";

  position:absolute;

  width:26px;
  height:26px;

  left:4px;
  top:4px;

  background:white;

  border-radius:50%;

  transition:0.3s;
}

input:checked + .slider{
  background:#22c55e;
}

input:checked + .slider:before{
  transform:translateX(30px);
}

.footer{
  text-align:center;

  color:#cbd5e1;

  margin-top:20px;

  font-size:14px;
}

</style>

</head>

<body>

<div class="container">

<div class="title">

<h1>ESP8266 Smart Home Hub</h1>

<p>
Live monitoring and door lock control
</p>

</div>


<div class="grid">


<!-- Air Quality -->

<div class="card">

<h2>Air Quality</h2>

<div
  class="value"
  id="airText">
  --
</div>

<div
  class="sub"
  id="airValue">
  MQ135 Value: --
</div>

<div
  class="status"
  id="airBadge">
  Waiting...
</div>

</div>


<!-- Door Status -->

<div class="card">

<h2>Door Status</h2>

<div
  class="value"
  id="doorText">
  --
</div>

<div class="sub">
Magnetic/limit switch feedback
</div>

<div
  class="status"
  id="doorBadge">
  Waiting...
</div>

</div>


<!-- Door Lock -->

<div class="card">

<h2>Lock Control</h2>

<div
  class="value"
  id="lockText">
  --
</div>

<div class="sub">
Switch to lock / unlock door
</div>


<div class="switch-box">

<span>
Door Lock Switch
</span>

<label class="toggle">

<input
  type="checkbox"
  id="lockSwitch"
  onchange="toggleLockSwitch()">

<span class="slider"></span>

</label>

</div>


<div class="btn-group">

<button
  class="btn-primary"
  onclick="unlockDoor()">
Unlock
</button>

<button
  class="btn-danger"
  onclick="lockDoor()">
Lock
</button>

</div>

</div>


<!-- Auxiliary Relay -->

<div class="card">

<h2>Aux Appliance</h2>

<div
  class="value"
  id="auxText">
  --
</div>

<div class="sub">
Relay controlled output
</div>

<div class="btn-group">

<button
  class="btn-secondary"
  onclick="toggleAux()">
Toggle Aux
</button>

</div>

</div>

</div>


<div class="footer">

ESP8266 Smart Home Hub •
Live data refresh every 1 second

</div>

</div>


<script>

async function unlockDoor(){

  await fetch('/unlock');

  updateData();

}


async function lockDoor(){

  await fetch('/lock');

  updateData();

}


async function toggleAux(){

  await fetch('/toggleAux');

  updateData();

}


async function toggleLockSwitch(){

  const sw =
    document.getElementById(
      'lockSwitch'
    );

  if(sw.checked){

    await fetch('/unlock');

  }else{

    await fetch('/lock');

  }

  updateData();

}


function setAirBadge(air){

  const badge =
    document.getElementById(
      "airBadge"
    );

  badge.className = "status";

  if(air === "GOOD"){

    badge.classList.add("good");

  }

  else if(air === "MODERATE"){

    badge.classList.add("moderate");

  }

  else{

    badge.classList.add("hazard");

  }

  badge.innerHTML = air;

}


function setDoorBadge(door){

  const badge =
    document.getElementById(
      "doorBadge"
    );

  badge.className = "status";

  if(door === "OPEN"){

    badge.classList.add("open");

  }

  else{

    badge.classList.add("closed");

  }

  badge.innerHTML = door;

}


async function updateData(){

  try{

    const response =
      await fetch('/status');

    const data =
      await response.json();


    document.getElementById(
      "airText"
    ).innerHTML = data.air;


    document.getElementById(
      "airValue"
    ).innerHTML =
      "MQ135 Value: "
      + data.airValue;


    document.getElementById(
      "doorText"
    ).innerHTML = data.door;


    document.getElementById(
      "lockText"
    ).innerHTML = data.lock;


    document.getElementById(
      "auxText"
    ).innerHTML = data.aux;


    setAirBadge(data.air);

    setDoorBadge(data.door);


    document.getElementById(
      "lockSwitch"
    ).checked =
      data.lock === "UNLOCKED";

  }

  catch(error){

    console.log(
      "Connection error:",
      error
    );

  }

}


setInterval(
  updateData,
  1000
);

updateData();

</script>

</body>

</html>
)rawliteral";

  return html;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  // Configure pins
  pinMode(RELAY_LOCK, OUTPUT);
  pinMode(RELAY_AUX, OUTPUT);
  pinMode(DOOR_SWITCH, INPUT_PULLUP);

  // Initial safe states
  lockDoor();

  // Active LOW relay
  digitalWrite(RELAY_AUX, HIGH);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  lcd.setCursor(0, 1);
  lcd.print("Please wait...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.println("WiFi Connected");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Show IP on LCD
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");

  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  delay(2000);


  // ========================================================
  // Web Server Routes
  // ========================================================

  // Home page
  server.on(
    "/",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {
      request->send(
        200,
        "text/html",
        webpage()
      );
    }
  );


  // Unlock door
  server.on(
    "/unlock",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {
      unlockDoor();

      request->send(
        200,
        "text/plain",
        "Door Unlocked"
      );
    }
  );


  // Lock door
  server.on(
    "/lock",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {
      lockDoor();

      request->send(
        200,
        "text/plain",
        "Door Locked"
      );
    }
  );


  // Toggle auxiliary relay
  server.on(
    "/toggleAux",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {
      toggleAuxRelay();

      request->send(
        200,
        "text/plain",
        "Aux Toggled"
      );
    }
  );


  // JSON status API
  server.on(
    "/status",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {
      int airValue = getAirValue();

      String airText =
        getAirQualityText(
          airValue
        );


      DynamicJsonDocument doc(256);

      doc["door"] =
        getDoorStatus();

      doc["lock"] =
        getLockStatus();

      doc["air"] =
        airText;

      doc["airValue"] =
        airValue;

      doc["aux"] =
        getAuxStatus();


      String json;

      serializeJson(
        doc,
        json
      );


      request->send(
        200,
        "application/json",
        json
      );
    }
  );


  // Start web server
  server.begin();

  Serial.println(
    "Web server started."
  );

  lcd.clear();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  // Automatic door locking
  if (
    lockRelayActive &&
    (millis() - unlockTime >= autoLockDelay)
  )
  {
    lockDoor();
  }


  // LCD update
  if (
    millis() - lastLCDUpdate >=
    lcdInterval
  )
  {
    lastLCDUpdate = millis();

    updateLCD();
  }
}
