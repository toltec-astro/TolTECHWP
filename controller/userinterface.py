""" TolTEC HWP Control Program - User interface
    ===========================================
    
    This interface object is the command line user
    interface. It is a child object of interface.
    
"""

import sys
import queue
import time
from interparent import InterParent
#from __builtin__ import False

class InterUser(InterParent):
    """ User Interface object: Receives commands from
        prompt and prints responses on screen.
    """
    def __call__(self):
        """ Object call: Runs a loop that runs forever and forwards
            user input.
        """
        # Setup
        self.lastagent = ''
        # Loop
        print(self.config['userinterface']['greeting'])
        while not self.exit:
            # Get response and print
            try:
                resp = self.respqueue.get(timeout=0.1)
            except queue.Empty:
                resp = ''
            if len(resp):
                self.log.debug('Got response <%s>' % resp)
                print(resp)
            # Wait for keyboard input
            command = input('> ')
            # If it's status -> print list of all agents
            if 'status' in command.lower()[:7]:
                # Print agents list
                print("List of Agents:")
                for a in self.agents:
                    print("  " + a)
                # Set command to empty to get all messages
                command = ''
            # If it's help -> print help message
            if 'help' in command.lower()[:4]:
                with open(self.config['userinterface']['helpfile'],'rt') as f:
                    print(f.read())
                # Print all help messages
                if 'help all' in command.lower()[:8]:
                    print('== Asking all agents for help: ==')
                    for a in self.agents:
                        self.sendtask(a + ' help')
                # Set command to empty to get all messages
                command = ''
            # If it's config -> print the configuration
            if 'config' in command.lower()[:7]:
                self.config.write(sys.stdout)
                # Set command to empty to get all messages
                command = ''
            # Check if exit
            if 'exit' in resp.lower()[:5] or 'exit' in command.lower()[:10]:
                self.exit = True
                # Set command to empty to get all messages
                command = ''
            # Empty command -> empty the queue
            if not len(command):
                resp = ' '
                while len(resp):
                    if 'exit' in resp.lower()[:5]:
                        self.exit = True
                    try:
                        time.sleep(1.0)
                        resp = self.respqueue.get(timeout=0.1)
                    except queue.Empty:
                        resp = ''
                    if len(resp):
                        self.log.debug('Got response <%s>' % resp)
                        print(resp)
            # Else send command
            else:
                agentincmd = True
                agent = ''
                # Check if command starts with valid agent name
                if command.find(' ')<0:
                    # No space -> No agent
                    agentincmd = False
                else:
                    # get possible agent name as first word of command
                    agent = command.split(' ')[0].strip().lower()
                if agentincmd:
                    if not agent in self.agents:
                        agentincmd = False
                    else:
                        self.lastagent = agent
                # Confirm if message should be sent to last agent
                if not agentincmd and len(self.lastagent):
                    print('Missing Agent: Confirm sending "%s" to "%s"?' 
                          % (command, self.lastagent))
                    resp = input('  For YES <return> for NO any_key then <return>')
                    if len(resp) < 1:
                        command = self.lastagent + ' ' + command
                # Send task
                self.sendtask(command)
        self.log.info('Exiting')
