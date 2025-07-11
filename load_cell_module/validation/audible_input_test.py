"""
Modulates a real time sine wave based on the normalized output of the
load cell device.
"""

import numpy as np
import sounddevice as sd
import serial
import time

def play_sound(modulation_value: float):
    sample_rate = 44100
    duration = 0.05
    base_freq = 220.0
    mod_range = 880.0

    t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)
    modulated_freq = base_freq + modulation_value * mod_range
    wave = 0.5 * np.sin(2 * np.pi * modulated_freq * t).astype(np.float32)
    sd.play(wave, samplerate=sample_rate)
    sd.wait()

if __name__ == "__main__":
    ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)

    print("Live input preview activated. Press Ctrl+C to stop.")
    try:
        while True:
            if ser.in_waiting:
                line = ser.readline().decode("utf-8", errors="ignore").strip()
                value = float(line)
                print(value)
                play_sound(value)
    except KeyboardInterrupt:
        print("Live input preview stopped.")
    finally:
        ser.close()

