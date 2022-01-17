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

#include <PubSubClient.h>
#include <ArduinoJson.h>

class PrintTask: public Task {

  static const int maxJson = 1024;
  static const int maxSerialPrint = 255;

  char data[maxSerialPrint];
  uint length;

public:

  Experiment& experiment;
  PubSubClient& mqttClient;
  DynamicJsonDocument doc;

  PrintTask (
    Experiment& _experiment,
    PubSubClient& _mqttClient
  ):
    Task(1000 * TASK_MILLISECOND, TASK_FOREVER),
    experiment{_experiment}, mqttClient{_mqttClient}, doc{1024}
  {};

  bool Callback () {
    length = snprintf(
      &data[0], maxSerialPrint,
      "\n%s\n%s\nfan_rpm:%d\n",
      experiment.internal.pp("I"), experiment.external.pp("E"), experiment.vars.fan_rpm);
    mqttClient.publish("esperimentino/data/text", &data[0], length);
    mqttClient.publish("esperimentino/data/var/Itemp", experiment.internal.pp_temp());
    mqttClient.publish("esperimentino/data/var/Isetpoint", experiment.internal.pp_setpoint());
    mqttClient.publish("esperimentino/data/var/Iheat", experiment.internal.pp_heat());
    mqttClient.publish("esperimentino/data/var/Etemp", experiment.external.pp_temp());
    mqttClient.publish("esperimentino/data/var/Esetpoint", experiment.external.pp_setpoint());
    mqttClient.publish("esperimentino/data/var/Eheat", experiment.external.pp_heat());
    Serial.print(data);

    return true;
  };

};
