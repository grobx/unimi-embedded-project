/*
   Experiment.hpp
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
#include "TemperaturePID.hpp"
#include "TaskSchedulerDefs.h"

#include <DallasTemperature.h>
#include <QuickPID.h>

class Experiment {

public:

  // configuration
  const BoardConfig& boardConfig;
  const NetworkConfig& networkConfig;
  const HeaterConfig& heaterConfig;
  const ExperimentConfig& defaultConfig;

  // working variables
  WorkVars vars;

  // scheduler
  Scheduler runner;
  // Scheduler temp_runner;

  // OneWire bus and temperature sensors
  OneWire oneWireBus;
  DallasTemperature sensors;

  // PID controllers
  TemperaturePID internal;
  TemperaturePID external;

  // current experiment configuration
  ExperimentConfig config;

  Experiment(
    const BoardConfig& _boardConfig,
    const NetworkConfig& _networkConfig,
    const HeaterConfig& _heaterConfig,
    const ExperimentConfig& _defaultConfig
  ):
    boardConfig{_boardConfig}, networkConfig{_networkConfig}, heaterConfig{_heaterConfig}, defaultConfig{_defaultConfig},
    oneWireBus(boardConfig.oneWireBusPin),
    sensors(&oneWireBus),
    internal(vars.internalTemp, vars.internalHeaterDuty, vars.internalSetpoint),
    external(vars.externalTemp, vars.internalSetpoint, vars.externalSetpoint)
  {}

  void Run(ExperimentConfig experimentConfig) {
    config = experimentConfig;

    digitalWrite(boardConfig.relayPin, config.useFan ? HIGH : LOW);

    internal.setParams(config.internalPidParams);

    external.setParams(config.externalPidParams);
    external.setpoint = config.setpoint;

    Serial.printf(
      "Running experiment %s, trying to reach temperature %.3f"
      " [ PID internal: %.3f,%.3f,%.3f | external:%.3f,%.3f,%.3f ]\n",
      config.useFan ? "USING FAN" : "WITHOUT FAN",
      config.setpoint,
      config.internalPidParams.Kp, config.internalPidParams.Ki, config.internalPidParams.Kd,
      config.externalPidParams.Kp, config.externalPidParams.Ki, config.externalPidParams.Kd
    );
  }

};
