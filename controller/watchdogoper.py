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
        # Variables for Pressure
        self.presslast = 100.0
        self.presskip = 0
        # Variables for Motor status
        self.galilast = ''
        self.galilskip = 0
    
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
                
            # Decide if shutdown needed (only if on)
            # Send requests (only if on)
            if self.enabled:
                self.sendtask('galil MOA')
            # Sleep
            time.sleep(2.0)