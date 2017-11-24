#!/usr/bin/env python

import re
import os
import fileinput
import math
from sets import Set
from pylab import *
from collections import OrderedDict
from IPython import embed

nodeIDs = range(1,26)

def parseLine(line):
    res = re.compile('^(\d+)\\tID:(\d+)\\t(.*)$').match(line)
    if res:
        return int(res.group(1)), int(res.group(2)), res.group(3)
    return None, None, None

def doParse(file):
    baseTime = None
    time = None
    linesParsedCount = 0
    sendCount = 0
    recvCount = {}
    recvRssi = {}
    
    for line in open(file, 'r').readlines():
        log = None
        module = None
        # match time, id, module, log; The common format for all log lines
        t, id, log = parseLine(line)
        if log != None:
            res = re.compile('^([^:]+):\\s*(.*)$').match(log)
            if res:
                module = res.group(1)
                log = res.group(2)

        if module == None or log == None:
            continue
        else:           
        
            time = t   
            # adjust time to baseTime
            if not baseTime:
                baseTime = time
            time -= baseTime
            time /= 1000000. # time from us to s
                        
            if module == "App":
                if log.startswith("sending"):
                    sendCount += 1
                if log.startswith("received"):
                    if not id in recvCount:
                        recvCount[id] = 0
                    recvCount[id] += 1
                    
                    res = re.compile('.*rssi ([-\d]+)').match(log)
                    if res:
                        rssi = int(res.group(1))
                        if not id in recvRssi:
                            recvRssi[id] = []
                        recvRssi[id].append(rssi)
				
            linesParsedCount += 1

    print "\nParsed %d lines" %(linesParsedCount)
    
    for id in recvCount.keys():
        print "%2u: %.2f %d" %(id, recvCount[id]/float(sendCount), np.mean(recvRssi[id]))
    
    print "Total number of receivers: %u" %(len(recvCount.keys()))
    print "Average number of receivers: %.2f" %(sum(recvCount.values())/float(sendCount))

            
def main():
    if len(sys.argv) < 2:
        return
    else:
        dir = sys.argv[1].rstrip('/')

    file = os.path.join(dir, "logs", "log.txt")
    print "\nProcessing %s" %(file)
    parsed = doParse(file)
    
main()
