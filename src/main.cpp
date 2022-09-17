#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"

const char *mqtt_broker = BROKER_IP;
const char *topic = "esp32/test";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(SSID);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  } else {
    Serial.println("WiFi connected\n");
  }
  Serial.println(WiFi.localIP());

  Serial.print("Connected.");

  client.setServer(mqtt_broker, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  } else {
    client.publish(topic, "Hello");
    client.subscribe(topic);
  }
  client.loop();
}