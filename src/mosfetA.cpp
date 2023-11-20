#include <Arduino.h>
#include <EscCore.h>


#define NAME "RFID4"
#define setMACAddress m_RFID4
AsyncTimer asynctimer(35);
AsyncWebServer server(80);
ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;
EscCore Core;

uint8_t m_RFID4[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA7};

int livecount = 0;

void setup(){
Serial.begin(115200);
pinMode(4,OUTPUT);
pinMode(2,OUTPUT);

digitalWrite(4,HIGH);



  Core.startup(setMACAddress, NAME, server);

}

void trigger(){
    digitalWrite(4,LOW);
    digitalWrite(2,LOW);
    delay(100);
    digitalWrite(4,HIGH);
    digitalWrite(2,HIGH);
    livecount++;


}

void loop(){
    trigger();

delay(random(0,10000));
WebSerial.printf("Number of times Triggered: %d", livecount);

delay(3000);

}