/*
   ComputeHeatTask.hpp
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
#include "Experiment.hpp"

/**
 * @brief Compute heater duty using pid then actuate heater
 */
class ComputeHeatTask: public Task {

public:

  Experiment& experiment;

  ComputeHeatTask (
    Experiment& _experiment,
    unsigned long aInterval = 1000 * TASK_MILLISECOND
  ):
    Task (aInterval, TASK_FOREVER),
    experiment{_experiment}
  {}

  bool Callback () {
    if (experiment.internal.enabled()) {
      experiment.internal.compute();
    } else {
      experiment.internal.heat =
        experiment.internal.temp < experiment.internal.setpoint ?
          experiment.heaterConfig.maxDuty :
          experiment.heaterConfig.minDuty ;
    }
    SafetyCheck ();
    ledcWrite(experiment.heaterConfig.heatChannel, experiment.internal.heat);
    return true;
  }

  bool OnEnable () {
    // Setup Heater PID
    experiment.internal.PID.SetMode(QuickPID::Control::automatic);
    experiment.internal.PID.SetOutputLimits(
      experiment.heaterConfig.minDuty,
      experiment.heaterConfig.maxDuty
    );

    // Configure PWM signal to mosfet for heating
    pinMode(experiment.boardConfig.heatingPin, OUTPUT);
    ledcSetup(experiment.heaterConfig.heatChannel, experiment.heaterConfig.freq, experiment.heaterConfig.resolution);
    ledcAttachPin(experiment.boardConfig.heatingPin, experiment.heaterConfig.heatChannel);

    return true;
  }

private:

  void SafetyCheck () {
    if (experiment.internal.temp == DEVICE_DISCONNECTED_C) {
      experiment.internal.setpoint = DEVICE_DISCONNECTED_C;
      experiment.internal.heat = experiment.heaterConfig.minDuty;
    }
  }

};
