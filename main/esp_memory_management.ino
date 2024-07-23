#ifndef ESP_MEMORY_MANAGEMENT
#define ESP_MEMORY_MANAGEMENT

#include <Preferences.h>

extern String yaml_configs;
extern Preferences preference;

void load_configs(){
  preference.begin("saved_configs", false);
  yaml_configs = preference.getString("yaml_configs", "empty");
  if(yaml_configs != "empty"){
    Serial.println("found configurations");
    Serial.println(yaml_configs);
  } else {
    Serial.println("didn't find old configurations");
  }

  preference.end();
}

void store_configs(){
  preference.begin("saved_configs", false);
  preference.putString("yaml_configs", yaml_configs);
  preference.end();
}

#endif


