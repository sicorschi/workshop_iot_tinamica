#include <ESP32Servo.h>
#include <PubSubClient.h>
#include "WiFi.h"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
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

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);
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
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    // Convert servo position to char array for publishing
    char statusMsg[10];
    servoValueString.toCharArray(statusMsg, 10);
    client.publish("esp32/servo/status", statusMsg);
    Serial.print("Published servo status: ");
    Serial.println(servoValueString);
  }
  delay(1000);
}