/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  This code controls 4 relays which will arm and disarm electromagnetic locks.

  Copyright [2023] [Proxonics (Pty) Ltd]

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  --------------------------------------------------------------------------
*/

#define NAME "thumbreader"
#define MACAD 0xC1 // Refer to Table in Conventions



/* Kernal*/
#include <Arduino.h>
#include <config.h>
#include <encode.h>

/* ESP-DASH */
#include <ArduinoJson.h>
#include <AsyncTimer.h>

/* Wifi and mDNS */
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>

/* ESP-NOW */
#include <esp_now.h>

/* Elegant OTA */
#include <AsyncElegantOTA.h>

/*************************WebSerial ****************************************/
#include <WebSerial.h>
/* Message callback of WebSerial */


/*************************WebSerial ****************************************/



// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};


/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* ESP-NOW Structures */

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;
String success;

// bool opendoor1 = false;
// bool opendoor2 = false;
// bool opendoor3 = false;
// bool opendoor4 = false;

float OPENVALUE = 30;
float tuneconstant = 0.5; // Value to start with. Callibration should sort this out.
float samples1[50];
float averages[10] = {40,40,40,40,40,40,40,40,40,40};
int val = 0;
float sum;
int avg1=40;
int avg2=40;
int ScannedSuccessful=0;
unsigned long locktimer = millis();


float currentValue = 0.0;
float previousEMA = 0.0;
float prevousEMA2 = 0.0;
float smoothingFactor = 0.8;  // Adjust this value based on your application



//Fast Flash to show SENT Data Succesfully
void sendDataLED(){
  // If it works... it works...
  digitalWrite(2,HIGH);
  asynctimer.setTimeout([]() {digitalWrite(2,LOW);},  200);
  asynctimer.setTimeout([]() {digitalWrite(2,HIGH);},  400);
  asynctimer.setTimeout([]() {digitalWrite(2,LOW);},  600);
}



void opensesame(int pin){
  digitalWrite(pin, LOW);
  Serial.println("Door Opened");
  WebSerial.println("Door Opened");
  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
    }, 3000);
  
  
  // asynctimer.setInterval([]() {esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  5000);

}



/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));

  if(rData.origin == masterserver && rData.sensor == train_thumb){
    opensesame(13);
  }


}

float emaFilter(float currentValue, float previousEMA, float smoothingFactor) {
  return (currentValue * smoothingFactor) + (previousEMA * (1 - smoothingFactor));
}

float absolute(float value){
  if (value < 0){
    return -value;
  } else
  return value;
}


void callibrateReader(){
for (int j=0; j<100; j++){
  val = hallRead();
  sum=sum + val;
  delay(10);
}
sum = sum/100;
OPENVALUE = emaFilter(sum, previousEMA, smoothingFactor);
WebSerial.printf("Thumb Reader Callibrated OpenValue is ");
WebSerial.println(OPENVALUE);
Serial.println("Baseline Value is: ");
Serial.println(OPENVALUE);

//Test if it triggers at all:
bool continuecalibration = true;
locktimer = millis();
while(continuecalibration){

  for (int j=0; j<20; j++){
    val = hallRead();
    sum=sum + val;
  }
  sum = sum/20;
  float filteredValue = emaFilter(sum, previousEMA, smoothingFactor);
  previousEMA = filteredValue;

  if (absolute(OPENVALUE - filteredValue) > (tuneconstant*OPENVALUE) && ((millis() - locktimer) > 100)){
    Serial.printf("Threshhold Adjusted: %f\n", OPENVALUE*tuneconstant);
    WebSerial.printf("Threshhold Adjusted: %f\n", OPENVALUE*tuneconstant);
    locktimer = millis();
    tuneconstant = tuneconstant + 0.15*tuneconstant; // increase by 10% each time.
  }

  if (millis() - locktimer > 10000){
    continuecalibration = false; // We have reached equilibrum, no triggering for 5 seconds
    Serial.printf("\n");
    tuneconstant = tuneconstant*1.2;
    Serial.printf("Final Threshhold: %f\n", OPENVALUE*tuneconstant);
    WebSerial.printf("Final Threshhold: %f\n", OPENVALUE*tuneconstant);
  }


}

  
Serial.println("Sucessfully Callibrated");
WebSerial.println("Sucessfully Callibrated");

}


void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);

  if (d == "callibrate"){
    WebSerial.println("Callibrating...");
    callibrateReader();
  }
}

 


void startWifi()
{
  /* Connect WiFi */
 WiFi.softAP(NAME, "pinecones", 0, 1, 4);
 WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);

  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  unsigned long wifitimeout = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if ((millis() - wifitimeout) > 10000) ESP.restart();
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }else
  {Serial.println("\n\nWIFI CONNECTED!");}

  if (!MDNS.begin(NAME))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
 
  /* Elegant OTA */
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  /* WEB SERIAL REQUIRED TO FUNCTION */
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  

  server.begin();
  MDNS.addService("http", "tcp", 80);
  WebSerial.println("mDNS responder started");
  WebSerial.println("WebSerial Service started");
}

void startespnow(){
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Register Callback Functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

}

unsigned long ttime;

void setup() {
  Serial.begin(115200);
  startWifi();
  startespnow();

  pinMode(2, OUTPUT);
  pinMode(13,OUTPUT); // Relay Trigger
  pinMode(18,OUTPUT);
  digitalWrite(18,LOW);
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  digitalWrite(13,HIGH);
  delay(2000);
  Serial.println("Callibrate Reader");
  WebSerial.println("Callibrate Reader");
  callibrateReader();
  ttime = millis();
  locktimer = millis();
  }



//On Start, callibratae values. If triggered 10 times within a minute, recalibrate.




void loop() {



//Take 25 Readings and Average Them.
for (int j=0; j<20; j++){
  val = hallRead();
  sum=sum + val;
}
sum = sum/20;
float filteredValue = emaFilter(sum, previousEMA, smoothingFactor);
previousEMA = filteredValue;
  
if (millis() - ttime > 2000){
  ttime = millis();
  WebSerial.printf("Read Value: %f\n", (absolute(filteredValue)));
  WebSerial.printf("Threshold Value: %f\n", OPENVALUE*tuneconstant);
  Serial.println(absolute(OPENVALUE - filteredValue));
}


if (absolute(OPENVALUE - filteredValue) > tuneconstant*OPENVALUE && ((millis() - locktimer) > 2000)){
  Serial.printf("OpenValue: %f\n", OPENVALUE);
  Serial.printf("filteredValue: %f\n", filteredValue);
  Serial.printf("Threshhold: %f\n", OPENVALUE*tuneconstant);
  Serial.printf("Difference: %f\n", absolute(OPENVALUE - filteredValue));
  locktimer = millis();;
  WebSerial.println("OpenSesame");
  opensesame(13); // 5s timeout til resets.

}




  
  asynctimer.handle();
}

