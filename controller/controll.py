""" TolTEC HWP Control Program - Controller
    =======================================
    
    This program manages the TolTEC half-wave plate rotator. It has
    the following functions:
    - Provide a user interface to allow local access to all HWP functions
    - Run the logger to receive logging messages
    - Communicate with the Galil motor controller
    - Handle commands from the telescope control system
    - Communicate with the readout program.
    
    Usage: python controll.py conigfile.cfg
           configfile.cfg is the configuration file to be used
    
    Author: Marc Berthoud - mgb11@cornell.edu

    2DO:
    ./ Make this text
    * First version
      * Copy text from HAWC autoreduce (make a queue from sample interface to galil)
      * Make main code with function for interface and for galil (just this file)
      * Make it work (with queue being passed with message)
      * Make galil and interface parent own files
      * Make interface user child (make all die on it)
        * Enter empty string -> query queue for lost messages
    * Logging:
      * Make logging message receiver (with queue for stop? but no response)
      * Fill logging message receiver (listen to port) - make functions to use it
    * Galil:
      * Make interface to talk to galil
      * Make full galil interface loop (look at code from Steve on HAWC)
    * Readout:
      * Make list of commands
      * Make command receiver which sends commands to the readout
    * Telecscope Control System
      * Make list of commands
      * Add interface for telescope control system
    * Idea (optional but maybe useful for debug): webserver interface

"""

### Preparation

# Imports
import sys
import queue
import time
import logging
import random
import threading
from distutils.command.config import config

class InterParent():
    """ Interface Parent Object: Sends messages to agents and handles
        optional responses from agents.
    """
    def __init__(self, config, name=''):
        """ Constructor: Set up variables
        """
        self.name = name
        self.queue = queue.Queue() # Queue object for responses
        self.config = config # configuration
        self.agents = {} # dictionary for agent queues
        self.exit = False # Indicates if loop should exit
        
    def __call__(self):
        """ Object call: Runs a loop that runs forever and generates
            tasks for agents.
        """
        # Loop
        while not self.exit:
            # get response (try again every 10s)
            try:
                resp = self.queue.get(timeout=2)
            except queue.Empty:
                print("Interface %s: Waiting" % self.name)
                resp = ''
            print('Interface %s - Got response <%s>' % (self.name, resp))
            if 'exit' in resp.lower():
                self.exit = True
            if random.random() < 0.3 and len(resp) ==0 :
                for a in self.agents:
                    print('Interface %s - Telling %s to work' % (self.name,a))
                    self.agents[a].put(('Work!',self.queue))
    
    def addagent(self, name, aqueue):
        """ AddAgent: adds an agent
        """
        self.agents[name] = aqueue
                
class AgentParent():
    """ Agent Parent Object: Receives messages (task, responsequeue pairs)
        and (optionally) returns answer.
    """
    def __init__(self, config, name = ''):
        """ Constructor: Set up variables
        """
        self.name = name
        self.queue = queue.Queue() # Queue object for querries
        self.config = config # configuration
        self.exit = False # Indicates that loop should exit
        
    def __call__(self):
        """ Object call: Run a loop that runs forever and handles tasks
        """
        # Loop
        while not self.exit:
            # Look for task
            task, respqueue = self.queue.get()
            print("Agent %s - Handling Task <%s>" % (self.name, task) )
            # Check if it's exit
            if 'exit' in task.lower():
                self.exit = True
            # Else just send task string back
            else:
                respqueue.put("%s: %s" % (self.name, task))    

def hwpcontrol(config):
    """ Run the HWP control
    """
    # Make interfaces and agents
    inboss = InterParent(config, 'Boss')
    agresp = AgentParent(config, 'Bee')
    inboss.addagent('Bee',agresp.queue)
    # Run them as threads (both as daemons such that they shut down on exit)
    inbth = threading.Thread(target = inboss)
    inbth.daemon = True
    agrth = threading.Thread(target = agresp)
    agrth.daemon = True
    inbth.start()
    agrth.start()
    # Wait
    time.sleep(7)
    inboss.queue.put('Master: Hi')
    time.sleep(3)
    # Send Quit command

if __name__ == '__main__':
    """ Main function for calling command line. Passes the configuration file
        name to the control function
    """
    # Check input
    if len(sys.argv) < 2:
        print("""Usage:
    python controll.py configfile.cfg
where
    configfile.cfg has to be the filepathname for a valid config file
""")
        exit()
    # Get config file name
    Config_FilePathName = sys.argv[1]
    # Call HWP control
    hwpcontrol(Config_FilePathName)
    print("That's All Folks!")

#
