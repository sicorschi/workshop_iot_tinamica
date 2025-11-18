# Smart Lights - Automated IoT Lighting System

## Table of Contents

1. [Introduction](#introduction)
2. [Features](#features)
3. [Requirements](#requirements)
   - [Hardware Requirements](#hardware-requirements)
   - [Software Requirements](#software-requirements)
4. [Hardware Setup](#hardware-setup)
   - [Component Connections](#component-connections)
   - [Wiring Diagram](#wiring-diagram)
5. [Code Implementation](#code-implementation)
   - [Library Imports](#library-imports)
   - [Pin Definitions](#pin-definitions)
   - [Web Server Interface](#web-server-interface)
   - [MQTT Communication](#mqtt-communication)
   - [Presence Detection](#presence-detection)
6. [Configuration](#configuration)
   - [WiFi Setup](#wifi-setup)
   - [MQTT Broker Settings](#mqtt-broker-settings)
7. [Usage](#usage)
   - [Web Interface Control](#web-interface-control)
   - [MQTT Control](#mqtt-control)
   - [Automatic Detection](#automatic-detection)
8. [Testing & Deployment](#testing--deployment)
9. [Troubleshooting](#troubleshooting)

---

## Introduction

The Smart Lights project demonstrates an intelligent lighting system that combines multiple control methods for energy-efficient smart city applications. This system automatically detects presence using an ultrasonic sensor and provides both web-based and MQTT remote control capabilities.

**Project Goals:**

- ✅ Automated presence-based lighting control
- ✅ Remote control via web interface
- ✅ MQTT integration for smart city networks
- ✅ Real-time status monitoring
- ✅ Energy-efficient operation

## Features

### 🔧 **Control Methods**

- **Automatic Detection**: HC-SR04 sensor triggers lights when presence detected
- **Web Interface**: Browser-based control with toggle switches
- **MQTT Commands**: Remote control via publish/subscribe messaging
- **Manual Override**: Direct web server control

### 📊 **Monitoring**

- **Real-time Status**: Live relay state publishing via MQTT
- **Distance Readings**: Continuous sensor data monitoring
- **Web Dashboard**: Visual interface for system status

### 🏗️ **Smart City Integration**

- **MQTT Protocol**: Industry-standard IoT communication
- **Scalable Design**: Multiple relay support
- **Network Connectivity**: WiFi-enabled for cloud integration

## Requirements

### Hardware Requirements

#### Core Components

- **ESP32 DEVKIT V1** or compatible microcontroller
- **HC-SR04 Ultrasonic Sensor** for presence detection
- **5V Relay Module** (optocoupler isolated recommended)
- **LED Strip/Light Bulb** or electrical load to control

#### Connections & Power

- **Breadboard** and **jumper wires** for prototyping
- **USB Cable** for programming and power
- **External Power Supply** (if controlling high-power loads)
- **Resistors**: 220Ω, 1kΩ (if needed for level shifting)

#### Optional Components

- **Enclosure** for protection in outdoor deployment
- **Power regulator** for stable 5V supply
- **Terminal blocks** for permanent installations

### Software Requirements

#### Development Environment

- **Arduino IDE** (version 2.0 or later)
- **ESP32 Arduino Core** (latest stable version)
- **CP210x USB Drivers** for ESP32 communication

#### Required Libraries

```cpp
#include "WiFi.h"           // ESP32 WiFi functionality
#include "ESPAsyncWebServer.h"  // Async web server
#include <PubSubClient.h>   // MQTT client
#include <Wire.h>           // I2C communication (if needed)
```

#### Network Infrastructure

- **WiFi Router** with internet access
- **MQTT Broker** (Raspberry Pi or cloud service)
- **Static IP Configuration** (recommended)

## Hardware Setup

### Component Connections

#### HC-SR04 Ultrasonic Sensor

| HC-SR04 Pin | ESP32 Pin | Description    |
| ----------- | --------- | -------------- |
| VCC         | 5V        | Power supply   |
| GND         | GND       | Ground         |
| Trig        | GPIO 5    | Trigger signal |
| Echo        | GPIO 18   | Echo response  |

#### Relay Module

| Relay Pin | ESP32 Pin | Description           |
| --------- | --------- | --------------------- |
| VCC       | 5V        | Power supply          |
| GND       | GND       | Ground                |
| IN        | GPIO 26   | Control signal        |
| COM       | AC/DC     | Common terminal       |
| NO        | Load      | Normally Open contact |

### Wiring Diagram

```
ESP32 DEVKIT V1
     ┌─────────────────┐
     │                 │
     │  GPIO 5  ───────┼─── HC-SR04 Trig
     │  GPIO 18 ───────┼─── HC-SR04 Echo
     │  GPIO 26 ───────┼─── Relay IN
     │                 │
     │  5V      ───────┼─── HC-SR04 VCC, Relay VCC
     │  GND     ───────┼─── Common Ground
     │                 │
     └─────────────────┘

HC-SR04 Sensor              Relay Module
┌─────────────┐             ┌─────────────┐
│ VCC  Trig   │             │ VCC  IN     │
│ GND  Echo   │             │ GND  COM NO │
└─────────────┘             └─────────────┘
                                    │
                            ┌───────┴───────┐
                            │   Light/Load  │
                            └───────────────┘
```

⚠️ **Safety Warning**: When controlling AC loads, ensure proper electrical isolation and follow local electrical codes.

## Code Implementation

### Library Imports

The project uses several libraries for different functionalities:

```cpp
#include "WiFi.h"                // WiFi connectivity
#include "ESPAsyncWebServer.h"   // Asynchronous web server
#include <PubSubClient.h>        // MQTT communication
#include <Wire.h>                // I2C protocol support
```

### Pin Definitions

Key hardware definitions and constants:

```cpp
// Sensor configuration
#define SOUND_SPEED 0.034        // Speed of sound in cm/μs
#define RELAY_NO false           // Relay type (false = NC, true = NO)
#define NUM_RELAYS 1             // Number of relays to control

// Pin assignments
const int trigPin = 5;           // HC-SR04 trigger pin
const int echoPin = 18;          // HC-SR04 echo pin
const int relay = 26;            // Relay control pin
int relayGPIOs[NUM_RELAYS] = {26}; // Relay GPIO array

// Network configuration
const char* ssid = "";           // WiFi network name
const char* password = "";       // WiFi password
const char* mqtt_server = "192.168.0.18"; // MQTT broker IP
```

### Web Server Interface

The system provides a responsive web interface with toggle switches:

```cpp
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px}
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Smart Lights Control</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
</body>
</html>
)rawliteral";
```

### MQTT Communication

MQTT enables remote control and status monitoring:

```cpp
// MQTT Topics
// Subscribe: "esp32/light" (receives on/off commands)
// Publish: "esp32/light/status" (sends current relay state)

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  if (String(topic) == "esp32/light") {
    if(messageTemp == "on"){
      digitalWrite(relay, HIGH);
      Serial.println("Lights ON!");
    }
    else if(messageTemp == "off"){
      digitalWrite(relay, LOW);
      Serial.println("Lights OFF!");
    }
  }
}
```

### Presence Detection

Automatic lighting control based on proximity:

```cpp
void loop() {
  // Ultrasonic sensor reading
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;

  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  // Automatic lighting trigger
  if (distanceCm < 5) {  // Presence detected within 5cm
    digitalWrite(relay, HIGH);
    Serial.println("Lights ON!");
  }

  // MQTT status publishing every 5 seconds
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    int relayValue = digitalRead(26);
    char relayString[8];
    dtostrf(relayValue, 1, 2, relayString);
    client.publish("esp32/light/status", relayString);
  }

  delay(1000);
}
```

## Configuration

### WiFi Setup

1. **Configure Network Credentials**:

```cpp
const char* ssid = "YourWiFiNetwork";
const char* password = "YourWiFiPassword";
```

2. **Set Static IP** (optional but recommended):

```cpp
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
```

### MQTT Broker Settings

Configure the MQTT broker connection:

```cpp
const char* mqtt_server = "192.168.1.18";  // Your MQTT broker IP
const int mqtt_port = 1883;                 // Standard MQTT port
const char* mqtt_user = "";                 // Username (if required)
const char* mqtt_password = "";             // Password (if required)
```

## Usage

### Web Interface Control

1. **Access the Interface**:

   - Connect to the same WiFi network as the ESP32
   - Open browser and navigate to the ESP32's IP address
   - Use the toggle switch to control lights manually

2. **Interface Features**:
   - **Real-time Status**: Shows current relay state
   - **Responsive Design**: Works on mobile devices
   - **Instant Control**: Toggle lights on/off

### MQTT Control

**Control Commands**:

```bash
# Turn lights ON
mosquitto_pub -h [broker_ip] -t "esp32/light" -m "on"

# Turn lights OFF
mosquitto_pub -h [broker_ip] -t "esp32/light" -m "off"

# Monitor status
mosquitto_sub -h [broker_ip] -t "esp32/light/status"
```

### Automatic Detection

- **Detection Range**: Objects within 5cm trigger the lights
- **Instant Response**: Lights turn on immediately when presence detected
- **Adjustable Threshold**: Modify the distance value in code as needed

## Testing & Deployment

### Initial Testing

1. **Upload Code**: Flash the firmware to ESP32
2. **Serial Monitor**: Check connection status and sensor readings
3. **Network Test**: Verify WiFi connection and IP assignment
4. **Sensor Test**: Wave hand near HC-SR04 sensor
5. **Web Test**: Access web interface and test manual control
6. **MQTT Test**: Send commands via MQTT client

### Deployment Checklist

- [ ] **Power Supply**: Ensure stable 5V supply for sensors
- [ ] **Network Connectivity**: Verify WiFi signal strength
- [ ] **Sensor Positioning**: Mount HC-SR04 at appropriate height
- [ ] **Relay Rating**: Confirm relay can handle your load
- [ ] **Electrical Safety**: Proper grounding and isolation
- [ ] **Weatherproofing**: Protect components if deployed outdoors

## Troubleshooting

### Common Issues

**🔧 Lights Not Responding**:

- Check relay wiring and power supply
- Verify GPIO pin connections
- Test relay with multimeter
- Confirm relay rating matches load

**📡 WiFi Connection Problems**:

- Verify SSID and password
- Check signal strength
- Restart ESP32 and router
- Try different WiFi channel

**🔍 Sensor Not Detecting**:

- Check HC-SR04 power supply (5V required)
- Verify trigger and echo pin connections
- Test with simple distance measurement code
- Ensure sensor faces detection area

**📱 Web Interface Issues**:

- Confirm ESP32 IP address
- Check firewall settings
- Try different browser
- Clear browser cache

**📨 MQTT Communication Fails**:

- Verify MQTT broker is running
- Check broker IP address and port
- Test broker with external MQTT client
- Confirm network connectivity

### Debug Commands

**Serial Monitor Debug**:

```cpp
Serial.print("Distance (cm): ");
Serial.println(distanceCm);
Serial.println("WiFi connected!");
Serial.println(WiFi.localIP());
```

**MQTT Test Commands**:

```bash
# Test broker connection
mosquitto_pub -h [broker_ip] -t "test" -m "hello"

# Monitor all topics
mosquitto_sub -h [broker_ip] -t "#"
```

---

## Integration with Smart City

This Smart Lights system can be integrated into larger smart city infrastructures:

- **Multiple Units**: Deploy several units with different MQTT topics
- **Central Control**: Use Node-RED for dashboard and automation
- **Data Analytics**: Collect usage patterns and optimize energy consumption
- **Emergency Systems**: Integrate with security and emergency response
- **Environmental Sensors**: Add weather data for intelligent control

---

**🌟 Happy Building!** Your automated lighting system is ready to make your space smarter and more energy-efficient!
