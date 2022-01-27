from time import time

# Terminate experment after this amount of minutes from goal_reached_t
min_from_goal_reached_t = 5

tot_goal_reached_counts = 10

def experiment_waiting(userdata, msg):
    return (
        msg.topic == 'esperimentino/data/json' and
        userdata['setup_done'] == False
    )

def experiment_started(userdata, msg):
    return (
        msg.topic == 'esperimentino/setup/done' and
        userdata['setup_done'] == False
    )

def experiment_running(userdata, msg):
    return (
        msg.topic == 'esperimentino/data/json' and
        userdata['setup_done'] == True and (
            userdata['goal_reached_t'] == None or
            time() - userdata['goal_reached_t'] < m2s(min_from_goal_reached_t)
        )
    )

def experiment_done(userdata, msg):
    return (
        msg.topic == 'esperimentino/data/json' and
        userdata['setup_done'] == True and
        userdata['goal_reached_count'] >= tot_goal_reached_counts and
        userdata['goal_reached_t'] != None and
        time() - userdata['goal_reached_t'] >= m2s(min_from_goal_reached_t)
    )

def m2s(m): return m * 60

def s2ms(s): return s * 1e3

def goal_reached(row): return row['Etemp'] >= (row['Esetp']) # + .0625)

def goal_reached_ms(userdata, readInterval):
    # passed time since goal reached in ms
    ms = userdata['goal_reached_count'] * readInterval
    return ms
