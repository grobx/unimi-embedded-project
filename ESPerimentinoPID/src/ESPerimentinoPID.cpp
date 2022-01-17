/*
   ESPerimentinoPID.cpp
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

/**
 * @file ESPerimentinoPID.cpp
 * @author Giuseppe Roberti (inbox@roberti.dev)
 * @brief Embedded System Project
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2021-2022
 */

/**
 * @mainpage Main Page
 * 
 * This is the documentation site for the project ESPerimentino PID. You can
 * browse the code and contribute by visiting https://github.com/roberti42/unimi-embedded-project
 */

#include "TaskSchedulerDefs.h"
#include <TaskScheduler.h>

#include "TemperatureReaderTask.hpp"
#include "ComputeHeatTask.hpp"
#include "ComputeInternalSetpointTask.hpp"
#include "SetpointInputTask.hpp"
#include "PrintTask.hpp"
#include "MQTTClientLoopTask.hpp"

#include "Experiment.hpp"
#include "LocalConfig.h"

Experiment experiment (boardConfig, networkConfig, heaterConfig);

TemperatureReaderTask internalTemperatureReaderTask (experiment, 0, experiment.vars.internalTemp);
TemperatureReaderTask externalTemperatureReaderTask (experiment, 1, experiment.vars.externalTemp);
ComputeHeatTask computeHeaterDutyTask (experiment);
ComputeInternalSetpointTask computeInternalSetpointTask (experiment);
SetpointInputTask setpointInputTask (experiment);
MQTTClientLoopTask mqttClientLoopTask (experiment);
PrintTask printSerialTask (experiment, mqttClientLoopTask.client);

void setup() {
  // Setup Serial
  Serial.begin(115200);

  // Initialize Temperature Sensors
  experiment.sensors.begin();

  // Internal Temperature Reader Task
  experiment.runner.addTask(internalTemperatureReaderTask);
  internalTemperatureReaderTask.enable();

  // External Temperature Reader Task
  experiment.runner.addTask(externalTemperatureReaderTask);
  externalTemperatureReaderTask.enable();

  // Compute Heater Duty Task
  experiment.runner.addTask(computeHeaterDutyTask);
  computeHeaterDutyTask.enable();

  // Compute Internal Setpoint Task (Ambient)
  experiment.runner.addTask(computeInternalSetpointTask);
  computeInternalSetpointTask.enable();

  // Setpoint Input Task
  experiment.runner.addTask(setpointInputTask);
  setpointInputTask.enable();

  // Printer Task
  experiment.runner.addTask(printSerialTask);
  printSerialTask.enable();

  // Enable MQTT client loop
  experiment.runner.addTask(mqttClientLoopTask);
  mqttClientLoopTask.enable();

  // Setup Experiment
  experiment.Setup();
}

void loop() { experiment.runner.execute(); }
