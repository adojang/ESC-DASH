#include <Arduino.h>

#include <EscCore.h>

#include <unity.h>





void setUp(void)
{
  //Functions run before each test 
}

void tearDown(void)
{
  // clean stuff up here
}

void EscCoreSetup(void)
{
  #define NAME "test"
  #define MACAD 0x00 // Refer to Table in Conventions
  AsyncWebServer server(80);
  uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD}; // Address of This Device
  EscCore Core;

  TEST_ASSERT(Core.startup(setMACAddress, NAME, server) == 0); 

}




void setup()
{
    delay(2000); // service delay
    UNITY_BEGIN();

    RUN_TEST(EscCoreSetup);


    UNITY_END(); // stop unit testing
}

void loop()
{
}