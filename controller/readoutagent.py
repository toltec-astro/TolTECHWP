""" TOLTEC HWP Control Program - Readout Control
    ==============================================
    
"""

helpmsg = """Communicates with the readout program
    start - starts readout process
    stop  - stops readout process

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

import socket
import threading
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
        self.readout_running = False


    def start(self):
        """ Function to start the HWP readout collecting
        """
        response = 'readout started'
        
        return response

    def stop(self):
        """ Function to start the HWP readout collecting
        """
        msg_to_send = 'exit'
        
        readout_listener_port = int(self.config['readout.address']['port'])
        self.send_message(msg_to_send, port=readout_listener_port)
        response = f'readout stopped {readout_listener_port}'
        return response

    def send_message(self, message, ip='127.0.0.1', port=5555):
        sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) # UDP
        sock.sendto(f'{message}\n'.encode('UTF-8'), (ip, port))
        sock.close()
    
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
                self.readout_thread = threading.Thread(target=hwpreadout, args=(self.config,))
                self.readout_thread.start()
                self.readout_running = True
                
                retmsg = response
            # ends the data collection
            elif 'stop' in task.lower():
                response = self.stop()
                self.readout_thread.join()
                retmsg = response
                self.readout_running = False

            # exit
            elif 'exit' in task.lower():
                if self.readout_running is True:
                    retmsg = "can't stop. readout running."
                else:
                    self.exit = True
            # help
            elif 'help' in task.lower():
                retmsg = helpmsg
            if len(retmsg):
                respqueue.put("%s: %s" % (self.name, retmsg))
        