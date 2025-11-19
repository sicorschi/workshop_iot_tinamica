#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <PubSubClient.h>
#include <Wire.h>

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define RELAY_NO false
#define NUM_RELAYS  1

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.0.18";
long duration;
float distanceCm;
const int trigPin = 14;
const int echoPin = 12;
const int relay = 26;

// Variables for automatic light control
bool presenceDetected = false;
bool lightsOn = false;
unsigned long lastPresenceTime = 0;
const unsigned long lightOffDelay = 10000; // 10 seconds in milliseconds

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("[MQTT] Message arrived on topic: '");
  Serial.print(topic);
  Serial.print("' with message: '");
  
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.print("' (length: ");
  Serial.print(length);
  Serial.println(")");
  
  // Convert message to lowercase for case-insensitive comparison
  messageTemp.toLowerCase();
  
  if (String(topic) == "esp32/light/output") {
    Serial.print("[MQTT] Processing light control command: ");
    Serial.println(messageTemp);
    
    if(messageTemp == "on"){
      Serial.println("[MQTT] Turning lights ON via MQTT");
      digitalWrite(relay, HIGH);
      lightsOn = true;
      lastPresenceTime = millis(); // Reset timer when manually turned on
      client.publish("esp32/light/status", "1");
      client.publish("esp32/light/auto", "Lights ON - Manual MQTT control");
    }
    else if(messageTemp == "off"){
      Serial.println("[MQTT] Turning lights OFF via MQTT");
      digitalWrite(relay, LOW);
      lightsOn = false;
      client.publish("esp32/light/status", "0");
      client.publish("esp32/light/auto", "Lights OFF - Manual MQTT control");
    }
    else {
      Serial.print("[MQTT] Unknown command: '");
      Serial.print(messageTemp);
      Serial.println("'. Expected 'on' or 'off'");
    }
  } else {
    Serial.print("[MQTT] Ignored message on unexpected topic: ");
    Serial.println(topic);
  }
}

void turnOnLights() {
  if (!lightsOn) {
    digitalWrite(relay, HIGH);
    lightsOn = true;
    Serial.println("Lights turned ON automatically!");
    client.publish("esp32/light/auto", "Lights ON - Presence detected");
  }
}

void turnOffLights() {
  digitalWrite(relay, LOW);
  lightsOn = false;
  Serial.println("Lights turned OFF automatically - No presence for 15 seconds");
  client.publish("esp32/light/auto", "Lights OFF - Auto timeout");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("[MQTT] Attempting MQTT connection...");
    
    // Create unique client ID using MAC address
    String clientId = "ESP32Light_" + WiFi.macAddress();
    clientId.replace(":", "");
    
    Serial.print("[MQTT] Client ID: ");
    Serial.println(clientId);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("[MQTT] Connected successfully!");
      
      // Subscribe to control topic
      Serial.print("[MQTT] Subscribing to topic: esp32/light/output... ");
      if (client.subscribe("esp32/light/output")) {
        Serial.println("SUCCESS");
      } else {
        Serial.println("FAILED");
      }
      
      // Publish connection status
      client.publish("esp32/light/connection", "Connected");
      client.publish("esp32/light/status", lightsOn ? "1" : "0");
      
      Serial.println("[MQTT] Ready to receive commands on 'esp32/light/output'");
      Serial.println("[MQTT] Send 'on' or 'off' to control lights");
      
    } else {
      Serial.print("[MQTT] Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(echoPin, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Read ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED/2;
  
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  
  // Check for presence detection (within 10cm)
  if (distanceCm < 10 && distanceCm > 0) {
    if (!presenceDetected) {
      presenceDetected = true;
      Serial.println("Presence detected!");
    }
    lastPresenceTime = millis(); // Update last presence time
    turnOnLights(); // Turn on lights immediately
  } else {
    presenceDetected = false;
  }
  
  // Check if lights should be turned off due to no presence
  unsigned long currentTime = millis();
  if (lightsOn && !presenceDetected && (currentTime - lastPresenceTime > lightOffDelay)) {
    turnOffLights();
  }
  
  // MQTT connection management
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Publish status updates every 5 seconds
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Publish relay status
    int relayValue = digitalRead(relay);
    char relayString[8];
    dtostrf(relayValue, 1, 0, relayString);
    client.publish("esp32/light/status", relayString);
    
    // Publish distance reading
    char distanceString[10];
    dtostrf(distanceCm, 1, 2, distanceString);
    client.publish("esp32/light/distance", distanceString);
    
    // Publish time since last presence
    unsigned long timeSincePresence = currentTime - lastPresenceTime;
    char timeString[10];
    dtostrf(timeSincePresence / 1000.0, 1, 0, timeString); // Convert to seconds
    client.publish("esp32/light/time_since_presence", timeString);
    
    Serial.printf("Status: Lights %s | Distance: %.2fcm | Time since presence: %lus\n", 
                  lightsOn ? "ON" : "OFF", distanceCm, timeSincePresence / 1000);
  }
  
  delay(200); // Reduced delay for more responsive detection
}