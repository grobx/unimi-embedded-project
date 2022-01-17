/*
   Config.h
   Copyright (C) 2022 Giuseppe Roberti

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

struct BoardConfig {
  const int relayPin;
  const int rpmCounterPin;
  const int oneWireBusPin;
  const int heatingPin;    // heating element mosfet pin
};

struct HeaterConfig {
  const int freq;          // PWM frequency
  const int heatChannel;   // PWM channel
  const int resolution;    // PWM resolution
  const int maxDuty;       // PWM max duty
  const float maxSetpoint; // max setpoint °C
};

struct NetworkConfig {
  const char* ssid; // WiFi SSID
  const char* password; // WiFi password
  const char* brokerAddress;
  const int port;
};

struct PidParams {
  float Kp, Ki, Kd; // input from user
};

struct ExperimentConfig {
  bool useFan;                         // input from user
  enum {
    QuickPID, SomethingElse
  } pidImplementation;                 // input from user
  bool pidAutotuning;                  // input from user
  float celsiusIncrement;              // input from user
  PidParams internalPidParams;         // input from user
  PidParams externalPidParams;         // input from user
};

struct WorkVars {
  float internalTemp;       // input from world
  float internalSetpoint;   // input from external PID
  float internalHeaterDuty; // output to world (heating element duty cycle)
  float externalTemp;       // input from world
  float externalSetpoint;   // input from user
  unsigned int fan_rpm;     // fan RPM
  unsigned long fan_last_t; // last time fan RISING
};
