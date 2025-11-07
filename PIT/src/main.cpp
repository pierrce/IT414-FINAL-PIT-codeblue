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

// UPDATED: Your Laravel server IP and port
const char* serverName = "http://10.16.42.197:8000/api/rfids";

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
  
  // WiFiMulti setup with your networks
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("GIGA NIGGA", "12345678910");
  //wifiMulti.addAP("ParaFiber_CFA8-5G", "charlene1997***");
  //wifiMulti.addAP("ElectTest", "12345678");
  //wifiMulti.addAP("Cloud Control Network", "ccv7network");

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
    Serial.println("WiFi disconnected — cannot send data.");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  if (!http.begin(client, serverName)) {
    Serial.println("Failed to connect to server.");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  http.setTimeout(10000);

  String jsonPayload = "{\"rfid_number\":\"" + rfidData + "\"}";
  int httpResponseCode = http.POST(jsonPayload);

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
      Serial.println("FOUND");
    } else {
      Serial.println("RFID NOT FOUND");
    }
    Serial.println();
  }

  http.end();
}