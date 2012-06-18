#!/usr/bin/env python
# Script to allow remote reading of the iRODS log files. Can be useful for
# applications intended to process the log files.
# Adil Hasan, 15/Jun/12

import getopt
import sys
import os
import glob
import ConfigParser
import time
import re

def usage():
    '''Function describing the script usage
    '''
    print "Script to read the iRODS log file"
    print "Usage: readRodsLog.py [-h][-v][-l][-f <file> [-s <starttime>]"
    print "   [-e <endtime>]]" 
    print "Options:"
    print "-h, --help               Prints this help"
    print "-v, --verbose            Prints verbose output"
    print "-l, --list               List all the log files"
    print "-f, --file               The log file to parse"
    print "-s, --start=<starttime>  Start time. Messages after this time will be" 
    print "                         parsed. Format must be YYYY-MM-DD::HH:MM:SS"
    print "-e, --end=<endtime>      End time. Messages up to this time will be" 
    print "                         parsed. Format must be YYYY-MM-DD::HH:MM:SS"
    print " "


def readLog(logPath, logFile, stime, etime, listFlag, verbose):
    '''Function to read the iRODS log file
    '''
    relogFile = None
    logFiles = glob.glob("%s/rodsLog*" % logPath)
    reFiles = glob.glob("%s/reLog*" % logPath)
    if (listFlag):
        print "rodsLog Files: "
        print "--------------"
        for logf in logFiles:
            print logf

        print "reLog Files: "
        print "------------"
        for relogf in reFiles:
            print relogf
        return

    # Get the most recent file if filename is not specified
    if (not logFile):
        maxtime = 0
        for logf in logFiles:
            mtime = os.path.getmtime(logf)
            if (mtime > maxtime):
                maxtime = mtime
                logFile = logf
        maxtime = 0
        for relogf in reFiles:
            mtime = os.path.getmtime(relogf)
            if (mtime > maxtime):
                maxtime = mtime
                relogFile = relogf
    
    print "-----------------------------------------------------------------"
    print "Log file is: ", logFile
    print "-----------------------------------------------------------------"
    log = file(logFile, 'r')
    logYear = logFile.split(".")[1]
    timeStr = re.compile("\d\d:\d\d:\d\d")
    for line in log:
        if (stime):
            printS = 0
        else:
            printS = 1
        if (etime):
            printE = 0
        else:
            printE = 1

        cpts = line.split()
        if (len(cpts) >= 3 and timeStr.search(cpts[2])):
            dateStamp = "%s-%s-%s::%s" % (logYear, cpts[0], cpts[1], cpts[2])
            timetup = time.strptime(dateStamp,"%Y-%b-%d::%H:%M:%S")
            logtime = time.mktime(timetup)
            if (stime and logtime >= stime):
                printS = 1
            if (etime and logtime <= etime):
                printE = 1
            
        if (printS and printE):
            print line
    log.close()

    if (not relogFile): return

    print "-----------------------------------------------------------------"
    print "Log file is: ", relogFile
    print "-----------------------------------------------------------------"
    relog = file(relogFile, 'r')
    relogYear = relogFile.split(".")[1]
    for reline in relog:
        if (stime):
            printS = 0
        else:
            printS = 1
        if (etime):
            printE = 0
        else:
            printE = 1

        cpts = reline.split()
        if (len(cpts) >=3 and timeStr.search(cpts[2])):
            dateStamp = "%s-%s-%s::%s" % (relogYear, cpts[0], cpts[1], cpts[2])
            timetup = time.strptime(dateStamp,"%Y-%b-%d::%H:%M:%S")
            relogtime = time.mktime(timetup)
            if (stime and relogtime >= stime):
                printS = 1
            if (etime and relogtime <= etime):
                printE = 1

        if (printS and printE):
            print reline

    relog.close()

if __name__ == '__main__':
    verbose = 0
    logFile = None
    listFlag = False
    stime = None
    etime = None
    opts, args = getopt.getopt(sys.argv[1:], 'hvlf:s:e:', ['help', 'verbose', 
        'list', 'file=', 'start=', 'end='])
    
    configFile = "%s/readlogs.cfg" % \
    os.path.abspath(os.path.dirname(sys.argv[0]))
    config = ConfigParser.ConfigParser()
    config.read(configFile)
    allowedUsers = config.get("RODSLOG", "allowedUsers").split(",")
    okFlag = 0
    for aUser in allowedUsers:
        aUs,aUsZ = aUser.split("@")
        if (aUs.strip() == os.environ['spClientUser'] and aUsZ.strip() ==
                os.environ['spClientRodsZone']):
            okFlag = 1
    
    if (okFlag == 0):
        print "You are not authorised to run this script"
        sys.exit(0)

    logPath = os.path.abspath(config.get("RODSLOG", "location"))
    print "log path is ", logPath   
    for opt, val in opts:
        if (opt == '-h' or opt == '--help'):
            usage()
            sys.exit(0)
        if (opt == '-v' or opt == '--verbose'):
            verbose = 1
        if (opt == '-l' or opt == '--list'):
            listFlag = True
        if (opt == '-f' or opt == '--file'):
            logFile = val.strip()
        if (opt == '-s' or opt == '--start'):
            stimet = time.strptime(val.strip(), "%Y-%m-%d::%H:%M:%S")
            stime = time.mktime(stimet)
        if (opt == '-e' or opt == '-end'):
            etimet = time.strptime(val.strip(), "%Y-%m-%d::%H:%M:%S")
            etime = time.mktime(etimet)

    readLog(logPath, logFile, stime, etime, listFlag, verbose)

