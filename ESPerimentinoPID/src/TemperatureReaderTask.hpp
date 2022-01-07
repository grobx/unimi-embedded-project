/*
   TemperatureReaderTask.hpp
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
 * @brief Read temperature indexed by index from a OneWire Dallas Temperature
 * sensor and write it to temp.
 */
class TemperatureReaderTask: public Task {

  Experiment& experiment;
  uint8_t index;
  float& temp;

public:

  TemperatureReaderTask (
    Experiment& _experiment,
    uint8_t _index,
    float& _temp
  ):
    Task(100 * TASK_MILLISECOND, TASK_FOREVER),
    experiment{_experiment},
    index{_index},
    temp{_temp}
  {};

  bool Callback () {
    experiment.sensors.requestTemperatures();
    temp = experiment.sensors.getTempCByIndex(index);
    return true;
  };

};
