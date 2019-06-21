""" Interface Agent Control Program - Configuration Agent
    =====================================================
    
    This is the agent for reading and changing the current 
    system configuration.

"""

helpmsg = """Configuration Agent: Read and edit current system configuration.
Possible commands are:
    conf all - returns the configuration
    conf "section" - returns the contents of a particular config section
    conf "section" "keyword" - returns the value of a particular keyword
    conf "section" "keyword" "value" - sets the value for that particular keyword
    conf help - returns this message
    conf exit - exits the command agent"""
    
import queue
import logging
from io import StringIO
from agentparent import AgentParent


# MovementAgent Object
class ConfigAgent(AgentParent):
    """ User Interface object: Receives commands from
        prompt and prints responses on screen.
    """

    def __init__(self, config, name = ''):
        """ Constructor: Set up variables
        """
        self.name = name.lower()
        self.queue = queue.Queue() # Queue object for querries
        self.config = config # configuration
        self.log = logging.getLogger('Agent.'+self.name)
        self.exit = False # Indicates that loop should exit
        
    def __call__(self):
        """ Object call: Runs a loop that runs forever and forwards
            user input.
        """
        ### Command loop
        while not self.exit:
            ### Look for task
            try:
                task, respqueue = self.queue.get(timeout = 0.1)
                task = task.strip()
            except queue.Empty:
                task = ''
            if len(task):
                self.log.debug('Got task <%s>' % task)
            else:
                continue
            #print(repr(self.comm))
            retmsg = ''
            # Exit
            if 'exit' in task[:4].lower():
                self.exit = True
            # Help message
            elif 'help' in task[:4].lower():
                retmsg = helpmsg
            # Print all config
            elif 'all' in task[:3].lower():
                sio = StringIO('')
                self.config.write(sio)
                retmsg = sio.getvalue()
                sio.close()
            # Check if it's just sectionname -> print section
            elif ' ' not in task:
                sect = task.strip()
                if self.config.has_section(sect):
                    retmsg = '[%s]\n' % sect
                    for k in self.config[sect]:
                        retmsg += '%s = %s\n' % (k, self.config[sect][k]) 
                else:
                    retmsg = 'Invalid section: %s' % sect
            # We have at least section and key
            else:
                # Get section
                sect, task = task.split(' ',1)
                if self.config.has_section(sect):
                    # If no further word, return value
                    if ' ' not in task:
                        key = task.strip()
                        if not key in self.config[sect]:
                            retmsg = 'Invalid key=%s in section=%s' % (sect,key)
                        else:
                            retmsg = self.config[sect][key]
                    else:
                        key, val = task.split(' ',1)
                        self.config[sect][key] = val
                        retmsg = 'set [%s] %s = %s' % (sect, key, val)
                else:
                    retmsg = 'Invalid section: %s' % sect
            # Send return message
            if len(retmsg):
                respqueue.put("%s: %s" % (self.name, retmsg))
        