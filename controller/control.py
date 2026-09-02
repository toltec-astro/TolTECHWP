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

    * Galil / General updates
      ./ Add read command in the galil
      ./ In galilagent and readoutagent, make sure correct commands are sent by
         only checking required number characters of new command.
    * Socket interface update
      # Requirements: persisent connection, answer every request < 0.1s
      # Setup: builtin socket library, no authentication, single connection at a time
        * Make sure 'exit' command works through socket interface
      ./ Plan with LLM for consisten socket, single communication channel
        --> Implement it
      ./ Single command test (use nc)
      ./ Common Exit
        Add exitEv = threading.Event() to controll.py then pass it to every object
          interfaces can set the event and all objects close when it's set
          then use exitEv.wait() instead of UserInterface.join() and wait 5s after that
        ./ Update galil program to close connection if exit is called and connection is open
      * Tests
        ./ Test single command, disconnect,
        ./ Test multiple commands, disconnect, reconnect, more commands,
        ./ Test single command, cut just after sending, see if new reestablish works
        * Test read (set up sim command with index that delays answer)
        ./ Test exit with userinterface or socketinterface make sure both work
    
    Ideas for LATER:
    * Watchdog: (there is currently no interface to get pressure and temperatures)
      * Set up for getting pressure information -> Stop motor if below 80psi
        * where to find channel which is pressure? A: Not in config, it's hardcoded.
      * check clear function
      * Set up for getting temperature information -> Stop motor if above 50C
      * Use fake file to check if values trigger shutdown in testing mode
      * Set up for getting info from galil (if connected and motor is moving or not)
        * What do I need from galil? Know if it works, know if error, MOA and speed
        * Possible to get error flags from galil and respond to them?
      * Option to shut down gracefully: stop rotation, turn off, disconnect
    * Galil:
      * Error handling, reconnect: 
        * Error message if a '?' is returned
        * If write / read when self.comm==None --> return error
        * Check number of command and report error if missing number of : or reports
          error if ?
      * Add initialization check and regular comcheck (with warning if lost signal)
      * Regularly set galil internal variable for galil watchdog to ABort after
        set time.
      * Add internal state machine for galil to check when executing commands
        * Have internal string variable configured -> initialized -> moving / stopped
    * Telecscope Control System ???
      * Make list of commands
      * Add interface for telescope control system
    * Readout:
      * Make list of commands
      * Make command receiver which sends commands to the readout
    * Web Server Interface:
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
        
    DONE:
    * Web Server Interface:
      ./ Make object and thread
      ./ Make simple server thread (responds with galil status)
      ./ Make post message window and get response (and timeout) and rest of queue

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
#from watchdogoper import WatchdogOper
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
    #opwat = WatchdogOper(config,'Watch')
    agconf = ConfigAgent(config, 'Conf')
    #agresp = AgentParent(config, 'Echo') - taken out add to lists below to reactivate
    aggal = GalilAgent(config, 'Galil')

    readou = ReadoutAgent(config, 'Readout')

    # Register agents with interfaces
    for agent in [aggal, agconf, readou]: #opwat]:
        inusr.addagent(agent)
        insock.addagent(agent)
        inweb.addagent(agent)
        #opwat.addagent(agent)

    # Run items as threads (as daemons such that they shut down on exit)
    threads = {}
    exitEv = threading.Event()
    for item in [logctrl, aggal, agconf, inusr, insock, inweb, readou ]: #, opwat]:
        item.exit = exitEv
        thread = threading.Thread(target = item)
        thread.daemon = True
        thread.start()
        threads[item.name] = thread
    # Wait and do some stuff
    time.sleep(2)
    #agresp.comqueue.put(('Do It',inusr.respqueue))
    # Wait until exit Event is set then wait 2s for everyone to exit
    exitEv.wait()
    time.sleep(2)

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

    For writing log file for the galil 2026-May
    ./ Review: 
      ./ Look at new simonsobs code for good ideas
    ./ Cleanup
      ./ Clean out ideas for later <-> DONE below
      ./ comment out watchdog operator 
    ./ Update userinterface
      ./ Remove doit - done
      ./ Use prompt_toolkit and patch_stdout
      ./ Fix problem with logger and patch_toolkit by writing custom loghandler
    ./ Galil interface
      ./ Update command to use splitting as in my SimonsObs code
      ./ Get data / improve handling of missing data due to busy controller (set all data to 0)
      ./ Convert and print data into file

"""
