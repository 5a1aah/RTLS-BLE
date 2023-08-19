/* NOT THIS */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <WiFi.h>
#include <PubSubClient.h>

  //Wifi 
const char* ssid =  "NXT-AP2";  //"1234"; 
const char* password =  "2899100*-+"; //"salah1234";
  //Mqtt
const char* mqtt_server = "3f271f1bd5ef4564bf5bb214a1d730e7.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_topic = "ble/rssi1";
//const char* mqtt_username = "mqtt_username";
//const char* mqtt_password = "mqtt_password";

WiFiClient espClient;
PubSubClient client(espClient);
BLEScan* pBLEScan;
SemaphoreHandle_t scanMutex;



class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // Not used here since we're performing scanning in the loop
        
    }
};



// Task for handling WiFi connection
void wifiTask(void* parameter) {
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        Serial.println("Connecting to WiFi...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    Serial.println("Connected to WiFi");
    vTaskDelete(NULL);
}

// Task for handling BLE scanning
void scanBLE(void* parameter) {
    while (1) {
        xSemaphoreTake(scanMutex, portMAX_DELAY);
        BLEScanResults foundDevices = BLEDevice::getScan()->start(5, false); // Scan for 5 seconds
        int count = foundDevices.getCount();
        xSemaphoreGive(scanMutex);

        char payload[10];
        for (int i = 0; i < count; i++) {
            BLEAdvertisedDevice device;
            xSemaphoreTake(scanMutex, portMAX_DELAY);
            device = foundDevices.getDevice(i);
            xSemaphoreGive(scanMutex);

            Serial.println("Found BLE Device: ");
            Serial.print("Name: ");
            Serial.print(device.getName().c_str()); // Convert to C-style string
            Serial.print(", MAC: ");
            Serial.print(device.getAddress().toString().c_str()); // Convert to C-style string
            Serial.print(", RSSI: ");
            Serial.print(device.getRSSI());
            snprintf(payload, sizeof(payload), "%d", device.getRSSI());

            // Publish RSSI data to MQTT topic
            if (client.connected()) {
                client.publish(mqtt_topic, payload);
            }
            Serial.println();
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds before scanning again
        xSemaphoreTake(scanMutex, portMAX_DELAY);
        BLEDevice::getScan()->clearResults(); // Delete results from BLEScan buffer to release memory
        xSemaphoreGive(scanMutex);
    }
}

// Task for handling MQTT
void mqttTask(void* parameter) {
    while (1) {
        if (!client.connected()) {
            client.connect("ESP32Client");
            Serial.println("Connecting to MQTT...");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        // Handle MQTT messages and other MQTT-related tasks here

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Adjust the delay as needed
    }
}

void setup() {
    Serial.begin(9600);
    BLEDevice::init("ESP32 Scanner");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);

    // Create a mutex to protect BLE scanning
    scanMutex = xSemaphoreCreateMutex();

    // Create tasks
    xTaskCreate(wifiTask, "WiFiTask", 10000, NULL, 1, NULL);
    xTaskCreate(scanBLE, "BLEScanTask", 10000, NULL, 1, NULL);
    xTaskCreate(mqttTask, "MQTTTask", 10000, NULL, 1, NULL);
}

void loop() {
    // The main loop is not used since tasks are handling the operations
}

    






