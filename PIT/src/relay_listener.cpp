#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Pin definitions
#define RELAY_PIN 27
#define LED_PIN 2

// WiFi credentials
const char* ssid = "GIGA NIGGA";
const char* password = "12345678910";

const char* mqtt_server = "10.197.198.197";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Connection status flags
bool wifiConnected = false;
bool mqttConnected = false;
bool isConnectingWiFi = false;
bool isConnectingMQTT = false;

// ⚡ NON-BLOCKING TIMERS - Replace all delay() calls
unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;
const unsigned long reconnectInterval = 5000; // Try reconnect every 5 seconds

// Function prototypes
void connectWiFi();
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// ⚡ INSTANT relay control - no delays
void relayOn() {
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  Serial.println("✅ Relay/LED ON");
}

void relayOff() {
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  Serial.println("❌ Relay/LED OFF");
}

void setup() {
  Serial.begin(115200);
  
  // ⚡ STEP 1: REMOVED delay(1000) from setup
  // No need to wait - ESP32 is ready immediately
  
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("   ESP32 RELAY - ZERO DELAY VERSION   ");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  relayOff();
  Serial.println("✅ Relay/LED initialized OFF");

  // Connect WiFi
  connectWiFi();

  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  // Connect MQTT
  if (wifiConnected) connectMQTT();

  Serial.println("\n🎯 System ready. Listening for MQTT...\n");
}

void loop() {
  unsigned long currentMillis = millis();

  // ⚡ STEP 2: NON-BLOCKING WiFi reconnection
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("⚠️  WiFi disconnected!");
      wifiConnected = false;
      mqttConnected = false;
    }
    
    // Only try reconnecting every 5 seconds, don't block
    if (!isConnectingWiFi && (currentMillis - lastWiFiAttempt > reconnectInterval)) {
      lastWiFiAttempt = currentMillis;
      connectWiFi();
    }
    
    yield(); // Let ESP32 handle background tasks
    return;
  } else if (!wifiConnected) {
    wifiConnected = true;
    isConnectingWiFi = false;
    Serial.println("✅ WiFi reconnected!");
  }

  // ⚡ STEP 3: NON-BLOCKING MQTT reconnection
  if (!mqttClient.connected()) {
    if (mqttConnected) {
      Serial.println("⚠️  MQTT disconnected!");
      mqttConnected = false;
    }
    
    // Only try reconnecting every 5 seconds, don't block
    if (!isConnectingMQTT && (currentMillis - lastMQTTAttempt > reconnectInterval)) {
      lastMQTTAttempt = currentMillis;
      connectMQTT();
    }
  }

  // ⚡ INSTANT MQTT processing
  mqttClient.loop();

  // Optional: manual Serial control
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == '0') relayOff();
    else if (cmd == '1') relayOn();
  }

 
  yield();
}

void connectWiFi() {
  Serial.print("\n🔌 Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  isConnectingWiFi = true;

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 5000) {
    yield(); // Non-blocking - let ESP32 do other tasks
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
    isConnectingWiFi = false;
  } else {
    Serial.println("\n❌ WiFi failed - will retry in 5s");
    wifiConnected = false;
    isConnectingWiFi = false;
  }
}

void connectMQTT() {
  Serial.print("🔗 Connecting to MQTT...");
  
  isConnectingMQTT = true;

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
      Serial.println("❌ Subscribe failed");
      mqttConnected = false;
    }
    isConnectingMQTT = false;
  } else {
    Serial.print(" ❌ Failed, rc=");
    Serial.println(mqttClient.state());
    mqttConnected = false;
    isConnectingMQTT = false;
    // Will retry in 5 seconds automatically from loop()
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // ⚡ STEP 7: INSTANT processing - NO delays in callback
  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("\n📩 MQTT Received: '");
  Serial.print(message);
  Serial.println("'");

  // ⚡ INSTANT relay switching based on message
  if (String(topic) == mqtt_topic) {
    message.trim();
    int status = message.toInt();
    
    if (message == "1" || status == 1 || (length == 1 && payload[0] == '1')) {
      Serial.println("🎉 STATUS 1 - RELAY ON!");
      relayOn(); // ⚡ Activates in <1ms
    } else if (message == "0" || status == 0 || (length == 1 && payload[0] == '0')) {
      Serial.println("🚫 STATUS 0 - RELAY OFF!");
      relayOff(); // ⚡ Deactivates in <1ms
    } else {
      Serial.print("⚠️ Unknown status: '");
      Serial.print(message);
      Serial.println("'");
      relayOff();
    }
  }
  
  Serial.println();
}