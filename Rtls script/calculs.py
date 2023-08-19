import paho.mqtt.client as mqtt
from pykalman import KalmanFilter

# Define the Kalman filter parameters
transition_matrix = [[1, 1], [0, 1]]  # State transition matrix
observation_matrix = [[1, 0]]  # Observation matrix
initial_state_mean = [0, 0]  # Initial state mean
initial_state_covariance = [[1, 0], [0, 1]]  # Initial state covariance
observation_covariance = 0.01  # Observation noise covariance
transition_covariance = [[0.01, 0], [0, 0.01]]  # Transition noise covariance
# Create a Kalman filter
kf = KalmanFilter(
    transition_matrices=transition_matrix,
    observation_matrices=observation_matrix,
    initial_state_mean=initial_state_mean,
    initial_state_covariance=initial_state_covariance,
    observation_covariance=observation_covariance,
    transition_covariance=transition_covariance
)

# Callback when a message is received
def on_message(client, userdata, message):
    payload = message.payload.decode('utf-8')
    rssi = int(payload)
    
    # Apply Kalman filter to the RSSI value
    filtered_rssi = kf.filter_update(filtered_state_mean=[0, 0], observation=rssi)
    
    print(f"Received RSSI: {rssi}, Filtered RSSI: {filtered_rssi[0][0]}")

# Setup MQTT client
client = mqtt.Client()
client.on_message = on_message

# Connect to MQTT broker
broker_address = "e7332aa7.ala.us-east-1.emqxsl.com"
client.connect(broker_address, port=1883)

# Subscribe to the MQTT topic
topic = "ble/rssi"
client.subscribe(topic)

# Start the MQTT loop
client.loop_forever()
