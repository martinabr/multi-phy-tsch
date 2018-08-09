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
radioToStr = ["1.2 kbps", "8 kbps", "50 kbps", "250 kbps", "1000 kbps", "250 kbps @ 2.4 GHz"]

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
    #print("Extracting statistics: reverse link stats")
    #statsFlat["rxCountBack"] = statsFlat.apply(lambda x, stats=stats: \
    #    0 if not (x.radio, x.channel, x.destination, x.source) in stats.index.tolist() else \
    #    stats.loc[x.radio, x.channel, x.destination, x.source].rxCount, axis=1)

    print("Extracting statistics: asymmetry indicator")
    statsFlat["asymmetry"] = statsFlat.apply(lambda x, stats=stats: \
        np.nan if x.destination == 0 \
        or not (x.radio, x.channel, x.destination, x.source) in stats.index.tolist() \
        or not (x.radio, x.channel, x.destination, 0) in stats.index.tolist() \
        or not (x.radio, x.channel, x.source, 0) in stats.index.tolist() \
        else \
        abs((x.rxCount / stats.loc[x.radio, x.channel, x.source, 0].txCount) \
        - (stats.loc[x.radio, x.channel, x.destination, x.source].rxCount / stats.loc[x.radio, x.channel, x.destination, 0].txCount)), axis=1)

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
    tsStats["reach"] = tsStats.apply(lambda x: 0 if x.txCount==0 else x.rxCount / x.txCount, axis=1)

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

def doBoxPlot(data, yaxis, xaxis, dir, ylabel, xlabel):
    fig, ax = plt.subplots(2, 3, figsize=(12,4), sharey=True)
    data.boxplot(column=yaxis, by=xaxis, ax=ax)
    fig.suptitle("")
    plt.subplots_adjust(hspace=0.5, wspace=0.1)
    plt.grid(True)
    allAxes = fig.get_axes()
    for i, currAx in enumerate(allAxes):
        currAx.set_title(radioToStr[int(currAx.get_title())])
        currAx.xaxis.set_major_formatter(FormatStrFormatter('%d'))
        currAx.xaxis.set_major_locator(plt.AutoLocator())
        if i > 2:
            currAx.set_xlabel(xlabel)
        else:
            currAx.xaxis.label.set_visible(False)
        if i % 3 == 0:
            currAx.set_ylabel(ylabel)
    fig.savefig(os.path.join(dir, "%s-per%s.pdf" %(yaxis, xaxis)), bbox_inches='tight')

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
    dfNoise = df[df.isNoise == True]
    dfComm = df[df.isNoise == False]

    # Process the parsed data
    pklFile = os.path.join(dir, 'cache-processed.pkl')
    if not force and os.path.exists(pklFile):
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
    doBoxPlot(dfNoise.groupby("radio"), 'noise', 'channel', dir, "Noise (dBm)", "Channel")
    doBoxPlot(dfNoise.groupby("radio"), 'noise', 'node', dir, "Noise dBm)", "Node ID")

    # Rx RSSI
    doBoxPlot(dfComm.groupby("radio"), 'rssi', 'channel', dir, "RSSI (dBm)", "Channel")
    doBoxPlot(dfComm.groupby("radio"), 'rssi', 'source', dir, "RSSI (dBm)", "Node ID")
    doBoxPlot(dfComm.groupby("radio"), 'rssi', 'destination', dir, "RSSI (dBm)", "Node ID")

    # Reach
    doBoxPlot(allStats["perSrc"].groupby("radio"), 'reach', 'channel', dir, "Reach (# nodes)", "Channel")
    doBoxPlot(allStats["perSrc"].groupby("radio"), 'reach', 'source', dir, "Reach (# nodes)", "Node ID")
    doBoxPlot(allStats["perDst"].groupby("radio"), 'reach', 'destination', dir, "Reach (# nodes)", "Node ID")

    # Link asymmetry
    doBoxPlot(allStats["perLink"].groupby("radio"), 'asymmetry', 'channel', dir, "Asymmetry (ratio)", "Channel")
    doBoxPlot(allStats["perLink"].groupby("radio"), 'asymmetry', 'source', dir, "Asymmetry (ratio)", "Node ID")
    doBoxPlot(allStats["perLink"].groupby("radio"), 'asymmetry', 'destination', dir, "Asymmetry (ratio)", "Node ID")


    # Timelines
    for radio in range(6):
        allStats["timeSeries"][radio]["reach"].plot()
        plt.savefig(os.path.join(dir, "timeline-%s-reach.pdf"%(str(radio))))
        allStats["timeSeries"][radio]["rssi"].plot()
        plt.savefig(os.path.join(dir, "timeline-%s-rssi.pdf"%(str(radio))))
        allStats["timeSeries"][radio]["noise"].plot()
        plt.savefig(os.path.join(dir, "timeline-%s-noise.pdf"%(str(radio))))

main()
