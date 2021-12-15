/*
 * HeaterPIDRadiant.ino
 * Copyright (C) 2021 Giuseppe Roberti
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <TaskScheduler.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <QuickPID.h>

//////////////////////
// temperature sensor
const int oneWireBusPin = 15;
const int heatingPin = 4;
OneWire oneWireBus = OneWire (oneWireBusPin);
DallasTemperature sensors = DallasTemperature (&oneWireBus);

//////////////////
// PWM properties
const int freq = 1000;
const int heatChannel = 0;
const int resolution = 8;
const int duty = 64;

///////
// PID
float temp;     // input
float heat;     // output
float setpoint; // target
float Kp=1, Kd=1000, Ki=0;
QuickPID heatPID (&temp, &heat, &setpoint, Kp, Ki, Kp, heatPID.Action::DIRECT);

////////////
// Heating?
boolean heating = false;

/////////
// Tasks
Scheduler runner;

void computeHeat ();
void reqTemp ();
void setpointInput ();
void printSerial ();

Task computeHeatTask (1000*TASK_MILLISECOND, TASK_FOREVER, computeHeat);
Task reqTempTask (100*TASK_MILLISECOND, TASK_FOREVER, reqTemp);
Task setpointInputTask (100*TASK_MILLISECOND, TASK_FOREVER, setpointInput);
Task printSerialTask (100*TASK_MILLISECOND, TASK_FOREVER, printSerial);

/////////////
// Main Code
void setup() {
  Serial.begin(115200);
  
  // Temperature Reader
  sensors.begin();

  runner.addTask(reqTempTask);
  reqTempTask.enable();

  // Heater PID
  pinMode(heatingPin, OUTPUT);

  runner.addTask(computeHeatTask);
  computeHeatTask.enable();

  heatPID.SetMode(heatPID.Control::TIMER);

  // configure PWM
  ledcSetup(heatChannel, freq, resolution);
  ledcAttachPin(heatingPin, heatChannel);

  // Setpoint Input
  runner.addTask(setpointInputTask);
  setpointInputTask.enable();

  // Printer
  runner.addTask(printSerialTask);
  printSerialTask.enable();
}

void loop() {
  runner.execute();
}

/////////////////////
// Compute Heat Task
void computeHeat () {
  heatPID.Compute();
  ledcWrite(heatChannel, needsHeating() ? duty : 0);
}

boolean needsHeating () {
  heating = heat > 0.0;

  return heating;
}

/////////////////////////
// Read Temperature Task
void reqTemp () {
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
}

//////////////////////
// Read Setpoint Task
void setpointInput () {
  if (Serial.available()) {
    setpoint = Serial.parseFloat();
    Serial.readStringUntil('\n');
    resetPID();
  }

//  int n = Serial.available();
//  if (n) {
//    String data = Serial.readString(n);
//    if (data.charAt(n) != '\n') return;
//    setpoint = data.parseFloat();
//    resetPID();
//  }
}

void resetPID () {
  heat = 0.0;
  heatPID.SetMode(heatPID.Control::MANUAL);
  heatPID.SetMode(heatPID.Control::TIMER);
}

//////////////
// Print Task
void printSerial () {
  Serial.printf("temp:%f,setpoint:%f,heat:%f,heating:%s,P:%f,I:%f,D:%f,X:%f\n",
    temp, setpoint, heat,
    heating ? "+1" : "-1",
    heatPID.GetPterm(), heatPID.GetIterm(), heatPID.GetDterm(),
    heatPID.GetPterm() + heatPID.GetIterm() + heatPID.GetDterm()
  );
}
