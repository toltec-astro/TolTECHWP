""" Control Program - Operator Parent
    =================================
    
    This is the parent object for all operators for the
    control program.
    
    Operators combine capabilities of agents and interfaces.
    They get commands or requests from interfaces but at the
    same time send such communications to agents. An operator
    can send a command based on an incoming command or by
    itself (internally triggered).
    
    Operators are callable objects to be started as a thread.
    
    OperatorParent has a call function which will copy
    incoming messages to the response queue unless an
    'Echo' agent exists in which case messages are passed to
    this agent and responses are sent back.
"""

import time
import queue
import logging
from interparent import InterParent
from agentparent import AgentParent

class OperatorParent(InterParent, AgentParent):
    """ Agent Parent Object: Receives messages (task, responsequeue pairs)
        and (optionally) returns answer.
    """
    def __init__(self, config, name = ''):
        """ Constructor: Set up variables
        """
        self.name = name
        self.comqueue = queue.Queue() # Queue object for queries
        self.respqueue = queue.Queue() # Queue objects for responses
        self.config = config # configuration
        self.agents = {}
        self.log = logging.getLogger('Operator.'+self.name)
        self.exit = False # Indicates that loop should exit
        
    def __call__(self):
        """ Object call: Run a loop that runs forever and handles tasks
        """
        # Loop
        while not self.exit:
            # Look for task
            task, respqueue = self.comqueue.get()
            self.log.debug("Agent %s - Handling Task <%s>" % (self.name, task) )
            # Check if it's exit
            if 'exit' in task.lower():
                self.exit = True
            # If Echo agent exists, send task to it
            elif 'echo' in self.agents:
                self.log.debug('%s: Sending %s to Echo' % (self.name, task))
                self.sendtask('echo %s' % task)
                # Get all responses
                respfull = ''
                resp = '%s: got responses' % self.name
                while len(resp):
                    respfull += resp + '\n  '
                    try:
                        resp = self.respqueue.get(timeout = 3)
                    except queue.Empty:
                        resp = ''
                    if len(resp):
                        self.log.debug('Got response <%s>' % resp)
                # Send all responses back
                respqueue.put(respfull.strip())
            # Else just send task string back
            else:
                respqueue.put("%s: Doing %s" % (self.name, task))
                time.sleep(1.0)
                respqueue.put("%s: %s is done" % (self.name, task))
