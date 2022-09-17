#include <Arduino.h>
#include <WiFi.h>

#include <PubSubClient.h>
#include <SPI.h>
#include <Adafruit_TCS34725.h>

#include "secrets.h"

#define WAIT_TIME_BEFORE_CONNECTION_RETRY 5000

const char *TAG = "MQTT Client";

const char *topic = "esp32/color";

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_600MS, TCS34725_GAIN_1X);
const int colorSensorLedPin = 26;
unsigned long sensorPreviousUpdateTime = 0;
#define SENSOR_UPDATE_TIME_INTERVAL 2000

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length) {
  ESP_LOGD(TAG, "Message arrived in topic: %s\nMessage:", topic);
  for (int i = 0; i < length; i++) {
    ESP_LOGD(TAG, "%c", (char) payload[i]);
  }
  ESP_LOGD(TAG, "\n");
}

void reconnect_mqtt() {
  while (!client.connected()) {
    ESP_LOGD(TAG, "Attempting MQTT connection");
    if (client.connect("arduinoClient")) {
      ESP_LOGD(TAG, "MQTT connection established");
      client.subscribe(topic);
    } else {
      ESP_LOGE(TAG, "MQTT connection failed, status=%d\nTry again in %d seconds", client.state(), WAIT_TIME_BEFORE_CONNECTION_RETRY);
      delay(WAIT_TIME_BEFORE_CONNECTION_RETRY);
    }
  }
}

void readColorSensor(char * hex) {
  float red, green, blue;

  tcs.getRGB(&red, &green, &blue);

  char r[12], g[12], b[12];
  sprintf(r, "%x", (int)red);
  sprintf(g, "%x", (int)green);
  sprintf(b, "%x", (int)blue);
  strcpy(hex, r);
  strcat(hex, g);
  strcat(hex, b);
}

void setup() {
  Serial.begin(115200);

  ESP_LOGD(TAG, "Connecting to WiFi SSID: %s", SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    ESP_LOGE(TAG, "WiFi connection failed, status=%d\nTry again in %d seconds", WiFi.status(), WAIT_TIME_BEFORE_CONNECTION_RETRY);
    delay(WAIT_TIME_BEFORE_CONNECTION_RETRY);
  }
  ESP_LOGD(TAG, "WiFi connection established\n");
  ESP_LOGD(TAG, "%s", WiFi.localIP().toString());

  if (tcs.begin()) {
    pinMode(colorSensorLedPin, OUTPUT);
    ESP_LOGD(TAG, "Found TCS34725 sensor");
    tcs.enable();
  } else {
    ESP_LOGD(TAG, "No TCS34725 found");
    return;
  }

  client.setServer(BROKER_IP, BROKER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  } else {
    if ((millis() - sensorPreviousUpdateTime) > SENSOR_UPDATE_TIME_INTERVAL) {
      char hex[36];
      readColorSensor(hex);
      client.publish(topic, hex);
      sensorPreviousUpdateTime = millis();
    }
  }
  
  client.loop();
}