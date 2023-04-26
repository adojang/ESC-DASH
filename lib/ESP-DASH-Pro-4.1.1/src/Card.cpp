#include "Card.h"

/*
  Constructor
*/
Card::Card(ESPDash *dashboard, const int type, const char* name, const char* symbol, const int min, const int max){
  _dashboard = dashboard;
  #if defined(ESP8266)
    _id = RANDOM_REG32;
  #elif defined(ESP32)
    _id = esp_random();
  #endif
  _type = type;
  _name = name;
  _symbol = symbol;
  _value_min = min;
  _value_max = max;
  _dashboard->add(this);
}

void Card::setIndex(const uint8_t index){
  _index = index;
  _index_changed = true;
}

/*
  Attach Function Callback
*/
void Card::attachCallback(std::function<void(int value)> cb){
  _callback = cb;
}


/*
  Attach Function Callback for String
*/
void Card::attachCallback(std::function<void(const char*)> cb){
  _callback_str = cb;
}


/*
  Attach Function Callback for XY
*/
void Card::attachCallback(std::function<void(int8_t x, int8_t y)> cb){
  _callback_xy = cb;
}

/*
  Value update methods
*/
void Card::update(int value, const char* symbol){
  /* Clear String if it was used before */
  if(_value_type == Card::STRING){
    _value_s = "";
  }
  /* Store new value */
  _value_type = Card::INTEGER;
  _symbol = symbol;
  if(_value_i != value)
    _value_changed = true;
  _value_i = value;
}

void Card::update(int value){
  /* Clear String if it was used before */
  if(_value_type == Card::STRING){
    _value_s = "";
  }
  /* Store new value */
  _value_type = Card::INTEGER;
  if(_value_i != value)
    _value_changed = true;
  _value_i = value;
}

void Card::update(float value, const char* symbol){
  /* Clear String if it was used before */
  if(_value_type == Card::STRING){
    _value_s = "";
  }
  /* Store new value */
  _value_type = Card::FLOAT;
  _symbol = symbol;
  if(_value_f != value)
    _value_changed = true;
  _value_f = value;
}

void Card::update(float value){
  /* Clear String if it was used before */
  if(_value_type == Card::STRING){
    _value_s = "";
  }
  /* Store new value */
  _value_type = Card::FLOAT;
  if(_value_f != value)
    _value_changed = true;
  _value_f = value;
}

void Card::update(const String &value, const char* symbol){
  update(value.c_str(), symbol);
}

void Card::update(const String &value){
  update(value.c_str());
}

void Card::update(const char* value, const char* symbol){
  if(_value_type == Card::STRING){
    if(strcmp(_value_s.c_str(), value) != 0)
      _value_changed = true;
  }
  
  _value_type = Card::STRING;
  _symbol = symbol;
  _value_s = value;
}

void Card::update(const char* value){
    if(_value_type == Card::STRING){
    if(strcmp(_value_s.c_str(), value) != 0)
      _value_changed = true;
  }
  
  _value_type = Card::STRING;
  _value_s = value;
}

void Card::update(bool value, const char* symbol){
  /* Clear String if it was used before */
  if(_value_type == Card::STRING){
    _value_s = "";
  }
  /* Store new value */
  _value_type = Card::INTEGER;
  _symbol = symbol;
  if(_value_i != value)
    _value_changed = true;
  _value_i = value;
}

void Card::update(bool value){
  /* Clear String if it was used before */
  if(_value_type == Card::STRING){
    _value_s = "";
  }
  /* Store new value */
  _value_type = Card::INTEGER;
  if(_value_i != value)
    _value_changed = true;
  _value_i = value;
}

// Size functions
void Card::setSize(const uint8_t xs, const uint8_t sm, const uint8_t md, const uint8_t lg, const uint8_t xl, const uint8_t xxl){
  _size.xs = xs;
  _size.sm = sm;
  _size.md = md;
  _size.lg = lg;
  _size.xl = xl;
  _size.xxl = xxl;
  _size_changed = true;
}

void Card::setSize(const CardSize &size){
  _size = size;
  _size_changed = true;
}

void Card::setTab(Tab *tab){
  _tab = tab;
  _tab_changed = true;
}

void Card::removeTab(){
  _tab = nullptr;
  _tab_changed = true;
}

/*
  Destructor
*/
Card::~Card(){
  _dashboard->remove(this);
}
