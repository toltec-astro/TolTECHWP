""" TolTEC HWP Control Program - User interface
    ===========================================
    
    This interface object is the command line user
    interface. It is a child object of interface.
    
"""

import sys
import queue
from interparent import InterParent

class InterUser(InterParent):
    """ User Interface object: Receives commands from
        prompt and prints responses on screen.
    """
    def __call__(self):
        """ Object call: Runs a loop that runs forever and forwards
            user input.
        """
        # Loop
        print(self.config['userinterface']['greeting'])
        while not self.exit:
            # Get response and print
            try:
                resp = self.queue.get(timeout=0.2)
            except queue.Empty:
                resp = ''
            if len(resp):
                self.log.debug('Got response <%s>' % resp)
                print(resp)
            # Wait for keyboard input
            command = input('> ')
            # If it's status -> print list of all agents
            if 'status' in command.lower():
                # Print agents list
                print("List of Agents:")
                for a in self.agents:
                    print("  " + a)
                # Set command to empty to get all messages
                command = ''
            # If it's help -> print help message
            if 'help' in command.lower():
                with open(self.config['userinterface']['helpfile'],'rt') as f:
                    print(f.read())
                # Set command to empty to get all messages
                command = ''
            # If it's config -> print the configuration
            if 'config' in command.lower():
                self.config.write(sys.stdout)
                # Set command to empty to get all messages
                command = ''                
            # Empty command -> empty queue
            if not len(command):
                resp = ' '
                while len(resp):
                    try:
                        resp = self.queue.get(timeout=1.0)
                    except queue.Empty:
                        resp = ''
                    if len(resp):
                        self.log.debug('Got response <%s>' % resp)
                        print(resp)
            # Else sent command
            else:
                self.sendtask(command)
            # Check if exit
            if 'exit' in resp.lower() or 'exit' in command.lower():
                self.exit = True
                self.log.info('Exiting')
