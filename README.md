# Gas-Detector

This code is designed as a test of the Flying Fish MQ-5 gas sensore
while helpfully reporting the data onto Influx DB for review.

Gas Detector - ESP8266/ESP32 InfluxDB Logger
A simple Arduino sketch for ESP8266 or ESP32 that reads a gas sensor (e.g., MQ-series) with both analog and digital outputs and sends the values to an InfluxDB v2 instance every 5 seconds. It also supports Over-The-Air (OTA) firmware updates.
Features

Reads analog (0–1023) and digital (HIGH/LOW) values from a gas sensor
Sends data to InfluxDB v2 using the Line Protocol over HTTPS
Automatic Wi-Fi reconnection with timeout and restart
Full Arduino OTA support (upload new firmware wirelessly)
Pauses sensor readings during OTA to avoid interference
Works on both ESP8266 and ESP32 (automatically detected)

Hardware Requirements

ESP8266 (NodeMCU, Wemos D1 Mini, etc.) or ESP32
Gas sensor with analog and digital output, e.g.:
MQ-2, MQ-3, MQ-5, MQ-7, etc.

Connections:
Analog output → A0 (ESP8266) or default ADC pin (ESP32)
Digital output → D0 / GPIO16 (ESP8266) or any GPIO with pull-up (ESP32).
Note: D0 must support INPUT_PULLUP (on ESP8266 it does).


Software Requirements

Arduino IDE with ESP8266/ESP32 board support installed
Libraries (installed via Library Manager):
ArduinoOTA (built-in for ESP32, install for ESP8266)
WiFiClientSecure (built-in)
HTTPClient (built-in for ESP32, install ESP8266HTTPClient for ESP8266)


Configuration
Create a file named secrets.h in the same folder as the sketch with the following content:
C++#ifndef SECRETS_H
#define SECRETS_H

// Wi-Fi credentials
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"

// InfluxDB v2 configuration
#define INFLUXDB_URL  "https://your-influxdb-domain.com"  // e.g., https://eu-central-1-1.aws.cloud2.influxdata.com
#define INFLUXDB_TOKEN "your_super_secret_auth_token"
#define INFLUXDB_ORG   "your_org_name_or_id"
#define INFLUXDB_BUCKET "your_bucket_name"

#endif
Never commit secrets.h to version control!


Pinout
Sensor Pin,ESP Pin,Notes
Analog Out,A0,"ADC input (0–1023 on ESP8266, 0–4095 on ESP32 but scaled)"
Digital Out,D0 (GPIO16),Configured with INPUT_PULLUP