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
#include "MQTTClient.hpp"

static ulong lastExperiment;

/**
 * @brief Setup WiFi and MQTT then read experiment from MQTT and run it.
 * 
 * When enabled, this Task will setup WiFi and MQTT and then overy time it ticks
 * it will handle the last received experiment, if any, by running it using
 * experiment.Run()
 */
class MQTTClientLoopTask: public Task {

  size_t length;
  static constexpr size_t maxJsonPrint = 255;
  uint8_t json[maxJsonPrint];

  static constexpr size_t maxJson = 2048;
  DynamicJsonDocument doc;

public:

  MQTTClient client;
  Experiment& experiment;

  MQTTClientLoopTask(
    Experiment& _experiment,
    unsigned long aInterval = 100 * TASK_MILLISECOND
  ):
    Task(aInterval, TASK_FOREVER),
     doc{maxJson}, client{_experiment}, experiment{_experiment}
  {}

  bool Callback () {
    if (HaveExperiment()) {
      ExperimentConfig config = ParseExperiment();

      experiment.Run(config);

      MarkExperimentDone();

      PublishSetupDone();
    }

    client.Loop();

    return true;
  }

  bool OnEnable() { return client.Setup(); }

private:

  bool HaveExperiment() { return client.LastRead() > lastExperiment; }

  void MarkExperimentDone() { lastExperiment = micros(); }

  /**
   * @brief Parse Experiment contained in lastPayload and change doc accordingly
   * 
   * This class will parse the ExperimentConfig contained in lastPayload
   * and change json doc accordingly so it can be for instance serialized
   * and sent as MQTT message (see PublishSetupDone())
   * 
   * @return ExperimentConfig 
   */
  ExperimentConfig ParseExperiment() {
    deserializeJson(doc, lastPayload);

    ExperimentConfig config = experiment.defaultConfig;

    JsonObject obj = doc.as<JsonObject>();

    Set(obj, "useFan", config.useFan);
    Set(obj, "setpoint", config.setpoint);
    Set(obj, "IKp", config.internalPidParams.Kp);
    Set(obj, "IKi", config.internalPidParams.Ki);
    Set(obj, "IKd", config.internalPidParams.Kd);
    Set(obj, "EKp", config.externalPidParams.Kp);
    Set(obj, "EKi", config.externalPidParams.Ki);
    Set(obj, "EKd", config.externalPidParams.Kd);

    return config;
  }

  void PublishSetupDone() {
    length = serializeJson(doc, json, maxJsonPrint);

    client.PublishSetupDone(json, length);
  }

private:

  template<typename T>
  void Set(JsonObject& obj, const char *key, T& ref) {
    if (obj.containsKey(key))
      ref = obj[key];
    else
      doc[key] = ref;
  }

};
