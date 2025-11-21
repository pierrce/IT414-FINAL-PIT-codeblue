#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Pin definitions
#define RELAY_PIN 27
#define LED_PIN 2

// WiFi credentials
const char* ssid = "GIGGA NIGGA";
const char* password = "12345678910";

// MQTT broker
const char* mqtt_server = "10.249.12.166";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Connection status flags
bool wifiConnected = false;
bool mqttConnected = false;

// Function prototypes
void connectWiFi();
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// Helper functions for active LOW relay
void relayOn() {
  digitalWrite(RELAY_PIN, LOW); // Active LOW: LOW = ON
  digitalWrite(LED_PIN, HIGH);  // LED normal HIGH = ON
  Serial.println("✅ Relay/LED ON");
}

void relayOff() {
  digitalWrite(RELAY_PIN, HIGH); // Active LOW: HIGH = OFF
  digitalWrite(LED_PIN, LOW);
  Serial.println("❌ Relay/LED OFF");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("   ESP32 RELAY/LED MQTT LISTENER      ");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  relayOff(); // Start OFF
  Serial.println("✅ Relay and LED initialized OFF");

  // Connect WiFi
  connectWiFi();

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  // Connect MQTT
  if (wifiConnected) connectMQTT();

  Serial.println("\n🎯 System ready. Listening for MQTT messages...\n");
}

void loop() {
  // WiFi reconnection check
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("⚠️  WiFi disconnected! Reconnecting...");
      wifiConnected = false;
      mqttConnected = false;
    }
    connectWiFi();
    return;
  } else if (!wifiConnected) {
    wifiConnected = true;
    Serial.println("✅ WiFi reconnected!");
  }

  // MQTT reconnection check
  if (!mqttClient.connected()) {
    if (mqttConnected) {
      Serial.println("⚠️  MQTT disconnected! Reconnecting...");
      mqttConnected = false;
    }
    connectMQTT();
  }

  mqttClient.loop();

  // Optional: manual Serial control
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '0') {
      relayOff();
    } else if (cmd == '1') {
      relayOn();
    }
  }
}

void connectWiFi() {
  Serial.print("\n🔌 Connecting to WiFi: ");
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
    Serial.print("   IP Address: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  } else {
    Serial.println("\n❌ WiFi connection failed!");
    wifiConnected = false;
  }
}

void connectMQTT() {
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 3) {
    Serial.print("🔗 Connecting to MQTT broker...");

    String clientId = "ESP32-RELAY-";
    clientId += String(WiFi.macAddress());
    clientId.replace(":", "");

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" ✅ Connected!");
      if (mqttClient.subscribe(mqtt_topic)) {
        Serial.print("📥 Subscribed to: ");
        Serial.println(mqtt_topic);
        mqttConnected = true;
      } else {
        Serial.println("❌ Failed to subscribe");
      }
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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

  Serial.print("\n📩 MQTT Message [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Toggle relay/LED based on message
  if (String(topic) == mqtt_topic) {
    if (message == "1") {
      relayOn();
    } else if (message == "0") {
      relayOff();
    } else {
      Serial.println("⚠️ Unknown MQTT message");
    }
  }
}
