## Content

1. Introduction / Welcome
2. Theory part
   - 1. What is IoT and brief history
   - 2. IoT concepts
   - 3. Devices
   - 4. Communication protocols
   - 5. Architecture
3. Practical part
   - 1. Setup
   - 2. Circuit board
   - 3. Architecture

**Note:**
CP210x_Universal_Windows_Driver in order to achieve the programming of PlatformIO with ESP32

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

```´
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

8. After all the configurations are done we can start the node-red tool

```
node-red-start
```

![alt text](assets/image-16.png)

9. After the instalation you can acces the interface on the http://raspberryPiIP:1880

![alt text](assets/image-17.png)

10. Insert the user credentials we set before, at the installation stage

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
