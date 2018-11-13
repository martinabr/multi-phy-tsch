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

SF0_SLOTUSE = 60
SF1_SLOTUSE = 1

NNODES = 25
radioToStr = ["1.2 kbps", "8 kbps", "50 kbps", "250 kbps", "1000 kbps", "250 kbps @ 2.4 GHz"]

pd.set_option('display.max_rows', 48)
pd.set_option('display.width', None)
pd.set_option('display.max_columns', None)

mpl.rcParams['boxplot.flierprops.marker'] = '+'
mpl.rcParams['boxplot.flierprops.markersize'] = 4
mpl.rcParams['boxplot.flierprops.markeredgecolor'] ='gray'

def parseTxData(log):
    res = re.compile('.*?{asn ([a-f\d]+).([a-f\d]+) link +(\d+) +(\d+) +(\d+) +(\d+) +(\d+) ch +(\d+)\} bc-[01]-0 tx LL-.*?->LL-.*?, len +([-\d]*)').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        return {'asn': asn_ls, # ignore MSB
              'radio': int(res.group(3)),
              'channel': int(res.group(8)),
              'len': int(res.group(9)),
              'slotsUsed': SF0_SLOTUSE if res.group(3) == '0' else SF1_SLOTUSE,
            }

def parseRxData(log):
    res = re.compile('.*?{asn ([a-f\d]+).([a-f\d]+) link +(\d+) +(\d+) +(\d+) +(\d+) +(\d+) ch +(\d+)\} bc-[01]-0 rx LL-(\d*)->LL-.*?, len +([-\d]*), seq +[-\d]*, rssi +([-\d]*), edr +([-\d]*)').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        return {'asn': asn_ls, # ignore MSB
              'radio': int(res.group(3)),
              'channel': int(res.group(8)),
              'source': int(res.group(9)),
              'len': int(res.group(10)),
              'rssi': int(res.group(11)),
              'absedr': abs(int(res.group(12))),
              'slotsUsed': SF0_SLOTUSE if res.group(3) == '0' else SF1_SLOTUSE,
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

    df = DataFrame(data)
    df = df.set_index("time")

    return df

def main():
    force = False

    if len(sys.argv) < 1:
        return
    else:
        dir = sys.argv[1].rstrip('/')
        if len(sys.argv) > 1:
            force = sys.argv[2] == "1"

    file = os.path.join(dir, "logs", "log.txt")

    # Parse the original log
    h5file = os.path.join(dir, 'cache-parsed.h5')
    if not force and os.path.exists(h5file):
        print("Loading %s file" %(h5file))
        df = pd.read_hdf(h5file,'df')
    else:
        df = doParse(dir)
        print("\nSaving to %s file" %(h5file))
        df.to_hdf(h5file,'df')

    # Process the parsed data
    groupAll = df.groupby([pd.Grouper(freq="1Min"), 'radio'])[['isTx', 'isRx', 'absedr', 'slotsUsed']]
    tsStats = groupAll.agg({'isTx': {'txCount': 'sum'}, 'isRx': {'rxCount': 'sum'}, 'slotsUsed': {'slotsUsed': 'sum'}, 'absedr': {'absedr': 'mean'}})

    groupAll = df.groupby(['radio','source'])
    perNodeStats = groupAll.agg({'isTx': {'txCount': 'sum'}, 'isRx': {'rxCount': 'sum'}, 'slotsUsed': {'slotsUsed': 'sum'}, 'absedr': {'absedr': 'mean'}})
    perNodeStats.columns = perNodeStats.columns.droplevel(0)

    embed()

main()
