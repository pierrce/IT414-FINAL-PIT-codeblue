#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Relay & LED Pins
#define RELAY_PIN 27
#define LED_PIN 2

// WiFi
const char* ssid = "JOSH 2.4G";
const char* password = "Empasis031622";

// MQTT
const char* mqtt_server = "192.168.1.37";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectWiFi();
void connectMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  connectWiFi();

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);

  connectMQTT();
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");

    if (mqttClient.connect("ESP32-RELAY")) {
      Serial.println("Connected!");
      mqttClient.subscribe(mqtt_topic);
    } else {
      Serial.println("Failed. Retrying...");
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("MQTT Received: ");
  Serial.println(msg);

  if (msg == "1") {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Relay ON");
  } else if (msg == "0") {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    Serial.println("Relay OFF");
  }
}
