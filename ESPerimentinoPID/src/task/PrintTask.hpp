/*
   PrintTask.hpp
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
#include "Config.h"
#include "Experiment.hpp"
#include "MQTTClient.hpp"

#include <ArduinoJson.h>

class PrintTask: public Task {

  static constexpr size_t maxSerialPrint = 255;
  uint8_t text[maxSerialPrint];

  unsigned int length;

  static constexpr size_t maxJson = 2048;
  StaticJsonDocument<maxJson> doc;

public:

  Experiment& experiment;
  MQTTClient& mqttClient;

  PrintTask (
    Experiment& _experiment,
    MQTTClient& _mqttClient,
    unsigned long aInterval = 100 * TASK_MILLISECOND
  ):
    Task(aInterval, TASK_FOREVER),
    experiment{_experiment}, mqttClient{_mqttClient}
  {}

  bool Callback () {
    PublishJson();

    PublishText();

    PublishVars();

    return true;
  }

private:

  void PublishJson () {
    doc["us"] = micros();
    doc["Itemp"] = experiment.internal.temp;
    doc["Isetp"] = experiment.internal.setpoint;
    doc["Iheat"] = experiment.internal.heat;
    doc["IP"] = experiment.internal.PID.GetPterm();
    doc["II"] = experiment.internal.PID.GetIterm();
    doc["ID"] = experiment.internal.PID.GetDterm();
    doc["Etemp"] = experiment.external.temp;
    doc["Esetp"] = experiment.external.setpoint;
    doc["Eheat"] = experiment.external.heat;
    doc["EP"] = experiment.external.PID.GetPterm();
    doc["EI"] = experiment.external.PID.GetIterm();
    doc["ED"] = experiment.external.PID.GetDterm();
    doc["fanRPM"] = experiment.vars.fan_rpm;
    length = serializeJson(doc, text);
    if (mqttClient.PublishJson(&text[0], length) == 0)
      Serial.printf("ERROR: unable to PublishJson!");
  }

  void PublishText() {
#ifdef DEBUG
    length = snprintf(
      (char*)&text[0], maxSerialPrint, "\n%s\n%s\nfan_rpm:%d\n",
      experiment.internal.pp("I"), experiment.external.pp("E"), experiment.vars.fan_rpm
    );

    mqttClient.PublishText(&text[0], length);
#endif
  }

  void PublishVars() {
#ifdef DEBUG
    mqttClient.PublishVar("Itemp", experiment.internal.pp_temp());
    mqttClient.PublishVar("Isetp", experiment.internal.pp_setpoint());
    mqttClient.PublishVar("Iheat", experiment.internal.pp_heat());
    mqttClient.PublishVar("Etemp", experiment.external.pp_temp());
    mqttClient.PublishVar("Esetp", experiment.external.pp_setpoint());
    mqttClient.PublishVar("Eheat", experiment.external.pp_heat());
#endif
  }

};
