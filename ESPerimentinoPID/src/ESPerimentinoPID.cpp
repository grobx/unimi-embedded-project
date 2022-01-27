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
 * @version 0.2
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2021-2022
 */

/**
 * @mainpage Main Page
 * 
 * This is the documentation site for the project ESPerimentino PID. You can
 * browse the code and contribute by visiting https://github.com/grobx/unimi-embedded-project
 */

#include "TaskSchedulerDefs.h"
#include <TaskScheduler.h>

#include "task/TemperatureReaderTask.hpp"
#include "task/ComputeHeatTask.hpp"
#include "task/ComputeInternalSetpointTask.hpp"
#include "task/SetpointInputTask.hpp"
#include "task/PrintTask.hpp"
#include "task/MQTTClientLoopTask.hpp"

#include "Experiment.hpp"
#include "LocalConfig.h"

Experiment experiment (boardConfig, networkConfig, heaterConfig, defaultConfig);

TemperatureReaderTask internalTemperatureReaderTask (
  experiment, 0, experiment.vars.internalTemp, heaterConfig.readInterval);

TemperatureReaderTask externalTemperatureReaderTask (
  experiment, 1, experiment.vars.externalTemp, heaterConfig.readInterval);

ComputeHeatTask computeHeaterDutyTask (
  experiment, 100 * TASK_MILLISECOND);

ComputeInternalSetpointTask computeInternalSetpointTask (
  experiment, 100 * TASK_MILLISECOND);

MQTTClientLoopTask mqttClientLoopTask (experiment, 100 * TASK_MILLISECOND);

// SetpointInputTask setpointInputTask (experiment, 100 * TASK_MILLISECOND);

PrintTask printSerialTask (
  experiment, mqttClientLoopTask.client, 100 * TASK_MILLISECOND);

void fan_setup() {
  // Relay PIN
  pinMode(boardConfig.relayPin, OUTPUT);

  // FAN RPM Counter PIN
  pinMode(boardConfig.rpmCounterPin, INPUT_PULLUP);
  attachInterrupt(
    boardConfig.rpmCounterPin,
    []() {
      unsigned long delta_t = micros() - experiment.vars.fan_last_t;
      experiment.vars.fan_rpm = 60000000 / delta_t;
      experiment.vars.fan_last_t = micros();
      if (experiment.vars.fan_rpm > 7000)
        experiment.vars.fan_rpm = 0;
    },
    RISING
  );
}

void setup() {
  // Setup Serial
  Serial.begin(115200);

  // Setup FAN
  fan_setup();

  // Initialize Digital Temperature Sensors
  experiment.sensors.begin();
  experiment.sensors.setWaitForConversion(false);

  // Register tasks into Scheduler and enable them
  for (auto t : std::vector<Task*> {
    &internalTemperatureReaderTask, // Internal Temperature Reader Task
    &externalTemperatureReaderTask, // External Temperature Reader Task
    &computeHeaterDutyTask,         // Compute Heater Duty Task
    &computeInternalSetpointTask,   // Compute Internal Setpoint Task (Ambient)
    // &setpointInputTask,             // Setpoint Input Task
    &printSerialTask,               // Printer Task
    &mqttClientLoopTask             // Enable MQTT client loop
  }) {
    experiment.runner.addTask(*t);
    t->enable();
  }
}

void loop() { experiment.runner.execute(); }
