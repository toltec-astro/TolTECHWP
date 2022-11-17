""" TolTEC HWPR Compressor Sensor Logger

    Logger program for TolTEC HWPR compressor sensor system.

    Configuration values at the start.

    2DO:
    - Basic program:
      ./ Configuration values
      ./ Open connection to sensor
      ./ LOOP
      - Test
    - Addons:
      - simple graph last 10min/24hr
      - socket interface to query latest string
"""

#### Settings
# Serial port to connect to arduino
#serialport = '/dev/tty.usbmodem14201' # mac
serialport = '/dev/tty.usbmodem1421101' # mac with USB extension
# Serial transfer rate
serialbaud = 19200
# Command to get values
readcmd = 'compress#'
# Log file name (can contain strftime formatting)
logfile = '/Users/berthoud/temp/ProTemp/TolTEC/data/complog%y%m%d.txt'
# Logging rate in seconds
lograte = 5

#### Imports
import serial
import time

#### Setup
# Setup timing
nextime = time.time() + lograte
# Setup connection
comm = serial.Serial(serialport,serialbaud,timeout=0.02)
print(f"Opened serial connection with {serialport}")
# Get backlog from serial port
time.sleep(2.0)
comm.read(1000)
#### Main Loop
while True:
    # Wait
    while time.time() < nextime:
        time.sleep(lograte/5.)
    # Set next time to take data
    nextime += lograte
    while nextime < time.time():
        nextime += lograte
    # Get the value
    comm.write(readcmd.encode())
    time.sleep(lograte/5.)
    try:
        dataval = comm.read(1000).decode().strip()
    except:
        dataval = '-'
    print(f"read {dataval}")
    # Add to logfile
    # On the fly converts strftime to current time
    with open(time.strftime(logfile),'at') as outf:
        outext = time.strftime("%y-%m-%d %H:%M:%S ")+dataval+'\n'
        outf.write(outext)
        outf.close()
