
//Control
#define masterserver 0x00
#define trainmaster 0x01
#define tombmaster 0x02
#define atticmaster 0x03

//Attic
#define attic_humanchain 0xA0
#define attic_bike 0xA1
#define attic_grandfatherclock 0xA2
#define attic_overrideButton 0xA3
#define attic_RFID1 0xA4
#define attic_RFID2 0xA5
#define attic_RFID3 0xA6
#define attic_RFID4 0xA7

//Tomb
#define tomb_sennet 0xB0
#define tomb_chalice 0xB1
#define tomb_ringReader 0xB2
#define tomb_tangrum 0xB3
#define tomb_maindoorOverride 0xA4
#define tomb_slidedoorOverride 0xA5

//Train
#define train_keypad 0xC0
#define train_thumb   0xC1
#define train_overrideButton 0xC2

//Template
#define TEMPLATE 0xEE


//Relay Control
#define doortime 5000 
#define door1 5 //GPIO Pins for Relays
#define door2 18
#define door3 19
#define door4 21

//Standard Send/Recieve Data Structure

typedef struct dataPacket {
int origin;
int sensor;
int data;
} dataPacket;