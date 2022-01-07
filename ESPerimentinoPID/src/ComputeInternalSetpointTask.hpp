/*
   ComputeInternalSetpointTask.hpp
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
 * @brief Compute the internal setpoint value using pid
 */
class ComputeInternalSetpointTask: public Task {

public:

  Experiment& experiment;

  ComputeInternalSetpointTask (Experiment& _experiment):
    Task (1000 * TASK_MILLISECOND, TASK_FOREVER),
    experiment{_experiment}
  {};

  bool Callback () {
    updateExternalLimits();
    experiment.external.compute();
    experiment.internal.setpoint = experiment.external.heat;
    return true;
  };

  bool OnEnable () {
    // Setup Ambient PID
    experiment.external.PID.SetMode(QuickPID::Control::timer);
    updateExternalLimits();

    return true;
  }

  void updateExternalLimits () {
    experiment.external.PID.SetOutputLimits(0.0, experiment.heaterConfig.maxSetpoint);
  }

};
