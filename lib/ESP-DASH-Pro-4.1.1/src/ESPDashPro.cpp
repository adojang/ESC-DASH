#include "ESPDashPro.h"

// Integral type to string pairs events
// ID, type
struct CardNames cardTags[] = {
  {GENERIC_CARD, "generic"},
  {TEMPERATURE_CARD, "temperature"},
  {HUMIDITY_CARD, "humidity"},
  {STATUS_CARD, "status"},
  {SLIDER_CARD, "slider"},
  {BUTTON_CARD, "button"},
  {PROGRESS_CARD, "progress"},
  {AIR_CARD, "air"},
  {ENERGY_CARD, "energy"},
  {TEXT_INPUT_CARD, "textinput"},
  {JOYSTICK_CARD, "joystick"}
};

// Integral type to string pairs events
// ID, type
struct ChartNames chartTags[] = {
  {BAR_CHART, "bar"},
  {LINE_CHART, "line"},
  {AREA_CHART, "area"},
  {PIE_CHART, "pie"}
};


/*
  Constructor
*/
ESPDash::ESPDash(AsyncWebServer* server, bool enable_default_stats) {
  _server = server;
  default_stats_enabled = enable_default_stats;

  // Initialize AsyncWebSocket
  _ws = new AsyncWebSocket("/dashws");

  // Attach AsyncWebServer Routes
  _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
    if(basic_auth){
      if(!request->authenticate(username, password))
      return request->requestAuthentication();
    }
    // respond with the compressed frontend
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", DASH_HTML, sizeof(DASH_HTML));
    response->addHeader("Content-Encoding","gzip");
    response->addHeader("Cache-Control","public, max-age=900");
    request->send(response);        
  });

  _server->on("/dash/assets/logo/mini", HTTP_GET, [this](AsyncWebServerRequest *request){
    // respond with the compressed frontend
    AsyncWebServerResponse *response = request->beginResponse_P(200, DASH_MINI_LOGO_MIME, DASH_MINI_LOGO, sizeof(DASH_MINI_LOGO));
    #if DASH_MINI_LOGO_GZIPPED == 1
      response->addHeader("Content-Encoding","gzip");
    #endif
    response->addHeader("Cache-Control","public, max-age=900");
    request->send(response);
  });

  _server->on("/dash/assets/logo/large", HTTP_GET, [this](AsyncWebServerRequest *request){
    // respond with the compressed frontend
    AsyncWebServerResponse *response = request->beginResponse_P(200, DASH_LARGE_LOGO_MIME, DASH_LARGE_LOGO, sizeof(DASH_LARGE_LOGO));
    #if DASH_LARGE_LOGO_GZIPPED == 1
      response->addHeader("Content-Encoding","gzip");
    #endif
    response->addHeader("Cache-Control","public, max-age=900");
    request->send(response);        
  });

  // Websocket Callback Handler
  _ws->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
    // Request Buffer
    StaticJsonDocument<200> json;

    if (type == WS_EVT_DATA) {
      AwsFrameInfo * info = (AwsFrameInfo * ) arg;
      if (info -> final && info -> index == 0 && info -> len == len) {
        if (info -> opcode == WS_TEXT) {
          data[len] = 0;
          deserializeJson(json, reinterpret_cast<const char*>(data));
          // client side commands parsing
          if (json["command"] == "get:layout") {
            Serial.println("Got get:layout");
            generateLayoutJSON(client, false);
          } else if (json["command"] == "ping") {
            return _ws->text(client->id(), "{\"command\":\"pong\"}");
          } else if (json["command"] == "get:stats") {
            generateLayoutJSON(client, true);
          } else if (json["command"] == "button:clicked") {
            // execute and reference card data struct to funtion
            uint32_t id = json["id"].as<uint32_t>();
            for(int i=0; i < cards.Size(); i++){
              Card *p = cards[i];
              if(id == p->_id){
                if(p->_callback != nullptr){
                  p->_callback(json["value"].as<int>());
                }
              }
            }
          } else if (json["command"] == "slider:changed") {
            // execute and reference card data struct to funtion
            uint32_t id = json["id"].as<uint32_t>();
            for(int i=0; i < cards.Size(); i++){
              Card *p = cards[i];
              if(id == p->_id){
                if(p->_callback != nullptr){
                  p->_callback(json["value"].as<int>());
                }
              }
            }
          } else if (json["command"] == "textinput:changed") {
            uint32_t id = json["id"].as<uint32_t>();
            for(int i=0; i < cards.Size(); i++){
              Card *p = cards[i];
              if(id == p->_id){
                if(p->_callback_str != nullptr){
                  p->_callback_str(json["value"].as<const char*>());
                }
              }
            }
          } else if (json["command"] == "joystick:changed") {
            uint32_t id = json["id"].as<uint32_t>();
            for(int i=0; i < cards.Size(); i++){
              Card *p = cards[i];
              if(id == p->_id){
                if(p->_callback_str != nullptr){
                  p->_callback_str(json["direction"].as<const char*>());
                }
                if(p->_callback_xy != nullptr){
                  p->_callback_xy(json["x"].as<int8_t>(), json["y"].as<int8_t>());
                }
              }
            }
          }
        }
      }
    }
  });

  // Attach Websocket Instance to AsyncWebServer
  _server->addHandler(_ws);
}


void ESPDash::setAuthentication(const char *user, const char *pass) {
  username = user;
  password = pass;
  basic_auth = true;
  _ws->setAuthentication(user, pass);
}

void ESPDash::setTitle(const char *title) {
  strncpy(_title, title, sizeof(_title));
  refreshLayout();
}

// Add Card
void ESPDash::add(Card *card) {
  cards.PushBack(card);
  refreshLayout();
}

// Remove Card
void ESPDash::remove(Card *card) {
  for(int i=0; i < cards.Size(); i++){
    Card *p = cards[i];
    if(p->_id == card->_id){
      cards.Erase(i);
      refreshLayout();
      return;
    }
  }
}


// Add Chart
void ESPDash::add(Chart *chart) {
  charts.PushBack(chart);
  refreshLayout();
}

// Remove Card
void ESPDash::remove(Chart *chart) {
  for(int i=0; i < charts.Size(); i++){
    Chart *p = charts[i];
    if(p->_id == chart->_id){
      charts.Erase(i);
      refreshLayout();
      return;
    }
  }
}

// Add Statistic
void ESPDash::add(Statistic *statistic) {
  statistics.PushBack(statistic);
  refreshStatistics();
}

// Remove Statistic
void ESPDash::remove(Statistic *statistic) {
  for(int i=0; i < statistics.Size(); i++){
    Statistic *p = statistics[i];
    if(p->_id == statistic->_id){
      statistics.Erase(i);
      refreshStatistics();
      return;
    }
  }
}

// Add Tab
void ESPDash::add(Tab *tab) {
  tabs.PushBack(tab);
  refreshLayout();
}

void ESPDash::remove(Tab *tab) {
  for(int i=0; i < tabs.Size(); i++){
    Tab *p = tabs[i];
    if(p->_id == tab->_id){
      // Loop through all cards and remove tab
      for(int j=0; j < cards.Size(); j++){
        Card *card = cards[j];
        if (card->_tab != nullptr) {
          if (card->_tab->_id == tab->_id) {
            card->_tab = nullptr;
          }
        }
      }
      // Loop through all charts and remove tab
      for(int j=0; j < charts.Size(); j++){
        Chart *chart = charts[j];
        if (chart->_tab != nullptr) {
          if (chart->_tab->_id == tab->_id) {
            chart->_tab = nullptr;
          }
        }
      }
      tabs.Erase(i);
      refreshLayout();
      return;
    }
  }
}

// generates the layout JSON string to the frontend
size_t ESPDash::generateLayoutJSON(AsyncWebSocketClient *client, bool changes_only) {
  String buf = "";
  buf.reserve(DASH_LAYOUT_JSON_SIZE);

  if (changes_only) {
    buf += "{\"command\":\"update:components\",";
  } else {
    buf += "{\"command\":\"update:layout\",";
    buf += "\"title\":\"" + String(_title) + "\",";
    buf += "\"opts\": {\"ml\":{\"w\":" + String(DASH_MINI_LOGO_WIDTH) +",\"h\":" + String(DASH_MINI_LOGO_HEIGHT) + "},\"fl\":{\"w\":" + String(DASH_LARGE_LOGO_WIDTH) + ",\"h\":" + String(DASH_LARGE_LOGO_HEIGHT) +"}},";
  }

  buf += "\"tabs\":[";

  // Generate Tab JSON
  StaticJsonDocument<128> tabdoc;
  uint8_t tab_count = 0;
  for (int i=0; i < tabs.Size(); i++) {
    Tab* t = tabs[i];
    if (changes_only) {
      if (t->_changed) {
        t->_changed = false;
      } else {
        continue;
      }
    }
    if (tab_count > 0) {
      buf += ",";
    }
    tabdoc["id"] = t->_id;
    tabdoc["title"] = t->_title;
    serializeJson(tabdoc, buf);
    tabdoc.clear();
    tab_count++;
  }
  
  buf += "],\"cards\":[";

  StaticJsonDocument<DASH_CARD_JSON_SIZE> carddoc;
  uint8_t card_count = 0;
  // Generate JSON for all Cards
  for (int i=0; i < cards.Size(); i++) {
    Card *c = cards[i];
    if (changes_only) {
      if (!c->_value_changed && !c->_tab_changed && !c-> _size_changed && !c-> _index_changed) {
        continue;
      }
    }
    if (card_count > 0) {
      buf += ",";
    }
    // Generate JSON
    JsonObject obj = carddoc.to<JsonObject>();
    generateComponentJSON(obj, c, changes_only);
    // Append to buf
    serializeJson(carddoc, buf);
    carddoc.clear();
    // Clear change flags
    if (changes_only) {
      c->_value_changed = false;
      c->_tab_changed = false;
      c-> _size_changed = false;
      c-> _index_changed = false;
    }
    card_count++;
  }

  buf += "],\"charts\":[";

  DynamicJsonDocument chartdoc(DASH_CHART_JSON_SIZE);
  uint8_t chart_count = 0;
  // Generate JSON for all Charts
  for (int i=0; i < charts.Size(); i++) {
    Chart *c = charts[i];
    if (changes_only) {
      if (!c->_value_changed && !c->_tab_changed && !c-> _size_changed && !c-> _index_changed) {
        continue;
      }
    }
    if (chart_count > 0) {
      buf += ",";
    }
    // Generate JSON
    JsonObject obj = chartdoc.to<JsonObject>();
    generateComponentJSON(obj, c, changes_only);
    // Append to buf
    serializeJson(chartdoc, buf);
    chartdoc.clear();
    // Clear change flags
    if (changes_only) {
      c->_value_changed = false;
      c->_tab_changed = false;
      c-> _size_changed = false;
      c-> _index_changed = false;
    }
    chart_count++;
  }

  buf += "]";

  if (!changes_only) {
    buf += ",\"stats\": [";
    // Generate JSON for all Statistics
    // Check if default statistics are needed
    if (default_stats_enabled) {
      // Hardware
      StaticJsonDocument<64> hardware;
      hardware["k"] = "Hardware";
      hardware["v"] = DASH_HARDWARE;
      serializeJson(hardware, buf);
      buf += ",";

      // SDK Version
      StaticJsonDocument<64> sdk;
      sdk["k"] = "SDK Version";
      #if defined(ESP8266)
        sdk["v"] = ESP.getCoreVersion();
      #elif defined(ESP32)
        sdk["v"] = String(esp_get_idf_version());
      #endif
      serializeJson(sdk, buf);
      buf += ",";

      // MAC Address
      StaticJsonDocument<64> mac;
      mac["k"] = "MAC Address";
      mac["v"] = WiFi.macAddress();
      serializeJson(mac, buf);
      buf += ",";

      // Free Heap
      StaticJsonDocument<64> heap;
      heap["k"] = "Free Heap (SRAM)";
      heap["v"] = ESP.getFreeHeap();
      serializeJson(heap, buf);
      buf += ",";

      // WiFi Mode
      StaticJsonDocument<64> wmode;
      wmode["k"] = "WiFi Mode";
      wmode["v"] = WiFi.getMode();
      serializeJson(wmode, buf);
      buf += ",";

      // WiFi Signal
      StaticJsonDocument<64> signal;
      signal["k"] = "WiFi Signal";
      signal["v"] = WiFi.RSSI();
      serializeJson(signal, buf);
    }

    // Loop through user defined stats
    StaticJsonDocument<128> obj;
    for (int i=0; i < statistics.Size(); i++) {
      Statistic *s = statistics[i];
      if (changes_only) {
        if (s->_changed) {
          s->_changed = false;
        } else {
          continue;
        }
      }

      if (default_stats_enabled || i > 0) {
        buf += ",";
      }

      obj["k"] = s->_key;
      obj["v"] = s->_value;
      serializeJson(obj, buf);
      obj.clear();
    }
    buf += "]";
  }

  // Close JSON
  buf += "}";

  // Store the length of the JSON string
  size_t total = buf.length();
  // Send resp
  if (client != nullptr) {
    _ws->text(client->id(), buf.c_str(), total);
  } else {
    _ws->textAll(buf.c_str(), total);
  }

  // Serial.println("Free Heap (During Update): "+String( ESP.getFreeHeap() ));
  // Return length
  return total;
}


/*
  Generate Card JSON
*/
void ESPDash::generateComponentJSON(JsonObject& doc, Card* card, bool change_only){
  doc["id"] = card->_id;

  if (!change_only || card->_tab_changed) {
    if (card->_tab != nullptr) {
      doc["tab"] = card->_tab->_id;
    }
    if (change_only) {
      card->_tab_changed = false;
    }
  }

  if (!change_only || card->_index_changed){
    doc["index"] = card->_index;
    if (change_only) {
      card->_index_changed = false;
    }
  }

  if(!change_only || card->_size_changed) {
    JsonObject size = doc.createNestedObject("size");
    size["xs"] = card->_size.xs;
    size["sm"] = card->_size.sm;
    size["md"] = card->_size.md;
    size["lg"] = card->_size.lg;
    size["xl"] = card->_size.xl;
    size["xxl"] = card->_size.xxl;
    if (change_only) {
      card->_size_changed = false;
    }
  }

  if(!change_only){
    doc["name"] = card->_name;
    doc["type"] = cardTags[card->_type].type;
    doc["value_min"] = card->_value_min;
    doc["value_max"] = card->_value_max;
  }
  doc["symbol"] = card->_symbol;

  switch (card->_value_type) {
    case Card::INTEGER:
      doc["value"] = card->_value_i;
      break;
    case Card::FLOAT:
      doc["value"] = String(card->_value_f, 2);
      break;
    case Card::STRING:
      doc["value"] = card->_value_s;
      break;
    default:
      // blank value
      break;
  }
}


/*
  Generate Chart JSON
*/
void ESPDash::generateComponentJSON(JsonObject& doc, Chart* chart, bool change_only){
  doc["id"] = chart->_id;

  if (!change_only || chart->_tab_changed) {
    if (chart->_tab != nullptr) {
      doc["tab"] = chart->_tab->_id;
    }
    if (change_only) {
      chart->_tab_changed = false;
    }
  }

  if (!change_only || chart->_index_changed){
    doc["index"] = chart->_index;
    if (change_only) {
      chart->_index_changed = false;
    }
  }

  if(!change_only || chart->_size_changed) {
    JsonObject size = doc.createNestedObject("size");
    size["xs"] = chart->_size.xs;
    size["sm"] = chart->_size.sm;
    size["md"] = chart->_size.md;
    size["lg"] = chart->_size.lg;
    size["xl"] = chart->_size.xl;
    size["xxl"] = chart->_size.xxl;
    if (change_only) {
      chart->_size_changed = false;
    }
  }

  if(!change_only){
    doc["name"] = chart->_name;
    doc["type"] = chartTags[chart->_type].type;
  }

  JsonArray xAxis = doc["x_axis"].to<JsonArray>();
  switch (chart->_x_axis_type) {
    case GraphAxisType::INTEGER:
      #if DASH_USE_LEGACY_CHART_STORAGE == 1
        for(int i=0; i < chart->_x_axis_i.Size(); i++)
          xAxis.add(chart->_x_axis_i[i]);
      #else
        if (chart->_x_axis_i_ptr != nullptr) {
          for(unsigned int i=0; i < chart->_x_axis_ptr_size; i++)
            xAxis.add(chart->_x_axis_i_ptr[i]);
        }
      #endif
      break;
    case GraphAxisType::FLOAT:
      #if DASH_USE_LEGACY_CHART_STORAGE == 1
        for(int i=0; i < chart->_x_axis_f.Size(); i++)
          xAxis.add(chart->_x_axis_f[i]);
      #else
        if (chart->_x_axis_f_ptr != nullptr) {
          for(unsigned int i=0; i < chart->_x_axis_ptr_size; i++)
            xAxis.add(chart->_x_axis_f_ptr[i]);
        }
      #endif
      break;
    case GraphAxisType::CHAR:
      #if DASH_USE_LEGACY_CHART_STORAGE == 1
        for(int i=0; i < chart->_x_axis_s.Size(); i++)
          xAxis.add(chart->_x_axis_s[i].c_str());
      #else
        if (chart->_x_axis_char_ptr != nullptr) {
          for(unsigned int i=0; i < chart->_x_axis_ptr_size; i++)
            xAxis.add(chart->_x_axis_char_ptr[i]);
        }
      #endif
      break;
    case GraphAxisType::STRING:
      #if DASH_USE_LEGACY_CHART_STORAGE == 1
        for(int i=0; i < chart->_x_axis_s.Size(); i++)
          xAxis.add(chart->_x_axis_s[i].c_str());
      #else
        if (chart->_x_axis_s_ptr != nullptr) {
          for(unsigned int i=0; i < chart->_x_axis_ptr_size; i++)
            xAxis.add(chart->_x_axis_s_ptr[i]);
        }
      #endif
      break;
    default:
      // blank value
      break;
  }
  
  JsonArray yAxis = doc["y_axis"].to<JsonArray>();
  switch (chart->_y_axis_type) {
    case GraphAxisType::INTEGER:
      #if DASH_USE_LEGACY_CHART_STORAGE == 1
        for(int i=0; i < chart->_y_axis_i.Size(); i++)
          yAxis.add(chart->_y_axis_i[i]);
      #else
        if (chart->_y_axis_i_ptr != nullptr) {
          for(unsigned int i=0; i < chart->_y_axis_ptr_size; i++)
            yAxis.add(chart->_y_axis_i_ptr[i]);
        }
      #endif
      break;
    case GraphAxisType::FLOAT:
      #if DASH_USE_LEGACY_CHART_STORAGE == 1
        for(int i=0; i < chart->_y_axis_f.Size(); i++)
          yAxis.add(chart->_y_axis_f[i]);
      #else
        if (chart->_y_axis_f_ptr != nullptr) {
          for(unsigned int i=0; i < chart->_y_axis_ptr_size; i++)
            yAxis.add(chart->_y_axis_f_ptr[i]);
        }
      #endif
      break;
    default:
      // blank value
      break;
  }
}

/* Send Card Updates to all clients */
void ESPDash::sendUpdates(bool force) {
  generateLayoutJSON(nullptr, !force);
}

void ESPDash::refreshLayout() {
  _ws->textAll("{\"command\":\"refreshLayout\"}");
}

void ESPDash::refreshStatistics() {
  generateLayoutJSON(nullptr, true);
}


/*
  Destructor
*/
ESPDash::~ESPDash(){
  _server->removeHandler(_ws);
  delete _ws;
}