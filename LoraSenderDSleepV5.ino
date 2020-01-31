/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/



#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 18
#define rst 23
#define dio0 26

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */


#include "HX711.h"
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 35;
const int LOADCELL_SCK_PIN = 25;
const int SenderID=2;

HX711 scale;
RTC_DATA_ATTR int bootCount = 0;
bool tareFlag=false;
RTC_DATA_ATTR float tareWeight=0;

const int batteryVoltagePin=14;


void loraTest()
{
  Serial.print("Sending packet: ");
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.endPacket();  
}


void loraSendData(String loraSendString)
{
  Serial.println("Sending packet: ");
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.println(loraSendString);
  LoRa.endPacket();  
  Serial.println("LoraSendString: "+loraSendString);
  Serial.println();
}

float getBatteryVoltage()
{
    //float VBAT = (2) * 3.30f * float(analogRead(batteryVoltagePin)) / 4096.0f;  // LiFePo4 battery
    float VBAT = float(analogRead(batteryVoltagePin))/964.0;
    return VBAT;
}

float getWeight(bool taring)
{
  float weight;
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  scale.set_scale(20.224);     // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.power_up();
  if (taring) 
  {
    delay(1000);
    Serial.print("Tare  weight (g): ");
    tareWeight=scale.get_units(10); // its actually the weight in grams 
    Serial.println(tareWeight);

    
    Serial.println("Scale initialized");  
    weight=scale.get_units(10)-tareWeight; // its actually the weight in grams 
    Serial.print("Weight after taring: ");
    Serial.println(weight);
    }               

  delay (1000); //When the ESP32 is started the weight measurement isn't stable (probably the temperature is rising). Thus it is important that the time from start to measurement is the same when taring, and when measurement is performed, respectively.
  Serial.print("weight (g): ");
  weight=scale.get_units(10)-tareWeight; // its actually the weight in grams 
  Serial.println(weight);

  scale.power_down();
  return weight;
}


void setup() {
  analogSetPinAttenuation(batteryVoltagePin, ADC_6db); //Measurement of battery voltage: half the input voltage of the ADC 
  
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  String loraSendString="";
  //loraTest();
  if (bootCount==0){ tareFlag=true;}
  else{tareFlag=false;}
  
  loraSendString=loraSendString+"i="+String(SenderID)+" "+"w="+String(getWeight(tareFlag))+" "+"v="+String(getBatteryVoltage());
  //occassionally send the battery voltage data
 
  //Send Lora Data
  loraSendData(loraSendString);
  
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every TIME_TO_SLEEP seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush(); 
  bootCount++;
  esp_deep_sleep_start();
}

void loop() 
{
}
