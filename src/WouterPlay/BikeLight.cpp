#include <Arduino.h>
#if defined(ESP8266)
  /* ESP8266 Dependencies */
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#elif defined(ESP32)
  /* ESP32 Dependencies */
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#endif
#include <ESPDash.h>

#include <Adafruit_NeoPixel.h>

#define PIN_WS2812B 13  // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 64   // The number of LEDs (pixels) on WS2812B LED strip

Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);
/* Your WiFi Credentials */
const char* ssid = "ESC_Rooms"; // SSID
const char* password = "harryhoudini"; // Password

/* Start Webserver */
AsyncWebServer server(80);

/* Attach ESP-DASH to AsyncWebServer */
ESPDash dashboard(&server); 

/* 
  Button Card
  Format - (Dashboard Instance, Card Type, Card Name)
*/
Card button(&dashboard, BUTTON_CARD, "Test Button");

int State = 0;

void setup() {
  Serial.begin(115200);
  ws2812b.begin();  // initialize WS2812B strip object (REQUIRED)
  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  /* Attach Button Callback */
  button.attachCallback([&](bool value){
    /* Print our new button value received from dashboard */
    Serial.println("Button Triggered: "+String((value)?"true":"false"));
    /* Make sure we update our button's value and send update to dashboard */
    button.update(value);
    State = (int)value;
    dashboard.sendUpdates();
  });
  /* Start AsyncWebServer */
  server.begin();
}

void loop() {
  // turn pixels to green one-by-one with delay between each pixel
  if(State == 1)
  {
  for (int pixel = 0; pixel < NUM_PIXELS; pixel++) {         // for each pixel
    ws2812b.setPixelColor(pixel, ws2812b.Color(255, 255, 51));  // it only takes effect if pixels.show() is called
  }
  ws2812b.show();
  }else
  {
  ws2812b.clear();
  ws2812b.show();
  }
}