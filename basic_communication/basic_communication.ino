#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


// -------------------------------------------------------------------------------------------------- //
// ------------------------------------------ SYSTEM CONSTS  ---------------------------------------- //
// -------------------------------------------------------------------------------------------------- //

//  ------------- UUIDs  -----------  //
#define BATTERY_SERVICE_UUID        "180F"
#define BATTERY_LEVEL_CHAR_UUID     "2A19"

#define HAND_CONFIG_SERVICE_UUID     "e0198000-7544-42c1-0000-b24344b6aa70"
#define CONFIG_ON_WRITE_CHARACTERISTIC_UUID "e0198000-7544-42c1-0001-b24344b6aa70"

#define SENSOR_SERVICE_UUID            "e0198002-7544-42c1-0000-b24344b6aa70"
#define SENSOR_ON_WRITE_CHARACTERISTIC_UUID "e0198002-7544-42c1-0001-b24344b6aa70"

#define DEVICE_INFO_SERVICE_UUID    "180A"
#define MANUFACTURER_NAME_CHAR_UUID "2A29"
#define MODEL_NUMBER_CHAR_UUID      "2A24"

// -------------------------------------------------------------------------------------------------- //
// -------------------------------------------------------------------------------------------------- //
// -------------------------------------------------------------------------------------------------- //



// BLE Server and Characteristics
BLECharacteristic *batteryLevelCharacteristic;
BLECharacteristic *manufacturerNameCharacteristic;
BLECharacteristic *modelNumberCharacteristic;
bool deviceConnected = false;



// ---------------------------------------------------------------------------------------------------------- //
// ------------------------------------------ CALLBACKS DEFENITIONS  ---------------------------------------- //
// ---------------------------------------------------------------------------------------------------------- //
// Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Client Disconnected");
      pServer->startAdvertising(); // Restart advertising
    }
};

class ConfigCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) { //execute one movement from the 'live control' or from the 'movement editing'
      // Check the payload size
      const uint8_t* dataPtr = pCharacteristic->getData();
      size_t dataSize = pCharacteristic->getLength();
      Serial.print("Payload size: ");
      Serial.println(dataSize);

      // Optionally print the payload data
      Serial.print("Payload data: ");
      for (size_t i = 0; i < dataSize; ++i) {
        Serial.print(dataPtr[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      };
};

class SensorCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) { //execute one movement from the 'live control' or from the 'movement editing'
      // Check the payload size
      const uint8_t* dataPtr = pCharacteristic->getData();
      size_t dataSize = pCharacteristic->getLength();
      Serial.print("Payload size: ");
      Serial.println(dataSize);

      // Optionally print the payload data
      Serial.print("Payload data: ");
      for (size_t i = 0; i < dataSize; ++i) {
        Serial.print(dataPtr[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      };
};
// ---------------------------------------------------------------------------------------------------------- //
// ---------------------------------------------------------------------------------------------------------- //
// ---------------------------------------------------------------------------------------------------------- //



// ------------------------------------------------------------------------------------- //
// ---------------------------  Services Setup Functions ------------------------------- //
// ------------------------------------------------------------------------------------- //

  // ---------------------- Battery Service Setup ------------------------- //
void batteryServiceSetup(BLEServer *pServer){
  // Create the Battery Service
  BLEService *batteryService = pServer->createService(BATTERY_SERVICE_UUID);

  // Create the Battery Level Characteristic
  batteryLevelCharacteristic = batteryService->createCharacteristic(
                                  BATTERY_LEVEL_CHAR_UUID,
                                  BLECharacteristic::PROPERTY_READ |
                                  BLECharacteristic::PROPERTY_NOTIFY
                                );

  batteryLevelCharacteristic->addDescriptor(new BLE2902());

  // Start the Battery Service
  batteryService->start();
}

  // ---------------------- Config Service Setup ------------------------ //
void configServiceSetup(BLEServer *pServer){

  BLEService *pConfigService = pServer->createService(HAND_CONFIG_SERVICE_UUID);
  BLECharacteristic *pConfigOnWriteCharacteristic = pConfigService->createCharacteristic(
                                         CONFIG_ON_WRITE_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pConfigOnWriteCharacteristic->setCallbacks(new ConfigCallbacks());
  pConfigService->start();
}

  // ---------------------- Sensor Service Setup ------------------------ //
void sensorServiceSetup(BLEServer *pServer){  
  BLEService *pSensorService = pServer->createService(SENSOR_SERVICE_UUID);
  BLECharacteristic *pSensorOnWriteCharacteristic = pSensorService->createCharacteristic(
                                         SENSOR_ON_WRITE_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pSensorOnWriteCharacteristic->setCallbacks(new SensorCallbacks());
  pSensorService->start();
}

  // ---------------------- Device Information Service Setup ------------------------ //
void devInfoServiceSetup(BLEServer *pServer){ 
  BLEService *deviceInfoService = pServer->createService(DEVICE_INFO_SERVICE_UUID);

  // Create the Manufacturer Name Characteristic
  manufacturerNameCharacteristic = deviceInfoService->createCharacteristic(
                                     MANUFACTURER_NAME_CHAR_UUID,
                                     BLECharacteristic::PROPERTY_READ
                                   );

  manufacturerNameCharacteristic->setValue("MyManufacturer");

  // Create the Model Number Characteristic
  modelNumberCharacteristic = deviceInfoService->createCharacteristic(
                                MODEL_NUMBER_CHAR_UUID,
                                BLECharacteristic::PROPERTY_READ
                              );

  modelNumberCharacteristic->setValue("ESP32_Model_1");

  // Start the Device Information Service
  deviceInfoService->start();
}

void setServices(BLEServer *pServer){
  batteryServiceSetup(pServer)
  configServiceSetup(pServer)
  sensorServiceSetup(pServer)
  devInfoServiceSetup(pServer)
}

void setAdvertizing(BLEServer *pServer){
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  //BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setMinInterval(100);
  pAdvertising->setMaxInterval(200);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->addServiceUUID(HAND_CONFIG_SERVICE_UUID);
  pAdvertising->addServiceUUID(SENSOR_SERVICE_UUID);
  pAdvertising->start();
  pAdvertising->setScanResponse(true);
  Serial.println("advertising started!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  BLEDevice::init("SMART_PROSTHESIS");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create services for BLE Server (battery , config , sensor , info)
  setServices(pServer)

  // Start advertising 
  setAdvertizing(pServer)

  // // Set initial battery level
  // batteryLevelCharacteristic->setValue((uint8_t)100);

  // Set GPIO2 as output (for an LED)
  pinMode(2, OUTPUT);
}

void loop() {
  if (deviceConnected) {
    // Toggle the LED
    digitalWrite(2, millis() / 1000 % 2 == 0 ? HIGH : LOW);

    // Simulate battery level change
    static uint8_t batteryLevel = 100;
    batteryLevel = batteryLevel > 0 ? batteryLevel - 1 : 100;
    batteryLevelCharacteristic->setValue(&batteryLevel, 1);
    batteryLevelCharacteristic->notify();

    Serial.print("Battery Level: ");
    Serial.println(batteryLevel);
    delay(1000);

  } else {
    Serial.println("Waiting for a client connection to notify...");
    delay(1000);
  }
}