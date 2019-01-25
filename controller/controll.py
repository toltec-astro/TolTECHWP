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
    
    Process: The programs has interfaces (user, web, port) which send
        tasks to agents (galil, reader). Each interface and agent has its
        own thread. Each agent has a input task queue where it gets
        a string for the task and a queue to respond to (optional).
        Generally interfaces expect one response from an agent for each task.

    Author: Marc Berthoud - mgb11@cornell.edu

    Questions:
    * Should I handle agents which don't respond differently?
    * Standard way to handle agents which take long time to respond?

    2DO:
    ./ Make this text
    * First version
      ./ Copy text from HAWC autoreduce (make a queue from sample interface to galil)
      ./ Make parent interface and parent agent test with queues and messages
      ./ Make main code with function for interface and for galil (just this file)
      ./ Make interface and agent parents own files - look at HAWC files first
      * Make interface user child (make all die on it) - look at HAWC inter and blimp first
        * Enter empty string -> query queue for all lost messages
        * Status: lists names of registered agents
      * Run tests with config file library
      * Add loading config file use it for greeting in user interface
    * Logging:
      * Look at HAWC log receiver for port
      * Make logging message receiver (with queue for stop? but no response)
      * Fill logging message receiver (listen to port) - make functions to use it
      * Add logging messages from user interface (query and response)
    * Galil:
      * Make interface to talk to galil
      * Make full galil interface loop (look at code from Steve on HAWC)
      * Add initialization and regular comcheck (with warning if lost signal)
    * Readout:
      * Make list of commands
      * Make command receiver which sends commands to the readout
    * Telecscope Control System
      * Make list of commands
      * Add interface for telescope control system
    * Idea (optional but maybe useful for debug): webserver interface
      * For one user only
      * For multiple users (could also do slackbot)

"""

### Preparation

# Imports
import sys
import queue
import time
import logging
import threading
from distutils.command.config import config
from agentparent import AgentParent
from interparent import InterParent

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
    time.sleep(5)
    inboss.queue.put('Master: Hi')
    time.sleep(2)
    agresp.queue.put(('Do It',inboss.queue))
    time.sleep(5)
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
