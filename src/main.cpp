#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"

#define WAIT_TIME_BEFORE_CONNECTION_RETRY 5000

const char *TAG = "MQTT Client";

const char *topic = "esp32/test";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length) {
  ESP_LOGD(TAG, "Message arrived in topic: %s\nMessage:", topic);
  for (int i = 0; i < length; i++) {
    ESP_LOGD(TAG, "%s", (char) payload[i]);
  }
  ESP_LOGD(TAG, "\n");
}

void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    ESP_LOGD(TAG, "Attempting MQTT connection");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      ESP_LOGD(TAG, "MQTT connection established");

      // Once connected, publish an announcement...
      client.publish(topic,"hello world");
      // ... and resubscribe
      client.subscribe(topic);

    } else {
      ESP_LOGE(TAG, "MQTT connection failed, status=%d\nTry again in %d seconds", client.state(), WAIT_TIME_BEFORE_CONNECTION_RETRY);
      delay(WAIT_TIME_BEFORE_CONNECTION_RETRY);
    }
  }
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

  client.setServer(BROKER_IP, BROKER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  } else {
    // client.publish(topic, "Hello");
    // client.subscribe(topic);
  }
  client.loop();
}