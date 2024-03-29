""" TolTEC HWP Control Program - Galil controller interface
    =======================================================
    
    This interface object is the interface to the Galil
    motor controller. It is a child object of interface.
    
"""

helpmsg = """Galil Agent: Communicates to Galil motor controller
    exit - shuts down agent
    open - (re)opens connection to galil with current config settings
    close - closes connection to galil
    conf - sends the config commands to the galil
    init - initializes the motor (do this after config)
    start - start currently set motor movement
    stop - stop motor movement using deceleration
    off - shuts motor off
    abort - interrupts HWP motion and shuts down the motor
    rotate [frequency] - sets rotation speed
    move [angle] - move by specified angle (only works if motor is stopped)
    goto [angle] - go to certain position (requires recent index)
    index - instructs the galil to search the index location (this sets the galil to zero)
    status - current system information
    any other command is directly send to the controller"""


# Preparation
import sys
import queue
import traceback
import logging
import time
import serial
import telnetlib
import math
from agentparent import AgentParent

# GalilAgent Object
class GalilAgent(AgentParent):
    """ User Interface object: Receives commands from
        prompt and prints responses on screen.
    """

    def __init__(self, config, name = ''):
        """ Constructor: Set up variables
        """
        # Call the parent constructor
        
        self.name = name
        self.comqueue = queue.Queue() # Queue object for querries
        self.config = config # configuration
        self.log = logging.getLogger('Agent.'+self.name)
        self.exit = False # Indicates that loop should exit
        self.comm = None # Variable for communication object (serial or Telnet)
                         # None if connection closed, 1 if open but simulgalil=1
        self.indextime = 0.0 # Unix timestamp of last index operation
        self.cntperev = 1 # encoder counts per revolution / is set below
        self.pos = 0 # Position
        self.speed = 0 # Velocity
        self.motoroff = 0 # Flag is motor is unpowered i.e. response of MOA
    
    def read(self):
        """ Function to read from galil (depends on readout method)
        """
        if isinstance(self.comm, serial.Serial):
            # Decode from bytes then strip
            return self.comm.read(1000).decode().strip()
        elif isinstance(self.comm, telnetlib.Telnet):
            return self.comm.read_eager()
        elif self.comm == 1 and len(self.config['galil']['simulategalil']):
            return 'sim'
        else:
            self.log.warn("Read failed - not connected")
        
    def write(self, message, debug=True):
        """ Function to write to galil
        """
        if debug: self.log.debug('Write: %s' % message)
        if self.comm != 1 and self.comm != None:
            self.comm.write(message.encode())
            
    def command(self, command):
        """ Function to send commands and returns response.
            Uses write and read, checks if comm is open.
        """
        if self.comm != None:
            self.write((command+'\n\r'))
            time.sleep(float(self.config['galil']['waittime']))
            retmsg = self.read()
            self.log.debug('Read: %s' % retmsg)
        else:
            retmsg = "Unable to send command, no open connection"
        return retmsg

    def open(self):
        """ Function to (re)connect to galil. Returns a message.
        """
        # Close any open connection
        if not self.comm == None:
            if not self.comm == 1:
                self.comm.close()
            self.comm = None
        # Open conection
        if len(self.config['galil']['simulategalil']):
            self.comm = 1
            retmsg = 'Simulated Galil connection open'
        elif self.config['galil']['comtype'] in ['serial']:
            # Open serial
            device = self.config['galil']['device']
            baud = int(self.config['galil']['baud'])
            try:
                self.comm = serial.Serial(device,baud,timeout=0.022)
                retmsg = 'Serial connection set up with %s' % device
            except Exception as err:
                traceback.print_tb(err.__traceback__)
                retmsg = 'Serial connection to %s FAILED' % device
                self.log.warn(retmsg)
                self.log.warn('   Message = %s' % str(err))
        elif self.config['galil']['comtype'] in ['telnet']:
            # Open telnet
            host = self.config['galil']['host']
            port = self.config['galil']['port']
            try:
                self.comm = telnetlib.Telnet(host,port,1.0)
                retmsg = 'Telnet connection set up with %s' % host
            except Exception as err:
                traceback.print_tb(err.__traceback__)
                retmsg = 'Telnet connection to %s FAILED' % host
                self.log.warn(retmsg)
        else:
            self.log.warn("Unable to connect, invalid ComType = %s" % 
                          self.config['galil']['comtype'])
        # Reset indextime
        self.indextime = 0.0
        return retmsg
    
    def __call__(self):
        """ Object call: Runs a loop that runs forever and forwards
            user input.
        """
        ### Setup
        self.cntperev = float(self.config['galil']['cntperev'])
        self.pos = 0
        ### Command loop
        while not self.exit:
            # Look for task
            datainterval = float(self.config['galil']['datainterval']) # get value in case it changed
            try:
                task, respqueue = self.comqueue.get(timeout = datainterval)
                task = task.strip()
            except queue.Empty:
                task = ''
            if len(task):
                self.log.debug('Got task <%s>' % task)
            ### Handle task
            #print(repr(self.comm))
            retmsg = ''
            # Exit
            if 'exit' in task.lower():
                self.exit = True
            # Help message
            elif 'help' in task.lower():
                retmsg = helpmsg
            # Connect
            elif 'open' in task.lower():
                retmsg = self.open()
            # Disconnect
            elif 'close' in task.lower():
                if self.comm != None:
                    if self.comm != 1:
                        self.comm.close()
                    self.comm = None
                    retmsg = 'Connection closed'
                else:
                    retmsg = 'Already closed'
            # configure
            elif 'conf' in task.lower():
                retmsg = self.command('RS')
                time.sleep(0.5)
                retmsg += self.command(self.config['galil']['confcomm'])
            # initialize
            elif 'init' in task.lower():
                retmsg = self.command(self.config['galil']['initcomm'])
            # start
            elif 'start' in task.lower():
                retmsg = self.command(self.config['galil']['startcomm'])
            # stop
            elif 'stop' in task.lower():
                retmsg = self.command(self.config['galil']['stopcomm'])
            # stop
            elif 'off' in task.lower():
                retmsg = self.command(self.config['galil']['offcomm'])
            # abort
            elif 'abort' in task.lower():
                retmsg = self.command(self.config['galil']['abortcomm'])
            # index
            elif 'index' in task.lower():
                retmsg = self.command(self.config['galil']['indexcomm'])
                self.indextime = time.time()
            # rotate by Hz
            elif 'rotate' in task.lower():
                print(task)
                freq = float(task[6:].strip())*self.cntperev
                comm = 'JGA=%d' % math.floor(freq)
                retmsg = self.command(comm)
            # move by angle
            elif 'move' in task.lower():
                distance = float(task[5:].strip())*self.cntperev/360.0
                comm = 'PRA=%d' % math.floor(distance)
                retmsg = self.command(comm)
            # goto angle
            elif 'goto' in task.lower():
                # check if reasonably recent index time is available
                if time.time() - self.indextime < 10*3600:
                    # get angle and get angle to travel
                    angle = float(task[5:].strip())*self.cntperev/360.0
                    deltangle = angle-self.pos
                    while deltangle > self.cntperev/2:
                        deltangle -= self.cntperev
                    while deltangle < -self.cntperev/2:
                        deltangle += self.cntperev
                    # send the command
                    comm = 'PRA=%d' % math.floor(deltangle)
                    retmsg = self.command(comm)
                else:
                    retmsg = "Error: Outdated Index time, INDEX the rotator before GOTO"
            # Status
            elif 'status' in task.lower():
                if self.comm != None:
                    retmsg = 'Connected to the controller'
                    retmsg += '\n       Position = %.0f counts  = %.3f deg' % (self.pos, self.pos*360/self.cntperev)
                    retmsg += '\n       Speed = %.0f counts/s  = %.5f Hz' % (self.speed, self.speed/self.cntperev)
                    retmsg += '\n       MotoOff = %d' % self.motoroff
                    retmsg += '\n       Last index %.2f hours ago' % ((time.time()-self.indextime)/3600.0)
                else:
                    retmsg = "No command connection to the controller"
            # Else it's a galil command
            elif len(task):
                retmsg = self.command(task)
            # No task -> Get data if connection is on
            elif self.comm != None:
                # Make command
                vlist = self.config['galil']['datavals'].split()
                stext = 'MG %s' % vlist[0]
                for s in vlist[1:]: stext += ',' + s
                stext += '\n\r'
                # Send - wait - return
                self.write(stext, debug=False)
                time.sleep(float(self.config['galil']['waittime']))
                rtext = self.read().strip()
                # Log the result
                self.log.debug('Data: %s' % rtext)
                try:
                    # update velocity, position and motor status
                    datas = [w.strip() for w in rtext.split()]
                    ind = vlist.index('_TPA')
                    self.pos = float(datas[ind])
                    ind = vlist.index('_TVA')
                    self.speed = float(datas[ind])
                    ind = vlist.index('_MOA')
                    self.motoroff = float(datas[ind])
                except:
                    # Warning message if couldn't read all data
                    self.log.warn('Error reading TPA, TVA or MOA')
            # Send return message
            if len(retmsg):
                respqueue.put("%s: %s" % (self.name, retmsg))
        ### On exit close connection
        self.comm.close()
        
