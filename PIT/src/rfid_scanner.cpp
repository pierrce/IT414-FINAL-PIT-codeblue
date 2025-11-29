#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

// RFID Pins (ESP32)
#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN);

// WiFi & API
const char* ssid = "GIGA NIGGA";
const char* password = "12345678910";
const char* serverName = "http://10.197.198.197:8000/api/rfids";

const char* mqtt_server = "10.197.198.197";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ⚡ REDUCED read delay for faster response
unsigned long lastRead = 0;
const unsigned long readDelay = 200; // 200ms to prevent duplicate reads
String lastTag = "";

// Connection flags
bool wifiConnected = false;
bool mqttConnected = false;

// ⚡ NON-BLOCKING reconnection timers
unsigned long lastWiFiCheck = 0;
const unsigned long wifiCheckInterval = 5000; // Check every 5 seconds
unsigned long lastMQTTCheck = 0;
const unsigned long mqttCheckInterval = 5000;

void connectWiFi();
void connectMQTT();
void sendRFID(String tag);
void checkConnections();

void setup() {
  Serial.begin(115200);

  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("   ESP32 RFID - ZERO DELAY VERSION    ");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  // Initialize SPI for MFRC522
  SPI.begin(18, 19, 23); // SCK, MISO, MOSI
  rfid.PCD_Init();
  Serial.println("✅ RFID Scanner Ready (Zero-delay mode)");
  Serial.println();

  connectWiFi();

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  if (wifiConnected) connectMQTT();

  Serial.println("🎯 System ready. Instant RFID scanning...\n");
}

void loop() {
  // ⚡ Non-blocking connection checks
  checkConnections();

  // ⚡ MQTT loop (instant processing)
  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  // ⚡ INSTANT RFID READ - No delays!
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String tag = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) tag += "0";
      tag += String(rfid.uid.uidByte[i], HEX);
    }
    tag.toUpperCase();

    // Only process if it's a new tag or enough time has passed
    unsigned long currentTime = millis();
    if (tag != lastTag || (currentTime - lastRead) > readDelay) {
      Serial.println();
      Serial.print("📇 RFID Detected: ");
      Serial.println(tag);

      sendRFID(tag); // ⚡ Instant API call

      lastTag = tag;
      lastRead = currentTime;
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // ⚡ yield() instead of delay() - allows ESP32 background tasks
  yield();
}

void checkConnections() {
  unsigned long currentTime = millis();

  // ⚡ Non-blocking WiFi check
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      wifiConnected = false;
      mqttConnected = false;
      Serial.println("⚠️  WiFi disconnected!");
    }
    
    // Only attempt reconnection every 5 seconds
    if (currentTime - lastWiFiCheck >= wifiCheckInterval) {
      lastWiFiCheck = currentTime;
      connectWiFi();
    }
  } else if (!wifiConnected) {
    wifiConnected = true;
    Serial.println("✅ WiFi reconnected!");
  }

  // ⚡ Non-blocking MQTT check
  if (wifiConnected && !mqttClient.connected()) {
    if (mqttConnected) {
      Serial.println("⚠️  MQTT disconnected!");
      mqttConnected = false;
    }
    
    // Only attempt reconnection every 5 seconds
    if (currentTime - lastMQTTCheck >= mqttCheckInterval) {
      lastMQTTCheck = currentTime;
      connectMQTT();
    }
  }
}

void connectWiFi() {
  Serial.print("🔌 Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // ⚡ COMPLETELY NON-BLOCKING - using millis() timer
  // OLD: delay(100) in loop - blocked for 2 seconds total!
  // NEW: yield() only - max 3 second timeout
  
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 3000) {
    yield(); // ⚡ Non-blocking - let ESP32 handle WiFi connection
    // No delay() at all!
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✅ Connected!");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  } else {
    Serial.println(" ❌ Timeout - will retry in 5s");
    wifiConnected = false;
  }
}

void connectMQTT() {
  Serial.print("🔗 Connecting to MQTT...");

  String clientId = "ESP32-SCANNER-";
  clientId += String(random(0xffff), HEX);

  // ⚡ Single connection attempt - no retry loop
  if (mqttClient.connect(clientId.c_str())) {
    Serial.println(" ✅ Connected!");
    mqttConnected = true;
  } else {
    Serial.print(" ❌ Failed, rc=");
    Serial.println(mqttClient.state());
    mqttConnected = false;
    // Will retry in 5 seconds from checkConnections()
  }
}

void sendRFID(String tag) {
  if (!wifiConnected) {
    Serial.println("❌ WiFi not connected. Skipping API call.");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, serverName)) {
    Serial.println("❌ Failed to connect to server.");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.setTimeout(3000); // ⚡ Reduced from 5000ms to 3000ms for faster response

  String jsonPayload = "{\"rfid_number\":\"" + tag + "\"}";
  int httpResponseCode = http.POST(jsonPayload);

  Serial.print("📡 HTTP Response: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();

    // ⚡ Quick parsing
    int rfidIndex = response.indexOf("\"rfid_number\":\"");
    String rfidNum = "";
    if (rfidIndex != -1) {
      int start = rfidIndex + 15;
      int end = response.indexOf("\"", start);
      rfidNum = response.substring(start, end);
    }

    int statusIndex = response.indexOf("\"status\":");
    int statusValue = 0;
    if (statusIndex != -1) {
      int start = statusIndex + 9;
      int end = response.indexOf(",", start);
      if (end == -1) end = response.indexOf("}", start);
      statusValue = response.substring(start, end).toInt();
    }

    bool registered = response.indexOf("\"registered\":true") != -1;

    // Display result
    Serial.print("RFID: ");
    Serial.print(rfidNum);
    Serial.print(" | STATUS: ");
    Serial.print(statusValue);
    Serial.print(" → ");
    Serial.println(registered ? "FOUND ✅" : "NOT FOUND ❌");

    // ⚡ INSTANT MQTT PUBLISH
    if (mqttClient.connected()) {
      String mqttMessage = String(statusValue);
      if (mqttClient.publish(mqtt_topic, mqttMessage.c_str())) {
        Serial.print("📤 Published to MQTT: ");
        Serial.println(mqttMessage);
      } else {
        Serial.println("❌ MQTT publish failed");
      }
    } else {
      Serial.println("⚠️ MQTT not connected");
    }
    
    Serial.println();
  } else {
    Serial.println("❌ API Error");
    if (httpResponseCode == -11) {
      Serial.println("   Connection failed - Check server");
    }
  }

  http.end();
}