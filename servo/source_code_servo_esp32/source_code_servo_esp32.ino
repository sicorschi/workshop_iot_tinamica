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
static const int servoPin = 26;
String servoValueString = String(0);
int currentServoPosition = 0; 
int pos1 = 0; 
int pos2 = 0;
long duration;
float distanceCm;
float levelDuration;
float levelDistanceCm;
const int trigPin = 13;
const int echoPin = 12;
const int levelTrigPin = 25;
const int levelEchoPin = 32;
// Variables for presence detection
bool presenceDetected = false;
bool doorOpen = false;
unsigned long doorOpenTime = 0;
const unsigned long doorOpenDuration = 5000; // Keep door open for 5 seconds

// Garbage level configuration
const float GARBAGE_CONTAINER_HEIGHT = 6.0; // Height of garbage container in cm
const float GARBAGE_EMPTY_DISTANCE = 3.0;   // Distance when container is empty
const float GARBAGE_FULL_DISTANCE = 0.9;     // Distance when container is full
float garbageLevel = 0.0;                    // Garbage level percentage
String garbageStatus = "EMPTY";              // Current garbage status
bool garbageAlertSent = false;               // Track if critical alert was sent
bool maintenanceAlertSent = false;           // Track if maintenance alert was sent


Servo servo1;

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
  
  if (String(topic) == "esp32/servo/control") {
    Serial.print("[MQTT] Processing servo control command: ");
    Serial.println(messageTemp);
    
    // Handle special commands
    messageTemp.toLowerCase();
    if (messageTemp == "open") {
      Serial.println("[MQTT] Opening door via MQTT command");
      openDoor();
      client.publish("esp32/servo/auto", "Door opened - Manual MQTT control");
    }
    else if (messageTemp == "close") {
      Serial.println("[MQTT] Closing door via MQTT command");
      closeDoor();
      client.publish("esp32/servo/auto", "Door closed - Manual MQTT control");
    }
    else {
      // Try to parse as numeric position
      int newPosition = messageTemp.toInt();
      if(newPosition >= 0 && newPosition <= 180 && messageTemp != "0" || messageTemp == "0"){
        Serial.print("[MQTT] Setting servo to position: ");
        Serial.println(newPosition);
        servo1.write(newPosition);
        currentServoPosition = newPosition;
        servoValueString = String(newPosition);
        char statusMsg[10];
        servoValueString.toCharArray(statusMsg, 10);
        client.publish("esp32/servo/status", statusMsg);
        client.publish("esp32/servo/auto", "Position set - Manual MQTT control");
        Serial.println("[MQTT] Servo position updated and status published!");
      } else {
        Serial.print("[MQTT] Invalid command: '");
        Serial.print(messageTemp);
        Serial.println("'. Expected 'open', 'close', or angle 0-180");
        client.publish("esp32/servo/error", "Invalid command. Use: open/close/0-180");
      }
    }
  } else if (String(topic) == "esp32/servo/position") {
    Serial.print("[MQTT] Processing slider position command: ");
    Serial.println(messageTemp);
    
    // Parse slider position value (0-180)
    int sliderPosition = messageTemp.toInt();
    if((sliderPosition >= 0 && sliderPosition <= 180 && messageTemp != "0") || messageTemp == "0"){
      Serial.print("[MQTT] Setting servo to slider position: ");
      Serial.println(sliderPosition);
      servo1.write(sliderPosition);
      currentServoPosition = sliderPosition;
      servoValueString = String(sliderPosition);
      
      // Update door state based on position
      doorOpen = (sliderPosition > 90);
      if (doorOpen) {
        doorOpenTime = millis(); // Reset timer for auto-close
      }
      
      char statusMsg[10];
      servoValueString.toCharArray(statusMsg, 10);
      client.publish("esp32/servo/status", statusMsg);
      client.publish("esp32/servo/door", doorOpen ? "OPEN" : "CLOSED");
      client.publish("esp32/servo/auto", "Position set - Slider control");
      
      Serial.println("[MQTT] Servo position updated via slider and status published!");
    } else {
      Serial.print("[MQTT] Invalid slider position: '");
      Serial.print(messageTemp);
      Serial.println("'. Must be 0-180");
      client.publish("esp32/servo/error", "Invalid slider position. Range: 0-180");
    }
  } else {
    Serial.print("[MQTT] Ignored message on unexpected topic: ");
    Serial.println(topic);
  }
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.println("[MQTT] Attempting MQTT connection...");
    
    // Create unique client ID using MAC address
    String clientId = "ESP32Servo_" + WiFi.macAddress();
    clientId.replace(":", "");
    
    Serial.print("[MQTT] Client ID: ");
    Serial.println(clientId);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("[MQTT] Connected successfully!");
      
      // Subscribe to control topic
      Serial.print("[MQTT] Subscribing to topic: esp32/servo/control... ");
      if (client.subscribe("esp32/servo/control")) {
        Serial.println("SUCCESS");
      } else {
        Serial.println("FAILED");
      }
      
      // Subscribe to slider position topic
      Serial.print("[MQTT] Subscribing to topic: esp32/servo/position... ");
      if (client.subscribe("esp32/servo/position")) {
        Serial.println("SUCCESS");
      } else {
        Serial.println("FAILED");
      }
      
      // Publish connection status and initial state
      client.publish("esp32/servo/connection", "Connected");
      char statusMsg[10];
      servoValueString.toCharArray(statusMsg, 10);
      client.publish("esp32/servo/status", statusMsg);
      
      Serial.println("[MQTT] Ready to receive commands on 'esp32/servo/control'");
      Serial.println("[MQTT] Commands: 'open', 'close', or angle (0-180)");
      Serial.println("[MQTT] Ready to receive slider positions on 'esp32/servo/position'");
      Serial.println("[MQTT] Slider range: 0-180 degrees");
      Serial.println("[MQTT] Published initial servo status");
      
    } else {
      Serial.print("[MQTT] Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void openDoor() {
  servo1.write(150);
  currentServoPosition = 150;
  servoValueString = String(150);
  doorOpen = true;
  doorOpenTime = millis();
  
  char statusMsg[10];
  servoValueString.toCharArray(statusMsg, 10);
  client.publish("esp32/servo/status", statusMsg);
  client.publish("esp32/servo/door", "OPEN");
  
  Serial.println("[DOOR] Door opened! Servo at 150 degrees");
  
  char distanceMsg[10];
  dtostrf(distanceCm, 1, 2, distanceMsg);
  client.publish("esp32/servo/distance", distanceMsg);
  
  client.publish("esp32/servo/auto", "Door opened - Presence detected");
}

void closeDoor() {
  servo1.write(0);
  currentServoPosition = 0;
  servoValueString = String(0);
  doorOpen = false;
  
  char statusMsg[10];
  servoValueString.toCharArray(statusMsg, 10);
  client.publish("esp32/servo/status", statusMsg);
  client.publish("esp32/servo/door", "CLOSED");
  
  Serial.println("[DOOR] Door closed! Servo at 0 degrees");
  
  client.publish("esp32/servo/auto", "Door closed - Timeout or manual");
}

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(levelTrigPin, OUTPUT);
  pinMode(levelEchoPin, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  servo1.write(0);
  currentServoPosition = 0;
  servoValueString = String(0);
  Serial.println("System initialized - Door control ready!");
}

void loop() {
  // MQTT connection management
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Read presence detection sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED/2;
  
  // Read garbage level sensor
  digitalWrite(levelTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(levelTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(levelTrigPin, LOW);
  levelDuration = pulseIn(levelEchoPin, HIGH);
  levelDistanceCm = levelDuration * SOUND_SPEED/2;
  
  // Calculate garbage level percentage
  if (levelDistanceCm > 0 && levelDistanceCm <= GARBAGE_EMPTY_DISTANCE) {
    // Calculate percentage: 0% = empty (max distance), 100% = full (min distance)
    garbageLevel = ((GARBAGE_EMPTY_DISTANCE - levelDistanceCm) / (GARBAGE_EMPTY_DISTANCE - GARBAGE_FULL_DISTANCE)) * 100.0;
    garbageLevel = constrain(garbageLevel, 0.0, 100.0); // Ensure 0-100% range
    
    // Determine garbage status
    if (garbageLevel >= 90) {
      garbageStatus = "CRITICAL - FULL";
      if (!garbageAlertSent) {
        client.publish("esp32/garbage/alert", "CRITICAL: Container is full! Immediate collection required.");
        garbageAlertSent = true;
        Serial.println("[GARBAGE] CRITICAL ALERT: Container full!");
      }
    } else if (garbageLevel >= 75) {
      garbageStatus = "HIGH";
      if (!maintenanceAlertSent) {
        client.publish("esp32/garbage/alert", "WARNING: Container is 75% full. Schedule collection soon.");
        maintenanceAlertSent = true;
        Serial.println("[GARBAGE] WARNING: Container 75% full");
      }
      garbageAlertSent = false; // Reset critical alert for next time
    } else if (garbageLevel >= 50) {
      garbageStatus = "MEDIUM";
      garbageAlertSent = false;
      maintenanceAlertSent = false;
    } else if (garbageLevel >= 25) {
      garbageStatus = "LOW";
      garbageAlertSent = false;
      maintenanceAlertSent = false;
    } else {
      garbageStatus = "EMPTY";
      garbageAlertSent = false;
      maintenanceAlertSent = false;
    }
  } else {
    garbageLevel = 0.0;
    garbageStatus = "SENSOR ERROR";
  }
  
  // Print sensor readings with proper formatting
  Serial.println("=====================================");
  Serial.println("[SENSORS] Current Readings:");
  Serial.print("[SENSORS]   Presence Detection: ");
  Serial.print(distanceCm, 2);
  Serial.println(" cm");
  Serial.println("[GARBAGE] Waste Container Status:");
  Serial.print("[GARBAGE]   Distance to waste:  ");
  Serial.print(levelDistanceCm, 2);
  Serial.println(" cm");
  Serial.print("[GARBAGE]   Fill Level:         ");
  Serial.print(garbageLevel, 1);
  Serial.println("%");
  Serial.print("[GARBAGE]   Status:             ");
  Serial.println(garbageStatus);
  
  // Add visual indicator for garbage level
  Serial.print("[GARBAGE]   Visual: [");
  int bars = (int)(garbageLevel / 10); // 10 bars for 100%
  for (int i = 0; i < 10; i++) {
    if (i < bars) {
      Serial.print("■");
    } else {
      Serial.print("□");
    }
  }
  Serial.print("] ");
  Serial.print(garbageLevel, 0);
  Serial.println("%");
  Serial.println("=====================================");
  
  // Check for presence detection (within 10cm)
  if (distanceCm < 10 && distanceCm > 0) {
    if (!presenceDetected) {
      presenceDetected = true;      
      Serial.println("[PRESENCE] Person detected! Opening door automatically");
      client.publish("esp32/servo/sensor", "Presence detected! Opening doors");
      client.publish("esp32/servo/presence", "DETECTED");
      openDoor();
    }
  } else {
    if (presenceDetected) {
      Serial.println("[PRESENCE] Person left detection area");
      client.publish("esp32/servo/presence", "CLEAR");
    }
    presenceDetected = false;
  }
  
  // Auto-close door after specified duration
  if (doorOpen && (millis() - doorOpenTime > doorOpenDuration)) {
    if (!presenceDetected) {
      Serial.println("[DOOR] Auto-closing door - timeout reached and no presence");
      closeDoor();
    } else {
      // Reset timer if presence is still detected
      doorOpenTime = millis();
      Serial.println("[DOOR] Extending door open time - presence still detected");
    }
  }
  
  // Publish regular status updates every 5 seconds
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Publish servo status
    char statusMsg[10];
    servoValueString.toCharArray(statusMsg, 10);
    client.publish("esp32/servo/status", statusMsg);
    
    // Publish distance readings
    char distanceMsg[10];
    dtostrf(distanceCm, 1, 2, distanceMsg);
    client.publish("esp32/servo/distance", distanceMsg);
    
    // Publish enhanced garbage level information
    char levelDistanceMsg[10];
    dtostrf(levelDistanceCm, 1, 2, levelDistanceMsg);
    client.publish("esp32/garbage/level_distance", levelDistanceMsg);
    
    char garbageLevelMsg[10];
    dtostrf(garbageLevel, 1, 1, garbageLevelMsg);
    client.publish("esp32/garbage/level_percentage", garbageLevelMsg);
    
    char garbageStatusMsg[20];
    garbageStatus.toCharArray(garbageStatusMsg, 20);
    client.publish("esp32/garbage/status", garbageStatusMsg);
    
    // Publish comprehensive garbage data as JSON
    String garbageDataJson = "{";
    garbageDataJson += "\"distance_cm\":" + String(levelDistanceCm, 2) + ",";
    garbageDataJson += "\"level_percentage\":" + String(garbageLevel, 1) + ",";
    garbageDataJson += "\"status\":\"" + garbageStatus + "\",";
    garbageDataJson += "\"container_height\":" + String(GARBAGE_CONTAINER_HEIGHT, 1) + ",";
    garbageDataJson += "\"empty_distance\":" + String(GARBAGE_EMPTY_DISTANCE, 1) + ",";
    garbageDataJson += "\"full_distance\":" + String(GARBAGE_FULL_DISTANCE, 1) + ",";
    garbageDataJson += "\"needs_collection\":" + String(garbageLevel >= 75 ? "true" : "false") + ",";
    garbageDataJson += "\"critical\":" + String(garbageLevel >= 90 ? "true" : "false") + ",";
    garbageDataJson += "\"timestamp\":" + String(millis());
    garbageDataJson += "}";
    
    char garbageJsonMsg[200];
    garbageDataJson.toCharArray(garbageJsonMsg, 200);
    client.publish("esp32/garbage/data", garbageJsonMsg);
    
    // Legacy topic for backward compatibility
    client.publish("esp32/garbage/level", levelDistanceMsg);
    
    // Publish door state and timing info
    client.publish("esp32/servo/door", doorOpen ? "OPEN" : "CLOSED");
    
    if (doorOpen) {
      unsigned long timeOpen = millis() - doorOpenTime;
      char timeOpenMsg[10];
      dtostrf(timeOpen / 1000.0, 1, 0, timeOpenMsg);
      client.publish("esp32/servo/time_open", timeOpenMsg);
    }
    
    Serial.printf("[STATUS] Door: %s | Servo: %s° | Presence: %.2fcm | Garbage: %.1f%% (%s)\\n", 
                  doorOpen ? "OPEN" : "CLOSED", servoValueString.c_str(), distanceCm, garbageLevel, garbageStatus.c_str());
  }
  
  delay(200); // Reduced delay for more responsive detection
}