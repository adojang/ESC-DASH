/*
  -----------------------------
  ESPDASH Pro - Benchmark Example
  -----------------------------
  Use this benchmark example to test if ESP-DASH Pro is working properly on your platform.

  Github: https://github.com/ayushsharma82/ESP-DASH
  WiKi: https://docs.espdash.pro

  Works with both ESP8266 & ESP32
  -------------------------------
*/

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

#include <ESPDashPro.h>


/* Your WiFi Credentials */
const char* ssid = ""; // SSID
const char* password = ""; // Password

/* Start Webserver */
AsyncWebServer server(80);

/* Attach ESP-DASH to AsyncWebServer */
ESPDash dashboard(&server, true);

// Custom Tabs
Tab settings(&dashboard, "Settings");

// Cards
Card generic(&dashboard, GENERIC_CARD, "Generic");
Card temp(&dashboard, TEMPERATURE_CARD, "Temperature", "°C");
Card hum(&dashboard, HUMIDITY_CARD, "Humidity", "%");
Card air(&dashboard, AIR_CARD, "Air", "hPa");
Card energy(&dashboard, ENERGY_CARD, "Energy", "%");
Card status(&dashboard, STATUS_CARD, "Status", "success");
Card progress(&dashboard, PROGRESS_CARD, "Progress", "%", 0, 100);
Card button(&dashboard, BUTTON_CARD, "Test Button");
Card slider(&dashboard, SLIDER_CARD, "Test Slider", "", 0, 255);
Card text(&dashboard, TEXT_INPUT_CARD, "Test Text Input");
Card joystick(&dashboard, JOYSTICK_CARD, "Test Joystick");
Card joystickX(&dashboard, JOYSTICK_CARD, "Joystick X", "lockX");
Card joystickY(&dashboard, JOYSTICK_CARD, "Joystick Y", "lockY");

// Charts
Chart bar(&dashboard, BAR_CHART, "Power Usage (kWh)");
Chart line(&dashboard, LINE_CHART, "Water Usage (L)");
Chart area(&dashboard, AREA_CHART, "Gas Usage (m3)");
Chart pie(&dashboard, PIE_CHART, "Pie Chart");

// Custom Statistics
Statistic stat1(&dashboard, "Statistic 1", "Value 1");
Statistic stat2(&dashboard, "Statistic 2", "Value 2");\

uint8_t test_status = 0;


/**
 * Note how we are keeping all the chart data in global scope.
*/
// Bar Chart Data
const char* BarXAxis[] = {"1/4/22", "2/4/22", "3/4/22", "4/4/22", "5/4/22", "6/4/22", "7/4/22", "8/4/22", "9/4/22", "10/4/22", "11/4/22", "12/4/22", "13/4/22", "14/4/22", "15/4/22", "16/4/22", "17/4/22", "18/4/22", "19/4/22", "20/4/22", "21/4/22", "22/4/22", "23/4/22", "24/4/22", "25/4/22", "26/4/22", "27/4/22", "28/4/22", "29/4/22", "30/4/22"};
int BarYAxis[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Line Chart
const char* LineXAxis[] = {"1/4/22", "2/4/22", "3/4/22", "4/4/22", "5/4/22", "6/4/22", "7/4/22", "8/4/22", "9/4/22", "10/4/22", "11/4/22", "12/4/22", "13/4/22", "14/4/22", "15/4/22", "16/4/22", "17/4/22", "18/4/22", "19/4/22", "20/4/22", "21/4/22", "22/4/22", "23/4/22", "24/4/22", "25/4/22", "26/4/22", "27/4/22", "28/4/22", "29/4/22", "30/4/22"};
int LineYAxis[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Area Chart
const char* AreaXAxis[] = {"1/4/22", "2/4/22", "3/4/22", "4/4/22", "5/4/22", "6/4/22", "7/4/22", "8/4/22", "9/4/22", "10/4/22", "11/4/22", "12/4/22", "13/4/22", "14/4/22", "15/4/22", "16/4/22", "17/4/22", "18/4/22", "19/4/22", "20/4/22", "21/4/22", "22/4/22", "23/4/22", "24/4/22", "25/4/22", "26/4/22", "27/4/22", "28/4/22", "29/4/22", "30/4/22"};
int AreaYAxis[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const char* PieXAxis[] = {"Biscuits", "Cookies", "Milk", "Thing4", "Thing5"};
int PieYAxis[] = {0, 0, 0, 0, 0};


unsigned long last_update_millis = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(); 
  /* Connect WiFi */
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Custom Dashboard Title
  dashboard.setTitle("Your Custom Title Here");

  /* Attach Button Callback */
  button.setTab(&settings);
  button.attachCallback([&](int value){
    /* Print our new button value received from dashboard */
    Serial.println("Button Triggered: "+String((value)?"true":"false"));
    /* Make sure we update our button's value and send update to dashboard */
    button.update(value);
    dashboard.sendUpdates();
  });

  // Set Slider Index
  slider.setIndex(1);

  // Set Slider Tab
  slider.setTab(&settings);

  /* Attach Slider Callback */
  slider.attachCallback([&](int value){
    /* Print our new slider value received from dashboard */
    Serial.println("Slider Triggered: "+String(value));
    /* Make sure we update our slider's value and send update to dashboard */
    slider.update(value);
    dashboard.sendUpdates();
  });

  // Set default text on our Text Input Card
  text.update("Test Text");
  
  text.attachCallback([&](const char* value){
    Serial.println("[Text Card] Text Input Callback Triggered: "+String(value));
    text.update(value);
    dashboard.sendUpdates();
  });

  joystick.setIndex(60);
  joystick.attachCallback([&](int8_t x, int8_t y){
    Serial.printf("[Joystick] X Axis: %d, Y Axis: %d\n", x, y);
  });

  joystickX.setIndex(61);
  joystickX.attachCallback([&](int8_t x, int8_t y){
    Serial.printf("[Joystick X] X Axis: %d, Y Axis: %d\n", x, y);
  });

  joystickY.setIndex(62);
  joystickY.attachCallback([&](int8_t x, int8_t y){
    Serial.printf("[Joystick Y] X Axis: %d, Y Axis: %d\n", x, y);
  });

  /* Start AsyncWebServer */
  server.begin();

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
}

void loop() {
  // Update Everything every 2 seconds using millis if connected to WiFi
  if (WiFi.isConnected() && millis() - last_update_millis > 2000) {
    last_update_millis = millis();

    // Randomize Bar Chart YAxis Values ( for demonstration purposes only )
    for(int i=0; i < 30; i++){
      BarYAxis[i] = (int)random(0, 200);
    }

    // Randomize Line Chart YAxis Values ( for demonstration purposes only )
    for(int i=0; i < 30; i++){
      LineYAxis[i] = (int)random(0, 500);
    }

    // Randomize Area Chart YAxis Values ( for demonstration purposes only )
    for(int i=0; i < 30; i++){
      AreaYAxis[i] = (int)random(0, 100);
    }

    // Randomize Pie Chart YAxis Values ( for demonstration purposes only )
    for(int i=0; i < 5; i++){
      PieYAxis[i] = (int)random(0, 100);
    }

    /* Update Chart Y Axis (yaxis_array, array_size) */
    bar.updateX(BarXAxis, 30);
    bar.updateY(BarYAxis, 30);

    line.updateX(LineXAxis, 30);
    line.updateY(LineYAxis, 30);

    area.updateX(AreaXAxis, 30);
    area.updateY(AreaYAxis, 30);

    pie.updateX(PieXAxis, 5);
    pie.updateY(PieYAxis, 5);

    // Update all cards with random values
    generic.update((int)random(0, 100));
    temp.update((int)random(0, 100));
    hum.update((int)random(0, 100));
    air.update((int)random(0, 1000));
    energy.update((int)random(0, 100));
    progress.update((int)random(0, 100));

    // Loop through statuses
    if(test_status == 0){
      status.update("Success Msg!", "success");
      test_status = 1;
    }
    else if(test_status == 1){
      status.update("Warning Msg!", "warning");
      test_status = 2;
    }
    else if(test_status == 2){
      status.update("Danger Msg!", "danger");
      test_status = 3;
    }
    else if(test_status == 3){
      status.update("Idle Msg!", "idle");
      test_status = 0;
    }
    
    // Get Free Heap 
    Serial.println("Free Heap (Before Update): "+String( ESP.getFreeHeap() ));
    dashboard.sendUpdates();
    Serial.println("Free Heap (After Update): "+String( ESP.getFreeHeap() ));
  }
}