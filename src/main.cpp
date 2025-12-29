#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>

#if defined(ESP32)
#include <ESPmDNS.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
// #include <ESP8266WiFiMulti.h>
// ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif


#include "secrets.h"

#define DIGITAL_PIN D0
#define ANALOG_PIN A0

// InfluxDB write interval (seconds)
const uint32_t WRITE_INTERVAL = 5;
volatile bool isOtaInProgress = false;
bool otaInProgress = false;

void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint8_t retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
    if (++retries >= 120) {
      Serial.println("\nWi-Fi connect timeout, restarting...");
      ESP.restart();
    }
  }
  Serial.println();
  Serial.print("Connected, IP=");
  Serial.println(WiFi.localIP());

  // Start OTA
  Serial.println("Starting ArduinoOTA...");
  ArduinoOTA.begin();
  Serial.println("ArduinoOTA started, listening on port 3232");
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Update Start");
    otaInProgress = true;  // Set flag to pause readings
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA Update End");
    otaInProgress = false;  // Resume readings
  });
}



String influxUrl() {
  String url = String(INFLUXDB_URL) + "/api/v2/write?org=" + INFLUXDB_ORG +
               "&bucket=" + INFLUXDB_BUCKET + "&precision=s";
  return url;
}

void sendToInflux(int analogValue, int digitalValue) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = influxUrl();

  if (!https.begin(client, url)) {
    Serial.println("Failed to begin HTTPS");
    return;
  }

  https.addHeader("Authorization", String("Token ") + INFLUXDB_TOKEN);
  https.addHeader("Content-Type", "text/plain; charset=utf-8");

  // Build line protocol: measurement=gas_sensor, add fields
  // Use integer type suffix 'i' for integer fields
  String line = "gas_sensor analog=" + String(analogValue) +
                "i,digital=" + String(digitalValue) + "i";

  int httpCode = https.POST(line);
  if (httpCode > 0) {
    // Serial.print("Influx HTTP code: ");
    // Serial.println(httpCode);
    if (httpCode >= 200 && httpCode < 300) {
      // Serial.println("Write ok");
    } else {
      String payload = https.getString();
      Serial.print("Response: ");
      Serial.println(payload);
    }
  } else {
    Serial.print("POST failed, error: ");
    Serial.println(https.errorToString(httpCode).c_str());
  }

  https.end();
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  WiFi.setHostname("GasDetector");
  ArduinoOTA.setHostname("GasDetector");

  connectWiFi();
}

void loop() {
  ArduinoOTA.handle();  // Handle OTA updates
  if (otaInProgress) {
    return;
  }

  if (isOtaInProgress) {
    // During OTA, only process OTA handler to minimize interference
    return;  // Skip all other code
  }

  static uint32_t lastWrite = 0;
  uint32_t now = millis() / 1000;
  if (now - lastWrite >= WRITE_INTERVAL) {
    int analogValue = analogRead(ANALOG_PIN);  // 0-1023
    int digitalValue = digitalRead(DIGITAL_PIN) == HIGH ? 1 : 0;

    Serial.print("Analog: ");
    Serial.print(analogValue);
    Serial.print("  Digital: ");
    Serial.println(digitalValue);

    sendToInflux(analogValue, digitalValue);
    lastWrite = now;
  }

  delay(100);
}
