""" TOLTEC HWP Control Program - Watchdog operator
    ==============================================
    
    This operator sends replies to system status messages.
    It regularly querries a list of values and will shut down
    the motor if any values are outside the expected ranges.
    
    If a value is not returned it counts for how long the value
    has been missing and aborts if it can't get information for
    a certain time. 
"""

helpmsg = """Watchdog Operator: Monitors system values
    exit - shuts down operator
    status - report system status
    enable - enables watchdog
    disable - disables watchdog"""

import sys
import queue
import time
import subprocess
from operatorparent import OperatorParent

class WatchdogOper(OperatorParent):
    """ Watchdog operator object: Receives commands and
        monitors system status, shutting down if status
        outside normal parameters
    """
    
    def __init__(self, config, name = ''):
        """ Constructor: Set up variables
        """
        super().__init__(config, name)
        # Flag to enable / disable watchdog
        self.enabled = True
        # Flat to shut down the motor
        self.shutdown = False
        # Variables for Pressure
        self.presslast = 100.0
        self.presskip = 0
        # Variables for Motor status
        self.galilast = ''
        self.galilskip = 0
        # Variables for Temperature
        self.templast = 300.0
        self.tempskip = 0
    
    def __call__(self):
        """ Object call: Runs a loop forever, handles and
            issues querries
        """
        # Setup
        task = ''
        # loop
        while not self.exit:
            ### Check for and handle commands
            try:
                task, respqueue = self.comqueue.get(block = False)
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
            elif 'enable' in task.lower():
                self.enabled = True
                retmsg = 'Enabled'
            elif 'disable' in task.lower():
                self.enabled = False
                retmsg = 'Disabled'
            elif 'clear' in task.lower():
                self.shutdown = False
                self.presskip = 0
                self.tempskip = 0
                self.presslast = self.config['watch']['plimit']+1
                self.templast = self.config['watch']['tlimit']-1
                retmsg = 'Flags cleared'
            elif 'status' in task.lower():
                retmsg = "Watchdog Status: "
                if self.enabled: retmsg+="Enabled"
                else: retmsg+="Disabled"
                retmsg += "\n  Galil Response = <%s>" % self.galilast
                if self.galilskip: retmsg += ", missing %d responses" % self.galilskip
                retmsg += "\n  Pressure = %f" % self.presslast
                if self.presskip: retmsg += ", missing %d responses" % self.presskip
            elif len(task):
                retmsg = "Invalid Command <%s>" % task
            # Send return message
            if len(retmsg):
                self.log.debug("sending: <%s>" % retmsg)
                respqueue.put("%s: %s" % ('watch', retmsg))
            ### Check responses
            # Increase all skips by 1
            self.presskip += 1
            self.galilskip += 1
            # Check responses -> Fill in values
            try:
                resp = self.respqueue.get(block = False)
                self.log.debug("Got response <%s>" % resp)
            except queue.Empty:
                 resp = ''
            while len(resp):
                # Treat response
                if resp.startswith('Galil:'):
                    self.galilskip = 0
                    self.galilast = resp[7:]
                else:
                    self.log.warn("Invalid response: %s" % resp)
                # Get next response
                try:
                    resp = self.respqueue.get(block = False)
                    self.log.debug("Got response <%s>" % resp)
                except queue.Empty:
                     resp = ''
            ### Load sensor data from log file
            # Get sensors data
            sub = subprocess.Popen(['tail', '-n 20', '/data/toltec/readout/sensors/data'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            output, _ = sub.communicate()
            rawsensors = output.decode()

            # Go through lines, search for values
            pnew = 0
            tnew = 0
            for l in rawsensors.split('\n'):
                try:
                    if '9\t' in l[:2]:
                        pnew = float(l.split()[2])
                    if '8\t' in l[:2]:
                        tnew = float(l.split()[2])
                except:
                    self.log.warn("Couldn't read sensor line <%s>" % l)
            # Update values
            self.presskip += 1
            self.tempskip += 1
            if pnew > 0:
                self.presslast = pnew
                self.presskip = 0
            if tnew > 0:
                self.templast = tnew
                self.tempskip = 0
            ### Decide if shutdown needed (only if on)
            # Compare values
            if self.presslast < self.config['watch']['plimit']:
                self.shutdown = True
            if self.templast > self.config['watch']['tlimit']:
                self.shutdown = True
            # Shut down if unable to read values
            if self.presskip > 3: self.shutdown = True
            if self.tempskip > 3: self.shutdown = True
            # Send requests (only if on)
            if self.shutdown and self.enabled:
                self.sendtask('galil MOA')
            # Sleep
            time.sleep(1.0)