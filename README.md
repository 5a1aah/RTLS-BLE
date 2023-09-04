# Real-Time Location System (RTLS) with ESP32 Anchors
![Alt Text](https://github.com/5a1aah/RTLS-BLE/blob/main/Anchor%20code/TDoA-1024x752.png)


This project implements an RTLS using ESP32 anchors for accurate position tracking of Bluetooth Low Energy (BLE) beacons.

## Table of Contents
- [Hardware Setup](#hardware-setup)
- [Changing MQTT Topic](#changing-mqtt-topic)
- [Position Calculation Script](#position-calculation-script)

## Hardware Setup

To set up the ESP32 anchors for accurate positioning:

1. Physically place the three ESP32 anchors in your environment at known positions. Ensure they have power and network connectivity.
(in this example
anchor1_position = (0, 0), 
anchor2_position = (2, 0), 
anchor3_position = (0, 3) )
change calculs.py based on your anchors positions.
2. change const char* macToFilter = "00:11:22:33:44:55"; to your beacon Mac in the anchor code.
3. Calibrate the anchors, if necessary, to account for variations in signal strength and positioning.


## Changing MQTT Topic

You can customize the MQTT topic used by the ESP32 anchors as follows:

1. Open the ESP32 anchor code in PlatformIO.

2. Locate the MQTT topic configuration in the code.

3. Update the `mqtt_topic` variable to anchor1/rssi for anchor 1 ;  anchor2/rssi for 2nd   anchor3/rssi for 3rd (or to your desired MQTT topics names).

4. Save the changes and upload the modified code to your ESP32 anchors.

## Position Calculation Script

The position calculation script uses trilateration to estimate the position of Bluetooth beacons based on data received from the ESP32 anchors. Here's how it works:

1. The ESP32 anchors collect RSSI data from nearby beacons and publish it to MQTT topics.

2. The Python script subscribes to these MQTT topics to receive RSSI data.

3. The script applies trilateration, a geometric technique, to calculate the position of beacons based on the distances from three known anchor points.

4. The estimated positions are then used for location tracking or visualization.

## Dependencies and Usage

To use the position calculation script:

1. Ensure you have the required dependencies installed. You can install them using pip:

   ```bash
   pip install paho-mqtt pykalman
   pip install pykalman


## Conclusion

Thank you for using our Real-Time Location System with ESP32 anchors. If you have any questions or need support, please reach out to us at [salaheddine23456789@gmail.com].



