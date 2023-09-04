import paho.mqtt.client as mqtt
from pykalman import KalmanFilter
import math
import tkinter as tk

# Create a Tkinter window
root = tk.Tk()
root.title("Estimated Position on Green Screen")

# Define the estimated position (replace with your actual coordinates)
x, y = 0.00, 0.00

# Create a canvas to draw the green screen
canvas = tk.Canvas(root, width=400, height=400, bg="green")
canvas.pack()

# Calculate the position on the canvas (adjust scale as needed)
canvas_x = x * 10  # Scale x position for the canvas
canvas_y = y * 10  # Scale y position for the canvas

# Draw a marker at the estimated position
marker = canvas.create_oval(canvas_x - 5, canvas_y - 5, canvas_x + 5, canvas_y + 5, fill="red")

# Function to update the marker's position (if needed)
def update_position(new_x, new_y):
    global canvas_x, canvas_y
    canvas_x = new_x * 10
    canvas_y = new_y * 10
    canvas.coords(marker, canvas_x - 5, canvas_y - 5, canvas_x + 5, canvas_y + 5)

# Example update (replace with your data update mechanism)
update_position(15.0, 25.0)

# Start the Tkinter main loop
root.mainloop()

# Calculate distance from RSSI using the Friis free space path loss formula
def calculate_distance(rssi, tx_power, path_loss_exponent):
    return 10 ** ((tx_power - rssi) / (10 * path_loss_exponent))


# Define anchor positions (x, y) in meters
anchor1_position = (0, 0)
anchor2_position = (2, 0)
anchor3_position = (0, 3)

# Calculate distance using trilateration
def trilateration(anchor1_distance, anchor2_distance, anchor3_distance):
    x1, y1 = anchor1_position
    x2, y2 = anchor2_position
    x3, y3 = anchor3_position
    
    A = 2 * x2 - 2 * x1
    B = 2 * y2 - 2 * y1
    C = anchor1_distance ** 2 - anchor2_distance ** 2 - x1 ** 2 + x2 ** 2 - y1 ** 2 + y2 ** 2
    D = 2 * x3 - 2 * x2
    E = 2 * y3 - 2 * y2
    F = anchor2_distance ** 2 - anchor3_distance ** 2 - x2 ** 2 + x3 ** 2 - y2 ** 2 + y3 ** 2
    
    x = (C*E - F*B) / (E*A - B*D)
    y = (C*D - A*F) / (B*D - A*E)
    
    return x, y



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


# Initialize variables for distances from each anchor
distance1 = None
distance2 = None
distance3 = None


def on_message(client, userdata, message):
    global distance1, distance2, distance3  # Allow modification of global variables
    
    topic = message.topic
    payload = message.payload.decode('utf-8')
    rssi = int(payload)
    
    if topic == "anchor1/rssi":
        filtered_rssi = kf.filter_update(filtered_state_mean=[0, 0], observation=rssi)
        distance1 = calculate_distance(filtered_rssi, 11, 2.1)
        print(f"Distance1: {distance1}")
    elif topic == "anchor2/rssi":
        filtered_rssi = kf.filter_update(filtered_state_mean=[0, 0], observation=rssi)
        distance2 = calculate_distance(filtered_rssi, 11, 2.2)
        print(f"Distance2: {distance2}")
    elif topic == "anchor3/rssi":
        filtered_rssi = kf.filter_update(filtered_state_mean=[0, 0], observation=rssi)
        distance3 = calculate_distance(filtered_rssi, 11, 2.1)
        print(f"Distance3: {distance3}")
    
    # Check if all distances are available
    if distance1 is not None and distance2 is not None and distance3 is not None:
        # Calculate position using trilateration
        x, y = trilateration(distance1, distance2, distance3)

        # Print estimated position
        print(f"Anchor 1 Distance: {distance1} meters")
        print(f"Anchor 2 Distance: {distance2} meters")
        print(f"Anchor 3 Distance: {distance3} meters")
        print(f"Estimated Position: ({x:.2f}, {y:.2f}) meters")
        update_position(x, y)
        
# Setup MQTT client
client = mqtt.Client()

# Connect to MQTT broker
broker_address = "e7332aa7.ala.us-east-1.emqxsl.com"
client.connect(broker_address, port=1883)


# Set the callbacks for each topic
#client.message_callback_add("anchor1/rssi", on_message_topic1)
#client.message_callback_add("anchor2/rssi", on_message_topic2)
#client.message_callback_add("anchor3/rssi", on_message_topic3)


# Set the callback for message reception
client.on_message = on_message


# Subscribe to the three topics
client.subscribe("anchor1/rssi")
client.subscribe("anchor2/rssi")
client.subscribe("anchor3/rssi")

# Start the MQTT loop
client.loop_forever()
