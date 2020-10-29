""" Control Program - Agent Parent
    ==============================
    
    This is the parent object for all agents for the
    control program.
    
    Agents get command or request messages from interfaces
    and act upon / respond to them. Each message consists
    of a string (the message) and a queue to respond to.
    
    Agents are callable objects to be started as a thread.
    
    AgentParent has a call function which will copy
    incoming messages to the response queue.
"""

import time
import queue
import logging

class AgentParent():
    """ Agent Parent Object: Receives messages (task, responsequeue pairs)
        and (optionally) returns answer.
    """
    def __init__(self, config, name = ''):
        """ Constructor: Set up variables
        """
        self.name = name
        self.comqueue = queue.Queue() # Queue object for querries
        self.config = config # configuration
        self.log = logging.getLogger('Agent.'+self.name)
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
            # Else just send task string back
            else:
                respqueue.put("%s: Doing %s" % (self.name, task))
                time.sleep(1.0)
                respqueue.put("%s: %s is done" % (self.name, task))
