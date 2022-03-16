#!/usr/bin/env python

import sys, os
import json
import random
import csv
import logging as log
from time import time, localtime, strftime
from paho.mqtt import client as mqtt_client
from os.path import realpath, dirname, join
from pathlib import Path
from lib.utils import *

base_dir = realpath(join(dirname(__file__), '..'))
report_dir = join(base_dir, 'report')
data_dir = join(report_dir, 'data')

now = time()
client_id = f'esperimentino-mqtt-{random.randint(0, 999)}'
topics = ["esperimentino/data/json", "esperimentino/setup/done"]
timestamp = strftime('%Y-%m-%d-%T', localtime(now))
config = json.loads(Path(join(base_dir, 'config.json')).read_bytes())
c = config['mqtt']

def connect_mqtt(args) -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            log.info("Connected!")
            topic = 'esperimentino/setup'
            msg = json.dumps(args)
            info = client.publish(topic, msg)
            log.info(f"Published `{topic}` with payload '{msg}'!")
        else:
            log.info("Failed to connect with error code: %d\n", rc)

    log.info(f'Connecting to MQTT broker {c["broker"]}:{c["port"]}.')

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.username_pw_set(c['username'], c['password'])
    client.connect(c['broker'], c['port'])

    return client

def subscribe(client: mqtt_client, data_writer, setup_fd):
    def on_message(client, userdata, msg):
        json_str = msg.payload.decode()
        row = json.loads(json_str)

        if experiment_waiting(userdata, msg): return

        elif experiment_started(userdata, msg):
            setup_fd.write(json_str)
            setup_fd.close()
            data_writer.writeheader()
            userdata['setup_done'] = True
            client.user_data_set(userdata)
            log.info(f"Experiment started! [{json_str}")

        elif experiment_running(userdata, msg):
            if userdata['goal_reached_t'] == None:
                if goal_reached(row):
                    userdata['goal_reached_count'] += 1
                elif userdata['goal_reached_count'] != 0:
                    userdata['goal_reached_count'] -= 1

                # we consider the goal reached after 20s
                if goal_reached_ms(userdata, config['heater']['readInterval']) >= s2ms(20):
                    log.info(f"Goal reached! [experiment ends within {min_from_goal_reached_t} minutes]")
                    userdata['goal_reached_t'] = time()

            data_writer.writerow(row)
            # FIXME flush data_writer fd ?
            client.user_data_set(userdata)

        elif experiment_done(userdata, msg):
            client.disconnect()
            template_file = join(report_dir, '.template.ipynb')
            notebook_file = join(report_dir, f'esperimentino-{timestamp}.ipynb')
            template = json.loads(Path(template_file).read_bytes())
            template['cells'][0]['source'] = [ f"timestamp = '{timestamp}'" ]
            with open(notebook_file, 'w+') as nf:
                nf.write(json.dumps(template, indent=2))
            log.info("Experiment Finished!")
        
        else: raise "We should not be here!"

    for topic in topics: client.subscribe(topic)

    userdata = {
        'setup_done': False,
        'goal_reached_t': None,
        'goal_reached_count': 0
    }

    client.on_message = on_message
    client.user_data_set(userdata)

def run(args):
    data_file = join(data_dir, f'esperimentino-{timestamp}.csv')
    setup_file = join(data_dir, f'esperimentino-{timestamp}.json')
    with open(data_file, 'w+') as data_fd:
        fieldnames = [
            'us', 'fanRPM',
            'Itemp','Isetp','Iheat','IP','II','ID',
            'Etemp','Esetp','Eheat','EP','EI','ED',
        ]
        data_writer = csv.DictWriter(data_fd, fieldnames=fieldnames)
        with open(setup_file, 'w+') as setup_fd:
            mqtt_client = connect_mqtt(args)
            subscribe(mqtt_client, data_writer, setup_fd)
            mqtt_client.loop_forever()

if __name__ == '__main__':
    log.basicConfig(
        format="%(levelname)s %(asctime)s - %(message)s",
        level=log.INFO
    )
    args = {}
    for a in sys.argv[1:]:
        arg, val = a.split('=')
        if (val == ""): continue
        if (val == 'true' or val == 'false'):
            val = bool(val == 'true')
        elif val.isnumeric():
            val = float(val)
        args[arg] = val
    run(args)
