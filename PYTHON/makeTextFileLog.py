#!/usr/bin/env python
# -*- coding: utf-8 -*-
__author__ = 'DanielSong'

import sys

while ("c"!=input("continue(g) or cancel (c)? :")): 
    try: 
        #raw_input
        fileNameInput = input("Enter your input:")
        SourceFileName = fileNameInput.split('.')[0]
        output = open(SourceFileName+"_Edit.log",'w')
        lineNum = 0
        for line in open(fileNameInput,'r'):
            lineNum = lineNum+1
            localLine = line.rstrip()
            if 'Memory Load:' in localLine:
                localLine = localLine.replace(' getDaylightState: [0]','')                
                n = len(localLine)
                pos = localLine.index('Memory Load:')
                timeTag = localLine[0:pos]
                localLine = localLine[pos:n]
                ## output.write(localLine+'\n')
                DataMaps = {}
                DataPairs = localLine.split(',')
                writeLine = "" + timeTag + "  "
                DoNotWrite = False
                for dataIndex in DataPairs:
                    localMap = dataIndex.split(':')
                    ##isdigit 不能检测正负号
                    if len(localMap)>1 and localMap[1].isdigit():
                        DataMaps[localMap[0]] = localMap[1]
                        writeLine += localMap[1] + " "
                        #output.write(localMap[1] + ' ')
                    else:
                        writeLine = ""
                        DoNotWrite = True
                        break
                if not DoNotWrite:
                    output.write(writeLine + '\n')
                
                ##print(DataMaps)
        output.close()
    except:
        print("something wrong\r\n")
print("Goodbye and Good luck ^^\r\n")
sys.exit()
