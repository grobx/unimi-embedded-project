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

WIFI_SSID=$(shell cat "config.json" | jq ".wifi.ssid")
WIFI_PASSWORD=$(shell cat "config.json" | jq ".wifi.password")

MQTT_BROKER:=$(shell cat "config.json" | jq ".mqtt.broker")
MQTT_PORT=$(shell cat "config.json" | jq ".mqtt.port")
MQTT_USERNAME=$(shell cat "config.json" | jq ".mqtt.username")
MQTT_PASSWORD=$(shell cat "config.json" | jq ".mqtt.password")

BOARD_RELAY_PIN=$(shell cat "config.json" | jq ".board.relayPin")
BOARD_RPM_COUNTER_PIN=$(shell cat "config.json" | jq ".board.rpmCounterPin")
BOARD_ONE_WIRE_BUS_PIN=$(shell cat "config.json" | jq ".board.oneWireBusPin")
BOARD_HEATING_PIN=$(shell cat "config.json" | jq ".board.heatingPin")

HEATER_READ_INTERVAL=$(shell cat "config.json" | jq ".heater.readInterval")
HEATER_FREQ=$(shell cat "config.json" | jq ".heater.freq")
HEATER_HEAT_CHANNEL=$(shell cat "config.json" | jq ".heater.heatChannel")
HEATER_RESOLUTION=$(shell cat "config.json" | jq ".heater.resolution")
HEATER_MIN_DUTY=$(shell cat "config.json" | jq ".heater.minDuty")
HEATER_MAX_DUTY=$(shell cat "config.json" | jq ".heater.maxDuty")
HEATER_MAX_SETPOINT=$(shell cat "config.json" | jq ".heater.maxSetpoint")
HEATER_MIN_SETPOINT=$(shell cat "config.json" | jq ".heater.minSetpoint")

EXPERIMENT_USE_FAN=$(shell cat "config.json" | jq ".experiment.useFan")
EXPERIMENT_SETPOINT=$(shell cat "config.json" | jq ".experiment.setpoint")
EXPERIMENT_INTERNAL_KP=$(shell cat "config.json" | jq ".experiment.internalPidParams.Kp")
EXPERIMENT_INTERNAL_KI=$(shell cat "config.json" | jq ".experiment.internalPidParams.Ki")
EXPERIMENT_INTERNAL_KD=$(shell cat "config.json" | jq ".experiment.internalPidParams.Kd")
EXPERIMENT_EXTERNAL_KP=$(shell cat "config.json" | jq ".experiment.externalPidParams.Kp")
EXPERIMENT_EXTERNAL_KI=$(shell cat "config.json" | jq ".experiment.externalPidParams.Ki")
EXPERIMENT_EXTERNAL_KD=$(shell cat "config.json" | jq .experiment.externalPidParams.Kd)

define LOCAL_CONFIG_H
#pragma once

#include <DallasTemperature.h>
#include "Config.h"
#include "TaskSchedulerDefs.h"

const BoardConfig boardConfig = {
  $(BOARD_RELAY_PIN), // relay PIN (actuate the FAN)
  $(BOARD_RPM_COUNTER_PIN), // rpm counter PIN (FAN feedback)
  $(BOARD_ONE_WIRE_BUS_PIN), // one wire bus PIN (digital temperature sensor)
  $(BOARD_HEATING_PIN) // heating PIN (for the MOSFET)
};

const HeaterConfig heaterConfig = {
  $(HEATER_READ_INTERVAL) * TASK_MILLISECOND,
  $(HEATER_FREQ), // PWM frequency
  $(HEATER_HEAT_CHANNEL), // PWM channel
  $(HEATER_RESOLUTION), // PWM resolution
  $(HEATER_MIN_DUTY), // PWM min duty value
  $(HEATER_MAX_DUTY), // PWM max duty value
  $(HEATER_MAX_SETPOINT), // max internal setpoint
  $(HEATER_MIN_SETPOINT) // min internal setpoint
};

const ExperimentConfig defaultConfig = {
  $(EXPERIMENT_USE_FAN), // useFan
  $(EXPERIMENT_SETPOINT), // setpoint
  { // internalPidParams
	$(EXPERIMENT_INTERNAL_KP),
	$(EXPERIMENT_INTERNAL_KI),
	$(EXPERIMENT_INTERNAL_KD)
  },
  { // externalPidParams
	$(EXPERIMENT_EXTERNAL_KP),
	$(EXPERIMENT_EXTERNAL_KI),
	$(EXPERIMENT_EXTERNAL_KD)
  }
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

.PHONY: flash gallery setup experiment subscribe report reports

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

report:
	cd report && jupyter nbconvert --to notebook --inplace --execute esperimentino-$(experiment).ipynb index.ipynb && jupyter trust esperimentino-$(experiment).ipynb index.ipynb

reports:
	cd report && jupyter nbconvert --to notebook --inplace --execute esperimentino-*.ipynb index.ipynb && jupyter trust esperimentino-*.ipynb index.ipynb
