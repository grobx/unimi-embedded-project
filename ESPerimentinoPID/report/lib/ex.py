import json
import csv
import glob
import pandas as pd
import numpy as np
import scipy.ndimage
import matplotlib.pyplot as plt
from os.path import join, splitext
from pathlib import Path
from datetime import timedelta
from IPython.display import display, Markdown

class Experiment:
    def experiments():
        res = {}
        for f in glob.glob('esperimentino-*.ipynb'):
            res[f] = str.split(splitext(f)[0], '-', 1)[-1]
        return res

    def results():
        res = {}
        f = lambda x: x[0] if x[0].update(x[1]) else x[0]
        for f,t in Experiment.experiments().items():
            e = Experiment(t)
            r = e.result()
            s = e.setup()
            r.update(s)
            res[r['timestamp']] = r
        return res

    def display_results():
        res = Experiment.results()
        keys = [
            'useFan', 'setpoint',
            'IKp', 'IKi', 'IKd', 'EKp', 'EKi', 'EKd',
            'IT0', 'ET0', 'incr', 'secs', 'max_temp',
            'p_goal', 'p_area', 'penality'
        ]
        return pd.DataFrame(res.values(), index=res.keys(), columns=keys)

    def display_reports():
        str = ""
        for f,t in Experiment.experiments().items():
            e = Experiment(t)
            p = e.result()['penality']
            str += f'- [Esperimentino del {t}](./{f}) [penality:{p}]\n'
        return display(Markdown(str))

    def __init__(self, timestamp):
        self.timestamp = timestamp

        # dirs
        self.basename = f'esperimentino-{self.timestamp}'
        self.data_dir = 'data'
        self.setup_file = join(self.data_dir, self.basename+'.json')
        self.data_file = join(self.data_dir, self.basename+'.csv')

        # setup json
        self.e = self.setup()

        # csv data
        self.data = self.data()

        # first and last row from data
        self.data_fst = self.data[0]
        self.data_lst = self.data[-1]

        # DataFrame
        self.idx = [int(d['us']) - int(self.data_fst['us']) for d in self.data]

        cln = ['Etemp','Esetp']
        self.df = pd.DataFrame(self.data, index=self.idx, columns=cln) \
            .astype('float')

        Icln = ['Itemp','Isetp']
        self.Idf = pd.DataFrame(self.data, index=self.idx, columns=Icln) \
            .astype('float')

    def timestamp(self): return self.timestamp

    def setup(self): return json.loads(Path(self.setup_file).read_text())

    def data(self):
        with open(self.data_file, 'r') as f:
            r = csv.DictReader(f, delimiter=',', quotechar='"')
            data = list(r)[1:]
        return data

    def result(self):
        timestamp = self.timestamp
        setpoint = float(self.e['setpoint'])
        toUseOrNotToUseFan = '' if self.e['useFan'] else 'without '
        IT0 = float(self.data_fst['Itemp'])
        ET0 = float(self.data_fst['Etemp'])
        incr = setpoint - ET0
        secs = (int(self.data_lst['us']) - int(self.data_fst['us'])) / 1e6
        mins = round(secs / 60)

        t_start = self.df.index[0]
        t_reached = self.df.query('Etemp >= Esetp').index[20]
        goal_delta = t_reached - t_start
        goal_secs = round(goal_delta / 1e6)
        goal_time = str(timedelta(seconds=goal_secs))

        min_temp = self.df['Etemp'].min()
        max_temp = self.df['Etemp'].max()
        t_max = self.df.query(f'Etemp >= {max_temp}').index[0]
        overheat = self.df['Etemp'][t_max] - self.df['Esetp'][t_max]

        overheat_delta = t_max - t_reached
        overheat_secs = round(overheat_delta / 1e6)
        overheat_time = str(timedelta(seconds=overheat_secs))

        # penality = 0.1 * (0.1 * goal_secs + 0.9 * overheat_secs) + 0.9 * (overheat * 1000)
        # penality /= incr

        # p_goal = goal_secs / incr
        # p_overheat = overheat_secs * overheat / 2
        # p_area = abs( self.df.loc[t_reached:]['Etemp'].sum() - self.df.loc[t_reached:]['Esetp'].sum() )
        # penality = 0.1 * p_goal + 0.9 * (0.9 * p_overheat + 0.1 * p_area)

        p_goal = goal_secs / incr
        p_area = abs(self.df.loc[t_reached:]['Etemp'] - self.df.loc[t_reached:]['Esetp'])
        p_area = p_area.sum() * p_area.max()
        penality = 0.1 * p_goal + 0.9 * p_area

        vs = vars()
        vs.pop('self')
        return vs

    def display(self):
        r = self.result()
        s = self.setup()

        # HEADER

        display(Markdown(f'# Experiment {self.timestamp}'))
        display(Markdown('[Go back](./index.ipynb) to the list of all experiments.'))

        # RESULTS

        display(Markdown(f'''
        This experiment was stared at {r['timestamp']}.
        Internal PID parameters (heating element): Kp={s['IKp']}, Ki={s['IKi']}, Kd={s['IKd']}.
        External PID parameters (ambient): Kp={s['EKp']}, Ki={s['EKi']}, Kd={s['EKd']}.
        The user asked to reach {r['setpoint']} °C, {r['toUseOrNotToUseFan']}using fan.
        The temperature of the box at t0 was {r['ET0']} °C, so we're asked to increment by {r['incr']} °C.
        The temperature of the heating element at t0 was {r['IT0']}.
        The experiment finished after {r['secs']} seconds [about {r['mins']} minutes].
        We have reached the goal after {r['goal_secs']} seconds [{r['goal_time']}].
        We have overheated by {r['overheat']} °C.
        The temperature continued to increase for {r['overheat_secs']} seconds [{r['overheat_time']}].
        The overhall penality of this experiment is {r['penality']}.
        '''))

        # GRAPH

        display(Markdown('## Graph (ambient + heating element)'))

        # xlabel = [x / 1e6 for x in self.idx]

        df = self.df
        # df.index = df.index / 1e6

        Idf = self.Idf
        # Idf.index = Idf.index / 1e6

        mss = np.arange(1, len(df.index), 1000) + [-1]
        xticks = [df.index[i] for i in mss] + [r['t_reached']]
        xlabels = [str(timedelta(seconds=round(df.index[i]/1e6))) for i in mss] + ['goal']

        ax = df \
            .plot.line(figsize=(16,8), xticks=xticks)
        ax.set_xticklabels(xlabels)
        plt.vlines(r['t_reached'], r['min_temp'], r['max_temp'], linestyles='dashed', colors='red')
        plt.scatter(r['t_max'], r['max_temp'], c="red")
        plt.text(r['t_max'], r['max_temp'], 'MAX', fontdict=dict(color='red'))

        Iax = Idf \
            .plot.line(figsize=(16,8), xticks=xticks, ylim=(0,70))
        Iax.set_xticklabels(xlabels)

        ax = df \
            .apply(lambda x: scipy.ndimage.gaussian_filter(x, 20)) \
            .plot.line(figsize=(16,8), xticks=xticks)
        ax.set_xticklabels(xlabels)
        plt.vlines(r['t_reached'], r['min_temp'], r['max_temp'], linestyles='dashed', colors='red')
        plt.scatter(r['t_max'], r['max_temp'], c="red")
        plt.text(r['t_max'], r['max_temp'], 'MAX', fontdict=dict(color='red'))

        Iax = Idf \
            .apply(lambda x: scipy.ndimage.gaussian_filter(x, 20)) \
            .plot.line(figsize=(16,8), xticks=xticks, ylim=(0,70))
        Iax.set_xticklabels(xlabels)
