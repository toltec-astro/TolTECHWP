""" TolTEC HWPR Compressor Sensor Logger

    Logger program for TolTEC HWPR compressor sensor system.

    Configuration values at the start.

"""

#### Settings
# Serial port to connect to arduino
#serialport = '/dev/tty.usbmodem14201' # mac
#serialport = '/dev/tty.usbmodem1421101' # mac with USB extension
serialport = '/dev/ttyACM0'

#serialport = '/dev/tty.usbmodem1421' # mac
#serialport = '/dev/tty.usbmodem1421101' # mac with USB extension
# Serial transfer rate
serialbaud = 19200
# Command to get values
readcmd = 'compress\n'
# Log file name (can contain strftime formatting)
#logfile = '/data/toltec/compressormon/comp_mon_log_%y%m%d.txt'
logfile = '/home/poluser/TolTECTEMP/compressormon/comp_mon_log_%y%m%d.txt'
#logfile = '/Users/berthoud/temp/complog%y%m%d.txt'
# Logging rate in seconds
lograte = 10
# Simulate flag, set True if no sensor is attached
simulatesensor = False

#### Imports
import serial
import time
from datetime import datetime, timezone
import os
import re
#import psycopg # to send data to external database
pgconnstring = os.getenv('PGCONNSTRING')

#### Setup
# Setup timing
nextime = time.time() + lograte
# Setup connection
if not simulatesensor:
    comm = serial.Serial(serialport,serialbaud,timeout=0.02)
    print("Opened serial connection with %s" % serialport)
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
    try:
        if simulatesensor: 
            dataval = 'Compressor pressure: 341cnts 1.67V 90.00Psi'
        else:
            comm.write(readcmd.encode())
            time.sleep(lograte/5.)
            dataval = comm.read(1000).decode().strip()
    except:
        # If fail to read, close then open connection, try to read again
        try:
            # Try to close connection
            try:
                comm.close()
            except:
                print("Failed Closing connection")
            # Open connection
            time.sleep(lograte/5.)
            comm = serial.Serial(serialport,serialbaud,timeout=0.02)
            print("Opened serial connection with %s" % serialport)
            # Clear serial buffer with empty command
            time.sleep(lograte/5.)
            comm.write('\n'.encode())
            time.sleep(lograte/5.)
            dataval = comm.read(1000).decode().strip()
            # Read data
            comm.write(readcmd.encode())
            time.sleep(lograte/5.)
            dataval = comm.read(1000).decode().strip()
        except:
            # This fails as well - return bad data
            dataval = '-'
    print("read %s" % dataval)
    # Check if logfile exists
    # On the fly converts strftime to current time
    logfname = time.strftime(logfile)
    logfnew = not os.path.exists(logfname)
    # Add to logfile
    with open(logfname, 'at') as outf:
        if logfnew:
            outf.write('Date,Time,Sensor_Cnts,Sensor_V,Press_Psi\n')
        outext = time.strftime("%y-%m-%d,%H:%M:%S,")
        dt = time.strftime("%y-%m-%d %H:%M:%S")
        raw, voltage, psi = re.findall("Compressor pressure: (.*)cnts (.*)V (.*)Psi", dataval)[0]
        print(datetime, raw, voltage, psi)
        outext += '%s,%s,%s\n' % (raw,voltage,psi)
        outf.write(outext)
        outf.close()
    # Send to 
    try:
        pass
        #raw, voltage, psi = re.findall("Compressor pressure: (.*)cnts (.*)V (.*)Psi", dataval)[0]
        #utcdt = datetime.now(timezone.utc)
        #with psycopg.connect(pgconnstring) as conn:
        #    with conn.cursor() as cur:
        #        cur.execute(
        #                "INSERT INTO comp_pressure (DATETIME, COUNTER, VOLTAGE, PSI, UTCDATETIME) VALUES (%s, %s, %s, %s, %s)",
        #            (dt, int(raw), float(voltage), float(psi), utcdt,)
        #        )
        #        conn.commit()
    except Exception as e: 
        print(e)
        pass
