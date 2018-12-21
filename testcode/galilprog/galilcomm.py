""" GALILCOMM

    Program to test Galil functions using a simple python program.

    ### To Run: type "python galilcomm.py" into a terminal

    ### Program Outline
    - confirm input parameters
    - open connection
    - check communication (ask for CR)
    - send start sequence
    - querry, print and save data for given time
    - send end sequence

    ### Log:
    - 2018-8: Copy made for TolTEC for further development
    - 2014-5: Used to record pupil and HWP wheel sensor data
    - 2012-??: Used to record pupil and filter wheel detent data

"""

### Compiler commands
import os
import telnetlib
import time
import string
import serial

### Setup Parameters
params = {}
# -- Communication
params['comtype'] = 'serial' # options are telnet (default) and serial
params['device'] = '/dev/tty.usbserial-AO003IDA' # for serial device
params['baud'] = '115200'
params['host'] = '192.168.2.2' # for telnet
params['port'] = '23' # for telnet
# -- Galil Commands
params['setup_commands'] = 'TP\n\r' # initial command
#params['start_commands'] = ''
params['start_commands'] = 'SHA;BGA\n\r' #'BGA\n\r'
params['stop_commands'] = 'TP;MOA\n\r' # 'TP;MOA\n\r'
#params['stop_commands'] = 'MG @AN[5]'
# -- Data Recording
params['data_vals'] = '_TPA _TEA _TTA _TVA'
params['logfile'] = 'ANoutput.txt'
# -- Sequence
#    pause - start - step - stop - pause - step - pause - step - pause till end
params['duration'] = '150' # '100' # Seconds of total time
params['stepn'] = '1'     # '10'  # number of steps
params['steptime'] = '145' # '9'   # Duration of each step in seconds
params['pausetime'] = '2' # '1'   # Seconds to pause between steps

com = None

### Function definition
def galread():
    """ Function to read (depends on readout method
    """
    if params['comtype'] in ['serial']:
        return com.read(1000)
    else:
        return com.read_eager()

### Print parameters, allow changes
print('Current Settings:')
for k in params.keys():
    print("  %s = %s" % (k, params[k]))

message = 'Press Enter to continue ("setting=value" to make changes) : '
rsp = raw_input(message)

# Change values
while len(rsp) > 0:
    # Change the value
    try:
        spl = rsp.split('=')
        k = spl[0].strip()
        v = spl[1].strip()
        params[k] = v
        print('Set %s = %s' % (k, v))
    except:
        print('Failed')
    # Print message
    print('Current Settings:')
    for k in params.keys():
        print("  %s = %s" % (k, params[k]))

    rsp = raw_input(message)    

### Start Communication
if params['comtype'] in ['serial']:
    # Open serial
    com = serial.Serial(params['device'],int(params['baud']),timeout=0.02)
    print('Serial set up with %s' % params['device'])
else:
    # Open telnet
    com = telnetlib.Telnet(params['host'], params['port'])
    print('Telnet set up to %s' % params['host'])
# Print CRLF twice - print result
time.sleep(0.1)
com.write('\n\r\n\r')
print( galread() )

### Sent and receive test
com.write('MG @IN[1]\n\r')
time.sleep(0.1)
print('Response to MG @IN[1] is = <%s>' % galread())
response = raw_input("Press Enter to continue (Ctrl-c to abort)")

### Send setup commands
print('Sending Setup = %s' % params['setup_commands'])
com.write(params['setup_commands'])
time.sleep(0.1)
print('Return = %s ' % galread())

### Send start commands

### Prepare time and data strings
#   Time0 is start time
#   Time1 is end time (from duration)
#   TimeS is end of next step time
#   TimeP is end of next pause time
#   if TimeS>TimeP -> we're during a step
#   if TimeP>TimeS -> we're during a pause
time0 = time.time()
time1 = time0 + float(params['duration'])
timeS = time0 - 1 # Starting with a pause
timeP = time0 + float(params['pausetime'])
stepcount = 0 # number of steps done

### Get list of parameters
vlist = params['data_vals'].split()

### Open file
outf = file(params['logfile'], 'wt')
s = 'Time\t'
s += string.join(['%s' % s for s in vlist],'\t')
outf.write(s+'\n')

### Get data
while time.time() < time1:
    # Create and Send request
    stext = 'MG %s' % vlist[0]
    for s in vlist[1:]:
        stext += ',' + s
    stext += '\n\r'
    com.write(stext)
    # Wait
    time.sleep(0.02)
    # Collect response
    rtext = galread()
    rfull = ''
    while len(rtext) > 0:
        rfull += rtext
        rtext = galread()
    rarr = rfull.split();
    rarr = rarr[:-1]
    # Print Values
    s = 'Time = %.2f' % (time.time() - time0 - 0.1)
    for i in range(len(rarr)):
        s += '\t%s = %s' % (vlist[i], rarr[i])
    print(s)
    # Add values to file
    s = '%.2f' % (time.time() - time0 - 0.1)
    for i in range(len(rarr)):
        s += '\t%s' % rarr[i]
    outf.write(s+'\n')
    # Check if done with steps
    if stepcount == int(params['stepn']):
        continue
    # Check if pause is over, send start command
    if time.time() > timeP and timeP > timeS:
        # Update times
        timeS = timeP + float(params['steptime'])
        # Send start command
        print('Sending Start = %s' % params['start_commands'])
        com.write(params['start_commands'])
        time.sleep(0.1)
        print('Return = %s ' % galread())
        # Continue
        continue
    # Check if step is over, step stop command
    if time.time() > timeS and timeS > timeP:
        # Update time / count
        timeP = timeS + float(params['pausetime'])
        stepcount += 1
        # Send stop command
        print('Sending Stop = %s' % params['stop_commands'])
        com.write(params['stop_commands'])
        time.sleep(0.1)
        print('Return = %s ' % galread())
        # Continue
        continue

### Send stop commands
print('Sending %s' % params['stop_commands'])
com.write(params['stop_commands'])
time.sleep(0.1)
print('Return = %s ' % galread())

### Close communication
com.close()

### Close file
outf.close()

### Bye Bye
print("That's all Folks!")

### Have a key press
# Something like enter command while still doing stuff in background
# Windows Python has msvcrt library with getch() and kbhit() commands
#import termios, fcntl, sys, os, time
#fd = sys.stdin.fileno()

#oldterm = termios.tcgetattr(fd)
#newattr = termios.tcgetattr(fd)
#newattr[3] = newattr[3] & ~termios.ICANON
# add & ~termios.ECHO to get rid of echo
#termios.tcsetattr(fd, termios.TCSANOW, newattr)

#oldflags = fcntl.fcntl(fd, fcntl.F_GETFL)
#fcntl.fcntl(fd, fcntl.F_SETFL, oldflags | os.O_NONBLOCK)

#c = 'a'
#cnt = 0

#while c != 'b':
#    cnt = cnt + 1
#    try:
#        c = sys.stdin.read(1)
#        print ' - got - %s %d' % (c,ord(c))
#    except IOError:
#        print('nada %d' % cnt)
#        time.sleep(0.1)

#termios.tcsetattr(fd, termios.TCSAFLUSH, oldterm)
#fcntl.fcntl(fd, fcntl.F_SETFL, oldflags)
