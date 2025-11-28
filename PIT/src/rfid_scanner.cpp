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

// WiFi & API - UPDATE THESE TO YOUR NETWORK
const char* ssid = "GIGA NIGGA";
const char* password = "12345678910";
const char* serverName = "http://10.197.198.197:8000/api/rfids";

const char* mqtt_server = "10.197.198.197";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// RFID timing
unsigned long lastRead = 0;
const unsigned long readDelay = 1000;
bool wifiConnected = false;
bool mqttConnected = false;

void connectWiFi();
void connectMQTT();
void sendRFID(String tag);

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("   ESP32 RFID SCANNER with MQTT       ");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  // Initialize SPI for MFRC522
  SPI.begin(18, 19, 23); // SCK, MISO, MOSI
  rfid.PCD_Init();
  Serial.println("✅ RFID Scanner Ready.");
  Serial.println();

  connectWiFi();

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  if (wifiConnected) connectMQTT();

  Serial.println("🎯 System ready. Scan RFID cards...\n");
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      wifiConnected = false;
      mqttConnected = false;
      Serial.println("⚠️  WiFi disconnected! Reconnecting...");
    }
    connectWiFi();
    delay(2000);
    return;
  } else if (!wifiConnected) {
    wifiConnected = true;
    Serial.println("✅ WiFi reconnected!");
  }

  // Check MQTT connection
  if (!mqttClient.connected()) {
    if (mqttConnected) {
      Serial.println("⚠️  MQTT disconnected! Reconnecting...");
      mqttConnected = false;
    }
    connectMQTT();
  }

  mqttClient.loop();

  // Read RFID card
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (millis() - lastRead > readDelay) {
      String tag = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) tag += "0";
        tag += String(rfid.uid.uidByte[i], HEX);
      }
      tag.toUpperCase();

      Serial.println();
      Serial.print("📇 RFID Detected: ");
      Serial.println(tag);

      sendRFID(tag);

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      lastRead = millis();
    }
  }

  delay(50);
}

void connectWiFi() {
  Serial.print("🔌 Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("   SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("   IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("   MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println();
    wifiConnected = true;
  } else {
    Serial.println("\n❌ Failed to connect to WiFi");
    Serial.print("   Status code: ");
    Serial.println(WiFi.status());
    wifiConnected = false;
  }
}

void connectMQTT() {
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 3) {
    Serial.print("🔗 Connecting to MQTT broker...");

    String clientId = "ESP32-SCANNER-";
    clientId += String(WiFi.macAddress());
    clientId.replace(":", "");

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" ✅ Connected!");
      mqttConnected = true;
      return;
    } else {
      Serial.print(" ❌ Failed, rc=");
      Serial.println(mqttClient.state());
      attempts++;
      if (attempts < 3) {
        Serial.println("⏳ Retrying in 3 seconds...");
        delay(3000);
      }
    }
  }

  if (!mqttConnected) Serial.println("⚠️ MQTT connection failed");
}

void sendRFID(String tag) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected. Cannot send RFID.");
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
  http.setTimeout(10000);

  String jsonPayload = "{\"rfid_number\":\"" + tag + "\"}";
  int httpResponseCode = http.POST(jsonPayload);

  Serial.print("📡 HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();

    // Parse rfid_number
    int rfidIndex = response.indexOf("\"rfid_number\":\"");
    String rfidNum = "";
    if (rfidIndex != -1) {
      int start = rfidIndex + 15;
      int end = response.indexOf("\"", start);
      rfidNum = response.substring(start, end);
    }

    // Parse status
    int statusIndex = response.indexOf("\"status\":");
    int statusValue = -1;
    if (statusIndex != -1) {
      int start = statusIndex + 9;
      int end = response.indexOf(",", start);
      if (end == -1) end = response.indexOf("}", start);
      statusValue = response.substring(start, end).toInt();
    }

    // Parse registered
    int registeredIndex = response.indexOf("\"registered\":");
    bool registered = false;
    if (registeredIndex != -1) {
      int start = registeredIndex + 13;
      String val = response.substring(start, start + 4);
      val.trim();
      registered = val.startsWith("true");
    }

    // ✅ Display output
    Serial.println();
    Serial.print("RFID: ");
    Serial.print(rfidNum);
    Serial.print(" | STATUS: ");
    Serial.print(statusValue);
    Serial.print(" → ");
    if (registered) {
      Serial.println("FOUND ✅");
    } else {
      Serial.println("RFID NOT FOUND ❌");
    }

    // 🔥 PUBLISH STATUS TO MQTT
    if (mqttClient.connected()) {
      String mqttMessage = String(statusValue);
      if (mqttClient.publish(mqtt_topic, mqttMessage.c_str())) {
        Serial.print("📤 Published to MQTT [");
        Serial.print(mqtt_topic);
        Serial.print("]: ");
        Serial.println(mqttMessage);
      } else {
        Serial.println("❌ Failed to publish to MQTT");
      }
    } else {
      Serial.println("⚠️ MQTT not connected. Cannot publish status.");
    }
    
    Serial.println();
  } else {
    Serial.println("❌ API Error. Check server IP, port, and WiFi connection.");
    if (httpResponseCode == -11) {
      Serial.println("   Connection failed (-11) - Check if Laravel server is running");
    }
  }

  http.end();
}