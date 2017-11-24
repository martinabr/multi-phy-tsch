#!/usr/bin/env python

import re
import os
import fileinput
import math
from sets import Set
from pylab import *
import pandas as pd
from pandas import *
from datetime import *
from collections import OrderedDict
from IPython import embed

NNODES = 25

pd.set_option('display.width', None)
pd.set_option('display.max_columns', None)

def parseTxData(log):
    res = re.compile('^\TSCH: {asn-([a-f\d]+).([a-f\d]+) link-(\d+)-(\d+)-(\d+)-(\d+) [\s\d-]*ch-(\d+)\} bc-([01])-0 (\d*) tx 0, st 0-1').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        return {'asn': asn_ls, # ignore MSB
              'radio': "cc1200" if res.group(3) == "0" else "cc2538",
              #'slotframe_len': int(res.group(4)),
              #'timeslot': int(res.group(5)),
              #'channel_offset': int(res.group(6)),
              'channel': int(res.group(7)),
              #'is_eb': res.group(8) == "0",
              #'packet_len': int(res.group(9)),
              'noise': 0,#int(res.group(10)),
            }
            
def parseRxData(log):
    res = re.compile('^\TSCH: {asn-([a-f\d]+).([a-f\d]+) link-(\d+)-(\d+)-(\d+)-(\d+) [\s\d-]*ch-(\d+)\} bc-([01])-0 (\d*) rx (\d*) rssi ([-\d]*),.* edr ([-\d]*)').match(log)
    if res:
        #asn_ms = int(res.group(1), 16)
        asn_ls = int(res.group(2), 16)
        return {'asn': asn_ls, # ignore MSB
              'radio': "cc1200" if res.group(3) == "0" else "cc2538",
              #'slotframe_len': int(res.group(4)),
              #'timeslot': int(res.group(5)),
              #'channel_offset': int(res.group(6)),
              'channel': int(res.group(7)),
              #'is_eb': res.group(8) == "0",
              #'packet_len': int(res.group(9)),
              'source': int(res.group(10)),
              'rssi': int(res.group(11)),
              #'edr': int(res.group(12))
            }

def parseLine(line):
    res = re.compile('^(\d+)\\tID:(\d+)\\t(.*)$').match(line)
    if res:
        return int(res.group(1)), int(res.group(2)), res.group(3)
    return None, None, None

def doParse(dir, force):
    file = os.path.join(dir, "logs", "log.txt")    
    h5file = os.path.join(dir, 'df.h5')

    if not force and os.path.exists(h5file):
        print "Loading %s file" %(h5file)
        return pd.read_hdf(h5file,'df')
    
    baseTime = None
    time = None
    lastPrintedTime = 0
    data = []

    print "\nProcessing %s" %(file)
    for line in open(file, 'r').readlines():
        # match time, id, module, log; The common format for all log lines
        t, id, log = parseLine(line)

        time = t   
        if not baseTime:
            baseTime = time
        if time - lastPrintedTime >= 60*1000000:
            print "%u,"%((time-baseTime) / (60*1000000)),
            sys.stdout.flush()
            lastPrintedTime = time
        time -= baseTime
        
        res = parseRxData(log)
        if res != None:
            res["time"] = timedelta(microseconds=time);
            res["destination"] = id;
            res["isTx"] = False;
            if res["source"] != 0:
                data.append(res)
        else:
            res = parseTxData(log)
            if res != None:
                res["time"] = timedelta(microseconds=time);
                res["source"] = id;
                res["destination"] = 0;
                res["isTx"] = True;
                data.append(res)

    df = DataFrame(data)
    df = df.set_index("time")
    
    # save as HDF5
    print "Saving to %s file" %(h5file)
    df.to_hdf(h5file,'df')
    
    return df

def getStats(df):
    # group by radio, channel and source
    print "Extracting statistics: grouping"
    g = df.groupby(['radio','channel','source','destination'])
    # compute txCount, rxCount and mean RSSI
    stats = g.agg({'isTx': 'sum', 'rssi': ["count", "mean"]})
    stats.columns = ["txCount", "rxCount", "rssi"]
    statsFlat = stats.reset_index()
    
    # compute revers link's rx count
    print "Extracting statistics: reverse link stats"
    statsFlat["rxCountBack"] = statsFlat.apply(lambda x, stats=stats: stats.loc[x.radio, x.channel, x.destination, x.source].rxCount if (x.radio, x.channel, x.destination, x.source) in stats.index.tolist() else 0, axis=1)
    # compute PRR from rxCount and TxCount
    print "Extracting statistics: link PRR"
    statsFlat["prr"] = statsFlat.apply(lambda x, stats=stats: min(1, x.rxCount / stats.loc[x.radio, x.channel, x.source, 0].txCount), axis=1)
    print "Extracting statistics: reverse link PRR"
    statsFlat["prrBack"] = statsFlat.apply(lambda x, stats=stats: min(1, x.rxCountBack / stats.loc[x.radio, x.channel, x.source, 0].txCount), axis=1)
    
    # now compute stats per source
    g2 = statsFlat.groupby(['radio','channel','source'])
    statsPerSrc = g2.agg({'txCount': 'sum', 'rxCount': 'sum', 'rssi': 'mean'})
    statsPerSrc["reach"] = statsPerSrc["rxCount"] / statsPerSrc["txCount"]
    statsPerSrcFlat = statsPerSrc.reset_index()
    return statsFlat, statsPerSrcFlat

def getTimeSeries(df, radio):
    # select logs for the target radtio
    dfRadio = df[(df.radio == radio)]
    # create a timeseries, grouped by rssi and isTx
    groupAll = dfRadio.groupby([pd.TimeGrouper("5Min"), 'channel'])[['noise','rssi','isTx']]
    # compute txCount, rxCount and mean RSSI
    tsStats = groupAll.agg({'noise': "mean", 'rssi': ["count", "mean"], 'isTx': 'sum'})
    tsStats.columns = ["txCount", "rxCount", "rssi", "noise"]
    # compute PRR and reach
    tsStats["reach"] = tsStats["rxCount"] / tsStats["txCount"]
    tsStats["prr"] = tsStats["reach"] / (NNODES-1)
    # keep only prr and unstack
    tsPrr = tsStats[['prr']].unstack()
    tsPrr.columns = tsPrr.columns.droplevel()
    # keep only reach  and unstack
    tsReach = tsStats[['reach']].unstack()
    tsReach.columns = tsReach.columns.droplevel()
    # keep only rssi and unstack
    tsRssi = tsStats[['rssi']].unstack()
    tsRssi.columns = tsRssi.columns.droplevel()
    # keep only nois and unstack
    tsNoise = tsStats[['noise']].unstack()
    tsNoise.columns = tsNoise.columns.droplevel()
    return tsPrr, tsReach, tsRssi, tsNoise
    
def main():    
    force = False
    if len(sys.argv) < 2:
        return
    else:
        dir = sys.argv[1].rstrip('/')
        if len(sys.argv) == 3:
            force = sys.argv[2] != "0"

    file = os.path.join(dir, "logs", "log.txt")
    
    df = doParse(dir, force)

    statsPerLink, statsPerSrc = getStats(df)
    tsPrrcc1200, tsReachcc1200, tsRssicc1200, tsNoisecc1200 = getTimeSeries(df, "cc1200")
    tsPrrcc2538, tsReachcc2538, tsRssicc2538, tsNoisecc2538 = getTimeSeries(df, "cc2538")

    print "Plotting"

    if len(tsPrrcc1200) > 0:
        #tsPrrcc1200.reset_index().plot()
        #plt.savefig(os.path.join(dir, "timeline-cc1200-prr.pdf"))
        tsReachcc1200.reset_index().plot()
        plt.savefig(os.path.join(dir, "timeline-cc1200-reach.pdf"))
        tsRssicc1200.reset_index().plot()    
        plt.savefig(os.path.join(dir, "timeline-cc1200-rssi.pdf"))
        tsNoisecc1200.reset_index().plot()    
        plt.savefig(os.path.join(dir, "timeline-cc1200-noise.pdf"))
        statsPerLink[(statsPerLink.source < statsPerLink.destination) & (statsPerLink.radio == "cc1200")].plot.scatter(x='prr',y='prrBack')
        plt.savefig(os.path.join(dir, "prr-correlation-cc1200.pdf"))
    if len(tsPrrcc2538) > 0:
        #tsPrrcc2538.reset_index().plot()
        #plt.savefig(os.path.join(dir, "timeline-cc2538-prr.pdf"))
        tsReachcc2538.reset_index().plot()
        plt.savefig(os.path.join(dir, "timeline-cc2538-reach.pdf"))
        tsRssicc2538.reset_index().plot()
        plt.savefig(os.path.join(dir, "timeline-cc2538-rssi.pdf"))
        tsNoisecc2538.reset_index().plot()    
        plt.savefig(os.path.join(dir, "timeline-cc2538-noise.pdf"))
        statsPerLink[(statsPerLink.source < statsPerLink.destination) & (statsPerLink.radio == "cc2538")].plot.scatter(x='prr',y='prrBack')
        plt.savefig(os.path.join(dir, "prr-correlation-cc2538.pdf"))
    
    statsPerSrc.groupby("radio").boxplot(column='reach', by='channel')
    plt.savefig(os.path.join(dir, "reach-perchannel.pdf"))
    statsPerSrc.groupby("radio").boxplot(column='reach', by='source')
    plt.savefig(os.path.join(dir, "reach-persource.pdf"))
    #stats.groupby("radio").boxplot(column='rxCount', by='destination')
    #plt.savefig(os.path.join(dir, "rxcount-perdestination.pdf"))
    statsPerSrc.groupby("radio").boxplot(column='rssi', by='channel')
    plt.savefig(os.path.join(dir, "rssi-perchannel.pdf"))
    statsPerSrc.groupby("radio").boxplot(column='rssi', by='source')
    plt.savefig(os.path.join(dir, "rssi-persource.pdf"))
    #stats.groupby("radio").boxplot(column='rssi', by='destination')
    #plt.savefig(os.path.join(dir, "rssi-perdestination.pdf"))

main()
