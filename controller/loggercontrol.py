""" TolTEC HWP Control Program - Logger
    ===================================
    
    This subtasks sets up the logging and opens a port for
    external logging messages. The message format for external
    messages is:
        LEVEL\tSender\tMessage
    The port that the logger is listening to is specified in
    the config file.
"""

import logging
import socket

class LoggerControl():
    """ Logger receiver: sets up logging then opens a port
        for external log messages.
    """
    
    def __init__(self, config, name=''):
        """ Constructor: Set up variables
        """
        self.name = name
        self.config = config # configuration
        self.log = logging.getLogger('control.logger')
    
    def __call__(self):
        """ Object call: does the log thing
        """
        ### Set up logging
        # Set root log level
        logging.getLogger().setLevel(logging.DEBUG)
        logformat = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        # Set stream logger
        shand = logging.StreamHandler()
        shand.setFormatter(logging.Formatter(logformat))
        shand.setLevel(self.config['logging']['level'])
        logging.getLogger().addHandler(shand)
        # Set up optional config file
        if len(self.config['logging']['logfile']):
            fhand = logging.FileHandler(self.config['logging']['logfile'])
            fhand.setFormatter(logging.Formatter(logformat))
            fhand.setLevel(logging.DEBUG)
            logging.getLogger().addHandler(fhand)
        # Test Message
        self.log.info('Logging is set up')
        ### Set up port server
        # Get log socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        port = int(self.config['logging']['port'])
        sock.bind(('localhost',port))
        sock.listen(5)
        # Endless loop getting messages
        while(True):
            # Get new connection
            conn, addr = sock.accept()
            #print('Conected with %s at address %s' % (addr[0],str(addr[1])))
            # Get the message
            reply = conn.recv(1024)
            #print('  Got message: %s' % reply)
            # Close the connection
            conn.close()
            ### Make log message
            # Split incoming message
            split = reply.decode().split('\t')
            if len(split) < 2:
                lvl = 'INFO'
                lgr = self.config['logging']['deflogger']
                msg = split[0]
            elif len(split) < 3:
                lvl = split[0]
                lgr = self.config['logging']['deflogger']
                msg = split[1]
            else:
                lvl = split[0]
                lgr = split[1]
                msg = '\t'.join(split[2:])
            # Get logging level
            log=logging.getLogger(lgr)
            if lvl.upper() in ['DEBUG','DEBUGGING']: log.debug(msg)
            elif lvl.upper() in ['INFO','INFORMATION']: log.info(msg)
            elif lvl.upper() in ['WARN','WARNING']: log.warn(msg)
            elif lvl.upper() in ['ERR','ERROR']: log.error(msg)
            elif lvl.upper() in ['CRIT','CRITICAL']: log.critical(msg)
            else: log.info(msg)

""" Communicate to this via Telnet or the following simple program:

import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', 50773))
message = 'error\tpipe.logtest.client\tTest Message'
s.sendall(message)
# to receive also data=s.recv(bufsize) # bufsize can be 100 or so
s.close()

"""
