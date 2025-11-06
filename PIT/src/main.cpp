#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

// Pin definitions for ESP32
#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN);
WiFiMulti wifiMulti;

// Update to match your Laravel API route
const char* serverName = "http://10.10.10.34/api/rfids"; // Changed to /api/rfids

// Variables
String rfidData = "";
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000;
bool wifiConnected = false;

// Function prototypes
void readRFID();
void sendRFIDData(String rfidData);
void connectToWiFi();

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize RFID
  SPI.begin(18, 19, 23);
  rfid.PCD_Init();
  Serial.println("RFID reader initialized");

  Serial.println("Scanning for available WiFi networks...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  
  delay(100);

  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.printf("%d networks found:\n", n);
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s (RSSI: %d dBm) Encryption: %d Channel: %d\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i),
                    WiFi.encryptionType(i),
                    WiFi.channel(i));
      delay(10);
    }
  }
  Serial.println("--------------------------------------------------");
  
  // WiFiMulti setup
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("GIGA NIGGA", "12345678910");
  wifiMulti.addAP("ElectTest", "12345678");
  wifiMulti.addAP("Cloud Control Network", "ccv7network");

  connectToWiFi();

  Serial.println("System ready. Scan RFID cards...");
  Serial.println();
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi (direct)…");

  WiFi.mode(WIFI_STA);
  WiFi.begin("GIGA NIGGA", "12345678910");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    wifiConnected = true;
  } else {
    Serial.println("\n❌ Failed to connect to WiFi");
    Serial.print("Status code: ");
    Serial.println(WiFi.status());
    wifiConnected = false;
  }
}

void loop() {
  if (wifiMulti.run() == WL_CONNECTED) {
    if (!wifiConnected) {
      wifiConnected = true;
      Serial.println("WiFi reconnected!");
    }

    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      readRFID();
    }
  } else {
    if (wifiConnected) {
      wifiConnected = false;
      Serial.println("WiFi disconnected! Reconnecting...");
    }
    delay(2000);
    connectToWiFi();
  }

  delay(50);
}

void readRFID() {
  if (millis() - lastReadTime < readInterval) {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  rfidData = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    rfidData.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""));
    rfidData.concat(String(rfid.uid.uidByte[i], HEX));
  }
  rfidData.toUpperCase();

  Serial.println();
  Serial.print("RFID Detected: ");
  Serial.println(rfidData);

  sendRFIDData(rfidData);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  lastReadTime = millis();
}

void sendRFIDData(String rfidData) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send data - WiFi disconnected");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  Serial.print("Attempting to connect to: ");
  Serial.println(serverName);

  if (!http.begin(client, serverName)) {
    Serial.println("❌ Failed to begin HTTP connection");
    return;
  }

  // Headers for Laravel API
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.setTimeout(10000);

  // Updated JSON payload to match your RfidController store method
  String jsonPayload = "{\"rfid_number\":\"" + rfidData + "\", \"status\":1}";
  
  Serial.println("Sending to Laravel API...");
  Serial.print("JSON Payload: ");
  Serial.println(jsonPayload);

  int httpResponseCode = http.POST(jsonPayload);
  
  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Server Response: ");
    Serial.println(response);
    
    if (httpResponseCode == 201) { // 201 Created - Laravel store success
      Serial.println("✅ Success! RFID data stored in Laravel.");
    } else if (httpResponseCode == 422) {
      Serial.println("❌ Error 422: Validation error. Check if rfid_number is unique in your database.");
    } else if (httpResponseCode == 404) {
      Serial.println("❌ Error 404: Endpoint not found. Check if /api/rfids route exists.");
    } else if (httpResponseCode == 500) {
      Serial.println("❌ Error 500: Server error. Check Laravel logs.");
    }
  } else {
    Serial.print("❌ Failed to connect. Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}