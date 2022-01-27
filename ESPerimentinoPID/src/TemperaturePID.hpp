/*
   TemperaturePID.hpp
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

#include "Config.h"

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#include <QuickPID.h>

class TemperaturePID {

  static const uint maxPPvarLength = 32;
  static const uint maxPPLength = 256;
  
  char PPvar[maxPPvarLength];
  char PP[maxPPLength];

public:

  float& temp;     // input
  float& heat;     // output
  float& setpoint; // target
  QuickPID PID;
  PidParams params;

  TemperaturePID (float& _input, float& _output, float& _target):
    temp{_input}, heat{_output}, setpoint{_target},
    PID(&temp, &heat, &setpoint)
  {
    PID.SetControllerDirection(QuickPID::Action::direct);
  }

  void reset () {
    // heat = experiment.heaterConfig.minDuty;
    PID.SetMode(QuickPID::Control::manual);
    PID.SetMode(QuickPID::Control::automatic);
  }

  bool compute () {
    return PID.Compute ();
  }

  void setParams (PidParams _params) {
    params = _params;
    PID.SetTunings(params.Kp, params.Ki, params.Kd);
  }

  bool enabled () {
    return params.Kp != 0.0 || params.Ki != 0.0 || params.Kd != 0.0;
  }

  char* pp (const char* prefix) {
    sprintf(&PP[0],
      "%stemp:%+8.3f,%ssetpoint:%+8.3f,%sheat:%+8.3f,%sP:%+9.3f,%sI:%+9.3f,%sD:%+9.3f,%sX:%+9.3f",
      prefix, temp,
      prefix, setpoint,
      prefix, heat,
      prefix, PID.GetPterm(),
      prefix, PID.GetIterm(),
      prefix, PID.GetDterm(),
      prefix, PID.GetPterm() + PID.GetIterm() + PID.GetDterm()
    );
    return &PP[0];
  }

  char* pp_temp () {
    snprintf(&PPvar[0], maxPPvarLength, "%+8.3f", temp);
    return &PPvar[0];
  }

  char* pp_setpoint () {
    snprintf(&PPvar[0], maxPPvarLength, "%+8.3f", setpoint);
    return &PPvar[0];
  }

  char* pp_heat () {
    snprintf(&PPvar[0], maxPPvarLength, "%+8.3f", heat);
    return &PPvar[0];
  }
};
