/*
   SetpointInputTask.hpp
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

class SetpointInputTask: public Task {

public:

  Experiment& experiment;

  SetpointInputTask (
    Experiment& _experiment,
    unsigned long aInterval = 100 * TASK_MILLISECOND
  ):
    Task(aInterval, TASK_FOREVER),
    experiment{_experiment}
  {}

  bool Callback () {
    if (Serial.available()) {
      experiment.external.setpoint = Serial.parseFloat();
      Serial.readStringUntil('\n');
      experiment.external.reset();
    }

    // int n = Serial.available();
    // if (n) {
    //   String data = Serial.readString(n);
    //   if (data.charAt(n) != '\n') return;
    //   setpoint = data.parseFloat();
    //   experiment.external.reset();
    // }

    return true;
  }

};
