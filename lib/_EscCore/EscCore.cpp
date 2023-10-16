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

#include "EscCore.h"


//Initialization
EscCore::EscCore(){
  ssid = WIFI_SSID; // SSID
  password = WIFI_PASS; // Password
}



void EscCore::recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);

  if (d == "ready"){
    WebSerial.println("You Are Ready for Action!");
  }

    if (d == "restart"){
    WebSerial.println("Restarting...");
    delay(2000);
    ESP.restart();
  }
}



int EscCore::startup(uint8_t* setMACAddress, const char* mdnsName, AsyncWebServer& server){
  Serial.println("EscCore 1.0 (c) Adriaan van Wijk 2023");

  
  /* Connect WiFi */
  WiFi.softAP(mdnsName, "pinecones", 0, 1, 4);
  WiFi.mode(WIFI_AP_STA);

  /* Set MAC Address */
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);
 

  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  unsigned long wifitimeout = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
    if ((millis() - wifitimeout) > 10000){
      Serial.println("Could not find Wifi Network, Restarting...");

      ESP.restart();
    }
  }
  Serial.println();
  Serial.println("******** ********");
  Serial.println("Wifi Start");

  

  /* MDNS Initialize */ 
  if (!MDNS.begin(mdnsName))
  {
    Serial.println("mDNS Fail! Please check your network...");
    return 1;
  } 
  else
    {
      Serial.println("mDNS Start");
    }

  /* Elegant OTA */
  AsyncElegantOTA.begin(&server, "admin", "admin1234");
  Serial.println("AsyncOTA Start");

  
  /*WebSerial*/
  WebSerial.begin(&server);
  Serial.println("WebSerial Start");
  
  //Callback Function
  auto callback = [&](uint8_t* data, size_t len) {
    recvMsg(data, len);
  };

  WebSerial.msgCallback(callback);

  server.on("/", HTTP_GET, [mdnsName](AsyncWebServerRequest *request){
String webstring = String(mdnsName) + ".local/update</h1>";
String webserial = String(mdnsName) + ".local/webserial</h1>";
String html1 = R"HTML(
    <!DOCTYPE html>
    <html>
    <head>
        <style>
            body {
                background-color: #f5f5f5;
                font-family: "Helvetica Neue", Arial, sans-serif;
                font-size: 24px;
                color: #333333;
                text-align: center;
                margin-top: 200px;
            }
            h1 {
                font-size: 48px;
                color: #147efb;
                margin-bottom: 20px;
            }
        </style>
    </head>
    <body>
        <h1>Update Webserver on )HTML";
        
  String html2  =   R"HTML(
        <h1>View WebSerial on )HTML";

String html3   =   R"HTML(    
    </body>
    </html>
)HTML";

String html = html1 + webstring + html2 + webserial + html3;
    request->send(200, "text/html", html);
});

  
  server.begin();
  Serial.println("WebServer Start");
  MDNS.addService("http", "tcp", 80);

  return 0;
}


