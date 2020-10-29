""" Control Program - Interface parent
    ==================================
    
    This is the parent object for all interfaces for the
    control program.
    
    Interfaces are responsible for communication between
    the parts of the program acting on the hardware (agents)
    and outside clients (such as humans at terminal or remote
    computer).
    
    Interfaces are responsible for communicating outside the
    program. Agents are registered by giving the interface
    a set of agent name and queue to communicate to the agent.
    Each communications consists of a message and a queue for
    the agent to put response messages.
    
    Interfaces are callable objects to be started as a thread.
    
    InterParent has a call function which will sent sample
    messages to all registered agents.
"""

import queue
import logging
import random

class InterParent():
    """ Interface Parent Object: Sends messages to agents and handles
        optional responses from agents.
    """
    def __init__(self, config, name=''):
        """ Constructor: Set up variables
        """
        self.name = name
        self.respqueue = queue.Queue() # Queue object for responses
        self.config = config # configuration
        self.agents = {} # dictionary for agent queues (use addagent)
        self.log = logging.getLogger('Interface.'+self.name)
        self.exit = False # Indicates if loop should exit
        
    def __call__(self):
        """ Object call: Runs a loop that runs forever and generates
            tasks for agents.
        """
        # Loop
        while not self.exit:
            # get response (try again every 10s)
            try:
                resp = self.respqueue.get(timeout=3)
                self.log.debug('Got response <%s>' % resp)
            except queue.Empty:
                resp = ''
            if 'exit' in resp.lower():
                self.exit = True
            if random.random() < 0.3 and len(resp) == 0 :
                for a in self.agents:
                    self.log.debug('Telling %s to work' % a)
                    self.sendtask(a+' Work!')
            if random.random() < 0.1 and len(resp) == 0 :
                self.sendtask('Noone Do Something!')
    
    def addagent(self, agent):
        """ AddAgent: registers an agent with the interface
        """
        self.agents[agent.name.strip().lower()] = agent.comqueue
        
    def sendtask(self, task):
        """ Forwards tasks in the format "Agent Task"
        """
        # Get agent and task
        try:
            agent, tsk = task.split(' ',1)
        except:
            self.respqueue.put('Invalid Task: %s' % task)
            return
        tsk = tsk.strip()
        agent = agent.strip().lower()
        # Check if agent is registered else send error to queue
        if not agent in self.agents:
            self.respqueue.put('Invalid Agent: %s' % agent)
        else:
            # Send task to agent
            self.agents[agent].put((tsk, self.respqueue))

