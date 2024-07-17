#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <DHT.h>  // Include the DHT sensor library

#define DHTPIN 15    // Pin where DHT sensor is connected
#define DHTTYPE DHT22 // Type of DHT sensor you are using

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pTempCharacteristic;
BLECharacteristic *pHumidityCharacteristic;
BLECharacteristic *pWiFiCredentialsCharacteristic;

DHT dht(DHTPIN, DHTTYPE);

class WiFiCredentialsCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    // Process received WiFi credentials (example implementation)
    if (value.length() > 0) {
      Serial.println("Received WiFi credentials:");
      Serial.println(value.c_str());

    }
  }
};

WiFiCredentialsCallbacks credentialsCallback;

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32_BLE");

  pServer = BLEDevice::createServer();
  pService = pServer->createService(BLEUUID("180D")); // Example service UUID

  pTempCharacteristic = pService->createCharacteristic(
                          BLEUUID("2A6E"), // Example temperature characteristic UUID
                          BLECharacteristic::PROPERTY_READ |
                          BLECharacteristic::PROPERTY_NOTIFY
                        );

  pHumidityCharacteristic = pService->createCharacteristic(
                              BLEUUID("2A6F"), // Example humidity characteristic UUID
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_NOTIFY
                            );

  pWiFiCredentialsCharacteristic = pService->createCharacteristic(
                                    BLEUUID("00000002-0000-0000-FDFD-FDFDFDFDFDFD"), // Service UUID
                                    BLECharacteristic::PROPERTY_WRITE
                                  );

  pService->start();

  pWiFiCredentialsCharacteristic->setCallbacks(&credentialsCallback);

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  dht.begin(); // Initialize DHT sensor
}

void loop() {
  delay(2000); // Delay between sensor readings

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Format temperature and humidity values as strings
  char tempStr[6];
  char humStr[6];
  dtostrf(temperature, 5, 1, tempStr); // Format temperature with 1 decimal place
  dtostrf(humidity, 5, 1, humStr);     // Format humidity with 1 decimal place

  // Update characteristics with formatted values
  pTempCharacteristic->setValue(tempStr);
  pTempCharacteristic->notify();

  pHumidityCharacteristic->setValue(humStr);
  pHumidityCharacteristic->notify();

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}
