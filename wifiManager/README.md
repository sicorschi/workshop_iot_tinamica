# WiFi Manager - ESP32 Web Configuration

## Table of Contents

1. [Introduction](#introduction)
2. [Requirements](#requirements)
3. [How It Works](#how-it-works)
   - [Process Flow Diagram](#process-flow-diagram)
   - [Step-by-Step Process](#step-by-step-process)
4. [Library Setup](#library-setup)
   - [ESPAsyncWebServer Fix](#espasyncwebserver-fix)
   - [Required Libraries](#required-libraries)
   - [Troubleshooting](#troubleshooting)
5. [Code Implementation](#code-implementation)
   - [Library Imports](#library-imports)
   - [Variable Declarations](#variable-declarations)
   - [File System Functions](#file-system-functions)
   - [WiFi Connection Functions](#wifi-connection-functions)
   - [Web Server Setup](#web-server-setup)
6. [File System Plugin](#file-system-plugin)
   - [Installation](#installation)
   - [Usage](#usage)
7. [Testing & Deployment](#testing--deployment)

---

## Introduction

The WiFi Manager is a crucial module that implements a web-based configuration system for ESP32 devices. This solution allows you to set up WiFi credentials without hardcoding them into your firmware, making your IoT devices more flexible and deployment-ready.

**Key Features:**

- ✅ Web-based WiFi configuration
- ✅ Access Point mode for initial setup
- ✅ Persistent storage using LittleFS
- ✅ Automatic connection retry mechanism
- ✅ LED status indication
- ✅ Simple web interface for device control

## Requirements

### Hardware

- **ESP32 development board** (DEVKIT V1 or compatible)
- **USB cable** for programming and power
- **Computer** with Arduino IDE installed

### Software

- **Arduino IDE** (version 2.0 or later)
- **ESP32 Arduino Core**
- **LittleFS file system support**
- **Compatible web browser** for configuration

### Network

- **WiFi network** you want to connect the ESP32 to
- **Network credentials** (SSID and password)

## How It Works

### Process Flow Diagram

![WiFi Manager Flow](assets/image.png)

### Step-by-Step Process

The WiFi Manager follows a smart configuration flow:

- When the ESP first starts, it tries to read the ssid.txt, pass.txt and ip.txt files\* (1);

- If the files are empty (2) (the first time you run the board), your board is set as an access point (3);

- Using any Wi-Fi enabled device with a browser, you can connect to the newly created Access Point (default name ESP-WIFI-MANAGER);

- After establishing a connection with the ESP-WIFI-MANAGER, you can go to the default IP address 192.168.4.1 to open a web page that allows you to configure your SSID and password (4);

- The SSID, password, and IP address inserted in the form are saved in the corresponding files: ssid.txt, pass.txt, and ip.txt (5);

- After that, the ESP board restarts (6);

- This time, after restarting, the files are not empty, so the ESP will try to connect to the network in station mode using the settings you’ve inserted in the form (7);

- If it establishes a connection, the process is completed successfully, and you can access the main web server page that can do whatever you want (control sensor readings, control outputs, display some text, etc.) (9). Otherwise, it will set the Access Point (3), and you can access the default IP address (192.168.4.1) to add another SSID/password combination.

## Library Setup

### ESPAsyncWebServer Fix

**⚠️ IMPORTANT:** The standard ESPAsyncWebServer library has compatibility issues with newer ESP32 Arduino Core versions. Follow the solution below to fix the compilation error.

#### Common Error

The error `ESPAsyncWebServer.h:1124:49: error: passing 'const AsyncServer' as 'this' argument discards qualifiers [-fpermissive]` occurs due to compatibility issues between the library and newer ESP32 cores.

#### Solution 1: Use Compatible Fork (Recommended)

1. **Remove the standard ESPAsyncWebServer library** if you have it installed:

   - Go to **Sketch** > **Include Library** > **Manage Libraries**
   - Search for "ESPAsyncWebServer" and uninstall it

2. **Install the compatible fork instead:**

   - Download the fixed library from: `https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip`
   - Or use this alternative fork: `https://github.com/dvarrel/ESPAsyncWebSrv/archive/main.zip`

3. **Install manually:**
   - Go to **Sketch** > **Include Library** > **Add .ZIP Library**
   - Select the downloaded zip file

#### Solution 2: Downgrade ESP32 Arduino Core (Alternative)

If you prefer to keep the standard library:

1. Go to **Tools** > **Board** > **Boards Manager**
2. Search for "ESP32"
3. Downgrade to version **2.0.9** or earlier
4. Restart Arduino IDE

### Required Libraries

1. **ESPAsyncWebServer** (using the compatible fork above)
2. **AsyncTCP** (dependency for ESPAsyncWebServer)

```arduino
// Install AsyncTCP first
// Download from: https://github.com/me-no-dev/AsyncTCP/archive/master.zip
```

### Troubleshooting

**If you still get compilation errors:**

1. **Check ESP32 Arduino Core Version:**

   - Go to **Tools** > **Board** > **Boards Manager**
   - Search for "ESP32 by Espressif Systems"
   - Use version **2.0.9** or **2.0.11** for best compatibility

2. **Verify Library Installation:**

   - Make sure both AsyncTCP and ESPAsyncWebServer are installed
   - Check that you're using the compatible fork, not the original library

3. **Alternative Libraries:**
   If issues persist, you can use these alternatives:
   - **WebServer.h** (built-in, synchronous)
   - **WiFiManager.h** (dedicated WiFi management library)

## Code Implementation

Now let's implement the WiFi Manager step by step.

### Library Imports

1. **Import the required libraries**

```arduino
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
```

### Variable Declarations

2. **Create the web server instance**

```arduino
AsyncWebServer server(80);
```

5. Create the webserver request variables to search in the HTTP Request

```arduino
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
```

6. Define the variables to save the values from the HTML form

```arduino
String ssid;
String pass;
String ip;
String gateway;
```

7. Create the file paths to store the values from the user input

```arduino
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";
```

8. Declare the station IP

```arduino
IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);
```

9. Define the timer interval variables

```arduino
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
```

10. Define the ESP32 led build in pin (2). Also define the variable to store the state of the current led value

```arduino
const int ledPin = 2;
String ledState;
```

### File System Functions

11. **Initialize LittleFS functionality**

```arduino
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}
```

12. Define the function to write file to the LittleFS

```arduino
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}
```

### WiFi Connection Functions

13. **Define the WiFi connection function**

```arduino
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }
  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());
  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }
  Serial.println(WiFi.localIP());
  return true;
}
```

### Web Server Setup

14. **Create the HTML processor function**

```arduino
String processor(const String& var) {
  if(var == "STATE") {
    if(digitalRead(ledPin)) {
      ledState = "ON";
    }
    else {
      ledState = "OFF";
    }
    return ledState;
  }
  return String();
}
```

15. **Main setup function - Initialize board and start web server**

```arduino
void setup() {
  Serial.begin(115200);
  initLittleFS();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);
  if(initWiFi()) {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    server.serveStatic("/", LittleFS, "/");
    // Route to set GPIO state to HIGH
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, HIGH);
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });

    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, LOW);
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    server.begin();
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    server.serveStatic("/", LittleFS, "/");
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(LittleFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(LittleFS, gatewayPath, gateway.c_str());
          }
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
  }
}
```

**⚠️ IMPORTANT:** There is no need to define anything inside the loop function, since everything is processed in the setup main function, but is important to define the function even if it's empty.

## File System Plugin

### Installation

To handle files efficiently on the ESP32, you need to install the LittleFS upload plugin for Arduino IDE.

### Setup Process

1. **Download the LittleFS plugin**

```
https://github.com/earlephilhower/arduino-littlefs-upload/releases
```

2. Download the .vsix file

3. Go to the your PC filesystem path, and create a new folder named **plugins**

```
C:\Users\IgorSicorschiGutu\.arduinoIDE
```

4. **Validate the plugin installation**
   - Open Arduino IDE and press **Ctrl + Shift + P**
   - Type "Upload LittleFS" to verify the plugin is available
   - The command "Upload LittleFS to Pico/ESP8266/ESP32" should appear

## Testing & Deployment

### First Time Setup

1. **Upload the code** to your ESP32
2. **Upload the data folder** using LittleFS plugin
3. **Open Serial Monitor** to see the boot process
4. **Connect to ESP-WIFI-MANAGER** WiFi network
5. **Navigate to 192.168.4.1** in your browser
6. **Enter your WiFi credentials** and IP settings
7. **Wait for ESP32 to restart** and connect to your network

### Normal Operation

- ESP32 will automatically connect to saved WiFi network
- Access the device using the configured IP address
- Control the built-in LED via web interface
- If connection fails, it will revert to AP mode for reconfiguration

### Troubleshooting

- **Can't see ESP-WIFI-MANAGER**: Check if ESP32 is in AP mode
- **Connection fails**: Verify WiFi credentials and signal strength
- **IP conflicts**: Use a different static IP address
- **Upload errors**: Check USB cable and drivers

---

**📝 Note:** This WiFi Manager provides a foundation for any ESP32 IoT project requiring flexible network configuration!
