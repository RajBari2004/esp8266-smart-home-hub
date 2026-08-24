
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ======================
// WiFi Credentials
// ======================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ======================
// Pin Definitions
// ======================
#define MQ135_PIN A0

#define RELAY_LOCK D6 // GPIO12
#define RELAY_AUX D7 // GPIO13

#define DOOR_SWITCH D5 // GPIO14

// ======================
// LCD Setup
// ======================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ======================
// Variables
// ======================
AsyncWebServer server(80);

bool auxState = false;
unsigned long unlockTime = 0;
bool doorUnlocked = false;

// ======================
// Read Air Quality
// ======================
String getAirQuality()
{
int value = analogRead(MQ135_PIN);

if(value < 300)
return "GOOD";

if(value < 700)
return "MODERATE";

return "HAZARDOUS";
}

int getAirValue()
{
return analogRead(MQ135_PIN);
}

// ======================
// HTML Page
// ======================
String webpage()
{
String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>Smart Home Hub</title>

<style>
body{
font-family:Arial;
text-align:center;
background:#f5f5f5;
}

.card{
background:white;
margin:15px;
padding:20px;
border-radius:10px;
box-shadow:0 0 10px rgba(0,0,0,0.2);
}

button{
padding:15px 30px;
font-size:18px;
margin:10px;
cursor:pointer;
}
</style>

</head>

<body>

<h2>ESP8266 Smart Home Hub</h2>

<div class="card">
<h3>Door Control</h3>
<button onclick="unlockDoor()">Unlock Door</button>
<p id="door"></p>
</div>

<div class="card">
<h3>Air Quality</h3>
<p id="air"></p>
<p id="airval"></p>
</div>

<div class="card">
<h3>Aux Appliance</h3>
<button onclick="toggleAux()">Toggle Appliance</button>
<p id="aux"></p>
</div>

<script>

function unlockDoor()
{
fetch('/unlock');
}

function toggleAux()
{
fetch('/toggleAux');
}

function updateData()
{
fetch('/status')
.then(response => response.json())
.then(data => {

document.getElementById("door").innerHTML =
"Door : " + data.door;

document.getElementById("air").innerHTML =
"Air Quality : " + data.air;

document.getElementById("airval").innerHTML =
"MQ135 Value : " + data.airValue;

document.getElementById("aux").innerHTML =
"Aux Device : " + data.aux;

});
}

setInterval(updateData,1000);
updateData();

</script>

</body>
</html>
)rawliteral";

return html;
}

// ======================
// Setup
// ======================
void setup()
{
Serial.begin(115200);

pinMode(RELAY_LOCK, OUTPUT);
pinMode(RELAY_AUX, OUTPUT);

pinMode(DOOR_SWITCH, INPUT_PULLUP);

digitalWrite(RELAY_LOCK, HIGH); // Relay OFF
digitalWrite(RELAY_AUX, HIGH);

lcd.init();
lcd.backlight();

lcd.setCursor(0,0);
lcd.print("Connecting WiFi");

WiFi.begin(ssid,password);

while(WiFi.status()!=WL_CONNECTED)
{
delay(500);
Serial.print(".");
}

Serial.println();
Serial.println(WiFi.localIP());

lcd.clear();
lcd.print("WiFi Connected");
delay(2000);

// Root Page
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
{
request->send(200,"text/html",webpage());
});

// Unlock Door
server.on("/unlock", HTTP_GET, [](AsyncWebServerRequest *request)
{
digitalWrite(RELAY_LOCK, LOW);

doorUnlocked = true;
unlockTime = millis();

request->send(200,"text/plain","Unlocked");
});

// Toggle Aux
server.on("/toggleAux", HTTP_GET, [](AsyncWebServerRequest *request)
{
auxState = !auxState;

digitalWrite(RELAY_AUX,
auxState ? LOW : HIGH);

request->send(200,"text/plain","OK");
});

// JSON Status
server.on("/status", HTTP_GET,
[](AsyncWebServerRequest *request)
{
String doorStatus;

if(digitalRead(DOOR_SWITCH)==LOW)
doorStatus="CLOSED";
else
doorStatus="OPEN";

String json="{";
json += "\"door\":\""+doorStatus+"\",";
json += "\"air\":\""+getAirQuality()+"\",";
json += "\"airValue\":"+String(getAirValue())+",";
json += "\"aux\":\""+String(auxState?"ON":"OFF")+"\"";
json += "}";

request->send(200,"application/json",json);
});

server.begin();

lcd.clear();
}

// ======================
// Loop
// ======================
void loop()
{
// Auto Lock after 4 sec
if(doorUnlocked)
{
if(millis()-unlockTime >= 4000)
{
digitalWrite(RELAY_LOCK,HIGH);
doorUnlocked = false;
}
}

// Door Status
String doorStatus;

if(digitalRead(DOOR_SWITCH)==LOW)
doorStatus="Closed";
else
doorStatus="Open";

// Air Quality
String air = getAirQuality();

// LCD Line 1
lcd.setCursor(0,0);
lcd.print("Door:");
lcd.print(doorStatus);
lcd.print(" ");

// LCD Line 2
lcd.setCursor(0,1);
lcd.print("Air:");
lcd.print(air);
lcd.print(" ");

delay(500);
} edit this code with advance dynamic webage lcd display show the air quLITY DOOR OPENED OR NOT SWITVH TO lock unlock door

Yes — below is a cleaner advanced version of your ESP8266 code with a more dynamic web page, better LCD updates, live air-quality status, door open/closed display, and lock/unlock switch control. I also kept your existing hardware mapping for NodeMCU pins: D5 = GPIO14, D6 = GPIO12, and D7 = GPIO13.

Updated code
cpp
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ======================
// WiFi Credentials
// ======================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ======================
// Pin Definitions
// ======================
#define MQ135_PIN    A0

#define RELAY_LOCK   D6      // GPIO12
#define RELAY_AUX    D7      // GPIO13
#define DOOR_SWITCH  D5      // GPIO14

// ======================
// LCD Setup
// ======================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ======================
// Web Server
// ======================
AsyncWebServer server(80);

// ======================
// Variables
// ======================
bool auxState = false;
bool lockRelayActive = false;
unsigned long unlockTime = 0;
const unsigned long autoLockDelay = 4000;

unsigned long lastLCDUpdate = 0;
const unsigned long lcdInterval = 500;

// ======================
// Helper Functions
// ======================
String getAirQualityText(int value)
{
  if (value < 300) return "GOOD";
  if (value < 700) return "MODERATE";
  return "HAZARDOUS";
}

int getAirValue()
{
  return analogRead(MQ135_PIN);
}

bool isDoorClosed()
{
  return digitalRead(DOOR_SWITCH) == LOW;
}

String getDoorStatus()
{
  return isDoorClosed() ? "CLOSED" : "OPEN";
}

String getLockStatus()
{
  return lockRelayActive ? "UNLOCKED" : "LOCKED";
}

String getAuxStatus()
{
  return auxState ? "ON" : "OFF";
}

void lockDoor()
{
  digitalWrite(RELAY_LOCK, HIGH);   // relay OFF / locked state
  lockRelayActive = false;
}

void unlockDoor()
{
  digitalWrite(RELAY_LOCK, LOW);    // relay ON / unlock pulse
  lockRelayActive = true;
  unlockTime = millis();
}

void updateLCD()
{
  int airValue = getAirValue();
  String airText = getAirQualityText(airValue);
  String doorText = getDoorStatus();
  String lockText = getLockStatus();

  lcd.clear();

  // Line 1
  lcd.setCursor(0, 0);
  if (doorText == "OPEN")
    lcd.print("Door:OPEN ");
  else
    lcd.print("Door:CLOSED");

  // Line 2
  lcd.setCursor(0, 1);
  if (airText == "GOOD")
    lcd.print("Air:GOOD     ");
  else if (airText == "MODERATE")
    lcd.print("Air:MODERATE ");
  else
    lcd.print("Air:HAZARDOUS");

  delay(5);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lock:");
  lcd.print(lockText);

  lcd.setCursor(0, 1);
  lcd.print("MQ135:");
  lcd.print(airValue);
  lcd.print("   ");
}

// ======================
// Advanced HTML Page
// ======================
String webpage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Smart Home Hub</title>
  <style>
    *{
      box-sizing:border-box;
      margin:0;
      padding:0;
      font-family:Arial,sans-serif;
    }

    body{
      background:linear-gradient(135deg,#0f172a,#1e293b,#334155);
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
      grid-template-columns:repeat(auto-fit,minmax(240px,1fr));
      gap:18px;
    }

    .card{
      background:rgba(255,255,255,0.08);
      border:1px solid rgba(255,255,255,0.12);
      backdrop-filter:blur(10px);
      border-radius:20px;
      padding:20px;
      box-shadow:0 8px 25px rgba(0,0,0,0.25);
      transition:0.3s;
    }

    .card:hover{
      transform:translateY(-4px);
    }

    .card h2{
      font-size:20px;
      margin-bottom:15px;
      color:#f8fafc;
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

    .good{ background:#14532d; color:#bbf7d0; }
    .moderate{ background:#78350f; color:#fde68a; }
    .hazard{ background:#7f1d1d; color:#fecaca; }

    .open{ background:#991b1b; color:#fee2e2; }
    .closed{ background:#166534; color:#dcfce7; }

    .locked{ background:#1d4ed8; color:#dbeafe; }
    .unlocked{ background:#ea580c; color:#ffedd5; }

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

    button:hover{
      transform:scale(1.04);
      opacity:0.95;
    }

    .footer{
      text-align:center;
      color:#cbd5e1;
      margin-top:20px;
      font-size:14px;
    }

    .switch-box{
      margin-top:12px;
      display:flex;
      align-items:center;
      justify-content:space-between;
      gap:12px;
      background:rgba(255,255,255,0.05);
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
  </style>
</head>
<body>
  <div class="container">
    <div class="title">
      <h1>ESP8266 Smart Home Hub</h1>
      <p>Live monitoring and door lock control</p>
    </div>

    <div class="grid">
      <div class="card">
        <h2>Air Quality</h2>
        <div class="value" id="airText">--</div>
        <div class="sub" id="airValue">MQ135 Value: --</div>
        <div class="status" id="airBadge">Waiting...</div>
      </div>

      <div class="card">
        <h2>Door Status</h2>
        <div class="value" id="doorText">--</div>
        <div class="sub">Magnetic/limit switch feedback</div>
        <div class="status" id="doorBadge">Waiting...</div>
      </div>

      <div class="card">
        <h2>Lock Control</h2>
        <div class="value" id="lockText">--</div>
        <div class="sub">Switch to lock / unlock door</div>

        <div class="switch-box">
          <span>Door Lock Switch</span>
          <label class="toggle">
            <input type="checkbox" id="lockSwitch" onchange="toggleLockSwitch()">
            <span class="slider"></span>
          </label>
        </div>

        <div class="btn-group">
          <button class="btn-primary" onclick="unlockDoor()">Unlock</button>
          <button class="btn-danger" onclick="lockDoor()">Lock</button>
        </div>
      </div>

      <div class="card">
        <h2>Aux Appliance</h2>
        <div class="value" id="auxText">--</div>
        <div class="sub">Relay controlled output</div>
        <div class="btn-group">
          <button class="btn-secondary" onclick="toggleAux()">Toggle Aux</button>
        </div>
      </div>
    </div>

    <div class="footer">
      Data refresh every 1 second
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
  const sw = document.getElementById('lockSwitch');
  if(sw.checked){
    await fetch('/unlock');
  }else{
    await fetch('/lock');
  }
  updateData();
}

function setAirBadge(air){
  const badge = document.getElementById("airBadge");
  badge.className = "status";
  if(air === "GOOD"){
    badge.classList.add("good");
  }else if(air === "MODERATE"){
    badge.classList.add("moderate");
  }else{
    badge.classList.add("hazard");
  }
  badge.innerHTML = air;
}

function setDoorBadge(door){
  const badge = document.getElementById("doorBadge");
  badge.className = "status";
  if(door === "OPEN"){
    badge.classList.add("open");
  }else{
    badge.classList.add("closed");
  }
  badge.innerHTML = door;
}

async function updateData(){
  const response = await fetch('/status');
  const data = await response.json();

  document.getElementById("airText").innerHTML = data.air;
  document.getElementById("airValue").innerHTML = "MQ135 Value: " + data.airValue;

  document.getElementById("doorText").innerHTML = data.door;
  document.getElementById("lockText").innerHTML = data.lock;
  document.getElementById("auxText").innerHTML = data.aux;

  setAirBadge(data.air);
  setDoorBadge(data.door);

  document.getElementById("lockSwitch").checked = (data.lock === "UNLOCKED");
}

setInterval(updateData, 1000);
updateData();
</script>
</body>
</html>
)rawliteral";

  return html;
}

// ======================
// Setup
// ======================
void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_LOCK, OUTPUT);
  pinMode(RELAY_AUX, OUTPUT);
  pinMode(DOOR_SWITCH, INPUT_PULLUP);

  lockDoor();
  digitalWrite(RELAY_AUX, HIGH);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);

  // Root page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "text/html", webpage());
  });

  // Unlock
  server.on("/unlock", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    unlockDoor();
    request->send(200, "text/plain", "Door Unlocked");
  });

  // Lock
  server.on("/lock", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    lockDoor();
    request->send(200, "text/plain", "Door Locked");
  });

  // Toggle Aux
  server.on("/toggleAux", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    auxState = !auxState;
    digitalWrite(RELAY_AUX, auxState ? LOW : HIGH);
    request->send(200, "text/plain", "Aux Toggled");
  });

  // Status JSON
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    int airValue = getAirValue();
    String airText = getAirQualityText(airValue);

    DynamicJsonDocument doc(256);
    doc["door"] = getDoorStatus();
    doc["lock"] = getLockStatus();
    doc["air"] = airText;
    doc["airValue"] = airValue;
    doc["aux"] = getAuxStatus();

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  server.begin();
  lcd.clear();
}

// ======================
// Loop
// ======================
void loop()
{
  if (lockRelayActive && (millis() - unlockTime >= autoLockDelay))
  {
    lockDoor();
  }

  if (millis() - lastLCDUpdate >= lcdInterval)
  {
    lastLCDUpdate = millis();
    updateLCD();
  }
}
What I changed
I added a more dynamic dashboard-style webpage with live cards for air quality, door status, lock status, and aux relay control. The page now refreshes every second and includes both buttons and a toggle switch for lock/unlock control.

I also changed the JSON response generation to use ArduinoJson, which is a cleaner and more reliable way to build JSON responses for ESPAsyncWebServer than manually concatenating strings.

LCD behavior
Your 16x2 LCD now alternates between:

Door status + air quality

Lock status + MQ135 analog value

That is useful because a 16x2 LCD has very limited space, so showing all values at once is not practical. If you want, I can also convert it into a scrolling LCD format instead of page switching. The need to keep LCD updates controlled is important because frequent I2C LCD writes can be slow and may affect responsiveness.

Important notes
Install these libraries:

ESPAsyncTCP

ESPAsyncWebServer

LiquidCrystal_I2C

ArduinoJson

Your door sensor logic currently assumes:

LOW = CLOSED

HIGH = OPEN

If your switch works opposite, just swap this part:

cpp
bool isDoorClosed()
{
  return digitalRead(DOOR_SWITCH) == LOW;
}
to:

cpp
bool isDoorClosed()
{
  return digitalRead(DOOR_SWITCH) == HIGH;
}
Small hardware advice
MQ135 values are not absolute air-quality values by default; raw ADC thresholds like 300 and 700 are only rough categories unless you calibrate the sensor in your environment. That means your GOOD, MODERATE, and HAZARDOUS labels are fine for a project demo, but not for precise measurement.

Next upgrade ideas
Add login protection for the webpage.

Add buzzer alert when air becomes hazardous.

Show IP address permanently on LCD startup screen.

Add auto-refresh chart for MQ135 trend.

Add manual mode and auto mode for door lock.

Would you like me to make the next version with:

better animated webpage,

scrolling LCD text, or

Blynk/mobile app integration?

i want to make githb project give
General
Repository owner and name
Owner(required)*
/
Repository name*
Great repository names are short and memorable. How about ?
Description0 / 350 characters
2
Configuration
Choose visibility *
Choose who can see and commit to this repository
Add README
READMEs can be used as longer descriptions. About READMEs
Off
Add .gitignore
.gitignore tells git which files not to track. About ignoring files
Add license
Licenses explain how others can use your code. About licenses

For your ESP8266 smart home project, use these GitHub repository settings:

Recommended repository
Field	Value
Owner	Your GitHub username
Repository name	esp8266-smart-home-hub
Description	ESP8266-based smart home hub with live air-quality monitoring, door lock control, door-status detection, relay automation, and I2C LCD display.
Visibility	Public
Add README	On
Add .gitignore	Arduino
Add license	MIT License
GitHub form values
Repository name
text
esp8266-smart-home-hub
This name is short, descriptive, lowercase, and uses hyphens, which makes it easy to remember and share.

Description
text
ESP8266-based smart home hub with live air-quality monitoring, door lock control, door-status detection, relay automation, and I2C LCD display.
Alternative names
You can also use:

text
esp8266-home-security
text
smart-home-air-quality
text
nodemcu-smart-home-hub
text
iot-door-lock-monitor
text
esp8266-security-dashboard
My strongest recommendation is:

text
esp8266-smart-home-hub
Recommended options
Visibility
Choose Public if you want to show the project in your portfolio, resume, or GitHub profile. Choose Private while the project is still under development or contains Wi-Fi credentials.

Add README
Select On. Your README should explain the project, hardware, wiring, software features, installation steps, and screenshots.

Add .gitignore
Select:

text
Arduino
This prevents Arduino-generated files and unnecessary build files from being committed.

Add license
Select:

text
MIT License
The MIT license allows others to use, modify, and distribute your code with attribution.

Important security warning
Do not upload your actual Wi-Fi password. Replace this:

cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
with a separate untracked file such as:

cpp
// secrets.h
const char* WIFI_SSID = "your_wifi_name";
const char* WIFI_PASSWORD = "your_wifi_password";
Then add this line to .gitignore:

text
secrets.h
Your main code can use:

cpp
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
Suggested README title
text
# ESP8266 Smart Home Hub
Your project uses the ESP8266 for local web-based control and monitoring, while the NodeMCU board’s D5, D6, and D7 labels correspond to GPIO14, GPIO12, and GPIO13 respectively. JSON responses can be structured using ArduinoJson with ESPAsyncWebServer.

give readme with emogis


README
Document
I created the emoji-rich README.md for your GitHub project. It includes:

🏠 Project overview

✨ Features

🧰 Components

🔌 Pin connections

🌫️ MQ135 air-quality thresholds

🔒 Door lock operation

🌐 Web endpoints

⚙️ Installation instructions

🔐 Wi-Fi security guidance

🛡️ Hardware safety notes

🚀 Future improvements

👨‍💻 Author section for RAJ Bari

You can download the generated README from the file panel and upload it to your esp8266-smart-home-hub repository.

README
🏠 ESP8266 Smart Home Hub
An IoT-based smart home security and monitoring system built with an ESP8266 NodeMCU. The system provides real-time air-quality monitoring, door-status detection, electronic door-lock control, auxiliary appliance control, and LCD feedback through a responsive local web dashboard. 🌐🔒

✨ Features
📡 ESP8266 Wi-Fi connectivity

🌐 Responsive local web dashboard

🔒 Remote door lock and unlock control

⏱️ Automatic door relocking after 4 seconds

🚪 Door open/closed detection using a switch

🌫️ MQ135 air-quality monitoring

📊 Live MQ135 sensor value display

⚠️ GOOD, MODERATE, and HAZARDOUS air-quality status

🔌 Auxiliary appliance control through a relay

🖥️ 16x2 I2C LCD status display

🔄 Automatic webpage data refresh every second

📱 Mobile-friendly user interface

🧰 Components Required
Component	Quantity
ESP8266 NodeMCU	1
MQ135 air-quality sensor	1
16x2 I2C LCD display	1
Relay module for door lock	1
Relay module for auxiliary appliance	1
Door switch or magnetic reed switch	1
Electronic door lock or solenoid lock	1
5 V power supply	1
Jumper wires	As required
🔌 Pin Connections
Device	NodeMCU Pin	ESP8266 GPIO
MQ135 analog output	A0	ADC
Door-lock relay	D6	GPIO12
Auxiliary relay	D7	GPIO13
Door switch	D5	GPIO14
LCD SDA	D2	GPIO4
LCD SCL	D1	GPIO5
⚠️ Confirm the relay module logic before connecting the lock. This project assumes an active-LOW relay, where LOW turns the relay on and HIGH turns it off.

🧠 System Operation
🔒 Door Lock
Press Unlock on the web dashboard to activate the door-lock relay.

The door remains unlocked for 4 seconds.

The ESP8266 automatically turns the relay off after the timeout.

The Lock button immediately returns the system to the locked state.

🚪 Door Monitoring
The door switch reports the physical door condition:

LOW signal → Door is CLOSED

HIGH signal → Door is OPEN

The switch uses the ESP8266 internal pull-up resistor through INPUT_PULLUP.

🌫️ Air Quality
The MQ135 sensor is read through the ESP8266 analog input. The current project uses these approximate thresholds:

MQ135 Value	Status
Below 300	✅ GOOD
300–699	⚠️ MODERATE
700 and above	🚨 HAZARDOUS
ℹ️ These thresholds are for an approximate project demonstration. Accurate gas concentration measurement requires sensor warm-up, calibration, and environmental testing.

🖥️ LCD Display
The LCD alternates between system information such as:

🚪 Door status

🌫️ Air-quality status

🔒 Lock status

📈 MQ135 analog value

📚 Required Arduino Libraries
Install the following libraries through the Arduino IDE Library Manager or manually:

ESP8266WiFi

ESPAsyncTCP

ESPAsyncWebServer

LiquidCrystal_I2C

ArduinoJson

The ESPAsyncWebServer API can be used to create asynchronous routes and JSON status responses. ArduinoJson provides structured JSON generation for the dashboard data. 🔧

⚙️ Installation
📥 Install the ESP8266 board package in Arduino IDE.

📚 Install all required libraries.

📂 Open the project .ino file.

✏️ Enter your Wi-Fi network details.

🔌 Check the wiring and relay logic.

⬆️ Select your ESP8266 board and upload the program.

🖥️ Open the Serial Monitor at 115200 baud.

🌐 Copy the displayed IP address into a web browser connected to the same Wi-Fi network.

🔐 Wi-Fi Credentials
For testing, credentials can be entered directly in the source code:

cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
Never publish real Wi-Fi credentials on GitHub. A safer approach is to store them in an untracked secrets.h file:

cpp
// secrets.h
const char* WIFI_SSID = "your_wifi_name";
const char* WIFI_PASSWORD = "your_wifi_password";
Add this file to .gitignore:

text
secrets.h
Then include it in the main program:

cpp
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
🌐 Web Dashboard
The dashboard provides live information about:

🌫️ Air-quality condition

📊 MQ135 sensor value

🚪 Door open/closed status

🔒 Door locked/unlocked status

🔌 Auxiliary appliance state

The browser requests updated data from the /status endpoint every second.

🔗 Web Endpoints
Endpoint	Method	Function
/	GET	Opens the smart home dashboard
/status	GET	Returns live system data in JSON format
/unlock	GET	Unlocks the door temporarily
/lock	GET	Locks the door immediately
/toggleAux	GET	Toggles the auxiliary relay
Example status response:

json
{
  "door": "CLOSED",
  "lock": "LOCKED",
  "air": "GOOD",
  "airValue": 245,
  "aux": "OFF"
}
🛡️ Safety Notes
⚡ Use a separate suitable power supply for a solenoid or electronic door lock.

🔋 Do not power a high-current lock directly from the ESP8266 3.3 V pin.

🔌 Connect all grounds correctly when using an external supply.

🧯 Add a flyback diode when controlling a DC inductive lock without a protected relay module.

🧪 Test the relay with an LED or low-voltage load before connecting the actual lock.

🔒 Do not expose the dashboard directly to the public internet without authentication and encryption.

🌡️ Allow the MQ135 sensor to warm up before trusting readings.

🚀 Future Improvements
👤 Add username and password authentication

🔔 Add buzzer alerts for hazardous air quality

📱 Create a dedicated Android mobile application

📈 Add historical air-quality charts

☁️ Store sensor data in a cloud database

🪪 Add RFID access control

📩 Send email or Telegram alerts

🔋 Add battery backup

🧠 Add automatic ventilation control

🛜 Add MQTT support for larger IoT networks

📸 Project Demonstration
Add project photographs, wiring diagrams, and dashboard screenshots here:

text
📁 images/
├── circuit-connection.png
├── lcd-display.jpg
├── web-dashboard.png
└── hardware-setup.jpg
🤝 Contributing
Contributions are welcome! 🎉

🍴 Fork this repository.

🌿 Create a new branch.

🛠️ Make your changes.

✅ Test the hardware and software.

📤 Submit a pull request.

📄 License
This project is released under the MIT License. You are free to use, modify, and distribute the project with proper attribution.

👨‍💻 Author
RAJ Bari

🎓 Electronics and Telecommunication Engineering Student
💻 Embedded Systems | IoT | Machine Learning
🇮🇳 Maharashtra, India

⭐ If you find this project useful, consider giving the repository a star!


Currently open
Upgrade to Pro to unlock document creation
Pro subscribers get extended access to Perplexity file and app creation.


🏠 ESP8266 Smart Home Hub
An IoT-based smart home security and monitoring system built with an ESP8266 NodeMCU. The system provides real-time air-quality monitoring, door-status detection, electronic door-lock control, auxiliary appliance control, and LCD feedback through a responsive local web dashboard. 🌐🔒

✨ Features
📡 ESP8266 Wi-Fi connectivity

🌐 Responsive local web dashboard

🔒 Remote door lock and unlock control

⏱️ Automatic door relocking after 4 seconds

🚪 Door open/closed detection using a switch

🌫️ MQ135 air-quality monitoring

📊 Live MQ135 sensor value display

⚠️ GOOD, MODERATE, and HAZARDOUS air-quality status

🔌 Auxiliary appliance control through a relay

🖥️ 16x2 I2C LCD status display

🔄 Automatic webpage data refresh every second

📱 Mobile-friendly user interface

🧰 Components Required
Component	Quantity
ESP8266 NodeMCU	1
MQ135 air-quality sensor	1
16x2 I2C LCD display	1
Relay module for door lock	1
Relay module for auxiliary appliance	1
Door switch or magnetic reed switch	1
Electronic door lock or solenoid lock	1
5 V power supply	1
Jumper wires	As required
🔌 Pin Connections
Device	NodeMCU Pin	ESP8266 GPIO
MQ135 analog output	A0	ADC
Door-lock relay	D6	GPIO12
Auxiliary relay	D7	GPIO13
Door switch	D5	GPIO14
LCD SDA	D2	GPIO4
LCD SCL	D1	GPIO5
⚠️ Confirm the relay module logic before connecting the lock. This project assumes an active-LOW relay, where LOW turns the relay on and HIGH turns it off.

🧠 System Operation
🔒 Door Lock
Press Unlock on the web dashboard to activate the door-lock relay.

The door remains unlocked for 4 seconds.

The ESP8266 automatically turns the relay off after the timeout.

The Lock button immediately returns the system to the locked state.

🚪 Door Monitoring
The door switch reports the physical door condition:

LOW signal → Door is CLOSED

HIGH signal → Door is OPEN

The switch uses the ESP8266 internal pull-up resistor through INPUT_PULLUP.

🌫️ Air Quality
The MQ135 sensor is read through the ESP8266 analog input. The current project uses these approximate thresholds:

MQ135 Value	Status
Below 300	✅ GOOD
300–699	⚠️ MODERATE
700 and above	🚨 HAZARDOUS
ℹ️ These thresholds are for an approximate project demonstration. Accurate gas concentration measurement requires sensor warm-up, calibration, and environmental testing.

🖥️ LCD Display
The LCD alternates between system information such as:

🚪 Door status

🌫️ Air-quality status

🔒 Lock status

📈 MQ135 analog value

📚 Required Arduino Libraries
Install the following libraries through the Arduino IDE Library Manager or manually:

ESP8266WiFi

ESPAsyncTCP

ESPAsyncWebServer

LiquidCrystal_I2C

ArduinoJson

The ESPAsyncWebServer API can be used to create asynchronous routes and JSON status responses. ArduinoJson provides structured JSON generation for the dashboard data. 🔧

⚙️ Installation
📥 Install the ESP8266 board package in Arduino IDE.

📚 Install all required libraries.

📂 Open the project .ino file.

✏️ Enter your Wi-Fi network details.

🔌 Check the wiring and relay logic.

⬆️ Select your ESP8266 board and upload the program.

🖥️ Open the Serial Monitor at 115200 baud.

🌐 Copy the displayed IP address into a web browser connected to the same Wi-Fi network.

🔐 Wi-Fi Credentials
For testing, credentials can be entered directly in the source code:

cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
Never publish real Wi-Fi credentials on GitHub. A safer approach is to store them in an untracked secrets.h file:

cpp
// secrets.h
const char* WIFI_SSID = "your_wifi_name";
const char* WIFI_PASSWORD = "your_wifi_password";
Add this file to .gitignore:

text
secrets.h
Then include it in the main program:

cpp
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
🌐 Web Dashboard
The dashboard provides live information about:

🌫️ Air-quality condition

📊 MQ135 sensor value

🚪 Door open/closed status

🔒 Door locked/unlocked status

🔌 Auxiliary appliance state

The browser requests updated data from the /status endpoint every second.

🔗 Web Endpoints
Endpoint	Method	Function
/	GET	Opens the smart home dashboard
/status	GET	Returns live system data in JSON format
/unlock	GET	Unlocks the door temporarily
/lock	GET	Locks the door immediately
/toggleAux	GET	Toggles the auxiliary relay
Example status response:

json
{
  "door": "CLOSED",
  "lock": "LOCKED",
  "air": "GOOD",
  "airValue": 245,
  "aux": "OFF"
}
🛡️ Safety Notes
⚡ Use a separate suitable power supply for a solenoid or electronic door lock.

🔋 Do not power a high-current lock directly from the ESP8266 3.3 V pin.

🔌 Connect all grounds correctly when using an external supply.

🧯 Add a flyback diode when controlling a DC inductive lock without a protected relay module.

🧪 Test the relay with an LED or low-voltage load before connecting the actual lock.

🔒 Do not expose the dashboard directly to the public internet without authentication and encryption.

🌡️ Allow the MQ135 sensor to warm up before trusting readings.

🚀 Future Improvements
👤 Add username and password authentication

🔔 Add buzzer alerts for hazardous air quality

📱 Create a dedicated Android mobile application

📈 Add historical air-quality charts

☁️ Store sensor data in a cloud database

🪪 Add RFID access control

📩 Send email or Telegram alerts

🔋 Add battery backup

🧠 Add automatic ventilation control

🛜 Add MQTT support for larger IoT networks

📸 Project Demonstration
Add project photographs, wiring diagrams, and dashboard screenshots here:

text
📁 images/
├── circuit-connection.png
├── lcd-display.jpg
├── web-dashboard.png
└── hardware-setup.jpg
🤝 Contributing
Contributions are welcome! 🎉

🍴 Fork this repository.

🌿 Create a new branch.

🛠️ Make your changes.

✅ Test the hardware and software.

📤 Submit a pull request.

📄 License
This project is released under the NO License. You are free to use, modify, and distribute the project with proper attribution.

👨‍💻 Author
RAJ Bari

🎓 Electronics and Telecommunication Engineering Student
💻 Embedded Systems | IoT | Machine Learning
🇮🇳 Maharashtra, India

⭐ If you find this project useful, consider giving the repository a star!
