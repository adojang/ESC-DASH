/*
____ ____ ___  ___  ____ ____ _  _ 
|___ [__  |__] |  \ |__| [__  |__| 
|___ ___] |    |__/ |  | ___] |  | 
                                   
ESP-DASH V3
---------------------
Author: Ayush Sharma
First Commit: Nov 5, 2017
Github URL: https://github.com/ayushsharma82/ESP-DASH

*/

#ifndef ESPDashPro_h
#define ESPDashPro_h

#ifndef USE_DASH_LOGO_GZIPPED
    #define USE_DASH_LOGO_GZIPPED 1
#endif

#include <functional>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Arduino.h"
#include "stdlib_noniso.h"
#include "logo.h"
#include "dash_webpage.h"
#include "vector.h"

#if defined(ESP8266)
    #define DASH_HARDWARE "ESP8266"
    #include "ESP8266WiFi.h"
    #include "ESPAsyncTCP.h"
#elif defined(ESP32)
    #define DASH_HARDWARE "ESP32"
    #include "WiFi.h"
    #include "AsyncTCP.h"
#endif

#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"
#include "Tab.h"
#include "Card.h"
#include "Chart.h"
#include "Statistic.h"

#ifndef DASH_LAYOUT_JSON_SIZE
  #define DASH_LAYOUT_JSON_SIZE 1024 * 5
#endif

#ifndef DASH_STATISTICS_JSON_SIZE
  #define DASH_STATISTICS_JSON_SIZE 1024 * 1
#endif

#ifndef DASH_CARD_JSON_SIZE
  #define DASH_CARD_JSON_SIZE 512
#endif

#ifndef DASH_CHART_JSON_SIZE
  #define DASH_CHART_JSON_SIZE 2048
#endif

#ifndef DASH_USE_LEGACY_CHART_STORAGE
  #define DASH_USE_LEGACY_CHART_STORAGE 0
#endif

// Forward Declaration
class Tab;
class Card;
class Chart;
class Statistic;

// ESPDASH Class
class ESPDash{
  private:
    AsyncWebServer* _server = nullptr;
    AsyncWebSocket* _ws = nullptr;

    char _title[32] = "DASH Pro";
    Vector<Tab*> tabs;
    Vector<Card*> cards;
    Vector<Chart*> charts;
    Vector<Statistic*> statistics;
    bool default_stats_enabled = false;
    bool basic_auth = false;
    const char *username;
    const char *password;

    // Generate layout json
    size_t generateLayoutJSON(AsyncWebSocketClient *client, bool changes_only = false);

    // Generate Component JSON
    void generateComponentJSON(JsonObject& obj, Card* card, bool change_only = false);
    void generateComponentJSON(JsonObject& obj, Chart* chart, bool change_only = false);

    // This method is called when a card/chart is added or removed
    void refreshLayout();

  public:

    ESPDash(AsyncWebServer* server, bool enable_default_stats = true);

    // Set Authentication
    void setAuthentication(const char *user, const char *pass);

    void setTitle(const char *title);

    // Add Card
    void add(Card *card);
    // Remove Card
    void remove(Card *card);

    // Add Chart
    void add(Chart *card);
    // Remove Chart
    void remove(Chart *card);

    // Add Statistic
    void add(Statistic *statistic);
    // Remove Statistic
    void remove(Statistic *statistic);

    // Add Tab
    void add(Tab *tab);
    // Remove Tab
    void remove(Tab *tab);

    // Notify client side to update values
    void sendUpdates(bool force = false);

    void refreshStatistics();
    
    ~ESPDash();
};

#endif
