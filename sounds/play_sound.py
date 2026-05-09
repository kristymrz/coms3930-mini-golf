import serial
import subprocess
import os
import glob

SOUND_FILE = os.path.join(os.path.dirname(__file__), "celebration.wav")

# Auto-detect the serial port
ports = glob.glob("/dev/tty.usbserial-*") + glob.glob("/dev/tty.SLAB_USBtoUART*")
if not ports:
    print("No ESP32 serial port found. Is it plugged in?")
    exit(1)

PORT = ports[0]
print(f"Listening on {PORT}...")

ser = serial.Serial(PORT, 115200, timeout=1)

while True:
    line = ser.readline().decode("utf-8", errors="ignore").strip()
    if line:
        print(line)
    if "[BALL]" in line:
        print("Ball detected — playing sound!")
        subprocess.Popen(["afplay", SOUND_FILE])
