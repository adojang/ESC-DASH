/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  This code is part of a multi-node project which involes Esc Rooms in Tygervallei,
  South Africa.

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

#include <EscCore.h>

#define NAME "escape"
#define setMACAddress m_masterserver

#pragma region mac
// Control
uint8_t m_masterserver[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00};
uint8_t m_trainmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01};
uint8_t m_tombmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02};
uint8_t m_atticmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03};

// Attic
uint8_t m_attic_humanchain[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA0};
uint8_t m_attic_bike[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1};
uint8_t m_clock[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2};
uint8_t m_attic_overrideButton[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA3};
uint8_t m_RFID1[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA4};
uint8_t m_RFID2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA5};
uint8_t m_RFID3[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA6};
uint8_t m_RFID4[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA7};
uint8_t m_attic_clock2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA8};
uint8_t m_attic_morse[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA9};


// Tomb
uint8_t m_tomb_sennet[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0};
uint8_t m_tomb_chalice[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB1};
uint8_t m_tomb_ringReader[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB2};
uint8_t m_tomb_tangrum[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB3};
uint8_t m_tomb_maindoorOverride[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB4};
uint8_t m_tomb_slidedoorOverride[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB5};

// Train
uint8_t m_train_keypad[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC0};
uint8_t m_train_thumb[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC1};
uint8_t m_train_overrideButton[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC2};
#pragma endregion mac

AsyncTimer asynctimer(35);
AsyncWebServer server(80);
ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;
EscCore Core;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Configuration and Setup */
bool checkrfidstuck = false;
bool RFID1_reset = false;
bool RFID2_reset = false;
bool RFID3_reset = false;
bool RFID4_reset = false;


unsigned long emergencyTrigger = 0;
bool RFID1_complete = false;
bool RFID2_complete = false;
bool RFID3_complete = false;
bool RFID4_complete = false;
bool rfiddoorlock = false;
unsigned long ttime = millis();
unsigned long time250 = millis();
int keypadtrigger = 0;

unsigned short attic_status_timer = 0;
unsigned short clock_status_timer = 0;
unsigned short bike_status_timer = 0;
unsigned short RFID1_status_timer = 0;
unsigned short RFID2_status_timer = 0;
unsigned short RFID3_status_timer = 0;
unsigned short RFID4_status_timer = 0;
unsigned short thumbreader_status_timer = 0;
unsigned short tomb_status_timer = 0;
unsigned short tomb_chalice_timer = 0;
unsigned short tomb_tangrum_timer = 0;
unsigned short tomb_sennet_timer = 0;


#pragma region Cards
/* Cards */

Card timesinceboot(&dashboard, ENERGY_CARD, "Time Since Boot", "min");
Card restart_master(&dashboard, BUTTON_CARD, "Restart Server"); //momentary
Card restart_attic(&dashboard, BUTTON_CARD, "Restart Attic"); //momentary

/*Online Status*/
Card attic_status(&dashboard, STATUS_CARD, "Attic", "warning");
Card clock_status(&dashboard, STATUS_CARD, "Clock", "warning");
Card bike_status(&dashboard, STATUS_CARD, "Bike", "warning");
Card RFID1_status(&dashboard, STATUS_CARD, "RFID1", "warning");
Card RFID2_status(&dashboard, STATUS_CARD, "RFID2", "warning");
Card RFID3_status(&dashboard, STATUS_CARD, "RFID3", "warning");
Card RFID4_status(&dashboard, STATUS_CARD, "RFID4", "warning");
Card thumbreader_status(&dashboard, STATUS_CARD, "Thumb Reader", "warning");
Card tomb_status(&dashboard, STATUS_CARD, "Spooky Tomb", "warning");
Card chalice_status(&dashboard, STATUS_CARD, "Chalice", "warning");
Card tangrum_status(&dashboard, STATUS_CARD, "Tangrum Puzzle", "warning");
Card sennet_status(&dashboard, STATUS_CARD, "Sennet Puzzle", "warning");



/* Attic */
Card humanchain_card(&dashboard, BUTTON_CARD, "Open Human Chain Door"); // momentary
Card trim1(&dashboard, BUTTON_CARD, "Trim Clock Up"); //momentary
Card trim2(&dashboard, BUTTON_CARD, "Trim Clock Down"); //momentary
Card clockjoystick(&dashboard, JOYSTICK_CARD, "Clock Control", "lockY");
Card reset_RFID(&dashboard, BUTTON_CARD, "Reset RFID Puzzle"); //momentary

Card lockdoor(&dashboard, BUTTON_CARD, "Lock RFID Door"); //momentary

Card lockdoor_status(&dashboard, STATUS_CARD, "Door Status", "success");
Card DOORTOUCH(&dashboard, STATUS_CARD, "DoorTouch", "idle");
// Card touchval(&dashboard, GENERIC_CARD, "DoorTouch Analog Read");

// Card clock_armed(&dashboard, STATUS_CARD, "Clock Status", "idle");
// Card clock_timeout(&dashboard, STATUS_CARD, "Clock Sensor", "success");

// Card clock_reset(&dashboard, BUTTON_CARD, "Arm/Disarm Clock");
Card trigger_clock(&dashboard, BUTTON_CARD, "Manually Trigger Clock"); //momentary
Card reset_clock(&dashboard, BUTTON_CARD, "Reset Clock Position"); //momentary

Card overide_rfid(&dashboard, BUTTON_CARD, "Override All RFID"); //momentary
Card attic_rfid1(&dashboard, STATUS_CARD, "RFID1 Status", "idle");
Card attic_rfid2(&dashboard, STATUS_CARD, "RFID2 Status", "idle");
Card attic_rfid3(&dashboard, STATUS_CARD, "RFID3 Status", "idle");
Card attic_rfid4(&dashboard, STATUS_CARD, "RFID4 Status", "idle");
Card attic_bikelight(&dashboard, BUTTON_CARD, "Bike Light");

/* Ancient Tomb */
Card maindoor_override(&dashboard, BUTTON_CARD, "Open Main Door"); //momentary
Card chalice_card(&dashboard, BUTTON_CARD, "Open Chalice Door"); //momentary
Card tangrumtomb_card(&dashboard, BUTTON_CARD, "Open Tomb"); //momentary
Card sennet_card(&dashboard, BUTTON_CARD, "Override Sennet Puzzle"); //momentary

/* All Aboard (Train) */
Card trainroomdoor_card(&dashboard, BUTTON_CARD, "Open Train Room Door"); //momentary
Card thumbreader_card(&dashboard, BUTTON_CARD, "Overide Thumb Reader"); //momentary

/* Tabs */
Tab attic(&dashboard, "Attic");
Tab train(&dashboard, "All Aboard");
Tab tomb(&dashboard, "Ancient Tomb");


#pragma endregion Cards

/* Functions */
void triggerDoor(int pin){

  //Write Before in case of a crash
  sData.origin = masterserver;
  sData.sensor = masterserver;
  sData.data = 1; // Door Open
  esp_err_t result = esp_now_send(m_train_keypad, (uint8_t *) &sData, sizeof(sData));

    digitalWrite(pin, HIGH);
  WebSerial.println("Door Opened");
  Serial.println("Door Opened");

  asynctimer.setTimeout([pin]() {

      
      sData.origin = masterserver;
      sData.sensor = masterserver;
      sData.data = 0; // Door closed
      esp_err_t result = esp_now_send(m_train_keypad, (uint8_t *) &sData, sizeof(sData));
      digitalWrite(pin, LOW);
      WebSerial.println("Door Closed");
      Serial.println("Door Closed");

      delay(50);
    }, 3000);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }


void handleStatus(unsigned short & checktimer, Card* checkCard){
    
        if(checktimer == 0){
          //Initialize Timer.
          checkCard->update("Connected", "success");
          checktimer = asynctimer.setTimeout([checkCard]() {
          checkCard->update("Disconnected", "danger");
          }, 3000);
      } else{
          //Cancel Old Timer, and set a new one.
          asynctimer.cancel(checktimer);
          checkCard->update("Connected", "success");
          checktimer = asynctimer.setTimeout([checkCard]() {
         checkCard->update("Disconnected", "danger");
         }, 3000);
    }
  }





void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));




  if(rData.origin == train_keypad && rData.sensor == train_keypad){
  keypadtrigger = rData.data;
  }

  // if(rData.origin == attic_humanchain && rData.sensor == attic_humanchain){
  //   touchval.update(rData.data); // analog data values of the door's touch sensor. // RETIRE
  //   dashboard.sendUpdates();
  // }

  if(rData.origin == atticmaster && rData.sensor == attic_humanchain)
  {
    if(rData.data == 1) DOORTOUCH.update("Touching", "success");
    if(rData.data == 0) DOORTOUCH.update("Not Touching", "danger");
    
    dashboard.sendUpdates();
  }


  if(rData.origin == attic_RFID1 && rData.sensor == attic_RFID1)
  {
    if(RFID1_complete == false){
      attic_rfid1.update("None", "warning");
      RFID1_reset = true;
      }     
    
    // if(rData.data == 0) {attic_rfid1.update("None", "warning");}  

    if (rData.data == 100){
      attic_rfid1.update("COMPLETE","success");
      RFID1_complete = true;
      RFID1_reset = false;
    }
    // attic_rfid1.update("COMPLETE", "success");
    dashboard.sendUpdates();
  }

  if(rData.origin == attic_RFID2 && rData.sensor == attic_RFID2)
  {
    if(RFID2_complete == false){
      attic_rfid2.update(rData.data, "warning");
      RFID2_reset = false;
    }

    if(rData.data == 0) {
      attic_rfid2.update("None", "warning");
      RFID2_reset = true;
      } 

    if (rData.data == 100){
      attic_rfid2.update("COMPLETE", "success");
      RFID2_complete = true;
    }
    dashboard.sendUpdates();
}

  if(rData.origin == attic_RFID3 && rData.sensor == attic_RFID3)
  {
    WebSerial.println("RFID3 Triggerd but is it updated?");
    if(RFID3_complete == false){
      attic_rfid3.update(rData.data, "warning");
      RFID3_reset = false;
      }

    if(rData.data == 0) {
      attic_rfid3.update("None", "warning");
      RFID3_reset = true;
      } 

    if (rData.data == 100){
     attic_rfid3.update("COMPLETE", "success");
    RFID3_complete = true;
    }
    dashboard.sendUpdates();
  }

  if(rData.origin == attic_RFID4 && rData.sensor == attic_RFID4)
  {
    if(RFID4_complete == false){
      attic_rfid4.update(rData.data, "warning");
      }
    
    if(rData.data == 1){ // This is equivalent to RESET because 1 is always active.
      RFID4_reset = true;
      } else{
         RFID4_reset = false;
      }

    if(rData.data == 0) {attic_rfid4.update("None", "warning");} 

    if (rData.data == 100){
      attic_rfid4.update("COMPLETE", "success");
      RFID4_complete = true;
    }
    dashboard.sendUpdates();
  }

  // if(rData.origin == attic_clock && rData.sensor == attic_clock){
  //   //Clock has triggered.
  //   if(rData.data == 1){
  //   clock_armed.update("Clock Disabled", "warning");
  //   clock_reset.update(0);
  //   dashboard.sendUpdates(); 
  //   }
  //   else{
  //   clock_armed.update("Clock Enable", "success");
  //   clock_reset.update(1);
  //   dashboard.sendUpdates(); 
  //   }


  // }

  /* Status Updates */
  if(rData.origin == atticmaster && rData.sensor == status_alive) handleStatus(attic_status_timer, &attic_status);
  if(rData.origin == attic_clock && rData.sensor == status_alive) handleStatus(clock_status_timer, &clock_status);
  if(rData.origin == attic_bike && rData.sensor == status_alive)  handleStatus(bike_status_timer, &bike_status);
  if(rData.origin == attic_RFID1 && rData.sensor == status_alive) handleStatus(RFID1_status_timer, &RFID1_status);
  if(rData.origin == attic_RFID2 && rData.sensor == status_alive) handleStatus(RFID2_status_timer, &RFID2_status);
  if(rData.origin == attic_RFID3 && rData.sensor == status_alive) handleStatus(RFID3_status_timer, &RFID3_status);
  if(rData.origin == attic_RFID4 && rData.sensor == status_alive) handleStatus(RFID4_status_timer, &RFID4_status);
  if(rData.origin == train_thumb && rData.sensor == status_alive) handleStatus(thumbreader_status_timer, &thumbreader_status);
  if(rData.origin == tombmaster && rData.sensor == status_alive) handleStatus(tomb_status_timer, &tomb_status);
  if(rData.origin == tomb_chalice && rData.sensor == status_alive) handleStatus(tomb_chalice_timer, &chalice_status);
  if(rData.origin == tomb_tangrum && rData.sensor == status_alive) handleStatus(tomb_tangrum_timer, &tangrum_status);
  if(rData.origin == tomb_sennet && rData.sensor == status_alive) handleStatus(tomb_sennet_timer, &sennet_status);







}



void startespnow(){ // Remeber to register mac addresses before sending data;
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void registermac(uint8_t address[]){
  memcpy(peerInfo.peer_addr, address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void configDash(){
  dashboard.setTitle("Escape Room Master Control");
  sData.origin = 0x00;

  /*Status Cards*/
  attic_status.update("Disconnected", "danger");
  clock_status.update("Disconnected", "danger");
  bike_status.update("Disconnected", "danger");
  RFID1_status.update("Disconnected", "danger");
  RFID2_status.update("Disconnected", "danger");
  RFID3_status.update("Disconnected", "danger");
  RFID4_status.update("Disconnected", "danger");
  thumbreader_status.update("Disconnected", "danger");
  tomb_status.update("Disconnected", "danger");
  chalice_status.update("Disconnected", "danger");
  tangrum_status.update("Disconnected", "danger");
  sennet_status.update("Disconnected", "danger");

  /* Attic */
  humanchain_card.setTab(&attic);
  // restart_attic.setTab(&attic);
  trim1.setTab(&attic);
  trim2.setTab(&attic);
  clockjoystick.setTab(&attic);
  reset_RFID.setTab(&attic);
  lockdoor.setTab(&attic);
  overide_rfid.setTab(&attic);
  lockdoor_status.setTab(&attic);
  DOORTOUCH.setTab(&attic);
  // clock_armed.setTab(&attic);
  // clock_reset.setTab(&attic);
  reset_clock.setTab(&attic);
  trigger_clock.setTab(&attic);
  // touchval.setTab(&attic);
  attic_rfid1.setTab(&attic);
  attic_rfid2.setTab(&attic);
  attic_rfid3.setTab(&attic);
  attic_rfid4.setTab(&attic);
  // humanchain_card.setSize(6,6,6,6,6,6);

  /* Tomb */
  sennet_card.setTab(&tomb);
  chalice_card.setTab(&tomb);
  maindoor_override.setTab(&tomb);
  tangrumtomb_card.setTab(&tomb);
  sennet_card.setSize(6,6,6,6,6,6);
  chalice_card.setSize(6,6,6,6,6,6);
  tangrumtomb_card.setSize(6,6,6,6,6,6);
  maindoor_override.setSize(6,6,6,6,6,6);
  
  /* Train */
  thumbreader_card.setTab(&train);
  trainroomdoor_card.setTab(&train);
  thumbreader_card.setSize(6,6,6,6,6,6);
  trainroomdoor_card.setSize(6,6,6,6,6,6);
}

void buttonTimeout(Card* cardptr, int timeout = 3000){
  cardptr->update(1);
  asynctimer.setTimeout([cardptr]() {
    cardptr->update(0);
    dashboard.sendUpdates();
  }, timeout);
  dashboard.sendUpdates();
}

void startButtonCB(){

restart_master.attachCallback([](int value){
buttonTimeout(&restart_master);
Serial.printf("Master Restart Triggered\n");
WebSerial.printf("Master Restart Triggered\n");
delay(1000);
ESP.restart();
});

/* Train */

  trainroomdoor_card.attachCallback([](int value){
  buttonTimeout(&trainroomdoor_card);
  Serial.printf("Train Room Triggered\n");
  WebSerial.println("Train Room Triggered via web");
  triggerDoor(5);  
});

thumbreader_card.attachCallback([](int value){
buttonTimeout(&thumbreader_card);
  Serial.printf("Thumbreader Triggered\n");
  WebSerial.println("Thumbreader Triggered via web");
  sData.origin = masterserver;
  sData.sensor = masterserver;
  sData.data = 1;
  esp_err_t result = esp_now_send(m_train_thumb, (uint8_t *) &sData, sizeof(sData));
  if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
  sData.data = 0;

});


/* Attic */

restart_attic.attachCallback([](int value){
buttonTimeout(&restart_attic);
Serial.printf("Attic Restart Triggered\n");

sData.data = 66;
esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
sData.data = 0;
});

trim1.attachCallback([](int value){
buttonTimeout(&trim1);
Serial.printf("Attic Trim 1 Triggered\n");

sData.data = 10;
esp_err_t result = esp_now_send(m_clock, (uint8_t *) &sData, sizeof(sData));
sData.data = 0;
});

trim2.attachCallback([](int value){
buttonTimeout(&trim2);
Serial.printf("Attic Trim 2 Triggered\n");
sData.data = 20;
esp_err_t result = esp_now_send(m_clock, (uint8_t *) &sData, sizeof(sData));
sData.data = 0;
});

clockjoystick.attachCallback([&](int8_t x, int8_t y){
  sData.origin = masterserver;
  sData.sensor = attic_clock;
  sData.data = y;


  if((millis() - time250) >= 250){
    Serial.println("sending dataa");
    esp_err_t result = esp_now_send(m_clock, (uint8_t *) &sData, sizeof(sData));
    time250 = millis();
  }
});


reset_RFID.attachCallback([](int value){ //Reset the RFID Values by restarting the RFID readers.
buttonTimeout(&reset_RFID);
Serial.println("RFID RESET triggered");
WebSerial.printf("RFID RESET triggered\n");

//Write TEMP so you can see its being reset:
attic_rfid1.update("Resetting...", "danger");
attic_rfid2.update("Resetting...", "danger");
attic_rfid3.update("Resetting...", "danger");
attic_rfid4.update("Resetting...", "danger");

checkrfidstuck = true;

dashboard.sendUpdates();

RFID1_complete = false;
RFID2_complete = false;
RFID3_complete = false;
RFID4_complete = false;

sData.origin = masterserver;
sData.sensor = masterserver;
sData.data = 77;

//Reset Attic
esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));

//Restart RFID
esp_now_send(m_RFID1, (uint8_t *) &sData, sizeof(sData));
esp_now_send(m_RFID2, (uint8_t *) &sData, sizeof(sData));
esp_now_send(m_RFID3, (uint8_t *) &sData, sizeof(sData));
esp_now_send(m_RFID4, (uint8_t *) &sData, sizeof(sData));



});

lockdoor.attachCallback([](int value){
lockdoor.update(value);
if (value == 1) {
  rfiddoorlock = true;
  WebSerial.printf("RFID Door Locked\n");
  Serial.printf("Door Locked\n");
  lockdoor_status.update("Door Locked", "warning");
  sData.data = 88;
  esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
  sData.data = 0;
}
if (value == 0) {
  rfiddoorlock = false;
  Serial.printf("Door Unlocked\n");
  WebSerial.printf("RFID Door Unlocked\n");
  lockdoor_status.update("Door Unlocked", "success");
  
  sData.data = 99;
  esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
  sData.data = 0;
  }

  dashboard.sendUpdates();
});

// clock_reset.attachCallback([](int value){
// clock_reset.update(value);
// if (value == 0) {
//   WebSerial.printf("Clock Locked\n");
//   Serial.printf("Clock Locked\n");
//   clock_armed.update("Clock Disabled", "warning");
//   sData.origin = masterserver;
//   sData.sensor = masterserver;
//   sData.data = 0;
//   esp_err_t result = esp_now_send(m_clock, (uint8_t *) &sData, sizeof(sData));
//   sData.data = 0;
// }
// if (value == 1) {
//   WebSerial.printf("Clock Armed\n");
//   Serial.printf("Clock Armed\n");
//   clock_armed.update("Clock Armed", "success");
//   sData.origin = masterserver;
//   sData.sensor = masterserver;
//   sData.data = 1;
//   esp_err_t result = esp_now_send(m_clock, (uint8_t *) &sData, sizeof(sData));
//   sData.data = 0;
//   }

//   dashboard.sendUpdates();
// });


overide_rfid.attachCallback([](int value){
buttonTimeout(&overide_rfid);
Serial.printf("RFID Override\n");
WebSerial.printf("RFID Override\n");

//Send override data to attic.
sData.origin = masterserver;
sData.sensor = masterserver; 
sData.data = 50;
esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
sData.data = 0;
  
dashboard.sendUpdates();


});

trigger_clock.attachCallback([](int value){
buttonTimeout(&trigger_clock);
Serial.printf("Clock Returned to Starting Position\n");
WebSerial.printf("Clock Returned to Starting Position\n");

sData.origin = masterserver;
sData.sensor = masterserver; 
sData.data = 42;
esp_err_t result = esp_now_send(m_clock, (uint8_t *) &sData, sizeof(sData));
sData.data = 0;
  
dashboard.sendUpdates();


});

reset_clock.attachCallback([](int value){
buttonTimeout(&reset_clock);
Serial.printf("Clock Returned to Starting Position\n");
WebSerial.printf("Clock Returned to Starting Position\n");

sData.origin = masterserver;
sData.sensor = masterserver; 
sData.data = 55;
esp_err_t result = esp_now_send(m_clock, (uint8_t *) &sData, sizeof(sData));
sData.data = 0;
  
dashboard.sendUpdates();


});


humanchain_card.attachCallback([](int value){
// humanchain_card.update(1);
buttonTimeout(&humanchain_card);
Serial.printf("Attic Door Opened\n");
WebSerial.printf("Attic Door Opened\n");
sData.sensor = attic_humanchain; 
sData.data = 1;
esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
sData.data = 0;
  
});

humanchain_card.attachCallback([](int value){
// humanchain_card.update(1);
buttonTimeout(&humanchain_card);
Serial.printf("Attic Door Opened\n");
WebSerial.printf("Attic Door Opened\n");
sData.sensor = attic_humanchain; 
sData.data = 1;
esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
sData.data = 0;
  
});


attic_bikelight.attachCallback([&](int value){
  Serial.println("BikeLight "+String((value == 1)?"true":"false"));
  attic_bikelight.update(value);
sData.origin = masterserver;
sData.sensor = attic_bike;
sData.data = value;
esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
sData.data = 0;

  dashboard.sendUpdates();
});

/* Tomb */

/* 0xB0 - sennet table */
sennet_card.attachCallback([](int value){
buttonTimeout(&sennet_card);
Serial.printf("Sennet Table Triggered\n");
sData.sensor = tomb_sennet; 
sData.data = 1;
esp_err_t result = esp_now_send(m_tomb_sennet, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
sData.data = 0;

});

/* 0xB1 - chalicessensor */
chalice_card.attachCallback([](int value){
buttonTimeout(&chalice_card);
Serial.printf("Chalice Sensor Triggered\n");
sData.origin = masterserver;
sData.sensor = tomb_chalice; 
sData.data = 1;
esp_err_t result = esp_now_send(m_tombmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
sData.data = 0;

});

/* 0xB2 - ringreader */
maindoor_override.attachCallback([](int value){
buttonTimeout(&maindoor_override);
Serial.printf("Ring Reader Triggered\n");
sData.origin = masterserver;
sData.sensor = masterserver; 
sData.data = 111;
esp_err_t result = esp_now_send(m_tombmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
sData.data = 0;

});

/* 0xB3 - tangrumtomb */
tangrumtomb_card.attachCallback([](int value){
buttonTimeout(&tangrumtomb_card);
Serial.printf("Tangram Tomb Triggered\n");
sData.origin = masterserver;
sData.sensor = tomb_tangrum; 
sData.data = 1;
esp_err_t result = esp_now_send(m_tombmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("Error. Probably not Registerd.");}
sData.data = 0;

});

  //Initialize Statues'
  lockdoor_status.update("Door Unlocked", "success");
  // clock_armed.update("ARMED", "success");
  // clock_timeout.update("Working", "success");
  // clock_reset.update(1);
  attic_rfid1.update("Unknown", "danger");
  attic_rfid2.update("Unknown", "danger");
  attic_rfid3.update("Unknown", "danger");
  attic_rfid4.update("Unknown", "danger");
  DOORTOUCH.update("Startup", "idle");
  dashboard.sendUpdates();
}

void updateTime(){

  int seconds = millis() / 1000;
  int minutes = seconds / 60;
  int hours = minutes / 60;



  if (seconds < 60){
    timesinceboot.update(seconds, "seconds");

  } else{
    if(minutes < 60){
    String timeA = String(minutes) + ":" + String(seconds % 60);
    timesinceboot.update(timeA, "min");

    } else {
      String timeB = String(hours) + ":" + String(minutes % 60) + ":" + String(seconds % 60);
        timesinceboot.update(timeB, "sec");
    }
  }
  dashboard.sendUpdates();
}

void setup() {
  Serial.begin(115200);
  
  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_clock);
  registermac(m_atticmaster);
  registermac(m_tombmaster);
  registermac(m_train_thumb);
  registermac(m_train_keypad);
  registermac(m_RFID1);
  registermac(m_RFID2);
  registermac(m_RFID3);
  registermac(m_RFID4);
  registermac(m_tomb_chalice);
  registermac(m_tomb_sennet);

  configDash();
  startButtonCB();
  

  #pragma region gpio
  //Signaling LED
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);

  pinMode(25,OUTPUT);  //Emergency Buttons
  digitalWrite(25,HIGH); // 25 is trigger pin HIGH. Source.
  pinMode(34,INPUT); // Pulldown input with 110nf cap and a resistor.

  pinMode(23, OUTPUT);
  digitalWrite(23,LOW); // 23 is GND makes a PULLDOWN resistor config.

  // Enable relay
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);



  #pragma endregion gpio



emergencyTrigger = millis();

  asynctimer.setInterval([]() {
     updateTime();
    }, 1000);

  //This seciton updates the RFID readings in the case that master restarts, so that backend is always equal to front end.

rData.data = 0;
rData.origin = atticmaster;
rData.sensor = atticmaster;
esp_now_send(m_RFID1, (uint8_t *) &rData, sizeof(rData));
esp_now_send(m_RFID2, (uint8_t *) &rData, sizeof(rData));
esp_now_send(m_RFID3, (uint8_t *) &rData, sizeof(rData));
esp_now_send(m_RFID4, (uint8_t *) &rData, sizeof(rData));
rData.origin = masterserver;
rData.sensor = masterserver;


}

unsigned long rtime = millis();

void loop() {


if (millis() - ttime > 2000){ //Use this to print data regularly
  ttime = millis();
}

if(checkrfidstuck == true && (millis() - rtime > 4000)){
  rtime = millis();
  //Do a check every second to see if any of them are false.
  //After 5sec, reset the faulty ones again.
  int allreset = 0;
  WebSerial.println("Check if RFID STUCK Run");
  WebSerial.printf("%d,%d,%d,%d\n", RFID1_reset,RFID2_reset,RFID3_reset,RFID4_reset);

  if(RFID1_reset) {allreset = allreset + 1;} else {esp_now_send(m_RFID1, (uint8_t *) &sData, sizeof(sData));}
  
  if(RFID2_reset) {allreset = allreset + 1;} else {esp_now_send(m_RFID2, (uint8_t *) &sData, sizeof(sData));}
  if(RFID3_reset) {allreset = allreset + 1;} else {esp_now_send(m_RFID3, (uint8_t *) &sData, sizeof(sData));}
  if(RFID4_reset) {allreset = allreset + 1;} else {esp_now_send(m_RFID4, (uint8_t *) &sData, sizeof(sData));}

  // I might have to restart the ATTIC too, if there ends up being a discrepancy...

  

  if(allreset == 4) checkrfidstuck = false;
}

//Emergency Escape Button
if ((analogRead(34) > 3500) && ((millis() - emergencyTrigger) >= 1000))
  {
  WebSerial.println(analogRead(34));
  emergencyTrigger = millis();
  WebSerial.println("Door Triggered");
  triggerDoor(5);
  delay(50);
}

if (keypadtrigger == 1) // Unlock the Door Steady State
  {
  WebSerial.println("Keypad Unlocked");
  keypadtrigger = 0;
  digitalWrite(5, HIGH);
}

if (keypadtrigger == 2) // Lock the Door Steady State
  {
  WebSerial.println("Keypad Locked");
  keypadtrigger = 0;
  digitalWrite(5, LOW);
}


  asynctimer.handle();
}