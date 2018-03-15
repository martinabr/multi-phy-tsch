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

NNODES = 25

pd.set_option('display.max_rows', 48)
pd.set_option('display.width', None)
pd.set_option('display.max_columns', None)

mpl.rcParams['boxplot.flierprops.marker'] = '+'
mpl.rcParams['boxplot.flierprops.markersize'] = 4
mpl.rcParams['boxplot.flierprops.markeredgecolor'] ='gray'

def parseNoiseData(log):
    res = re.compile('.*?{asn ([a-f\d]+).([a-f\d]+) link +(\d+) +(\d+) +(\d+) +(\d+) ch +(\d+)\} RSSI sample: ([-\d]*)').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        return {'asn': asn_ls, # ignore MSB
              'radio': int(res.group(3)),
              'channel': int(res.group(7)),
              'noise': int(res.group(8)),
            }

def parseTxData(log):
    res = re.compile('.*?{asn ([a-f\d]+).([a-f\d]+) link +(\d+) +(\d+) +(\d+) +(\d+) ch +(\d+)\} bc-[01]-0 tx').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        return {'asn': asn_ls, # ignore MSB
              'radio': int(res.group(3)),
              'channel': int(res.group(7)),
            }

def parseRxData(log):
    #res = re.compile('.*?{asn-([a-f\d]+).([a-f\d]+) link-(\d+)-(\d+)-(\d+)-(\d+) [\s\d-]*ch-(\d+)\} bc-([01])-0 (\d*) rx LL-(\d*)->LL-NULL, len [-\d]*, seq [-\d]* rssi ([-\d]*)').match(log)
    res = re.compile('.*?{asn ([a-f\d]+).([a-f\d]+) link +(\d+) +(\d+) +(\d+) +(\d+) ch +(\d+)\} bc-[01]-0 rx LL-(\d*)->LL-NULL, len +[-\d]*, seq +[-\d]*, rssi +([-\d]*)').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        return {'asn': asn_ls, # ignore MSB
              'radio': int(res.group(3)),
              'channel': int(res.group(7)),
              'source': int(res.group(8)),
              'rssi': int(res.group(9)),
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
            res["isNoise"] = False;
            if res["source"] != 0:
                data.append(res)
        else:
            res = parseTxData(log)
            if res != None:
                res["time"] = timedelta(seconds=time);
                res["source"] = id;
                res["destination"] = 0;
                res["isTx"] = True;
                res["isNoise"] = False;
                data.append(res)
            else:
                res = parseNoiseData(log)
                if res != None:
                    res["time"] = timedelta(seconds=time);
                    res["node"] = id;
                    res["isNoise"] = True;
                    data.append(res)

    df = DataFrame(data)
    df = df.set_index("time")

    return df

def getStats(df):
    # group by radio, channel and source
    print("Extracting statistics: grouping")
    g = df.groupby(['radio','channel','source','destination'])
    # compute txCount, rxCount and mean RSSI
    stats = g.agg({'isTx': {'txCount': 'sum'}, 'rssi': {'rxCount': "count"}})
    stats.columns = stats.columns.droplevel(0)
    statsFlat = stats.reset_index()

    # compute reverse link's rx count
    print("Extracting statistics: reverse link stats")
    statsFlat["rxCountBack"] = statsFlat.apply(lambda x, stats=stats: \
        stats.loc[x.radio, x.channel, x.destination, x.source].rxCount if (x.radio, x.channel, x.destination, x.source) in stats.index.tolist() else 0, axis=1)

    print("Extracting statistics: asymmetry indicator")
    statsFlat["asymmetry"] = statsFlat.apply(lambda x, stats=stats: 0 if x.destination == 0 or not (x.radio, x.channel, x.destination, 0) in stats else \
        abs((x.rxCount / stats.loc[x.radio, x.channel, x.source, 0].txCount) \
        - (x.rxCountBack / stats.loc[x.radio, x.channel, x.destination, 0].txCount)), axis=1)

    # now compute stats per source
    print("Extracting statistics: per source")
    g2 = statsFlat.groupby(['radio','channel','source'])
    statsPerSrc = g2.agg({'txCount': 'sum', 'rxCount': 'sum'})
    statsPerSrc["reach"] = statsPerSrc["rxCount"] / statsPerSrc["txCount"]
    statsPerSrcFlat = statsPerSrc.reset_index()

    # now compute stats per destination
    print("Extracting statistics: per destination")
    g3 = statsFlat.groupby(['radio','channel','destination'])
    statsPerDst = g3.agg({'txCount': 'sum', 'rxCount': 'sum'})
    statsPerDstFlat = statsPerDst.reset_index()
    statsPerDstFlat["txCount"] = statsPerDstFlat.apply(lambda x, stats=statsPerDst: stats.loc[x.radio, x.channel, 0].txCount, axis=1)
    statsPerDstFlat["reach"] = statsPerDstFlat["rxCount"] / statsPerDstFlat["txCount"]

    print("Extracting statistics: done")
    return { 'perLink': statsFlat, \
             'perSrc': statsPerSrcFlat, \
             'perDst': statsPerDstFlat }

def getTimeSeries(df, radio):
    print("Extracting time-series statistics for radio %s" %(str(radio)))

    # select logs for the target radtio
    dfRadio = df[(df.radio == radio)]
    # create a timeseries, grouped by rssi and isTx
    groupAll = dfRadio.groupby([pd.Grouper(freq="5Min"), 'channel'])[['noise','rssi','isTx']]
    # compute txCount, rxCount and mean RSSI
    tsStats = groupAll.agg({'noise': {'noise': "mean"}, 'rssi': { 'rxCount': "count", 'rssi': "mean"}, 'isTx': {'txCount': 'sum'}})
    # compute reach
    tsStats["reach"] = tsStats["rxCount"] / tsStats["txCount"]
    # unstack
    tsStats = tsStats.unstack()

    # keep only reach  and unstack
    #tsReach = tsStats[['reach']].unstack()
    #tsReach.columns = tsReach.columns.droplevel()
    # keep only rssi and unstack
    #tsRssi = tsStats[['rssi']].unstack()
    #tsRssi.columns = tsRssi.columns.droplevel()
    # keep only nois and unstack
    #tsNoise = tsStats[['noise']].unstack()
    #tsNoise.columns = tsNoise.columns.droplevel()
    return tsStats

def main():
    if len(sys.argv) < 1:
        return
    else:
        dir = sys.argv[1].rstrip('/')

    file = os.path.join(dir, "logs", "log.txt")

    # Parse the original log
    h5file = os.path.join(dir, 'df.h5')
    if os.path.exists(h5file):
        print("Loading %s file" %(h5file))
        df = pd.read_hdf(h5file,'df')
    else:
        df = doParse(dir)
        print("\nSaving to %s file" %(h5file))
        df.to_hdf(h5file,'df')
    dfNoise = df[df.isNoise == True]
    dfComm = df[df.isNoise == False]

    # Process the parsed data
    pklFile = os.path.join(dir, 'allStats.pkl')
    if os.path.exists(pklFile):
        print("Loading %s file" %(pklFile))
        pickleFile = open(pklFile, 'rb')
        allStats = pickle.load(pickleFile)
    else:
        allStats = {}
        allStats.update(getStats(dfComm))
        allStats["timeSeries"] = {}
        for radio in range(6):
            allStats["timeSeries"][radio] = getTimeSeries(df, radio)

        print("\nSaving to %s file" %(pklFile))
        pickleFile = open(pklFile, 'wb')
        pickle.dump(allStats, pickleFile)


    # Test on per-ASN grouping
    #g = df.groupby(['asn'])
    #stats = g.agg({'isTx': {'txCount': 'sum'}, 'rssi': {'rxCount': "count"}})
    #stats.columns = stats.columns.droplevel(0)
    #stats = stats[stats.txCount == 1]["rxCount"]
    #df.apply(lambda x, stats=stats: 0 if x.isTx == False else \
    #    stats.loc[x.asn].rxCount, axis=1)

    print("Plotting")

    # Noise
    dfNoise.groupby("radio").boxplot(column='noise', by='channel')
    plt.savefig(os.path.join(dir, "noise-perchannel.pdf"))
    dfNoise.groupby("radio").boxplot(column='noise', by='node')
    plt.savefig(os.path.join(dir, "noise-pernode.pdf"))

    # Rx RSSI
    dfComm.groupby("radio").boxplot(column='rssi', by='channel')
    plt.savefig(os.path.join(dir, "rssi-perchannel.pdf"))
    dfComm.groupby("radio").boxplot(column='rssi', by='source')
    plt.savefig(os.path.join(dir, "rssi-persource.pdf"))
    dfComm.groupby("radio").boxplot(column='rssi', by='destination')
    plt.savefig(os.path.join(dir, "rssi-perdestination.pdf"))

    # Reach
    allStats["perSrc"].groupby("radio").boxplot(column='reach', by='channel')
    plt.savefig(os.path.join(dir, "reach-perchannel.pdf"))
    allStats["perSrc"].groupby("radio").boxplot(column='reach', by='source')
    plt.savefig(os.path.join(dir, "reach-persource.pdf"))
    allStats["perDst"].groupby("radio").boxplot(column='reach', by='destination')
    plt.savefig(os.path.join(dir, "reach-perdestination.pdf"))

    # Link asymmetry
    allStats["perLink"].groupby("radio").boxplot(column='asymmetry', by='channel')
    plt.savefig(os.path.join(dir, "asymmetry-perchannel.pdf"))
    allStats["perLink"].groupby("radio").boxplot(column='asymmetry', by='source')
    plt.savefig(os.path.join(dir, "asymmetry-persource.pdf"))
    allStats["perLink"].groupby("radio").boxplot(column='asymmetry', by='destination')
    plt.savefig(os.path.join(dir, "asymmetry-perdestination.pdf"))

    # Timelines
    for radio in range(6):
        allStats["timeSeries"][radio]["reach"].plot()
        plt.savefig(os.path.join(dir, "timeline-%s-reach.pdf"%(str(radio))))
        allStats["timeSeries"][radio]["rssi"].plot()
        plt.savefig(os.path.join(dir, "timeline-%s-reach.pdf"%(str(radio))))
        allStats["timeSeries"][radio]["noise"].plot()
        plt.savefig(os.path.join(dir, "timeline-%s-noise.pdf"%(str(radio))))

main()
