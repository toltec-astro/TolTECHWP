""" TolTEC HWP Control Program - Socket interface
    ===========================================
    
    This interface object is the socket
    interface. It is a child object of interface.
    
"""

import sys
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
        self.log.info("Listening on port %d" % port)
        # Receive loop
        while not self.exit:
            # Get new connection
            conn, addr = sock.accept()
            self.log.debug('Conected with %s at address %s' % (addr[0],str(addr[1])))
            # Get the command
            command = conn.recv(1024)
            command = command.decode()
            self.log.debug('Got command: %s' % command )
            # Deal with special commands
            # Check if exit
            if 'exit' in command.lower()[:5]:
                self.exit = True
                command = ''
            # No special command -> send task to agents
            if len(command):
                # Send the command
                self.sendtask(command)
                # Wait for response
                try:
                    response = self.respqueue.get(timeout = 0.1)
                except queue.Empty:
                    response = ''
                # Return response
                if len(response):
                    conn.sendall( response.encode())
                    self.log.debug('Sending %s' % response)
            # Close the connection
            conn.close()
            self.log.debug('Connection Closed')

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
