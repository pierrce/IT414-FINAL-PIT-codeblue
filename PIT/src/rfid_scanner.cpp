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
const char* ssid = "GIGGA NIGGA";
const char* password = "12345678910";
const char* serverName = "http://10.249.12.166:8000/api/rfids";

// MQTT
const char* mqtt_server = "10.249.12.166";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// RFID timing
unsigned long lastRead = 0;
const unsigned long readDelay = 2000;

void connectWiFi();
void connectMQTT();
void sendRFID(String tag);

void setup() {
  Serial.begin(115200);
  delay(500);

  // Initialize SPI for MFRC522
  SPI.begin(18, 19, 23); // SCK, MISO, MOSI
  rfid.PCD_Init();
  Serial.println("RFID Scanner Ready.");

  connectWiFi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  connectMQTT();
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  static bool waitingForCard = true;

  if (waitingForCard) {
    Serial.print("Waiting for RFID on WiFi: ");
    Serial.println(WiFi.SSID());
    waitingForCard = false;
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (millis() - lastRead > readDelay) {
      String tag = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) tag += "0"; // pad single hex digits
        tag += String(rfid.uid.uidByte[i], HEX);
      }

      tag.toUpperCase(); // in-place conversion, fixed

      Serial.print("RFID TAG: ");
      Serial.println(tag);

      sendRFID(tag);

      rfid.PICC_HaltA();
      lastRead = millis();
      waitingForCard = true;
    }
  }

  delay(100);
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP32-RFID")) {
      Serial.println("Connected!");
      mqttClient.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Retrying in 2s...");
      delay(2000);
    }
  }
}

void sendRFID(String tag) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot send RFID.");
    connectWiFi();
    return;
  }

  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"rfid_number\":\"" + tag + "\"}";
  int code = http.POST(json);

  Serial.print("API Response Code: ");
  Serial.println(code);

  if (code > 0) {
    String response = http.getString();
    Serial.println("API Response: " + response);

    // Optional: parse status from JSON if present
    int idx = response.indexOf("\"status\":");
    if (idx != -1) {
      int status = response.substring(idx + 9, idx + 10).toInt();

      String msg = String(status);
      mqttClient.publish(mqtt_topic, msg.c_str());

      Serial.print("MQTT Sent: ");
      Serial.println(msg);
    }
  } else {
    Serial.println("API Error. Check server IP, port, and WiFi connection.");
    if (code == -11) Serial.println("Connection failed (-11)");
  }

  http.end();
}
