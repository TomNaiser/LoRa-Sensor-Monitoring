#include <SPI.h>
#include <LoRa.h>


//define the pins used by the lora transceiver module
#define ss 18
#define rst 23
#define dio0 26
#define LED_BUILTIN 22         // LED an ESP32 LOLIN32
#define debug false
#include <cppQueue.h>

typedef struct strRec {
  char date[11];
  char t[9];
  int sensor;
  float weight;
  float  batteryVoltage;
  int  signalStrength;
} DataRecord;


Queue  q(sizeof(DataRecord), 10, FIFO); // Instantiate queue

int WatchdogCounter=0; //Just for debugging purposes

const int weightHistoryBufferLength=5;
int weightHistoryBuffer[weightHistoryBufferLength]={0,0,0,0,0}; //Weight History  Buffer
 
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  Serial.begin(115200);

  //setup LoRa Transceiver Modul (Waage)
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  Serial.println("Setting up LoRa: ");
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  if (debug) Serial.println("LoRa initialized");
}
 
void loop()
{
  String LoraDataString="";
  int packetSize = LoRa.parsePacket();
  //Serial.print("PacketSize");
  //Serial.println(packetSize);
  if (packetSize) 
  {
    // received a packet
    if (debug) Serial.print("Received packet: ");

    // read packet
    while (LoRa.available()) 
    {
      String LoRaData = LoRa.readString();
      if (debug){
        Serial.print("Lora-Daten: ");
        Serial.println(LoRaData); 
      }
      LoraDataString+=LoRaData;
    }
    delay(10);
    if (debug) Serial.println();
    if (debug) Serial.println(LoraDataString);
    // print RSSI of packet
    int Rssi=LoRa.packetRssi();
    if (debug) Serial.print("' with RSSI ");
    if (debug) Serial.println(String(Rssi));
    
    if (LoraDataString.length()>0)
    {
      DataRecord measData;
      measData.signalStrength=Rssi;

      int delimiterPosList[10];//up to 10 parameters in the lora packet
      int previousIndex=-1;
      int lastIndex=LoraDataString.length();

      //Parse loRa packet for terms like a=3434.23 b=2323.23 c=2  (separated by space)
      if (debug) Serial.println();
      if (debug) Serial.println("************ Parsing Lora packet ************");
      while (LoraDataString.indexOf(" ",previousIndex+1)>=0)
      {
        int currentIndex=LoraDataString.indexOf(" ",previousIndex+1);  
        if (debug) Serial.println("PreviousIndex,CurrentIndex: "+String(previousIndex)+" , "+String(currentIndex));
        String currentSubstring = LoraDataString.substring(previousIndex+1, currentIndex);
        if (debug) Serial.println("CurrentSubstring: "+currentSubstring);
        previousIndex=currentIndex;      


        if(currentSubstring.indexOf("i=") >= 0)   //Sensor ID
        {
          String LoraData=currentSubstring;
          LoraData.replace("i=","");
          measData.sensor=LoraData.toFloat();
        } 
        
        if (currentSubstring.indexOf("v=") >=0)   //voltage packet
        {
          String LoraData=currentSubstring;
          LoraData.replace("v=","");
          measData.batteryVoltage=LoraData.toFloat();
        }
      
        if(currentSubstring.indexOf("w=") >= 0)   //weight packet
        {
          String LoraData=currentSubstring;
          LoraData.replace("w=","");
          measData.weight=LoraData.toFloat();
        }         
      }
      String currentSubstring=LoraDataString.substring(previousIndex+1,lastIndex);
      if (debug) Serial.println("CurrentSubstring: "+currentSubstring);

      if(currentSubstring.indexOf("i=") >= 0)   //sensor index packet
      {
        String LoraData=currentSubstring;
        LoraData.replace("i=","");
        measData.sensor=LoraData.toFloat();
      } 
      
      if (currentSubstring.indexOf("v=") >=0)   //voltage packet
      {
        String LoraData=currentSubstring;
        LoraData.replace("v=","");
        measData.batteryVoltage=LoraData.toFloat();
      }
      
      if(currentSubstring.indexOf("w=") >= 0)   //weight packet
      {
        String LoraData=currentSubstring;
        LoraData.replace("w=","");
        measData.weight=LoraData.toFloat();
      } 
      q.push(&measData); //Push measurement to the back of the  queue    
    }
  }

//if there are records in the queue -> send
  if (q.getCount()>0)
  {
    DataRecord rec;
    if (debug) Serial.println(q.getCount());
    q.peek(&rec);  

    Serial.print("Sensor ID: "+String(rec.sensor)+'\t');
    Serial.print("Weight (g): "+String(rec.weight)+'\t');
    Serial.print("Battery Voltage: "+String(rec.batteryVoltage)+'\t');
    Serial.println("RSSID: "+String(rec.signalStrength));

    Serial.println(" ");
    
    q.pop(&rec);
  }
  
  delay(1);
  if (WatchdogCounter %1000==0)
  {
      if (debug) Serial.println("WatchdogCounter: "+String(WatchdogCounter));  
  }
  WatchdogCounter++;
}
