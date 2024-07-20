#ifndef HAND_FUNCTIONS
#define HAND_FUNCTIONS
#include "classes.h"
#include <map>
#include <vector>
#include <ESP32Servo.h>

void sensor_1_func(std::vector<double> params, const uint8_t *payload) {
  uint8_t sensor_id = payload[0]
  uint8_t sensor_value = payload[1]
  Serial.print("Got sensor id: ");
  Serial.println(sensor_id);
  Serial.print("With value: ");
  Serial.println(sensor_value);
  for (double param : params) {
    Serial.print("param: ");
    Serial.println(param);
  }

  Servo_motor* finger1_servo = (dynamic_cast<Servo_motor*>(hand->get_output_by_name("finger1_servo")));
  int control_pin = finger1_servo->control_pin;
  Servo myServo;
  myServo.attach(control_pin);
  if(sensor_value > 0 && sensor_value < 90){
    finger_1_up_to_90_fast(&my_servo , sensor_value)
  }
  if(sensor_value > 90 && sensor_value < 180){
    finger_1_more_than_90_slow(&my_servo , sensor_value)
  }
}

void finger_1_up_to_90_fast(Servo* my_servo , const uint8_t angle){
  myServo->write(0);  // Move the servo to 0 degrees
  delay(500);    
  myServo->write(angle); // Move the servo to 90 degrees
  delay(500);     
}

void finger_1_more_than_90_slow(Servo* my_servo , const uint8_t angle){
  myServo->write(0);  // Move the servo to 0 degrees
  delay(1500);    
  myServo->write(angle); // Move the servo to 90 degrees
  delay(1500);      
}

std::map<String, FuncPtr> func_map = {
  {"sensor_1_func", sensor_1_func}
};


#endif /* HAND_FUNCTIONS */