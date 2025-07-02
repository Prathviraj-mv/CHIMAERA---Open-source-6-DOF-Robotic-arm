import pygame
import serial
import time

# Initialize Serial
ser = serial.Serial('COM6', 9600, timeout=1)
time.sleep(2)

# Initialize Pygame Joystick
pygame.init()
pygame.joystick.init()
joystick = pygame.joystick.Joystick(0)
joystick.init()

# Configuration
angle_step = 30
angle =10
# Angle limits
limits = {
    'x': (20, 160),    # Shoulder
    'y': (-30, 110),   # Elbow
    'v': (-100, 200),  # Base
    'z': (0, 180),     # Wrist
    'a': (-10, 10),  # New stepper (adjust as needed)
    'c': (0, 180)      # New servo
}

# Initialize angles
angles = {
    'x': 0,   # Shoulder
    'y': 10,  # Elbow
    'v': 0,   # Base
    'z': 90,  # Wrist
    'a': 0,   # New stepper
    'c': 90   # New servo
}

# Button mappings (adjust as needed)
BUTTON_B_INDEX = 1  # Home
BUTTON_X_INDEX = 2  # Send c45
BUTTON_Y_INDEX = 3  # Send c-45

print("Controller initialized. Use joysticks to control the robotic arm. Press 'B' to home, 'X'/'Y' to control gripper.")

def send_angle(axis, angle):
    command = f"{axis}{angle}\n"
    ser.write(command.encode())
    print(f"Sent: {command.strip()}")

def send_home():
    ser.write(b'h\n')
    print("Sent: h (home)")

    # Reset tracked angles to home position
    angles['x'] = 0
    angles['y'] = 10
    angles['v'] = 0
    angles['z'] = 90
    angles['a'] = 0
    angles['c'] = 90

while True:
    pygame.event.pump()

    # Left joystick
    axis_x = joystick.get_axis(0)  # Left X (Shoulder)
    axis_y = joystick.get_axis(1)  # Left Y (Elbow)

    # Right joystick
    axis_z = joystick.get_axis(2)  # Right X (Wrist)
    axis_v = joystick.get_axis(3)  # Right Y (Base)

    # Shoulder ('x')
    if axis_x > 0.5:
        angles['x'] = min(angles['x'] + angle_step, limits['x'][1])
        send_angle('x', angles['x'])
        time.sleep(0.2)
    elif axis_x < -0.5:
        angles['x'] = max(angles['x'] - angle_step, limits['x'][0])
        send_angle('x', angles['x'])
        time.sleep(0.2)

    # Elbow ('y')
    if axis_y > 0.5:
        angles['y'] = min(angles['y'] + angle_step, limits['y'][1])
        send_angle('y', angles['y'])
        time.sleep(0.2)
    elif axis_y < -0.5:
        angles['y'] = max(angles['y'] - angle_step, limits['y'][0])
        send_angle('y', angles['y'])
        time.sleep(0.2)

    # Wrist ('z')
    if axis_z > 0.5:
        angles['z'] = min(angles['z'] + angle, limits['z'][1])
        send_angle('z', angles['z'])
        time.sleep(0.2)
    elif axis_z < -0.5:
        angles['z'] = max(angles['z'] - angle, limits['z'][0])
        send_angle('z', angles['z'])
        time.sleep(0.2)

    # Base ('v')
    if axis_v > 0.5:
        angles['v'] = min(angles['v'] + angle_step, limits['v'][1])
        send_angle('v', angles['v'])
        time.sleep(0.2)
    elif axis_v < -0.5:
        angles['v'] = max(angles['v'] - angle_step, limits['v'][0])
        send_angle('v', angles['v'])
        time.sleep(0.2)

    # Additional stepper ('a') controlled by D-pad (example: up/down)
    hat = joystick.get_hat(0)  # (x, y)
    if hat[1] == 1:  # D-pad up
        angles['a'] = min(angles['a'] + angle_step, limits['a'][1])
        send_angle('a', angles['a'])
        time.sleep(0.2)
    elif hat[1] == -1:  # D-pad down
        angles['a'] = max(angles['a'] - angle_step, limits['a'][0])
        send_angle('a', angles['a'])
        time.sleep(0.2)

    # Check Button B to home
    if joystick.get_button(BUTTON_B_INDEX):
        send_home()
        time.sleep(0.5)  # Debounce

    # Check Button X to close gripper (send c45)
    if joystick.get_button(BUTTON_X_INDEX):
        angles['c'] = 45
        send_angle('c', angles['c'])
        time.sleep(0.5)  # Debounce

    # Check Button Y to open gripper (send c-45)
    if joystick.get_button(BUTTON_Y_INDEX):
        angles['c'] = -45
        send_angle('c', angles['c'])
        time.sleep(0.5)  # Debounce
