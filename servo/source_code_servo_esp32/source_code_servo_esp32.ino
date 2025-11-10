#include <ESP32Servo.h>
#include <PubSubClient.h>
#include "WiFi.h"

//define sound speed in cm/uS
#define SOUND_SPEED 0.034

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long lastSensorMsg = 0;
char msg[50];
int value = 0;
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.0.18";
static const int servoPin = 13;
String servoValueString = String(0);
int currentServoPosition = 0; 
int pos1 = 0; 
int pos2 = 0;
long duration;
float distanceCm;
const int trigPin = 4;
const int echoPin = 5;
// Variables for presence detection
bool presenceDetected = false;
bool doorOpen = false;
unsigned long doorOpenTime = 0;
const unsigned long doorOpenDuration = 5000; // Keep door open for 5 seconds


Servo servo1;

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "esp32/servo") {
    Serial.print("Changing servo position to: ");
    int newPosition = messageTemp.toInt();
    if(newPosition >= 0 && newPosition <= 180){
      Serial.println(newPosition);
      servo1.write(newPosition);
      currentServoPosition = newPosition;
      servoValueString = String(newPosition);
      // Publish the new servo status immediately
      char statusMsg[10];
      servoValueString.toCharArray(statusMsg, 10);
      client.publish("esp32/servo/status", statusMsg);
      Serial.println("Servo position updated and status published!");
    } else {
      Serial.println("Invalid servo position! Must be between 0-180 degrees.");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/servo");
      // Publish initial servo status
      char statusMsg[10];
      servoValueString.toCharArray(statusMsg, 10);
      client.publish("esp32/servo/status", statusMsg);
      Serial.println("Published initial servo status");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void openDoor() {
  servo1.write(135);
  currentServoPosition = 135;
  servoValueString = String(135);
  doorOpen = true;
  doorOpenTime = millis();
  // Publish door status
  char statusMsg[10];
  servoValueString.toCharArray(statusMsg, 10);
  client.publish("esp32/servo/status", statusMsg);
  Serial.println("Door opened! Servo at 135 degrees");
}

void closeDoor() {
  servo1.write(0);
  currentServoPosition = 0;
  servoValueString = String(0);
  doorOpen = false;
  // Publish door status
  char statusMsg[10];
  servoValueString.toCharArray(statusMsg, 10);
  client.publish("esp32/servo/status", statusMsg);
  Serial.println("Door closed! Servo at 0 degrees");
}

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Initialize servo to 0 position
  servo1.write(0);
  currentServoPosition = 0;
  servoValueString = String(0);
  Serial.println("System initialized - Door control ready!");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED/2;
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  // Check for presence detection (object within 10cm)
  if (distanceCm < 10 && distanceCm > 0) {
    if (!presenceDetected) {
      presenceDetected = true;      
      // Publish presence detection message
      const char* sensorMsg = "Presence detected! Opening doors";
      client.publish("esp32/servo/sensor", sensorMsg);
      Serial.println("Presence detected! Publishing to MQTT...");
      // Open the door
      openDoor();
    }
  } else {
    presenceDetected = false;
  }
  // Auto-close door after specified duration
  if (doorOpen && (millis() - doorOpenTime > doorOpenDuration)) {
    if (!presenceDetected) { // Only close if no presence detected
      closeDoor();
    } else {
      // Reset timer if presence is still detected
      doorOpenTime = millis();
    }
  }
  // Publish regular status updates every 5 seconds
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    // Publish servo position
    char statusMsg[10];
    servoValueString.toCharArray(statusMsg, 10);
    client.publish("esp32/servo/status", statusMsg);
    // Publish distance reading
    char distanceMsg[10];
    dtostrf(distanceCm, 1, 2, distanceMsg);
    client.publish("esp32/servo/distance", distanceMsg);
    Serial.print("Published servo status: ");
    Serial.print(servoValueString);
    Serial.print(" | Distance: ");
    Serial.println(distanceCm);
  }
  delay(500);
}