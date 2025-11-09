# WiFi Manager

## Introduction

Necessary module to implement a solution in order to have a web server running inside the ESP32 board to display an interface asking for the WiFi network credentials.

## Requirements

- ESP32 device board
- WiFi credentials of the network we want to connect

## How it works

### Diagram

![alt text](assets/image.png)

- When the ESP first starts, it tries to read the ssid.txt, pass.txt and ip.txt files\* (1);

- If the files are empty (2) (the first time you run the board), your board is set as an access point (3);

- Using any Wi-Fi enabled device with a browser, you can connect to the newly created Access Point (default name ESP-WIFI-MANAGER);

- After establishing a connection with the ESP-WIFI-MANAGER, you can go to the default IP address 192.168.4.1 to open a web page that allows you to configure your SSID and password (4);

- The SSID, password, and IP address inserted in the form are saved in the corresponding files: ssid.txt, pass.txt, and ip.txt (5);

- After that, the ESP board restarts (6);

- This time, after restarting, the files are not empty, so the ESP will try to connect to the network in station mode using the settings you’ve inserted in the form (7);

- If it establishes a connection, the process is completed successfully, and you can access the main web server page that can do whatever you want (control sensor readings, control outputs, display some text, etc.) (9). Otherwise, it will set the Access Point (3), and you can access the default IP address (192.168.4.1) to add another SSID/password combination.

## Code

**⚠️ IMPORTANT:** The standard ESPAsyncWebServer library has compatibility issues with newer ESP32 Arduino Core versions. Follow the solution below to fix the compilation error.

### Fix for ESPAsyncWebServer Library Error

The error `ESPAsyncWebServer.h:1124:49: error: passing 'const AsyncServer' as 'this' argument discards qualifiers [-fpermissive]` occurs due to compatibility issues between the library and newer ESP32 cores.

**Solution 1: Use the Compatible Fork (Recommended)**

1. **Remove the standard ESPAsyncWebServer library** if you have it installed:

   - Go to **Sketch** > **Include Library** > **Manage Libraries**
   - Search for "ESPAsyncWebServer" and uninstall it

2. **Install the compatible fork instead:**

   - Download the fixed library from: `https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip`
   - Or use this alternative fork: `https://github.com/dvarrel/ESPAsyncWebSrv/archive/main.zip`

3. **Install manually:**
   - Go to **Sketch** > **Include Library** > **Add .ZIP Library**
   - Select the downloaded zip file

**Solution 2: Downgrade ESP32 Arduino Core (Alternative)**

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

### Main

1. Import the libraries

```arduino
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
```

4. Create the webserver at the specific port

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

11. Initialize the LittleFS functionality

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

13. Define the function to return the connection status

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

14. Define the processor function to replace the placeholder of the build in led board in the html web page

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

15. Define the setup function to init the board configuration and start the webserver

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

## Filesystem

Add Filesystem plugin to handle files inside the board in order to perform better the webserver WiFi Manager

1. Download the plugin, copy and paste the url and dow load the **.vsix** file

```
https://github.com/earlephilhower/arduino-littlefs-upload/releases
```

2. Download the .vsix file

3. Go to the your PC filesystem path, and create a new folder named **plugins**

```
C:\Users\IgorSicorschiGutu\.arduinoIDE
```

4. Validate the new plugin by opening Arduino IDE and type **Ctrl + Shift + P** to open the command palette and type Upload LittleFS to Pico/ESP8266/ESP32
