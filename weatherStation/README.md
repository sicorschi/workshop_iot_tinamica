# Weather station

Project to build a weather station using the **ESP32** devkit board and the **DHT11** temperature and humidity sensor.

## Requirements

### ESP32

### DHT11

These sensors contain a chip that does analog to digital conversion and spit out a digital signal with the temperature and humidity. This makes them very easy to use with any microcontroller.

#### Sensor PINOUT

![alt text](assets/image.png)

![alt text](assets/image-2.png)

## Connection schema

![alt text](assets/image-3.png)

## Install the Arduino Libraries to use the sensors

1. Go to **Sketch** > **Include Library** > **Manage Libraries**

2. Type **DHT sensor library**

![alt text](assets/image-4.png)

3. Install the next library, type in the same search box **Adafruit Unified Sensor**

![alt text](assets/image-5.png)

4. Install the **pubsubclient** library by downloading the .zip folder

```
https://github.com/knolleary/pubsubclient/archive/master.zip
```

5. Rename the downloaded folder from **pubsubclient-master** to **pubsubclient**

6. Go to **Sketch** > **Include Library** > **Add .zip library**

7. Restart the Arduino IDE

## Workflow

![alt text](assets/image-6.png)

Go to **Menu** > **Manage palette** > **Install tab** > Type **dashboard** and install the dashboard modal

![alt text](assets/image-8.png)

Go to **Menu** > **Import** > **Clipboard** and copy and paste the following string

```
[{"id":"9e58624.7faaba","type":"mqtt out","z":"c02b79b2.501998","name":"","topic":"esp32/output","qos":"","retain":"","broker":"10e78a89.5b4fd5","x":610,"y":342,"wires":[]},{"id":"abf7079a.653be8","type":"mqtt in","z":"c02b79b2.501998","name":"","topic":"esp32/temperature","qos":"2","broker":"10e78a89.5b4fd5","x":484,"y":249,"wires":[["cc79021b.9a751","21eae8f8.2971b8"]]},{"id":"83cf37cf.c76988","type":"ui_switch","z":"c02b79b2.501998","name":"","label":"Output","group":"61285987.c20328","order":0,"width":0,"height":0,"passthru":true,"decouple":"false","topic":"","style":"","onvalue":"on","onvalueType":"str","onicon":"","oncolor":"","offvalue":"off","offvalueType":"str","officon":"","offcolor":"","x":469,"y":342,"wires":[["9e58624.7faaba"]]},{"id":"cc79021b.9a751","type":"debug","z":"c02b79b2.501998","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"false","x":681,"y":216,"wires":[]},{"id":"4aecba01.78ce64","type":"mqtt in","z":"c02b79b2.501998","name":"","topic":"esp32/humidity","qos":"2","broker":"10e78a89.5b4fd5","x":473,"y":133,"wires":[["22efa7b7.544a28","df37e6b7.64c1c8"]]},{"id":"22efa7b7.544a28","type":"debug","z":"c02b79b2.501998","name":"","active":true,"tosidebar":true,"console":false,"tostatus":false,"complete":"false","x":670,"y":100,"wires":[]},{"id":"21eae8f8.2971b8","type":"ui_chart","z":"c02b79b2.501998","name":"","group":"61285987.c20328","order":0,"width":0,"height":0,"label":"Temperature","chartType":"line","legend":"false","xformat":"HH:mm:ss","interpolate":"linear","nodata":"","dot":false,"ymin":"","ymax":"","removeOlder":1,"removeOlderPoints":"","removeOlderUnit":"3600","cutout":0,"useOneColor":false,"colors":["#1f77b4","#aec7e8","#ff7f0e","#2ca02c","#98df8a","#d62728","#ff9896","#9467bd","#c5b0d5"],"useOldStyle":false,"x":681,"y":276,"wires":[[],[]]},{"id":"df37e6b7.64c1c8","type":"ui_gauge","z":"c02b79b2.501998","name":"","group":"61285987.c20328","order":0,"width":0,"height":0,"gtype":"gage","title":"Humidity","label":"%","format":"{{value}}","min":0,"max":"100","colors":["#00b3d9","#0073e6","#001bd7"],"seg1":"33","seg2":"66","x":660,"y":160,"wires":[]},{"id":"10e78a89.5b4fd5","type":"mqtt-broker","z":"","name":"","broker":"localhost","port":"1883","clientid":"","usetls":false,"compatmode":true,"keepalive":"60","cleansession":true,"birthTopic":"","birthQos":"0","birthPayload":"","closeTopic":"","closeQos":"0","closePayload":"","willTopic":"","willQos":"0","willPayload":""},{"id":"61285987.c20328","type":"ui_group","z":"","name":"Main","tab":"e7c46d5e.a1283","disp":true,"width":"6","collapse":false},{"id":"e7c46d5e.a1283","type":"ui_tab","z":"","name":"Dashboard","icon":"dashboard"}]
```

This should import the node-red box workflow

![alt text](assets/image-7.png)

Click on **Deploy** button to deploy the flow

Open a new browser tab and type

```
http://raspberrypiIP:1880/ui
```

![alt text](assets/image-9.png)
