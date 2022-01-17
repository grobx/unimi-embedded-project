/*
   MQTTClientLoopTask.hpp
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

#include "TaskSchedulerDefs.h"
#include "SetpointInputTask.hpp"
#include "Config.h"
#include "Experiment.hpp"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static const uint maxPayloadLength = 1024;
static const uint maxTopicLength = 255;

static ulong lastRead, lastExperiment;
static char lastTopic[maxTopicLength];
static uint lastPayloadLength;
static uint8_t lastPayload[maxPayloadLength];

/**
 * @brief Setup WiFi and MQTT then read experiment from MQTT and run it.
 * 
 * When enabled, this Task will setup WiFi and MQTT and then overy time it ticks
 * it will handle the last received experiment, if any, by running it using
 * experiment.Run()
 */
class MQTTClientLoopTask: public Task {

public:

  WiFiClient wifiClient;
  PubSubClient client;
  Experiment& experiment;
  DynamicJsonDocument doc;

  MQTTClientLoopTask(
    Experiment& _experiment
  ):
    Task(100 * TASK_MILLISECOND, TASK_FOREVER),
    client(wifiClient),
    experiment{_experiment}, doc{1024}
  {}

  bool Callback () {
    if (lastRead > lastExperiment) {
      client.publish("esperimentino/data/debug/recv_payload", lastPayload, lastPayloadLength);

      deserializeJson(doc, lastPayload);

      ExperimentConfig config = {
        true,                       // useFan
        ExperimentConfig::QuickPID, // pidImplementation
        false,                      // pidAutotuning
        0.00,                       // celsiusIncrement
        { 100.00,   1.00,  50.00 }, // internalPidParams
        { 200.00,   1.00,   1.00 }, // externalPidParams
      };

      JsonObject obj = doc.as<JsonObject>();

      if (obj.containsKey("useFan")) config.useFan = obj["useFan"];
      if (obj.containsKey("celsiusIncrement")) config.celsiusIncrement = obj["celsiusIncrement"];
      if (obj.containsKey("internalPidParams")) {
        config.internalPidParams.Kp = obj["internalPidParams"]["Kp"];
        config.internalPidParams.Ki = obj["internalPidParams"]["Ki"];
        config.internalPidParams.Kd = obj["internalPidParams"]["Kd"];
      }
      if (obj.containsKey("externalPidParams")) {
        config.internalPidParams.Kp = obj["externalPidParams"]["Kp"];
        config.internalPidParams.Ki = obj["externalPidParams"]["Ki"];
        config.internalPidParams.Kd = obj["externalPidParams"]["Kd"];
      }

      experiment.Run(config);

      lastExperiment = millis();
    }

    client.loop();

    return true;
  }

  bool OnEnable() {
    WiFiSetup();

    MQTTSetup();

    return true;
  }

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
    if (client.connect("id", "mqtt", "q1w2e3r4t5")) {
      client.subscribe("esperimentino/setup");
      client.publish("esperimentino", "running ...");
      Serial.printf("MQTT server setup done [%s:%d]\n", experiment.networkConfig.brokerAddress, experiment.networkConfig.port);
    }
  }

  static void HandleMessage(char* topic, byte* payload, uint plength) {
    strncpy(&lastTopic[0], topic, strlen(topic));

    lastPayloadLength = plength <= maxPayloadLength ? plength : maxPayloadLength;
    memcpy(&lastPayload[0], payload, lastPayloadLength);

    lastRead = millis();

    Serial.print("Message arrived [");
    Serial.print(lastTopic);
    Serial.print("] ");
    for (int i = 0; i < lastPayloadLength; i++) Serial.print((char)lastPayload[i]);
    Serial.println();
  }

};
