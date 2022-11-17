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
    * Watchdog:
      * Set up for getting pressure information -> Stop motor if below 80psi
        * where to find channel which is pressure? A: Not in config, it's hardcoded.
      * check clear function
      * Set up for getting temperature information -> Stop motor if above 50C
      * Use fake file to check if values trigger shutdown in testing mode
      * Set up for getting info from galil (if connected and motor is moving or not)
        * What do I need from galil? Know if it works, know if error, MOA and speed
        * Possible to get error flags from galil and respond to them?
    * Galil:
      * Make status variables and command (speed, 
      * Make sure init happens 
      * Error handling, reconnect
      * Make abort function
      * Check number of command and report error if missing number of : or reports
        error if ?
      * Add initialization check and regular comcheck (with warning if lost signal)
      * Regularly set galil internal variable for galil watchdog to ABort after
        set time.
    * Readout:
      * Make list of commands
      * Make command receiver which sends commands to the readout
    * Telecscope Control System
      * Make list of commands
      * Add interface for telescope control system
    * Server Interface:
      * Make object and thread
      * Make simple server thread (responds with galil status)
      * Make post message window and get response (and timeout) and rest of queue
      * Add to list last log messages (make autoupdate, use HAWC code)
    * Updates:
      * Make sure galil response is properly formatted
      * Parse galil response for errors and report error messages
      * Report detailed commands sent to galil in interface (not only log)
    * Idea (optional but maybe useful for debug): webserver interface
      * For one user only
      * For multiple users (could also do slackbot)
      * All interfaces should print messages at info and higher level
        (each inter collects last 10 messages in FIFO queue - purge when printed out)

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
#from operatorparent import OperatorParent
from userinterface import InterUser
from socketinterface import InterSocket
from webinterface import InterWeb
from loggercontrol import LoggerControl
from galilagent import GalilAgent
from configagent import ConfigAgent
from watchdogoper import WatchdogOper
from readoutagent import ReadoutAgent

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
    inweb = InterWeb(config,'Web')
    opwat = WatchdogOper(config,'Watch')
    agconf = ConfigAgent(config, 'Conf')
    agresp = AgentParent(config, 'Echo')
    aggal = GalilAgent(config, 'Galil')

    readou = ReadoutAgent(config, 'Readout')

    # Register agents with interfaces
    for agent in [agresp, aggal, agconf, opwat, readou]:
        inusr.addagent(agent)
        insock.addagent(agent)
        inweb.addagent(agent)
        opwat.addagent(agent)
        insock.addagent(agent)

    # Run items as threads (as daemons such that they shut down on exit)
    threads = {}
    for item in [logctrl, agresp, aggal, agconf, opwat, inusr, insock, inweb, readou]:
        thread = threading.Thread(target = item)
        thread.daemon = True
        thread.start()
        threads[item.name] = thread
    # Wait and do some stuff
    time.sleep(2)
    agresp.comqueue.put(('Do It',inusr.respqueue))
    # Join with User Interface thread
    threads['User'].join()

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

""" Completed Tasks:
    * Make this text
    * First version
      * Copy text from HAWC autoreduce (make a queue from sample interface to galil)
      * Make parent interface and parent agent test with queues and messages
      * Make main code with function for interface and for galil (just this file)
      * Make interface and agent parents own files - look at HAWC files first
    * Make interface user child (make all die on it) - look at HAWC inter and blimp first
      * Run tests with config file library: how to open, access and print config
      * Add loading config file use it for greeting in user interface
      * Get help file text in config file
      * Make conf command to list config file
    * Logging:
      * Make logging message receiver (with queue for stop? but no response)
      * Fill logging message receiver (listen to port) - make functions to use it
      * Add logging messages from agents and interfaces (query and response)
    * Galil:
      * Use code from galilcomm.py, look at code from HAWC irc
      * Set up file and connection configuration
      * Make galilcom and reconnect functions (both use self.comm)
      * Set up connection and forward messages
      * Allow (re)connect and disconnect command
      * Require exit command for disconnect at end
      * Make interface to talk to galil
      * Make full galil interface loop (look at code from Steve on HAWC)
    * Watchdog:
      * Rename comqueue and respqueue (command queue for agents, response queue for interfaces)
      * Make operator - interface and agent at the same time 
      * Set it up and test operator (be an echo and periodically send messages to galil)
      * Set up variable, then status, on and off commands
      * Set up communication with galil (check MOA)
    * Socket interface

"""
