/*    Author : Salah-eddine EL ABBASSI  
      https://www.linkedin.com/in/salah-eddine-el-abbassi-3a99b923a/                                   */


#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <WiFi.h>
#include <PubSubClient.h>

  //Wifi 
const char* ssid =    "èèèèèèè";   //Wifi name;
const char* password =  "éééééééééé";   // password;
  
  // MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "anchor1/rssi";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;


//const char* mqtt_server = "192.168.43.239";
//const char* mqtt_password = ""; // No password required
//const int mqtt_port = 1883;
//const char* mqtt_topic = "anchor1/rssi";
//const char* mqtt_username = "mqtt_username";
//const char* mqtt_password = "mqtt_password";

WiFiClient espClient;
PubSubClient client(espClient);
BLEScan* pBLEScan;
SemaphoreHandle_t scanMutex;


// MAC addresses of BLE devices to filter
const char* macToFilter = "00:11:22:33:44:55";

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // Filter devices based on MAC address
        Serial.print("callback function ");
        if (advertisedDevice.getAddress().equals(BLEAddress(macToFilter))) {
            Serial.print("Found BLE Device: ");
            Serial.print("Name: ");
            Serial.print(advertisedDevice.getName().c_str()); // Convert to C-style string
            Serial.print(", MAC: ");
            Serial.print(advertisedDevice.getAddress().toString().c_str());
            Serial.print(", RSSI: ");
            Serial.println(advertisedDevice.getRSSI());

            char payload[10];
            snprintf(payload, sizeof(payload), "%d", advertisedDevice.getRSSI());

            // Publish RSSI data to MQTT topic
            if (client.connected()) {
                client.publish(topic, payload);
            }
        }
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

        //char payload[10];
        for (int i = 0; i < count; i++) {
            BLEAdvertisedDevice device;
            xSemaphoreTake(scanMutex, portMAX_DELAY);
            device = foundDevices.getDevice(i);
            xSemaphoreGive(scanMutex);

            // Handle the device in the callback
            device.getRSSI(); // This will trigger the callback

            /* Serial.println("Found BLE Device: ");
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
            Serial.println(); */
        } 

        vTaskDelay(10000 / portTICK_PERIOD_MS); // Wait for 5 seconds before scanning again
        xSemaphoreTake(scanMutex, portMAX_DELAY);
        BLEDevice::getScan()->clearResults(); // Delete results from BLEScan buffer to release memory
        xSemaphoreGive(scanMutex);
    }
}

// Task for handling MQTT
void mqttTask(void* parameter) {
client.setServer(mqtt_broker, mqtt_port);
    //client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
    /*while (1) {
        if (!client.connected()) {
            client.setServer(mqtt_server, mqtt_port);
            client.connect("ESP32Clienthvjh");
            Serial.println("Connecting to MQTT...");

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        else {
         Serial.println("Mqtt connected");
        }

        // Handle MQTT messages and other MQTT-related tasks here

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Adjust the delay as needed
    }*/
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
