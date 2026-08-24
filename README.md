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
This project is released under the NoLicense. You are free to use, modify, and distribute the project with proper attribution.

👨‍💻 Author
RAJ Bari

🎓 Electronics and Telecommunication Engineering Student
💻 Embedded Systems | IoT | Machine Learning
🇮🇳 Maharashtra, India

⭐ If you find this project useful, consider giving the repository a star!
