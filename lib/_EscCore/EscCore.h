/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------                          
  Author: Adriaan van Wijk
  Date: 16 October 2023

  This code is part of a multi-node project involving Escape Rooms in Tygervallei,
  South Africa.

  Copyright (c) 2023 Proxonics (Pty) Ltd

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at:

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  --------------------------------------------------------------------------
*/

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
     * @return 0 if successful, 1 if failed.
     */
    int startup(uint8_t* setMACAddress,  const char* mdnsName, AsyncWebServer& server); // macAddress is the mac address the ESP32 will be set to.


  
  private:
    const char* ssid; // setup in config.h
    const char* password; // setup in config.h
    void recvMsg(uint8_t *data, size_t len);
};



#endif