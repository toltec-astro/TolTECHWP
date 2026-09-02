""" TolTEC HWP Control Program - Socket interface
    ===========================================
    
    This interface object is the socket
    interface. It is a child object of interface.
    
"""

import queue
import socket
from interparent import InterParent

class InterSocket(InterParent):
    """ Socket Interface object: Receives commands from
        socket and sends back responses.
    """
    
    def __call__(self):
        """ Object call: Runs a loop that runs forever and forwards
            user input.
        """
        # Make socket, bind and listen
        port = int(self.config['socketinterface']['port'])
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(('localhost',port))
        sock.listen(5)
        sock.settimeout(1.0)
        self.log.info("Listening on port %d" % port)
        # Receive loop
        while not self.exit.is_set(): # outer: over all sessions
            # Get new connection
            # Runs every second to check if self.exit has been set
            try:
                conn, addr = sock.accept()
            except socket.timeout:
                continue
            self.log.debug('Conected with %s at address %s' % (addr[0],str(addr[1])))
            try:
                while not self.exit.is_set(): # inner: one session
                    # Get the command
                    command = conn.recv(1024)
                    command = command.decode()
                    self.log.debug('Got command: %s' % command )
                    # Deal with special commands
                    # Check if exit
                    if 'exit' in command.lower()[:4]:
                        self.exit.set()
                        command = ''
                    # No special command -> send task to agents
                    if len(command):
                        # Send the command
                        self.sendtask(command)
                        # Wait for response.
                        fullresp = ''
                        resp = ' '
                        while len(resp): # Loop to get all responses
                            try:
                                resp = self.respqueue.get(timeout = 0.1)
                            except queue.Empty:
                                resp = ''
                            if len(resp):
                                fullresp += resp
                        # Return response
                        if len(fullresp):
                            conn.sendall( fullresp.encode())
                            self.log.debug('Sending %s' % fullresp)
                    else:
                        break
            except (ConnectionResetError, BrokenPipeError, OSError):
                self.log.warning(('Client connection lost'))
            finally:
                # Close the connection
                conn.close()
                self.log.debug('Connection Closed, waiting for new client')
        self.log.debug('Exiting')

"""
Simple send / receive code:

import socket
s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(iprt)
s.sendall(msg.encode())
resp = s.recv(100)
print(resp.decode())
s.close()
"""
