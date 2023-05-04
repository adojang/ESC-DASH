/*
  --------------------------------------------------------------------------
  Keypad 2
  Adriaan van Wijk
  22 May 2023

  12 Key Keypad that opens an electromagnetic lock when the correct code is entered.

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

#define NAME "keypad2"
#define MACAD 0xC2 // Refer to Table in Conventions


/* Kernal*/
#include <Arduino.h>
#include <config.h>

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

/* Keypad Library*/
#include <Adafruit_Keypad.h>
/* Setup Keypad*/
const byte ROWS = 4; // rows
const byte COLS = 3; // columns
// define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
// ESP RIGHT SIDE
byte rowPins[ROWS] = {19, 4, 16, 5}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {18, 21, 17};   // connect to the column pinouts of the keypad

// initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};

/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char *ssid = WIFI_SSID;     // SSID
const char *password = WIFI_PASS; // Password

/* ESP-NOW Structures */
typedef struct dataPacket
{
  int trigger = 0;
} dataPacket;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;

bool turnlighton = false;

/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

  if (rData.trigger == 1)
  {
    // You should never use delay in this function. It might cause the ESP-NOW to crash.
    turnlighton = true;
  }
  else
  {
    turnlighton = false;
  }

  // Add your code here to do something with the data recieved.
  // It's probably best to use a flag instead of calling it directly here. Not Sure
}

void startwifi()
{

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  /* MDNS */
  if (!MDNS.begin(NAME))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  Serial.printf("*** PROGRAM START ***\n\n");

  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void startespnow()
{
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register Callback Functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void setup()
{
  Serial.begin(115200);
  startwifi();
  startespnow();

  // Make any Edits you need to add below this line ------------------------------

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  customKeypad.begin(); // Startup for Keypad
}

void loop()
{

  // This line is sort of required. It automatically sends the data every 5 seconds. Don't know why. But hey there it is.
  asynctimer.setInterval([]()
  { esp_now_send(broadcastAddress, (uint8_t *)&sData, sizeof(sData)); },
  5000);

  customKeypad.tick();

  while (customKeypad.available())
  {
    keypadEvent e = customKeypad.read();
    Serial.print((char)e.bit.KEY);
    if (e.bit.EVENT == KEY_JUST_PRESSED)
      Serial.println(" pressed");
    else if (e.bit.EVENT == KEY_JUST_RELEASED)
      Serial.println(" released");
  }

  // Required for the asynctimer to work.
  asynctimer.handle();
}
