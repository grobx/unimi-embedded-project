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
  const int relayPin;       // relay fan
  const int rpmCounterPin;  // fan feedback
  const int oneWireBusPin;  // digital temperature sensor
  const int heatingPin;     // heating element mosfet
};

struct HeaterConfig {
  const int readInterval;   // read temperature interval (internal and external)
  const int freq;           // PWM frequency
  const int heatChannel;    // PWM channel
  const int resolution;     // PWM resolution
  const float minDuty;      // PWM min duty
  const float maxDuty;      // PWM max duty
  const float maxSetpoint;  // max setpoint °C
  const float minSetpoint;  // min setpoint °C
};

struct NetworkConfig {
  const char* ssid;           // WiFi SSID
  const char* password;       // WiFi password
  const char* brokerAddress;  // MQTT broker address
  const int port;             // MQTT broker port
  const char* mqttUser;       // MQTT username
  const char* mqttPass;       // MQTT password
};

struct PidParams {
  float Kp; // proportional PID parameter
  float Ki; // integrative PID parameter
  float Kd; // derivative PID parameter
};

struct ExperimentConfig {
  bool useFan;                  // wether the fan is on or off
  float setpoint;               // target setpoint specified by user
  PidParams internalPidParams;  // internal PID parameters
  PidParams externalPidParams;  // external PID parameters
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
