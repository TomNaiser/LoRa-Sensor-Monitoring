# -*- coding: utf-8 -*-
"""
Created on Tue Jan 28 10:14:42 2020

@author: Thomas
"""

#!/usr/bin/env python


#Lora Base Server
#Reads Data from the ESP32 Lora Receiver  via serial Port and generates a CSV file for each LoRa sensor client


import time
import serial
import re
import datetime
import os.path


class sensorData(object):
    def __init__(self, sensorID=[0],weight=[0],voltage=[0],RSSID=[0],swarmAlarm=[0],receiveTimePosix=[0],receiveTime=[0] ):
      self.sensorID = sensorID
      self.weight = weight
      self.voltage=voltage
      self.RSSID=RSSID
      self.swarmAlarm=swarmAlarm
      self.receiveTimePosix=receiveTimePosix
      self.receiveTime=receiveTime
      
def appendLineToFile(filename,content):
    

    if os.path.isfile(filename)==False:
        with open(filename,"a") as myfile:
            myfile.write("Posix-Zeit\tSensorID\tGewicht\tSpannung\tRSSID\tZeit\n")
    else:
        with open(filename, "a") as myfile:
            myfile.write(content)
            
            
            
            
def getOutputDataLine(mySensorData):
    outputStr=""
    outputStr=outputStr+str(mySensorData.receiveTimePosix)+"\t"          
    outputStr=outputStr+str(int(mySensorData.sensorID[0]))+"\t"
    outputStr=outputStr+str(mySensorData.weight[0])+"\t"
    outputStr=outputStr+str(mySensorData.voltage[0])+"\t"
    outputStr=outputStr+str(int(mySensorData.RSSID[0]))+"\t"
    outputStr=outputStr+str(mySensorData.receiveTime)+"\n" #put datetime to the last column, otherwise kst2 plot seems to confuse the columns
    return outputStr
        
    
def parseSerialInput(IDString,serialInputString):
    value=None
    if IDString in serialInputString:
        value=[float(s) for s in re.findall(r'-?\d+\.?\d*', serialInputString)]
    return value

def convertDateTime(dt):
    timestring=""
    timestring=timestring+str(dt.year)+'-'
    timestring=timestring+str(dt.month).zfill(2)+'-'
    timestring=timestring+str(dt.day).zfill(2)+" "
    timestring=timestring+str(dt.hour).zfill(2)+":"
    timestring=timestring+str(dt.minute).zfill(2)+":"
    timestring=timestring+str(dt.second).zfill(2)
    print (timestring)
    return timestring


myDataFileName="/home/pi/BienenWaageMessdaten"

ser = serial.Serial(
        port='/dev/ttyUSB0',
        #port='COM15',
        baudrate = 115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
)

measurements=[]
currentSensorData=sensorData(sensorID=[-1])
datasetReady=False
while 1:
        x=ser.readline()
        #print(x)
        parsedInput=str(x).split('\\t',)
        
        SensorIDVal=parseSerialInput("Sensor ID",parsedInput[0])
        if SensorIDVal is not None:
            currentSensorData=sensorData(sensorID=SensorIDVal)        
            
        if "Sensor ID" in parsedInput[0]:
            
            d = datetime.datetime.now()
            timeNow=time.mktime(d.timetuple()) #get posix time
            currentSensorData.receiveTime=convertDateTime(d)
            currentSensorData.receiveTimePosix=timeNow
             
            for paramString in parsedInput:
                
                SensorIDVal=parseSerialInput("Sensor ID",paramString)
                if SensorIDVal is not None:
                    currentSensorData.sensorID=SensorIDVal
                
                weightVal=parseSerialInput("Weight",paramString) 
                if weightVal is not None:
                    currentSensorData.weight=weightVal
                 
                voltageVal=parseSerialInput("Voltage",paramString)
                if voltageVal is not None:
                    currentSensorData.voltage=voltageVal
                     
                rssidVal=parseSerialInput("RSSID",paramString)
                if rssidVal is not None:
                    currentSensorData.RSSID=rssidVal
                 
                swarmAlarmVal=parseSerialInput("Swarm Alarm",paramString)
                if swarmAlarmVal is not None:
                     currentSensorData.swarmAlarm=swarmAlarmVal
        else:
            continue
        
        outputDataLine=getOutputDataLine(currentSensorData)
        print(outputDataLine)
        appendLineToFile(myDataFileName+'Sensor'+str(int(currentSensorData.sensorID[0]))+'.txt',outputDataLine)
# =============================================================================
#         SensorIDVal=parseSerialInput("Sensor ID",str(x))
#       
#         if SensorIDVal is not None:  
#             datasetReady=True
#             #timeNow=str(datetime.datetime.now())
#             
#             d = datetime.datetime.now()
# 
#             timeNow=time.mktime(d.timetuple())
#             #timeNow=convertDateTime(datetime.datetime.now())
#             print("Create new SensorDataSet with sensorID: ",SensorIDVal)
#             newSensorData=sensorData(sensorID=SensorIDVal,receiveTime=timeNow)
#             print("Created new SensorDataSet with sensorID: ",SensorIDVal)
#             #append currentSensorData after the first Dataset has been read completely
#             #Thats when SensorIDVal is call for the second time
#             print ("CurrentSensorData: ",currentSensorData.sensorID)
#             if currentSensorData.sensorID[0]>-1: # if the currentSensorData is not completely new...
#                 measurements.append(currentSensorData)
#                 outputDataLine=getOutputDataLine(currentSensorData)
#                 csd=currentSensorData.sensorID
#                 print ("Output: ",outputDataLine)
#                 appendLineToFile(myDataFileName+'Sensor'+str(int(SensorIDVal[0]))+'.txt',outputDataLine)
#                 
# 
#     
#             currentSensorData=newSensorData
#         
#         if datasetReady:
#             weightVal=parseSerialInput("Weight",str(x)) 
#             if weightVal is not None:
#                 currentSensorData.weight=weightVal
#             
#             voltageVal=parseSerialInput("Voltage",str(x))
#             if voltageVal is not None:
#                 currentSensorData.voltage=voltageVal
#                 
#             rssidVal=parseSerialInput("RSSID",str(x))
#             if rssidVal is not None:
#                 currentSensorData.RSSID=rssidVal
#             
#             swarmAlarmVal=parseSerialInput("Swarm Alarm",str(x))
#             if swarmAlarmVal is not None:
#                 currentSensorData.swarmAlarm=swarmAlarmVal
#         
# =============================================================================
            
        
        
