#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
float accelerationX = 0;
float accelerationY = 0;
float accelerationZ = 0;
float rotationX = 0;
float rotationY = 0;
float rotationZ = 0;
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.0.18";

Adafruit_MPU6050 mpu;

void callback(char* topic, byte* message, unsigned int length) {
  // MQTT callback function for receiving messages
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32GyroClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
  
  // Setup MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
}

void loop() {
  // Check MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values and publish to MQTT */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  
  // Convert float to string for MQTT publishing
  char accelXStr[10];
  dtostrf(a.acceleration.x, 1, 2, accelXStr);
  client.publish("esp32/gyro/accelerationx", accelXStr);
  
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  
  char accelYStr[10];
  dtostrf(a.acceleration.y, 1, 2, accelYStr);
  client.publish("esp32/gyro/accelerationy", accelYStr);
  
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  
  char accelZStr[10];
  dtostrf(a.acceleration.z, 1, 2, accelZStr);
  client.publish("esp32/gyro/accelerationz", accelZStr);
  
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  
  char rotXStr[10];
  dtostrf(g.gyro.x, 1, 2, rotXStr);
  client.publish("esp32/gyro/rotationx", rotXStr);
  
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  
  char rotYStr[10];
  dtostrf(g.gyro.y, 1, 2, rotYStr);
  client.publish("esp32/gyro/rotationy", rotYStr);
  
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  
  char rotZStr[10];
  dtostrf(g.gyro.z, 1, 2, rotZStr);
  client.publish("esp32/gyro/rotationz", rotZStr);
  
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  
  char tempStr[10];
  dtostrf(temp.temperature, 1, 2, tempStr);
  client.publish("esp32/gyro/temperature", tempStr);
  
  Serial.println(" degC");

  Serial.println("");
  delay(500);
}