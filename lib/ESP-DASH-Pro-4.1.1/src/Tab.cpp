#include "Tab.h"

Tab::Tab(ESPDash *dashboard, const char* title) {
  _dashboard = dashboard;
  #if defined(ESP8266)
    _id = RANDOM_REG32;
  #elif defined(ESP32)
    _id = esp_random();
  #endif
  strncpy(_title, title, sizeof(_title));
  _dashboard->add(this);
}

void Tab::update(const char* title) {
  strncpy(_title, title, sizeof(_title));
  _changed = true;
}

Tab::~Tab() {
  _dashboard->remove(this);
}