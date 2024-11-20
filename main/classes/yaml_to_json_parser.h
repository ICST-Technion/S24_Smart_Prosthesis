#ifndef YAML_TO_JSON_PARSER
#define YAML_TO_JSON_PARSER
#include <Arduino.h>
#include <ArduinoJson.h>
#include <YAMLDuino.h>
#include <string>
#include <vector>
#include "hand_functions.h"
#define STACK_SIZE 2048
extern Hand* hand;
extern TaskHandle_t hw_Management_Handle;
extern TaskHandle_t process_Logic_Handle;
extern volatile bool is_semaphore_being_deleted;
void HW_management(void* pvParameters);
void process_payload_and_manage_logic(void* pvParameters);
void store_configs();

/**
 * @brief Configures the system based on the provided JSON document.
 * 
 * This function parses the JSON document to extract sensor and output configurations. It creates and initializes `Sensor` and `DC_motor`
 * objects based on the configurations and adds them to the `Hand` object. It also prints the debug information of the `Hand` object.
 * 
 * @param doc A `JsonDocument` containing the system configuration in JSON format.
 */
void config_system(JsonDocument doc) {
  JsonArray input_list = doc["inputs"].as<JsonArray>();

  for (JsonVariant value : input_list) {
    const char *input_name = value["name"];
    Serial.print("input_name: ");
    Serial.println(input_name);
    int id = value["id"];
    Serial.print("id: ");
    Serial.println(id);
    const char *sensor_type = value["type"];
    Serial.print("sensor_type: ");
    Serial.println(sensor_type);
    const char *func_name = value["function"]["name"];
    Serial.print("func_name: ");
    Serial.println(func_name);
    JsonObject func_params_list = value["function"]["parameters"].as<JsonObject>();
    std::map<String, double> func_params;
    for (JsonPair param : func_params_list) {
      String param_name = param.key().c_str();
      double param_value = param.value();
      func_params[param_name] = param_value;
      Serial.print("prarm name: ");
      Serial.println(param_name);
      Serial.print("param value: ");
      Serial.println(param_value);
    }
    String str_func_name(func_name);
    FuncPtr sensor_func = func_map[str_func_name];
    if(sensor_func == NULL){
      Serial.print("error! coulndt find the pointer to function:");
      Serial.println(str_func_name);
    }
    Func_of_input func_of_input(sensor_func, func_params);
    Sensor *sensor = new Sensor(id, input_name, sensor_type, func_of_input);
    hand->add_input(sensor);
  }

  JsonArray output_list = doc["outputs"].as<JsonArray>();
  for (JsonVariant value : output_list) {
    const char *output_name = value["name"];
    const char *output_type = value["type"];
    if (strcmp(output_type, "DC_motor") == 0) {
      JsonArray pins_list = value["pins"].as<JsonArray>();
      int in1_pin;
      int in2_pin;
      int sense_pin;
      int safety_threshold;
      for (JsonVariant pin : pins_list) {
        const char *pin_type = pin["type"];
        if (strcmp(pin_type, "in1_pin") == 0) {
          in1_pin = pin["pin_number"];
        }
        if (strcmp(pin_type, "in2_pin") == 0) {
        in2_pin = pin["pin_number"];
        }
        if (strcmp(pin_type, "sense_pin") == 0) {
        sense_pin = pin["pin_number"];
        }
        safety_threshold = value["safety_threshold"];
      }
      DC_motor *dc_motor = new DC_motor(output_name, output_type, in1_pin, in2_pin, sense_pin,safety_threshold);
      hand->add_output(dc_motor);
    }
  }
  hand->debug_print();
}

/**
 * @brief Resets the system before applying new configurations.
 * 
 * This function stops all active DC motors, deletes any existing tasks related to hardware management and logic processing, and
 * deletes existing semaphores. It sets a flag indicating that semaphores are being deleted.
 */
void reset_before_new_configs(){
  for(Output* output : hand->outputs){
    if(output->type == "DC_motor"){
      DC_motor* motor_ptr = (DC_motor*)output;
      motor_ptr->set_state(STOP);
    } 
  }
  vTaskDelay(1000);    
  if (hw_Management_Handle != NULL) {
      vTaskDelete(hw_Management_Handle);
  }
  if (process_Logic_Handle != NULL) {
      vTaskDelete(process_Logic_Handle);
  }
  is_semaphore_being_deleted = true;
  vSemaphoreDelete(xMutex_state);
  vSemaphoreDelete(xMutex_payload);
}

/**
 * @brief Converts a YAML string to JSON and configures the system accordingly.
 * 
 * This function parses the provided YAML string into a JSON document. If the file type is `config_system`, it clears the current configuration,
 * applies the new configuration using `config_system`, and stores the updated
 * configurations. If requested, it calls `reset_before_new_configs`
 * to reset the system, recreates semaphores and tasks for hardware management and logic processing.
 * 
 * @param yaml_str A pointer to a YAML formatted string containing the configuration.
 * @param recreate_resources A boolean flag indicating whether to recreate system resources (e.g., semaphores and tasks) after applying new configurations.
 */
void yaml_to_json(const char *yaml_str, bool recreate_resources) {
  JsonDocument doc;
  DeserializationError error = deserializeYml(doc, yaml_str);
  if (error) {
    Serial.print(F("deserializeYml() failed: "));
    Serial.println(error.f_str());
    return;
  }
  const char *file_type = doc["file_type"];
  Serial.print("file type: ");
  Serial.println(file_type);
  // Currently the system supports config_system file type, also used for sanity check for the yaml.
  if (strcmp(file_type, "config_system") == 0) {
    if (recreate_resources)
      reset_before_new_configs();
    hand->clear_hand();
    config_system(doc);
    store_configs();
    if (recreate_resources) {
      // Recreate the mutexes
      xMutex_state = xSemaphoreCreateMutex();
      xMutex_payload = xSemaphoreCreateMutex();
      is_semaphore_being_deleted = false;
      xTaskCreate(HW_management, "HW_management", STACK_SIZE ,NULL ,1, &hw_Management_Handle);
      xTaskCreate(process_payload_and_manage_logic , "process_payload_and_manage_logic", STACK_SIZE ,NULL ,1, &process_Logic_Handle);
    }
  } else {
    Serial.print("Received unknown file type: ");
    Serial.println(file_type);
  }
}

#endif /* YAML_TO_JSON_PARSER */