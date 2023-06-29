#ifndef EscCore_h
#define EscCore_h

/* Kernal*/
#include <Arduino.h>
#include <config.h>
#include "encode.h"
/* ESP-DASH */
#include <ESPDashPro.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTimer.h>

/* Wifi and mDNS */
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>

/* ESP-NOW */
#include <esp_now.h>

/* Elegant OTA */
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

/* WebSerial */
#include <WebSerial.h>



/**
 * EscCore Initializes Wifi, AsyncOTA, WebSerial, and mDNS using various popular libraries.
 *
 * It is configured to be highly compatable and 
 * can be used with ESP-NOW as well as ESP-DASH
 * @param Credits go to Ayush Sharma, and the creators of mDNS and the esp32-arduino library.
 * @param Maintained by Adriaan van Wijk.

 */
class EscCore
{
  public:
    EscCore(); // Constructor declaration
    /**
     * Perform a setup all the code.
     * 
     * @param setMACAddress The MAC address set for the ESP32, useful for ESP-NOW communication
     * @param mdnsName The name for the DNS and Wifi. Access via http://name.local
     * @param server The server type object. Declare as AsyncWebServer server(80) somewhere before setup().
     */
    void startup(uint8_t* setMACAddress,  const char* mdnsName, AsyncWebServer& server); // macAddress is the mac address the ESP32 will be set to.


  private:
    const char* ssid;
    const char* password;
    void recvMsg(uint8_t *data, size_t len);
};



#endif