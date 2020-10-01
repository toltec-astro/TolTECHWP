""" TolTEC HWP Control Program - Web interface
    ==========================================
    
    This interface object is the online web user
    interface. It is a child object of interface.
    
    2DO:
    # Make advanced page and interfaces:
      - formulate the queries
        * request for log info ?log=yymmddhhmmss of last log
        * sending of command ?command=string%20of%20command
      - Make html (basic page with field for entry and field for log)
      - Add JS for getting log (every 5s)
      - Add local code for sending log info
      - Add JS for sending command
      - Add local code for receiving command
"""

import sys
import queue
import time
import subprocess
from interparent import InterParent
from wsgiref.simple_server import make_server

class InterWeb(InterParent):
    """ User Interface object: Receives commands from
        prompt and prints responses on screen.
    """
    
    
    def controll_app(self,environ,start_response):
        page = """
            <html>
            <body>
            <h1>TolTEC HWP Interface</h1>
            <form method="get">Command: <input type="text" name="command">
                          <input type="submit" value="Send"></form>
            <p><b>Log:</b>
            <pre>%s
            </pre>
            <p>Counter %d
            </body></html>
        """
        self.count += 1 # Just to make sure sth happens on the page
        #print(environ.items()) # prints out all environment
        # Check if there's a command
        qstring = environ['QUERY_STRING']
        command = ''
        if 'command=' in qstring:
            command = qstring[8:].replace('+',' ').strip()
        if len(command):
            self.sendtask(command)
        # Wait and get responses
        resp = ' '
        while len(resp):
            try:
                time.sleep(1.0)
                resp = self.respqueue.get(timeout=0.1)
            except queue.Empty:
                resp = ''
            if len(resp):
                self.log.debug('Got response <%s>' % resp)
        # Get tail of log to send back
        logfname = self.config['logging']['logfile']
        proc = subprocess.Popen(['tail', '-n', '20', logfname], stdout=subprocess.PIPE)
        log = [l.decode('utf-8') for l in proc.stdout.readlines()]
        log = ''.join(log).strip().replace('<','&lt;').replace('>','&gt;')
        # Start response
        status = '200 OK'
        headers = [('Content-type', 'text/html; charset=utf-8')]
        start_response(status, headers)
        ret = (page % (log,self.count))
        return([ret.encode('utf-8')])
    
    def __call__(self):
        self.count = 0
        httpd = make_server('',8000,self.controll_app)
        httpd.serve_forever()