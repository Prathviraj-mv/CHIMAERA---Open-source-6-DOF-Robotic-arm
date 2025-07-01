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

# Angle limits
limits = {
    'x': (20, 160),   # Shoulder
    'y': (-30, 110),   # Elbow
    'v': (-100, 200),  # Base
    'z': (0, 180)      # Wrist
}

# Initialize angles
angles = {
    'x': 0,   # Shoulder
    'y': 0,   # Elbow
    'v': 0,   # Base
    'z': 90   # Wrist
}

# Button B index (typically 1 for Xbox controllers, adjust if needed)
BUTTON_B_INDEX = 1

print("Controller initialized. Use joysticks to control the robotic arm. Press 'B' to home.")

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
        angles['z'] = min(angles['z'] + angle_step, limits['z'][1])
        send_angle('z', angles['z'])
        time.sleep(0.2)
    elif axis_z < -0.5:
        angles['z'] = max(angles['z'] - angle_step, limits['z'][0])
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

    # Check if Button B is pressed to home
    if joystick.get_button(BUTTON_B_INDEX):
        send_home()
        time.sleep(0.5)  # Debounce delay
