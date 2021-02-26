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
    disable - disables watchdog
    clear - clears any raised problems"""

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
                self.galilskip = 0
                self.presskip = 0
                self.tempskip = 0
                self.presslast = float(self.config[self.name.lower()]['plimit'])+1
                self.templast = float(self.config[self.name.lower()]['tlimit'])-1
                retmsg = 'Flags cleared'
            elif 'status' in task.lower():
                retmsg = "Watchdog Status: "
                if self.enabled: retmsg+="Enabled"
                else: retmsg+="Disabled"
                retmsg += "\n  Galil Response = <%s>" % self.galilast
                if self.galilskip: retmsg += ", missing %d responses" % self.galilskip
                retmsg += "\n  Pressure = %f" % self.presslast
                if self.presskip: retmsg += ", missing %d responses" % self.presskip
                retmsg += "\n  Temperature = %f" % self.templast
                if self.tempskip: retmsg += ", missing %d responses" % self.tempskip
                if self.shutdown: retmsg += "\n Shutdown Active"
                else: retmsg += "\n Shutdown Off"
            elif len(task):
                retmsg = "Invalid Command <%s>" % task
            # Send return message
            if len(retmsg):
                self.log.debug("sending: <%s>" % retmsg)
                respqueue.put("%s: %s" % (self.name, retmsg))
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
                print(resp)
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
            sub = subprocess.Popen(['tail','-n','20','/data/toltec/readout/sensors/data'],
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            rawsensors, stderr = sub.communicate()
            # Go through lines, search for values
            pnew = -1
            tnew = -1
            for l in rawsensors.decode().split('\n'):
                #print(l)
                try:
                    if '10\t' in l[:3]:
                        pnew = float(l.split()[1])
                    if '0\t' in l[:2]:
                        tnew = float(l.split()[1])
                except:
                    self.log.warn("Couldn't read sensor line <%s>" % l)
            # Update values
            self.presskip += 1
            self.tempskip += 1
            if pnew >= 0:
                self.presslast = pnew
                self.presskip = 0
            if tnew >= 0:
                self.templast = tnew
                self.tempskip = 0
            ### Decide if shutdown needed (only if on)
            # Compare values
            if self.presslast < float(self.config[self.name.lower()]['plimit']):
                if not self.shutdown:
                    self.log.warn('Shutting down due to low pressure P=%f' % self.presslast)
                self.shutdown = True
            if self.templast > float(self.config[self.name.lower()]['tlimit']):
                if not self.shutdown:
                    self.log.warn('Shutting down due to high temperature T=%f' % self.templast)
                self.shutdown = True
            # Shut down if unable to read values
            if self.presskip > 3:
                if not self.shutdown:
                    self.log.warn('Shutting down due to missing P values')
                self.shutdown = True
            if self.tempskip > 3:
                if not self.shutdown:
                    self.log.warn('Shutting down due to missing T values')
                self.shutdown = True
            # Send requests (only if on)
            if self.shutdown and self.enabled:
                self.sendtask('galil AB')
            # Sleep
            time.sleep(1.0)
