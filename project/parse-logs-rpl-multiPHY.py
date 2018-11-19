#!/usr/bin/env python

import re
import os
import fileinput
import math
from pylab import *
import pandas as pd
from pandas import *
from datetime import *
from collections import OrderedDict
from IPython import embed
import matplotlib as mpl
import pickle

# Slotframe len 119
# 60 for sf0
# 59 for sf1
slotUsedMap = [60, 1]

# 3 preamble + 2 SFD + 1 len + 2 CRC
PHY_HDR = 8
ACK_LEN = 7
byteAirTimeMap = [6666.7, 8]

TICK_TIME = 31.25

pd.set_option('display.max_rows', 48)
pd.set_option('display.width', None)
pd.set_option('display.max_columns', None)

mpl.rcParams["figure.figsize"] = (4, 3)
mpl.rcParams['boxplot.flierprops.marker'] = '+'
mpl.rcParams['boxplot.flierprops.markersize'] = 4
mpl.rcParams['boxplot.flierprops.markeredgecolor'] ='gray'

# Air time in usec
def calcAirTime(radio, plen):
    return (plen + PHY_HDR) * byteAirTimeMap[radio]

def parseTopoData(log):
    res = re.compile('.*?NS: updating link, child 6G-(\d+), parent 6G-(\d+), lifetime \d+, hop count (\d+)').match(log)
    if res:
        return {'child': int(res.group(1)),
              'parent': int(res.group(2)),
              'hops': int(res.group(3)),
            }

def parseTxData(log):
    res = re.compile('.*?{asn ([a-f\d]+).([a-f\d]+) link +(\d+) +(\d+) +(\d+) +(\d+) +(\d+) ch +(\d+)\} bc-[01]-0 tx LL-.*?->LL-.*?, len +([-\d]*)').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        radio = int(res.group(3))
        plen = int(res.group(9))
        return {'asn': asn_ls, # ignore MSB
              'radio': radio,
              'channel': int(res.group(8)),
              'len': plen,
              'slotsUsed': slotUsedMap[radio],
              'airTime': calcAirTime(radio, plen),
            }

def parseRxData(log):
    res = re.compile('.*?{asn ([a-f\d]+).([a-f\d]+) link +(\d+) +(\d+) +(\d+) +(\d+) +(\d+) ch +(\d+)\} ([bu]c)-[01]-0 rx LL-(\d*)->LL-.*?, len +([-\d]*), seq +[-\d]*, rssi +([-\d]*), edr +([-\d]*)').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        radio = int(res.group(3))
        plen = int(res.group(11))
        isUc = res.group(9) == "uc"
        return {'asn': asn_ls, # ignore MSB
              'radio': radio,
              'channel': int(res.group(8)),
              'source': int(res.group(10)),
              'len': plen,
              'rssi': int(res.group(12)),
              'absedr': abs(int(res.group(13))),
              'slotsUsed': slotUsedMap[radio],
              'airTime': calcAirTime(radio, ACK_LEN) if isUc else 0,
            }

def parseLine(line):
    res = re.compile('\s*([.\d]+)\\tID:(\d+)\\t(.*)$').match(line)
    if res:
        return float(res.group(1)), int(res.group(2)), res.group(3)
    return None, None, None

def doParse(dir):
    file = os.path.join(dir, "logs", "log.txt")

    time = None
    lastPrintedTime = 0
    data = []
    hopsMap = {}

    print("\nProcessing %s" %(file))
    for line in open(file, 'r').readlines():
        # match time, id, module, log; The common format for all log lines
        time, id, log = parseLine(line)

        if time - lastPrintedTime >= 60:
            print("%u, "%(time / 60),end='', flush=True)
            lastPrintedTime = time

        res = parseRxData(log)
        if res != None:
            res["time"] = timedelta(seconds=time);
            res["destination"] = id;
            res["isTx"] = False;
            res["isRx"] = True;
            if res["source"] != 0:
                data.append(res)
        else:
            res = parseTxData(log)
            if res != None:
                res["time"] = timedelta(seconds=time);
                res["source"] = id;
                res["destination"] = 0;
                res["isTx"] = True;
                res["isRx"] = False;
                data.append(res)
            else:
                res = parseTopoData(log)
                if res != None:
                    hopsMap[res["child"]] = res["hops"]

    # fake item required for boxplot
    data.append({'radio': 0, 'destination': 18, 'absedr': 0, 'time': timedelta(seconds=0)})
    df = DataFrame(data)
    df = df.set_index("time")

    return df, hopsMap

def main():
    force = False

    if len(sys.argv) < 1:
        return
    else:
        dir = sys.argv[1].rstrip('/')

    df, hopsMap = doParse(dir)

    # Process the parsed data
    df0 = df[df.radio==0]
    df1 = df[df.radio==1]

    g = df.groupby([pd.Grouper(freq="1Min"), 'radio'])[['isTx', 'isRx', 'absedr', 'slotsUsed', 'airTime']]
    ts = g.agg({'isTx': {'txCount': 'sum'},
                'isRx': {'rxCount': 'sum'},
                'slotsUsed': {'slotsUsed': 'sum'},
                'absedr': {'absedr': 'mean'},
                'airTime': {'airTime': 'sum'},
                })
    # group to columns
    ts = ts.unstack()
    # drop last time sample
    ts = ts.drop(ts.index[-1])
    # set NaN to 0
    ts.slotsUsed = ts.slotsUsed.fillna(0)
    ts.txCount = ts.txCount.fillna(0)
    ts.rxCount = ts.rxCount.fillna(0)
    ts.airTime = ts.airTime.fillna(0)
    # Compute channel utilisation (%) from air time (cumulative used per 1-min chunk)
    ts.channelUtilization = 100. * ((ts.airTime / 1000000.) / 60)
    # Convert EDR from ticks to usec
    df0.absedr = df0.absedr * TICK_TIME
    df1.absedr = df1.absedr * TICK_TIME

    # Time series plots
    fig, axes = plt.subplots(2, 1, figsize=(5,4), sharex=True)
    ts.txCount.plot(ax = axes[0])
    axes[0].legend(["Slotframe 0", "Slotframe 1"])
    axes[0].set_ylabel("Tx count (#)")
    axes[0].grid(True)

    # Ait time time series
    ts.channelUtilization.plot(ax = axes[1])
    axes[1].set_xlabel("Time (min)")
    axes[1].set_ylabel("Channel utilization (%)")
    axes[1].grid(True)
    axes[1].get_legend().remove()
    plt.xticks(np.arange(0, 7*600*10**9, step=600*10**9),
               np.arange(0, 70, step=10))
    plt.minorticks_off()

    plt.savefig(os.path.join(dir, "rpl-tsch-airTime.pdf"), bbox_inches='tight')

    # Per node hop count and EDR
    fig, axes = plt.subplots(3, 1, figsize=(5,6), sharex=True)
    plt.subplots_adjust(hspace=0.3)
    axes[0].grid(True, zorder=0)
    axes[0].bar(hopsMap.keys(), hopsMap.values(), zorder=3)
    axes[0].set_ylabel("Hops (#)")
    axes[0].set_yticks(np.arange(0, 3, step=1))
    axes[0].set_title("Hop count")
    df0[df0.destination!=0].boxplot(column="absedr", by="destination", ax=axes[1])
    axes[1].set_ylabel("Error ($\mu$sec)")
    axes[1].set_title("Sync. error on Slotframe 0")
    axes[1].xaxis.label.set_visible(False)
    df1[df1.destination!=0].boxplot(column="absedr", by="destination", ax=axes[2])
    axes[2].set_ylabel("Error ($\mu$sec)")
    axes[2].set_title("Sync. error on Slotframe 1")
    axes[2].set_xlabel("Node ID")
    axes[2].tick_params(axis='x', labelsize=7)
    fig.suptitle("")
    plt.savefig(os.path.join(dir, "rpl-tsch-sync.pdf"), bbox_inches='tight')

main()
