#    Makefile
#    Copyright (C) 2021-2022 Giuseppe Roberti

#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

PHONY=flash gallery setup experiment subscribe notebooks

WIFI_SSID=$(shell cat "config.json" | jq ".wifi.ssid")
WIFI_PASSWORD=$(shell cat "config.json" | jq ".wifi.password")
MQTT_BROKER:=$(shell cat "config.json" | jq ".mqtt.broker")
MQTT_PORT=$(shell cat "config.json" | jq ".mqtt.port")
MQTT_USERNAME=$(shell cat "config.json" | jq ".mqtt.username")
MQTT_PASSWORD=$(shell cat "config.json" | jq ".mqtt.password")
READ_INTERVAL=$(shell cat "config.json" | jq ".readInterval")

define LOCAL_CONFIG_H
#pragma once

#include <DallasTemperature.h>
#include "Config.h"
#include "TaskSchedulerDefs.h"

const BoardConfig boardConfig = {
  21, // relay PIN (actuate the FAN)
  19, // rpm counter PIN (FAN feedback)
  15, // one wire bus PIN (digital temperature sensor)
  4   // heating PIN (for the MOSFET)
};

const HeaterConfig heaterConfig = {
  $(READ_INTERVAL) * TASK_MILLISECOND,
  1000,  // PWM frequency
  10,    // PWM channel
  8,     // PWM resolution
  0.0,   // PWM min duty value
  255.0, // PWM max duty value
  60.0,  // max internal setpoint
  DEVICE_DISCONNECTED_C   // min internal setpoint
};

const ExperimentConfig defaultConfig = {
  true,                       // useFan
  0.00,                       // setpoint
  {   0.00,   0.00,   0.00 }, // internalPidParams
  {   0.00,   0.00,   0.00 }, // externalPidParams
};

const NetworkConfig networkConfig = {
  $(WIFI_SSID), // WiFi SSID
  $(WIFI_PASSWORD), // WiFi password
  $(MQTT_BROKER), // MQTT broker address
  $(MQTT_PORT), // MQTT port
  $(MQTT_USERNAME), // MQTT username
  $(MQTT_PASSWORD) // MQTT password
};
endef
export LOCAL_CONFIG_H

flash:
	pio run -t compiledb
	pio run -t upload

clean:
	@rm src/LocalConfig.h
	pio run -t clean

src/LocalConfig.h:
	@echo "$$LOCAL_CONFIG_H" > src/LocalConfig.h

gallery:
	cd docs/imgs &&	\
	convert -resize 50% -quality 100% -delay 1000 -dispose previous gallery/*.jpg gallery.gif

setup:
	@./script/setup.py \
		useFan=$(useFan) \
		setpoint=$(setpoint) \
		IKp=$(IKp) IKi=$(IKi) IKd=$(IKd) \
		EKp=$(EKp) EKi=$(EKi) EKd=$(EKd)

experiment:
	@./script/start-esperimentino.py \
		useFan=$(useFan) \
		setpoint=$(setpoint) \
		IKp=$(IKp) IKi=$(IKi) IKd=$(IKd) \
		EKp=$(EKp) EKi=$(EKi) EKd=$(EKd)

subscribe:
	mosquitto_sub -h ${MQTT_BROKER} -p ${MQTT_PORT} \
		-u ${MQTT_USERNAME} -P ${MQTT_PASSWORD} \
		-t "esperimentino/setup" \
		-t "esperimentino/setup/done" \
		-t "esperimentino/data/json" \
		-F '@Y-@m-@dT@H:@M:@S:@N@z : %t : %J' --pretty

notebooks:
	cd report && jupyter nbconvert --to notebook --inplace --execute *.ipynb && jupyter trust *.ipynb
