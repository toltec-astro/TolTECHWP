""" TOLTEC HWP Control Program - Readout Control
    ==============================================
    
"""

helpmsg = """Communicates with the readout program
    exit - shuts down agent

    any galil command is send to the controller"""


# Preparation
import sys
import queue
import traceback
import logging
import time
import serial
import telnetlib
from agentparent import AgentParent

from readout.diagnostics import readout as hwpreadout

class ReadoutAgent(AgentParent):
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
        self.log = logging.getLogger('Agent.' + self.name)
        self.exit = False # Indicates that loop should exit
        self.comm = None # Variable for communication object (serial or Telnet)
                         # None if connection closed, 1 if open but simulgalil=1


    def start(self):
        """ Function to start the HWP readout collecting
        """
        response = 'readout started'
        hwpreadout(self.config)
        return response

    def stop(self):
        """ Function to start the HWP readout collecting
        """
        msg_to_send = 'stop'
        response = 'readout stopped'
        return response
    
    def __call__(self):
        """ Object call: Runs a loop that runs forever and forwards
            user input.
        """

        # endless loop
        while not self.exit:


            # wait for commmand/task, otherwise empty task var
            datainterval = 1 
            try: #float(self.config['galil']['datainterval'])
                task, respqueue = self.comqueue.get(timeout = datainterval)
                task = task.strip()
            except queue.Empty:
                task = ''
            if len(task):
                self.log.debug('Got task <%s>' % task)


            retmsg = ''
            # starts the data collection
            if 'start' in task.lower():
                response = self.start()
                retmsg = response
            # ends the data collection
            elif 'stop' in task.lower():
                response = self.stop()
                retmsg = response
            # exit
            elif 'exit' in task.lower():
                self.exit = True
            # help
            elif 'help' in task.lower():
                retmsg = helpmsg
            if len(retmsg):
                respqueue.put("%s: %s" % (self.name, retmsg))
        
        ### on exit close connection
        self.comm.close()