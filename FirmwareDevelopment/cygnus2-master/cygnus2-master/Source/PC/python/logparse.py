#
# Script to parse a log file and convert data to CSV files
# Written in Python 3.6
#

import sys
import re
import csv
import ctypes
from ctypes import cdll, windll, c_long, c_int, c_char_p, create_string_buffer
from glob import glob
# define DLL and function
lib = ctypes.CDLL('Mesh.dll')
func = lib['decode']
func.restype = ctypes.c_int

decodeddata = create_string_buffer(2048)

# constants
SLOTS_PER_SHORT_FRAME = 20
SHORT_FRAMES_PER_LONG_FRAME = 8                # 4,8 or 136
LONG_FRAMES_PER_SUPER_FRAME = 64               # 16 or 64
DLCCH_SLOT_START = 0
DLCCH_SLOTS_PER_SHORT_FRAME = 5
RACH1_SLOT_START = 5
RACH1_SLOTS_PER_SHORT_FRAME = 8
RACH2_SLOT_START = 13
RACH2_SLOTS_PER_SHORT_FRAME = 2
DCH_SLOT_START = 16
DCH_SLOTS_PER_SHORT_FRAME = 4
INITIAL_VALUE = -1

DCH_SLOTS_PER_LONG_FRAME = DCH_SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME

HEADER_LENGTH = 2
LINE_NUMBER_INDEX = 0
SLOTID_INDEX = 1
SHORT_FRAME_LIST_LENTH = HEADER_LENGTH + SLOTS_PER_SHORT_FRAME
DLCCH_LIST_LENTH = HEADER_LENGTH + DLCCH_SLOTS_PER_SHORT_FRAME
RACH1_LIST_LENGTH = HEADER_LENGTH + RACH1_SLOTS_PER_SHORT_FRAME
RACH2_LIST_LENGTH = HEADER_LENGTH + RACH2_SLOTS_PER_SHORT_FRAME
DCH_LIST_LENGTH = HEADER_LENGTH + DCH_SLOTS_PER_LONG_FRAME

TEST_SYSTEM_ID = "337"

# log fields
DATETIME_FIELD = 1
SLOTID_FIELD = 2
DIRECTION_FIELD = 3
ADDR_FIELD = 4
FREQ_FIELD = 5
RSSI_FIELD = 6
SNR_FIELD = 7
DATA_FIELD = 8

# datetime fields
YEAR_FIELD = 1
MONTH_FIELD = 2
DAY_FIELD = 3
TIME_FIELD = 4

# decode fields
SLOT_IDX_FIELD = 1
SHORT_FRAME_IDX_FIELD = 2
LONG_FRAME_IDX_FIELD = 3

# system ID fields
SYSTEM_ID_FIELD = 1

def mesh_decode(instring):
    global decodeddata
    binstring = instring.encode('ascii')
    result = func(binstring, decodeddata)
    return result

def decodeMessage(instring):
    mesh_decode(instring)
    isDecodeOk = False
    sysId = 0
    isHeartbeat = False
    slotInSuperframeIdx = 0
    global SHORT_FRAMES_PER_LONG_FRAME

    sysIdRe = re.compile("SystemId=([-0-9]*)")
    s = sysIdRe.search(decodeddata.value.decode())
    if s:
        isDecodeOk = True
        sysId = s.group(SYSTEM_ID_FIELD)

        heartbeatRe = re.compile("HEARTBEAT slotIdx=([0-9]*) ShortFrameIdx=([0-9]*) LongFrameIdx=([0-9]*) ")
        r = heartbeatRe.search(decodeddata.value.decode())
        if r:
            isHeartbeat = True
            slotIdx = r.group(SLOT_IDX_FIELD)
            shortFrameIdx = r.group(SHORT_FRAME_IDX_FIELD)
            logFrameIdx = r.group(LONG_FRAME_IDX_FIELD)
            slotInSuperframeIdx = int(logFrameIdx) * SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME + \
                                  int(shortFrameIdx) * SLOTS_PER_SHORT_FRAME + \
                                  int(slotIdx)

    return isDecodeOk, sysId, isHeartbeat, slotInSuperframeIdx


def usage():
    print("logparse <logfilename(s)> [ <system Id> ]")


def getSlotTypeFlag(slotIndex, start, numberOfSlots):
    if ((slotIndex >= start) and (slotIndex < (start + numberOfSlots))):
        flag = True
    else:
        flag = False
    return flag

def clearList(aList, length):
    for x in range(0, length):
        aList[x] = ''

def testSlotParameters(shortFramesPerLongFrame, longFramesPerSuperFrame, maxSlot_Id):
    if (SLOTS_PER_SHORT_FRAME * shortFramesPerLongFrame * (longFramesPerSuperFrame - 1)) <= maxSlot_Id and \
            maxSlot_Id < (SLOTS_PER_SHORT_FRAME * shortFramesPerLongFrame * longFramesPerSuperFrame):
        flag = True
    else:
        flag = False
    return flag

def logSlot(aSlotId, aDirection, aAddr, aFreq, aRssi, aSnr, aData):
    global lineCount
    global longFrameDiscontinuityCount
    global corruptLineCount
    global countLfi
    global currentLfi
    global LONG_FRAMES_PER_SUPER_FRAME
    global DCH_LIST_LENGTH

    # derive slot indices
    slotIdx = aSlotId % SLOTS_PER_SHORT_FRAME
    shortFrameIdx = aSlotId // SLOTS_PER_SHORT_FRAME % SHORT_FRAMES_PER_LONG_FRAME
    longFrameIdx = aSlotId // (SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME) % LONG_FRAMES_PER_SUPER_FRAME

    # set slot flags
    isDLCCH = getSlotTypeFlag(slotIdx, DLCCH_SLOT_START, DLCCH_SLOTS_PER_SHORT_FRAME)
    isRACH1 = getSlotTypeFlag(slotIdx, RACH1_SLOT_START, RACH1_SLOTS_PER_SHORT_FRAME)
    isRACH2 = getSlotTypeFlag(slotIdx, RACH2_SLOT_START, RACH2_SLOTS_PER_SHORT_FRAME)
    isDCH = getSlotTypeFlag(slotIdx, DCH_SLOT_START, DCH_SLOTS_PER_SHORT_FRAME)

    #print(aSlotId,longFrameIdx,shortFrameIdx,slotIdx,isDLCCH,isRACH1,isRACH2,isDCH)

    # check for new short frame
    if (slotIdx == 0):
        # new sfi
        # print('new sfi',shortFrameIdx)
        # write row to shortframe output files
        shortframerssiout.writerow(sf_rssi_list)
        shortframesnrout.writerow(sf_snr_list)
        shortframefreqout.writerow(sf_freq_list)
        dlcchout.writerow(dlcch_list)
        rach1out.writerow(rach1_list)
        rach2out.writerow(rach2_list)

        # clear lists used in shortframe output files
        clearList(sf_rssi_list, SHORT_FRAME_LIST_LENTH)
        clearList(sf_snr_list, SHORT_FRAME_LIST_LENTH)
        clearList(sf_freq_list, SHORT_FRAME_LIST_LENTH)
        clearList(dlcch_list, DLCCH_LIST_LENTH)
        clearList(rach1_list, RACH1_LIST_LENGTH)
        clearList(rach2_list, RACH2_LIST_LENGTH)

        # set slotids
        sf_rssi_list[SLOTID_INDEX] = aSlotId
        sf_snr_list[SLOTID_INDEX] = aSlotId
        sf_freq_list[SLOTID_INDEX] = aSlotId
        dlcch_list[SLOTID_INDEX] = aSlotId
        rach1_list[SLOTID_INDEX] = aSlotId
        rach2_list[SLOTID_INDEX] = aSlotId

    # check for new long frame
    if (shortFrameIdx == 0) and (slotIdx == 0):
        # new lfi
        # print('new lfi',longFrameIdx)
        # write row to dch output files
        dchrssiout.writerow(dch_rssi_list)
        dchsnrout.writerow(dch_snr_list)
        dchfreqout.writerow(dch_freq_list)
        dchout.writerow(dch_list)

        # clear dch lists
        clearList(dch_rssi_list, DCH_LIST_LENGTH)
        clearList(dch_snr_list, DCH_LIST_LENGTH)
        clearList(dch_freq_list, DCH_LIST_LENGTH)
        clearList(dch_list, DCH_LIST_LENGTH)

        # set slotids
        dch_rssi_list[SLOTID_INDEX] = aSlotId
        dch_snr_list[SLOTID_INDEX] = aSlotId
        dch_freq_list[SLOTID_INDEX] = aSlotId
        dch_list[SLOTID_INDEX] = aSlotId

        # check new lfi
        if longFrameIdx != ((currentLfi + 1) % LONG_FRAMES_PER_SUPER_FRAME) and currentLfi != INITIAL_VALUE:
            print('WARNING: long frame index discontinuity at line', lineCount, longFrameIdx, currentLfi, (currentLfi + 1), LONG_FRAMES_PER_SUPER_FRAME)
            longFrameDiscontinuityCount = longFrameDiscontinuityCount + 1

        currentLfi = longFrameIdx

        # increment lfi count
        countLfi = countLfi + 1

    #print(month, day, time, year, aSlotId, aDirection, aAddr, aFreq, aRssi, aSnr, aData)
    # store direction / rssi in short frame rssi list
    sf_rssi_list[LINE_NUMBER_INDEX] = lineCount
    if aDirection == 'r':
        sf_rssi_list[slotIdx + HEADER_LENGTH] = aRssi
    else:
        sf_rssi_list[slotIdx + HEADER_LENGTH] = aDirection

    # store direction / snr in short frame snr list
    sf_snr_list[LINE_NUMBER_INDEX] = lineCount
    if aDirection == 'r':
        sf_snr_list[slotIdx + HEADER_LENGTH] = aSnr
    else:
        sf_snr_list[slotIdx + HEADER_LENGTH] = aDirection

    # store freq channel in short frame feq list
    sf_freq_list[LINE_NUMBER_INDEX] = lineCount
    sf_freq_list[slotIdx + HEADER_LENGTH] = aFreq

    if (isDLCCH == True):
        # store direction / rssi in dlcch list
        dlcch_list[LINE_NUMBER_INDEX] = lineCount
        dlcch_list[slotIdx - DLCCH_SLOT_START + HEADER_LENGTH] = aDirection

    if (isRACH1 == True):
        # store direction / rssi in rach1 list
        rach1_list[LINE_NUMBER_INDEX] = lineCount
        if aDirection == 'r':
            rach1_list[slotIdx - RACH1_SLOT_START + HEADER_LENGTH] = aRssi
        else:
            rach1_list[slotIdx - RACH1_SLOT_START + HEADER_LENGTH] = aDirection

    if (isRACH2 == True):
        # store direction / rssi in rach2 list
        rach2_list[LINE_NUMBER_INDEX] = lineCount
        if aDirection == 'r':
            rach2_list[slotIdx - RACH2_SLOT_START + HEADER_LENGTH] = aRssi
        else:
            rach2_list[slotIdx - RACH2_SLOT_START + HEADER_LENGTH] = aDirection

    if (isDCH == True):
        # store direction / rssi in dch rssi list
        dch_rssi_list[LINE_NUMBER_INDEX] = lineCount
        if aDirection == 'r':
            dch_rssi_list[(DCH_SLOTS_PER_SHORT_FRAME * shortFrameIdx) + slotIdx - DCH_SLOT_START + HEADER_LENGTH] = aRssi
        else:
            dch_rssi_list[(DCH_SLOTS_PER_SHORT_FRAME * shortFrameIdx) + slotIdx - DCH_SLOT_START + HEADER_LENGTH] = aDirection

        # store direction / snr in dch rssi list
        dch_snr_list[LINE_NUMBER_INDEX] = lineCount
        if aDirection == 'r':
            dch_snr_list[(DCH_SLOTS_PER_SHORT_FRAME * shortFrameIdx) + slotIdx - DCH_SLOT_START + HEADER_LENGTH] = aSnr
        else:
            dch_snr_list[(DCH_SLOTS_PER_SHORT_FRAME * shortFrameIdx) + slotIdx - DCH_SLOT_START + HEADER_LENGTH] = aDirection

        # store freq in dch freq list
        dch_freq_list[LINE_NUMBER_INDEX] = lineCount
        dch_freq_list[(DCH_SLOTS_PER_SHORT_FRAME * shortFrameIdx) + slotIdx - DCH_SLOT_START + HEADER_LENGTH] = aFreq

        # store direction in dch list
        dch_list[LINE_NUMBER_INDEX] = lineCount
        dch_list[(DCH_SLOTS_PER_SHORT_FRAME * shortFrameIdx) + slotIdx - DCH_SLOT_START + HEADER_LENGTH] = aDirection


def parseLogFile(aFileName):
    global shortframerssiout
    global shortframesnrout
    global shortframefreqout
    global dlcchout
    global rach1out
    global rach2out
    global dchrssiout
    global dchsnrout
    global dchfreqout
    global dchout

    # Declare lists
    global sf_rssi_list
    global sf_snr_list
    global sf_freq_list
    global dlcch_list
    global rach1_list
    global rach2_list
    global dch_rssi_list
    global dch_snr_list
    global dch_freq_list
    global dch_list

    # Declare global variables
    #global  DCH_SLOTS_PER_LONG_FRAME = DCH_SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME

    global HEADER_LENGTH
    global LINE_NUMBER_INDEX
    global SHORT_FRAMES_PER_LONG_FRAME
    global LONG_FRAMES_PER_SUPER_FRAME
    global DCH_LIST_LENGTH

    print('---------------------------------------------------')
    print('parsing ' + aFileName);

    # set file names
    infile = aFileName
    shortframerssioutfile = infile+"_sf_rssi.csv"
    shortframesnroutfile = infile+"_sf_snr.csv"
    shortframefreqoutfile = infile+"_sf_freq.csv"
    dlcchoutfile = infile+"_dlcch.csv"
    rach1outfile = infile+"_rach1.csv"
    rach2outfile = infile+"_rach2.csv"
    dchrssioutfile = infile+"_dch_rssi.csv"
    dchsnroutfile = infile+"_dch_snr.csv"
    dchfreqoutfile = infile+"_dch_freq.csv"
    dchoutfile = infile+"_dch.csv"

    # declare variables
    IsSynchronised = False
    HeartbeatSyncOKCount = 0
    HeartbeatSyncErrorCount = 0
    MessageDecodeOKCount = 0
    MessageDecodeErrorCount = 0
    SysIdOKCount = 0
    SysIdErrorCount = 0

    if (infile):
        lines = open(infile,'r', encoding='ansi').readlines()

        ####################################################################
        print('Pass 1 - determine the slot parameters used')
        pass1Re = re.compile("(\[.*\] )?\[([0-9]*),.*\]")
        maxSlotId = 0
        for line in lines:
            m = pass1Re.search(line)
            if m:
                slotId = int(m.group(SLOTID_FIELD))
                if maxSlotId < slotId:
                    maxSlotId = slotId

        #print('maxSlotId',maxSlotId)
        if testSlotParameters(4, 16, maxSlotId):
            SHORT_FRAMES_PER_LONG_FRAME = 4
            LONG_FRAMES_PER_SUPER_FRAME = 16
        elif testSlotParameters(136, 16, maxSlotId):
            SHORT_FRAMES_PER_LONG_FRAME = 136
            LONG_FRAMES_PER_SUPER_FRAME = 16
        elif testSlotParameters(4, 64, maxSlotId):
            SHORT_FRAMES_PER_LONG_FRAME = 4
            LONG_FRAMES_PER_SUPER_FRAME = 64
        elif testSlotParameters(8, 64, maxSlotId):
            SHORT_FRAMES_PER_LONG_FRAME = 8
            LONG_FRAMES_PER_SUPER_FRAME = 64
        elif testSlotParameters(16, 64, maxSlotId):
            SHORT_FRAMES_PER_LONG_FRAME = 16
            LONG_FRAMES_PER_SUPER_FRAME = 64
        elif testSlotParameters(136, 64, maxSlotId):
            SHORT_FRAMES_PER_LONG_FRAME = 136
            LONG_FRAMES_PER_SUPER_FRAME = 64
        else:
            #print( maxSlotId)
            print('Failed to extract slot parameters')
            return

        print('number of Short frames in a Long frame =', SHORT_FRAMES_PER_LONG_FRAME)
        print('number of Long frames in a Super frame =', LONG_FRAMES_PER_SUPER_FRAME)


        ####################################################################
        print('Pass 2 - extract fields')
        currentSlotId = INITIAL_VALUE
        currentSfi = INITIAL_VALUE
        global currentLfi
        currentLfi = INITIAL_VALUE
        global countLfi
        countLfi = 0
        DCH_SLOTS_PER_LONG_FRAME = DCH_SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME

        DCH_LIST_LENGTH = HEADER_LENGTH + DCH_SLOTS_PER_LONG_FRAME

        # open files
        shortframerssiout = csv.writer(open(shortframerssioutfile, 'w', newline=''), delimiter=',')
        shortframesnrout = csv.writer(open(shortframesnroutfile, 'w', newline=''), delimiter=',')
        shortframefreqout = csv.writer(open(shortframefreqoutfile, 'w', newline=''), delimiter=',')
        dlcchout = csv.writer(open(dlcchoutfile, 'w', newline=''), delimiter=',')
        rach1out = csv.writer(open(rach1outfile, 'w', newline=''), delimiter=',')
        rach2out = csv.writer(open(rach2outfile, 'w', newline=''), delimiter=',')
        dchrssiout = csv.writer(open(dchrssioutfile, 'w', newline=''), delimiter=',')
        dchsnrout = csv.writer(open(dchsnroutfile, 'w', newline=''), delimiter=',')
        dchfreqout = csv.writer(open(dchfreqoutfile, 'w', newline=''), delimiter=',')
        dchout = csv.writer(open(dchoutfile, 'w', newline=''), delimiter=',')

        # Declare lists
        sf_rssi_list = []
        sf_snr_list = []
        sf_freq_list = []
        dlcch_list = []
        rach1_list = []
        rach2_list = []
        dch_rssi_list = []
        dch_snr_list = []
        dch_freq_list = []
        dch_list = []

        # Initialise lists with headings
        sf_rssi_list.append('line')
        sf_rssi_list.append('slotid')
        sf_snr_list.append('line')
        sf_snr_list.append('slotid')
        sf_freq_list.append('line')
        sf_freq_list.append('slotid')
        for x in range(0, SHORT_FRAME_LIST_LENTH - HEADER_LENGTH):
            sf_rssi_list.append('slot ' + str(x))
            sf_snr_list.append('slot ' + str(x))
            sf_freq_list.append('slot ' + str(x))
        dlcch_list.append('line')
        dlcch_list.append('slotid')
        for x in range(0, DLCCH_LIST_LENTH - HEADER_LENGTH):
            dlcch_list.append('slot ' + str(x))
        rach1_list.append('line')
        rach1_list.append('slotid')
        for x in range(0, RACH1_LIST_LENGTH - HEADER_LENGTH):
            if (x & 1) == 0:
                ch = ' DATA'
            else:
                ch = ' ACK'
            rach1_list.append(str(x) + ch)
        rach2_list.append('line')
        rach2_list.append('slotid')
        for x in range(0, RACH2_LIST_LENGTH - HEADER_LENGTH):
            if (x & 1) == 0:
                ch = ' DATA'
            else:
                ch = ' ACK'
            rach2_list.append(str(x) + ch)
        dch_rssi_list.append('line')
        dch_rssi_list.append('slotid')
        dch_snr_list.append('line')
        dch_snr_list.append('slotid')
        dch_freq_list.append('line')
        dch_freq_list.append('slotid')
        dch_list.append('line')
        dch_list.append('slotid')
        for x in range(0, DCH_LIST_LENGTH - HEADER_LENGTH):
            dch_rssi_list.append('addr ' + str(x))
            dch_snr_list.append('addr ' + str(x))
            dch_freq_list.append('addr ' + str(x))
            dch_list.append('addr ' + str(x))

        # write lists to output files
        shortframerssiout.writerow(sf_rssi_list)
        shortframesnrout.writerow(sf_snr_list)
        shortframefreqout.writerow(sf_freq_list)
        dlcchout.writerow(dlcch_list)
        rach1out.writerow(rach1_list)
        rach2out.writerow(rach2_list)
        dchrssiout.writerow(dch_rssi_list)
        dchsnrout.writerow(dch_snr_list)
        dchfreqout.writerow(dch_freq_list)
        dchout.writerow(dch_list)

        # clear all lists
        clearList(sf_rssi_list, SHORT_FRAME_LIST_LENTH)
        clearList(sf_snr_list, SHORT_FRAME_LIST_LENTH)
        clearList(sf_freq_list, SHORT_FRAME_LIST_LENTH)
        clearList(dlcch_list, DLCCH_LIST_LENTH)
        clearList(rach1_list, RACH1_LIST_LENGTH)
        clearList(rach2_list, RACH2_LIST_LENGTH)
        clearList(dch_rssi_list, DCH_LIST_LENGTH)
        clearList(dch_snr_list, DCH_LIST_LENGTH)
        clearList(dch_freq_list, DCH_LIST_LENGTH)
        clearList(dch_list, DCH_LIST_LENGTH)

        global lineCount
        global longFrameDiscontinuityCount
        global corruptLineCount
        lineCount = 0
        longFrameDiscontinuityCount = 0
        corruptLineCount = 0
        errorCount = 0
        warningCount = 0
        bannerCount = 0

        logEntryRe = re.compile("(\[.*\] )?\[([0-9]*),([rtxe]),([0-9]*),([0-9]*),([-0-9]*),([-0-9]*),([0-9A-F]*).*\]")
        corruptRe = re.compile("(\[.*\] )?\[[0-9]*,")
        bannerRe = re.compile("Cygnus2 Ready")
        errorRe = re.compile("error")
        warningRe = re.compile("warning")
        startRxContinuousRe = re.compile("Start Rx Continuous mode")
        for line in lines:
            lineCount = lineCount + 1
            m = logEntryRe.search(line)
            if m:
                datetime = m.group(DATETIME_FIELD)
                slotId = int(m.group(SLOTID_FIELD))
                direction = m.group(DIRECTION_FIELD)
                addr = m.group(ADDR_FIELD)
                freq = m.group(FREQ_FIELD)
                rssi = m.group(RSSI_FIELD)
                snr = m.group(SNR_FIELD)
                data = m.group(DATA_FIELD)
                month = ''
                day = ''
                time = ''
                year = ''
                date = ''

                # is datetime present
                if datetime:
                    # extract fields from datetime
                    datetimeRe = re.compile("\[([0-9]*)-([0-9]*)-([0-9]*) ([0-9.:]*)\]")
                    d = datetimeRe.search(datetime)
                    if d:
                        year = d.group(YEAR_FIELD)
                        month = d.group(MONTH_FIELD)
                        day = d.group(DAY_FIELD)
                        time = d.group(TIME_FIELD)
                        date = day + "-" + month + "-" + year
                #print(day,month,year,time,slotId,direction,addr,freq,rssi,snr,data)

                if IsSynchronised == False:
                    if slotId == 0:
                        print("not synchronised - discarding line", lineCount)
                        continue
                    else:
                        IsSynchronised = True

                if direction=='r' or direction=='t':
                    # decode message
                    decodedOkFlag, extractedSysId, slotIdFlag, extractedSlotInSuper, = decodeMessage(data)
                    if decodedOkFlag == True:
                        MessageDecodeOKCount = MessageDecodeOKCount + 1

                        # check system id
                        if extractedSysId == TEST_SYSTEM_ID:
                            SysIdOKCount = SysIdOKCount + 1
                        else:
                            print('WARNING: system ID error at line', lineCount, extractedSysId)
                            SysIdErrorCount = SysIdErrorCount + 1

                    else:
                        print('WARNING: message decode error at line', lineCount)
                        MessageDecodeErrorCount = MessageDecodeErrorCount + 1

                    # check slot ids in heartbeats
                    if slotIdFlag == True:
                        if extractedSlotInSuper == slotId:
                            HeartbeatSyncOKCount = HeartbeatSyncOKCount + 1
                        else:
                            print('WARNING: heartbeat sync error at line', lineCount)
                            HeartbeatSyncErrorCount = HeartbeatSyncErrorCount + 1

                if currentSlotId == INITIAL_VALUE:
                    # first pass - set currnet slot id
                    currentSlotId = slotId
                else:
                    # create slot with dummy data
                    while currentSlotId != slotId:
                        #print("empty slot",currentSlotId)
                        logSlot(currentSlotId, '', '', '', '', '', '')
                        currentSlotId = (currentSlotId + 1) % \
                                        (SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME * LONG_FRAMES_PER_SUPER_FRAME)               # 16 or 64

                #print("slot containing data",slotId)
                logSlot(currentSlotId, direction, addr, freq, rssi, snr, data)

                currentSlotId = currentSlotId + 1

            else:
                n = corruptRe.search(line)
                if n:
                    print('WARNING: corrupted log entry at line', lineCount)
                    corruptLineCount = corruptLineCount + 1

                q = bannerRe.search(line)
                if q:
                    print('WARNING: banner detected at line', lineCount)
                    bannerCount = bannerCount + 1
                    IsSynchronised = False

                r = errorRe.search(line.lower())
                if r:
                    print('WARNING: ERROR detected at line', lineCount)
                    errorCount = errorCount + 1

                s = warningRe.search(line.lower())
                if s:
                    print('WARNING: WARNING detected at line', lineCount)
                    warningCount = warningCount + 1

                t = startRxContinuousRe.search(line)
                if t:
                    print('WARNING: Start Rx Continuous mode detected at line', lineCount)
                    IsSynchronised = False

    # write last lines to output files
    shortframerssiout.writerow(sf_rssi_list)
    shortframesnrout.writerow(sf_snr_list)
    shortframefreqout.writerow(sf_freq_list)
    dlcchout.writerow(dlcch_list)
    rach1out.writerow(rach1_list)
    rach2out.writerow(rach2_list)
    dchrssiout.writerow(dch_rssi_list)
    dchsnrout.writerow(dch_snr_list)
    dchfreqout.writerow(dch_freq_list)
    dchout.writerow(dch_list)

    countLfi = countLfi + 1
    
    # print statistics
    print('number of startup banners = ' + str(bannerCount))
    print('number of long frames = ' + str(countLfi))
    print('number of corrupted lines = ' + str(corruptLineCount))
    print('number of long frame discontinuities = ' + str(longFrameDiscontinuityCount))
    print('number of Heartbeats with correct sync = ' + str(HeartbeatSyncOKCount))
    print('number of Heartbeats with incorrect sync = ' + str(HeartbeatSyncErrorCount))
    print('number of Messages correctly decoded = ' + str(MessageDecodeOKCount))
    print('number of Messages failing to decode = ' + str(MessageDecodeErrorCount))
    print('number of Messages with correct system ID = ' + str(SysIdOKCount))
    print('number of Messages with incorrect system ID = ' + str(SysIdErrorCount))
    print('number of Error Messages detected = ' + str(errorCount))
    print('number of Warning Messages detected = ' + str(warningCount))
    print('---------------------------------------------------')
    print

def analyseDCH(aFileList):
    global DCH_LIST_LENGTH

    dchanalysisfile = "dch_analysis.csv"

    num_files = len(aFileList)

    # Declare lists
    header_list = []
    dchTransmit_list = []
    dchReceive_list = []
    dchCombinedReceive_list = []
    dchSuccessRate_list = []
    dchCombinedSuccessRate_list = []

    # Initialise lists
    header_list.append('')
    header_list.append('')
    for x in range(0, DCH_LIST_LENGTH - HEADER_LENGTH):
        header_list.append('addr ' + str(x))

    dchTransmit_list.append('tx count')
    dchTransmit_list.append('')

    dchReceive_list.append('rx count')
    dchReceive_list.append('')

    for i in range(0, DCH_LIST_LENGTH - HEADER_LENGTH):
        dchTransmit_list.append(0)
        dchReceive_list.append(0)
        dchCombinedReceive_list.append([])

    # open file
    dchanalysis = csv.writer(open(dchanalysisfile, 'w', newline=''), delimiter=',')

    # step through file list
    for x in aFileList:
        # clear receive list
        for i in range(HEADER_LENGTH, DCH_LIST_LENGTH):
            dchReceive_list[i] = 0

        dchFile = x + "_dch.csv"
        try:
            lines = open(dchFile, 'r', encoding='ansi').readlines()
            reader = csv.reader(lines, delimiter=',')
            for row in reader:
                for i in range(HEADER_LENGTH, DCH_LIST_LENGTH):
                    if row[i] == 't':
                        dchTransmit_list[i]+=1;
                        addr = i - HEADER_LENGTH
                    if row[i] == 'r':
                        dchReceive_list[i]+=1;
            dchReceive_list[1] = 'addr ' + str(addr);

            dchCombinedReceive_list[addr] = list(dchReceive_list)
        except OSError:
            print("Failed to open", dchFile)

    #print("T", dchTransmit_list)
    #print("R", dchCombinedReceive_list)

    # calculate success rate
    for rxl in dchCombinedReceive_list:
        lrxl = len(rxl)
        if lrxl <= HEADER_LENGTH:
            dchCombinedSuccessRate_list.append(list(rxl))
        else:
            dchSuccessRate_list = []
            dchSuccessRate_list.append('rx success rate')
            dchSuccessRate_list.append(rxl[1])
            for y in range(HEADER_LENGTH,lrxl):
                tcnt = dchTransmit_list[y]
                rcnt = rxl[y]
                if tcnt > 0:
                    success_rate = (rcnt / tcnt) * 100
                else:
                    success_rate = 0
                dchSuccessRate_list.append(success_rate)
            dchCombinedSuccessRate_list.append(list(dchSuccessRate_list))

    #print("S", dchCombinedSuccessRate_list)

    # write lists to output files
    dchanalysis.writerow(header_list)
    dchanalysis.writerow(dchTransmit_list)
    dchanalysis.writerow([])
    dchanalysis.writerow([])

    dchanalysis.writerow(header_list)
    for i in range(0, DCH_LIST_LENGTH - HEADER_LENGTH):
        dchanalysis.writerow(dchCombinedReceive_list[i])
    dchanalysis.writerow([])
    dchanalysis.writerow([])

    dchanalysis.writerow(header_list)
    for i in range(0, DCH_LIST_LENGTH - HEADER_LENGTH):
        dchanalysis.writerow(dchCombinedSuccessRate_list[i])


if '__main__' == __name__:
    infile = ""

    # check for arguments
    if len(sys.argv) < 2:
        usage()
        sys.exit(2)

    print('logparse')

    # split wildcard definitions into a list
    file_list = [f for files in sys.argv[1:] for f in glob(files)]

    # check for system id
    if len(sys.argv) == 3:
        TEST_SYSTEM_ID = sys.argv[2]

    # parse each file
    for x in range(0, len(file_list)):
        parseLogFile(file_list[x])

    # analyse all DCH files generated
    analyseDCH(file_list)