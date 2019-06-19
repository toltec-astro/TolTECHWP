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
    ./ First version
      ./ Copy text from HAWC autoreduce (make a queue from sample interface to galil)
      ./ Make parent interface and parent agent test with queues and messages
      ./ Make main code with function for interface and for galil (just this file)
      ./ Make interface and agent parents own files - look at HAWC files first
      ./ Make interface user child (make all die on it) - look at HAWC inter and blimp first
      ./ Run tests with config file library: how to open, access and print config
      ./ Add loading config file use it for greeting in user interface
      ./ Get help file text in config file
      ./ Make conf command to list config file
    ./ Logging:
      ./ Make logging message receiver (with queue for stop? but no response)
      ./ Fill logging message receiver (listen to port) - make functions to use it
      ./ Add logging messages from agents and interfaces (query and response)
    * Galil:
      ./ Use code from galilcomm.py, look at code from HAWC irc
      ./ Set up file and connection configuration
      ./ Make galilcom and reconnect functions (both use self.comm)
        * checks number of commands and throws error if missing number of
          commands or ? received
        * to test: generate both of these with wrong commands
      ./ Set up connection and forward messages
      * Error handling, reconnect 
      ./ Allow (re)connect and disconnect command
      ./ Require exit command for disconnect at end
      ./ Make interface to talk to galil
      ./ Make full galil interface loop (look at code from Steve on HAWC)
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
import os
import sys
import queue
import time
import logging
import threading
import configparser
from distutils.command.config import config
from agentparent import AgentParent
from interparent import InterParent
from userinterface import InterUser
from socketinterface import InterSocket
from loggercontrol import LoggerControl
from galilagent import GalilAgent
from configagent import ConfigAgent

def hwpcontrol(confilename):
    """ Run the HWP control
    """
    # Load config file
    config = configparser.ConfigParser()
    config.read(confilename)
    # Make interfaces and agents
    logctrl = LoggerControl(config, 'Log')
    inusr = InterUser(config, 'User')
    insock = InterSocket(config,'Socket')
    agconf = ConfigAgent(config, 'Conf')
    agresp = AgentParent(config, 'Echo')
    aggal = GalilAgent(config, 'Galil')
    # Register agents with interfaces
    inusr.addagent('Echo',agresp.queue)
    inusr.addagent('Galil',aggal.queue)
    inusr.addagent('Conf',agconf.queue)
    insock.addagent('Echo',agresp.queue)
    insock.addagent('Galil',aggal.queue)
    insock.addagent('Conf',agconf.queue)
    # Run them as threads (both as daemons such that they shut down on exit)
    logth = threading.Thread(target = logctrl)
    logth.daemon = True
    agrth = threading.Thread(target = agresp)
    agrth.daemon = True
    aggth = threading.Thread(target = aggal)
    aggth.daemon = True
    agcon = threading.Thread(target = agconf)
    agcon.daemon = True
    inuth = threading.Thread(target = inusr)
    inuth.daemon = True
    insth = threading.Thread(target = insock)
    insth.daemon = True
    logth.start()
    agrth.start()
    inuth.start()
    aggth.start()
    agcon.start()
    insth.start()
    # Wait and do some stuff
    time.sleep(2)
    agresp.queue.put(('Do It',inusr.queue))
    # Join with User Interface thread
    inuth.join()

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
