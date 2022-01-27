#!/usr/bin/env python

import sys
import os
import json
from os.path import realpath, dirname, join
from pathlib import Path

base_dir = realpath(join(dirname(__file__), '..'))
report_dir = join(base_dir, 'report')
c = json.loads(Path(join(base_dir, 'config.json')).read_bytes())['mqtt']

if __name__ == "__main__":
    args = {}
    for a in sys.argv[1:]:
        arg, val = a.split('=')
        if (val == ""): continue
        if (val == 'true' or val == 'false'):
            val = bool(val)
        elif val.isnumeric():
            val = float(val)
        args[arg] = val
    msg = json.dumps(args)
    cmd = f"mosquitto_pub -h '{c['broker']}' -p {c['port']} -u '{c['username']}' -P '{c['password']}' -t 'esperimentino/setup' -m '{msg}'"
    print(cmd)
    os.system(cmd)
