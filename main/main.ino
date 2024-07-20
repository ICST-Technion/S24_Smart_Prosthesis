#ifndef MAIN
#define MAIN
#include "wifi_communication.ino"
//#include "BLE_communication.ino"
#include "yaml_to_json_parser.ino"

Hand* hand;

void setup() {
  Serial.begin(115200);
  delay(10000);
  bring_up_wifi_server();
  //init_BLE();
  hand = new Hand();
}

void loop() {
  digitalWrite(2, millis() / 1000 % 2 == 0 ? HIGH : LOW);
  server.handleClient();
  if (configs_waiting)
  {
    yaml_to_json(yaml_configs.c_str());
    configs_waiting = false;
  }
  if (command_received) {
    int id = commandPayload[0];
    Sensor* sensor = (Sensor*)(hand->get_input_by_id(id));
    if (sensor) {
      sensor->func_of_input_obj.execute_func(commandPayload);
    } else {
      Serial.print("Didn't find sensor by id: ");
      Serial.println(id);
    }
    command_received = false;
  }
}

#endif /* MAIN */