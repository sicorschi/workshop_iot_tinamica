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

4. Restart the Arduino IDE
