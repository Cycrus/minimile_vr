"""
Reads the serial port and saves its input into a csv file.
Used to check the output measurements of the load cell device.
"""

import serial
import time

if __name__ == "__main__":
    ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    filename = "load_cell_data.csv"

    with open(filename, "w") as file:
        print("Logging serial data. Press Ctrl+C to stop.")
        try:
            while True:
                if ser.in_waiting:
                    line = ser.readline().decode("utf-8", errors="ignore").strip()
                    print(line)
                    file.write(line + "\n")
        except KeyboardInterrupt:
            print("\nLogging stopped.")
        finally:
            ser.close()
