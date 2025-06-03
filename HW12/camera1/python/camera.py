import pgzrun # pip install pgzero

import serial
ser = serial.Serial('COM6') # the name of your port here
print('Opening port: ' + str(ser.name))

import numpy as np

# Set the window size
SCALE = 5
WIDTH = 400
HEIGHT = 400
def update():
    selection_endline = 'c'+'\n'
     
    # send the command 
    ser.write(selection_endline.encode())

def draw():
    reds = np.zeros((60, 80), dtype=np.uint8)
    greens = np.zeros((60, 80), dtype=np.uint8)
    blues = np.zeros((60, 80), dtype=np.uint8)

    t = 0
    while t < 4800:
        dat_str = ser.read_until(b'\n')
        try:
            parts = list(map(int, dat_str.strip().split()))
            if len(parts) != 4:
                continue  # Skip lines that don't have exactly 4 values
            i, r, g, b = parts
        except ValueError:
            print(f"Skipping invalid line: {dat_str}")
            continue

        row = i // 80
        col = i % 80
        if 0 <= row < 60 and 0 <= col < 80:
            reds[row][col] = r
            greens[row][col] = g
            blues[row][col] = b
            t += 1  # Only increment t if a valid line is processed

    screen.fill((0, 0, 0))
    for x in range(60):
        for y in range(80):
            color = (reds[x][y], greens[x][y], blues[x][y])
            screen.draw.filled_rect(
                Rect((y * SCALE, x * SCALE), (SCALE, SCALE)),
                color
            )



pgzrun.go()