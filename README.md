# IoT Workshop - Smart City Exercises

## Table of Contents

1. [Introduction](#introduction)
2. [Requirements](#requirements)
   - [Hardware Requirements](#hardware-requirements)
   - [Software Requirements](#software-requirements)
   - [Network Requirements](#network-requirements)
3. [Raspberry Pi Setup](#raspberry-pi)
   - [Setup Raspberry Pi OS](#setup-raspberry-pi-os)
   - [Start Raspberry Pi](#start-raspberry-pi)
   - [Setup MQTT Broker](#setup-broker)
   - [Start MQTT Broker](#start-broker)
   - [Install Node-Red](#install-node-red)
4. [ESP32 Development](#esp32)
   - [Install Arduino IDE](#install-arduino-ide)
5. [Workshop Projects](#workshop-projects)
   - [Weather Station](#weather-station)
   - [Smart Lights](#smart-lights)
   - [Servo Door Control](#servo-door-control)
   - [WiFi Manager](#wifi-manager)

---

## Introduction

Welcome to the IoT Workshop - Smart City Exercises! The aim of this practical workshop is to build 4 examples of Smart City elements: a smart dumpster with garbage level monitoring, automated smart lighting, a weather monitoring station, and an optional WiFi manager for device configuration.

### What You'll Learn

- **IoT Fundamentals**: Understanding IoT concepts, architecture, and communication protocols
- **Hardware Integration**: Working with ESP32 microcontrollers, sensors like HC-SR04, and actuators like servomotor
- **MQTT Communication**: Implementing publish-subscribe messaging for IoT devices
- **Data Visualization**: Creating dashboards with Node-RED for monitoring and control
- **Real-world Applications**: Building practical smart city solutions including:
  - Weather monitoring stations
  - Automated lighting systems
  - Smart door controls with presence detection
  - WiFi configuration management

### Workshop Structure

1. **Setup Phase**: Configuring Raspberry Pi as MQTT broker and Node-RED server
2. **Practical Projects**: Building and programming ESP32-based IoT devices
3. **Integration**: Connecting all components into a smart city ecosystem

## Requirements

### Hardware Requirements

#### Raspberry Pi Setup

- **Raspberry Pi 3 or 4** with microSD card (16GB minimum)
- **Power supply** for Raspberry Pi (5V, 3A recommended)
- **Ethernet cable** or WiFi connection
- **MicroSD card reader**

#### ESP32 Development Kit

- **ESP32 DEVKIT V1** or compatible board
- **USB cable** (USB-A to Micro-USB) for programming
- **Breadboard** for prototyping
- **Jumper wires** (male-to-male, male-to-female)

#### Sensors and Components

- **DHT11** temperature and humidity sensor
- **HC-SR04** ultrasonic distance sensor
- **SG90 Servo motor**
- **Relay module** (5V, optocoupler isolated)
- **LED strips** or light bulbs for automation
- **Resistors** (220Ω, 1kΩ, 10kΩ)

### Software Requirements

#### Development Environment

- **Arduino IDE** (version 2.0 or later)
- **Java Runtime Environment** (required for Arduino IDE)
- **CP210x USB Driver** for ESP32 programming
- **Advanced IP Scanner** (for network device discovery)
- **SSH client** (PuTTY for Windows, Terminal for macOS/Linux)

#### Libraries and Dependencies

- **ESP32 Arduino Core**
- **PubSubClient** (MQTT client library)
- **ESP32Servo** library
- **DHT sensor library**
- **ESPAsyncWebServer** (compatible fork)
- **AsyncTCP** library

### Network Requirements

- **WiFi network** with internet access
- **Router access** for port configuration (if needed)
- **Static IP assignment** capability (recommended)
- **MQTT broker access** (Raspberry Pi will host the broker)

### Recommended Tools

- **Multimeter** for troubleshooting
- **Wire strippers** for cable preparation
- **Small screwdriver set**
- **Anti-static wrist strap** (optional but recommended)

---

**Note:**
CP210x_Universal_Windows_Driver in order to achieve the programming of PlatformIO with ESP32 or if you have troubleshooting Arduino IDE

## Raspberry Pi

### Setup Raspberry Pi OS

1. Install the Raspberry Pi Imager

```
https://www.raspberrypi.com/software/
```

2. Start the application

3. Choose device type in the first option

```
Raspberry Pi 3
```

![Choose device](assets/image.png)

4. Choose the SO type

```
Raspberry Pi OS (other) > Raspberry Pi OS Lite (64-bit) With NO desktop environment
```

![Choose OS1](assets/image-1.png)

![Choose OS2](assets/image-2.png)

5. Choose the device storage

![Choose storage](assets/image-3.png)

6. Click on **Next**

7. Before flashing the OS to the Raspberry Pi board, we need to setup some configurations. Click on **Edit Settings**

![Edit settings](assets/image-4.png)

8. Edit the information in the **General** block

**Username and Password**

```
username: igor
password: igor
```

**LAN configuration**

```
SSID: XXXXX
Password: YYYYY
```

![General tab](assets/image-5.png)

9. Edit the information in the **Services** block

Use password auth

![Services tab](assets/image-6.png)

10. Click on YES

11. Click YES on the warning panel to override the previous data

![alt text](assets/image-7.png)

12. Start the writting process

![alt text](assets/image-8.png)

![alt text](assets/image-9.png)

### Start Raspberry Pi

1. Insert the microSD inside the Raspberry Pi board and plug the device

2. Open the **Advanced IP Scanner**

![alt text](assets/image-10.png)

3. Start the search to find the Raspberry Pi board IP

![alt text](assets/image-11.png)

4. Access the board by opening a terminal and run the **ssh** command

```
ssh igor@192.168.0.17
```

![alt text](assets/image-12.png)

5. Type YES and insert the password

![alt text](assets/image-13.png)

![alt text](assets/image-14.png)

### Configure WiFi Credentials (Post-Setup)

If you need to change the WiFi credentials after the initial setup, follow these steps via SSH:

#### Method 1: Using wpa_supplicant (Recommended)

1. **Edit the wpa_supplicant configuration file:**

   ```bash
   sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
   ```

2. **Add or modify the network configuration:**

   ```bash
   # Basic configuration
   country=US  # Change to your country code (ES for Spain)
   ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
   update_config=1

   # Your WiFi network
   network={
       ssid="YOUR_WIFI_NAME"
       psk="YOUR_WIFI_PASSWORD"
       key_mgmt=WPA-PSK
   }
   ```

3. **For WPA2/WPA3 Enterprise networks:**

   ```bash
   network={
       ssid="YOUR_ENTERPRISE_WIFI"
       key_mgmt=WPA-EAP
       eap=PEAP
       identity="your_username"
       password="your_password"
       phase2="auth=MSCHAPV2"
   }
   ```

4. **For open networks (no password):**

   ```bash
   network={
       ssid="OPEN_WIFI_NAME"
       key_mgmt=NONE
   }
   ```

5. **Save and apply changes:**

   ```bash
   # Save the file (Ctrl+X, then Y, then Enter in nano)

   # Restart wpa_supplicant service
   sudo systemctl restart wpa_supplicant

   # Or restart networking
   sudo systemctl restart networking

   # Or reboot the Pi (recommended)
   sudo reboot
   ```

#### Method 2: Using raspi-config (GUI)

```bash
sudo raspi-config
# Navigate to: Network Options → Wi-Fi
```

#### Method 3: Using wpa_cli (Command Line)

```bash
# Interactive mode
sudo wpa_cli

# In wpa_cli:
> scan
> scan_results
> add_network
> set_network 0 ssid "YOUR_WIFI_NAME"
> set_network 0 psk "YOUR_WIFI_PASSWORD"
> enable_network 0
> save_config
> quit
```

#### Verify Connection

```bash
# Check WiFi status
iwconfig

# Check network interfaces
ip addr show

# Test internet connectivity
ping google.com

# Check wpa_supplicant status
sudo systemctl status wpa_supplicant
```

#### Multiple Networks Configuration

You can configure multiple WiFi networks with priorities:

```bash
country=ES
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1

network={
    ssid="HomeWiFi"
    psk="homepassword123"
    priority=1
}

network={
    ssid="OfficeWiFi"
    psk="officepassword456"
    priority=2
}
```

**Important Notes:**

- Always backup the configuration: `sudo cp /etc/wpa_supplicant/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf.backup`
- Set the correct country code (ES for Spain, US for United States, etc.)
- Higher priority numbers mean higher priority networks
- If your password has special characters, escape them or use quotes

### Setup Broker

1. Install mosquitto broker

```
sudo apt update && sudo apt upgrade
```

```
sudo apt install -y mosquitto mosquitto-clients
```

2. Make sure the mosquitto starts at raspberry pi boot

```
sudo systemctl enable mosquitto.service
```

3. Check mosquitto installed version

```
mosquitto -v
```

4. Enable remote communications, we need to edit the configuration file. Remote Access (No auth)

```
sudo nano /etc/mosquitto/mosquitto.conf
```

5. Paste 2 lines at the end of the file

```
listener 1883
allow_anonymous true
```

6. In order the changes to take effect execute the restart of the service

```
sudo systemctl restart mosquitto
```

```
sudo systemctl status mosquitto
```

### Start Broker

1. Get the local Raspberry Pi IP

```
hostname -I
```

2. Test the mosquitto fresh install broker. Run mosquitto broker in the background

```
mosquitto -d
```

3. Subscribe to a specific topic to start receiving the data

```
mosquitto_sub -d -t testTopic
```

4. Publish data to the topic

```
mosquitto_pub -d -t testTopic -m "Hello world!"
```

### Install Node-Red

1. Install Node-Red tool to manage the incoming MQTT communications

```
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered)
```

By default Node-Red is installed in Raspberry Pi OS however its recommencded to run the previous command to update to the newest versions

![alt text](assets/image-15.png)

2. Once its installed run the command to configure the tool

```
node-red admin init
```

3. Press Enter to create a Node-RED Settings file on /home/pi/.node-red/settings.js

```
Do you want to set up user security? Yes.
Enter a username and press Enter (you need to remember it later).
Enter a password and press Enter (you need to remember it later).
Then, you need to define user permissions. We’ll set full access, make sure the full access option is highlighted in blue and press Enter.
You can add other users with different permissions if you want. We’ll just create one user for now. You can always add other users later.
Do you want to enable the Projects feature? No.
Enter a name for your flows file. Press Enter to select the default name flows.json.
Provide a passphrase to encrypt your credentials file.
Learn more about what is a passphrase.
Select a theme for the editor. Simply press Enter to select default.
Press Enter again to select the default text editor.
Allow Function nodes to load external modules? Yes.
Node-RED configuration was successful. All settings are saved on settings.js.
```

4. Start the node red service

```
sudo systemctl start nodered.service
```

5. Check the node red service status

```
sudo systemctl status nodered.service
```

6. To run node-red on boot run the following:

```
sudo systemctl enable nodered.service
```

7. Reboot the board

```
sudo reboot now
```

8. Access again the raspberry ip by ssh

```
ssh igor@192.168.0.17
```

9. After all the configurations are done we can start the node-red tool

```
node-red-start
```

![alt text](assets/image-16.png)

10. After the instalation you can acces the interface on the http://raspberryPiIP:1880

![alt text](assets/image-17.png)

11. Insert the user credentials we set before, at the installation stage

![alt text](assets/image-18.png)

**NOTE:** Install new palette dashboard: Go to **Manage palette** > **Install** section and search for **node-red-dashboard**

## ESP32

### Install Arduino IDE

1. Previous to run the IDE we need to install JAVA Runtime environment

```
https://www.java.com/es/download/
```

2. Download the Arduino IDE

```
https://www.arduino.cc/en/software/
```

3. Install the ESP32 board into the Arduino IDE

Go to **File** > **Preferences**

Paste and copy the next link to the \_\_Additional boards manager URLs

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

4. Open **Boards Manager**. Go to **Tools** > **Board** > **Boards Manager**

Search for **ESP32** and select the library

![alt text](assets/image-19.png)

5. After this restart the Arduino IDE

6. Go to **Tools** > **Board** and check that you have the ESP32 board available

7. Test and upload an example build in code. Go to **File** > **Examples** > **WiFi** > **WiFiScan**

8. Select the board in the board searchbox

![alt text](assets/image-20.png)

9. Copy and paste

```
doit
```

9. Select the **DOIT ESP32 DEVKIT V1** option and the selected port **COM 7 (USB)**

10. Click on Upload button

11. Open the Serial Monitor to see the output of the code. Go to **Tools** > **Serial Monitor** and select the 115200 baud as the transmission rate.

---

## Workshop Projects

After completing the setup, you can proceed with the following IoT projects:

### Weather Station

Build a complete weather monitoring system using DHT11 sensor to collect temperature and humidity data, publish it via MQTT, and visualize it on a Node-RED dashboard.

📁 **Project Location**: `/weatherStation/`

### Smart Lights

Create an automated lighting system with presence detection using HC-SR04 sensor and relay control for energy-efficient smart city lighting.

📁 **Project Location**: `/smartLights/`

### Servo Door Control

Develop an automatic door system with presence detection that opens/closes doors using servo motors, perfect for smart building applications.

📁 **Project Location**: `/servo/`

### WiFi Manager

Implement a WiFi configuration system that allows devices to connect to different networks without hardcoding credentials, essential for scalable IoT deployments.

📁 **Project Location**: `/wifiManager/`

---

**Happy Building! 🚀**
