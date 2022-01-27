/*
   MQTTClient.hpp
   Copyright (C) 2021-2022 Giuseppe Roberti

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "Experiment.hpp"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static const size_t maxPayloadLength = 1024;
static const size_t maxTopicLength = 255;

static ulong lastRead;
static char lastTopic[maxTopicLength];
static uint lastPayloadLength;
static uint8_t lastPayload[maxPayloadLength];

class MQTTClient {

  static constexpr size_t maxVarName = 16;
  char varTopicName[maxVarName];

  const char* setupDoneTopic = "esperimentino/setup/done";
  const char* setupTopic = "esperimentino/setup";
  const char* textTopic = "esperimentino/data/text";
  const char* jsonTopic = "esperimentino/data/json";
  const char* varTopic = "esperimentino/data/var";

  Experiment& experiment;
  WiFiClient wifiClient;

public:

  PubSubClient client;

  MQTTClient(Experiment& _experiment):
    experiment{_experiment}, client{wifiClient}
  {}

  bool Setup() {
    WiFiSetup();

    MQTTSetup();

    return true;
  }

  ulong LastRead() { return lastRead; }

  bool Loop() { return client.loop(); }

  bool Publish(const char* topic, const uint8_t* payload, unsigned int plength) {
    if (client.beginPublish(topic, plength, false)) {
      if (client.write(payload, plength) != plength) return false;

      return client.endPublish();
    }

    return false;
  }

  bool PublishJson(const uint8_t* json, unsigned int length) {
    return Publish(jsonTopic, json, length);
  }

  bool PublishText(const uint8_t* text, unsigned int length) {
    return Publish(textTopic, text, length);
  }

  bool PublishVar(const char* name, const uint8_t* value) {
    return Publish(
      varTopicName,
      value,
      snprintf(varTopicName, maxVarName, "%s/%s", varTopic, name)
    );
  }

  bool PublishSetupDone(const uint8_t* setup, unsigned int length) {
    return Publish(setupDoneTopic, setup, length);
  }

private:

  void WiFiSetup() {
    delay(10);

    Serial.printf("Connecting to WiFi '%s' ...", experiment.networkConfig.ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(experiment.networkConfig.ssid, experiment.networkConfig.password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      // FIXME Serial.print("."); FIXME
    }

    randomSeed(micros());

    Serial.print(" connected [IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println("]");
  }

  void MQTTSetup() {
    client.setServer(experiment.networkConfig.brokerAddress, experiment.networkConfig.port);

    client.setCallback(HandleMessage);

    if (client.connect(
          "esperimentino-board",
          experiment.networkConfig.mqttUser,
          experiment.networkConfig.mqttPass)
    ) {
      client.subscribe(setupTopic);

      client.publish("esperimentino", "running ...");

      Serial.printf(
        "MQTT server setup done [%s:%d]\n",
        experiment.networkConfig.brokerAddress,
        experiment.networkConfig.port
      );
    }
  }

  static void HandleMessage(char* topic, byte* payload, uint plength) {
    strncpy(&lastTopic[0], topic, strlen(topic));

    lastPayloadLength = plength <= maxPayloadLength ? plength : maxPayloadLength;
    memcpy(&lastPayload[0], payload, lastPayloadLength);

    lastRead = micros();

#ifdef DEBUG
    Serial.print("Message arrived [");
    Serial.print(lastTopic);
    Serial.print("] ");
    for (int i = 0; i < lastPayloadLength; i++) Serial.print((char)lastPayload[i]);
    Serial.println();
#endif
  }

};
