
import time
import ctypes
import threading
import configparser
import numpy as np
import queue
import socket

from s826board import S826Board, errhandle

# everything is in MHz (microseconds)
class ListenerControlThread(threading.Thread):
    """ Listener Control Thread """

    def __init__(self, config, queue, shutdown_queue):
        threading.Thread.__init__(self)

        self.queue     = queue
        self.port = int(config['readout.address']['port'])
        self.shutdown_flag  = False

    def run(self):
        print(f'Starting: {threading.current_thread().name}')

        listen_socket = self.make_socket(self.port)
        while True:
            # get message
            message, addr = listen_socket.recvfrom(1024)
            decoded_message = message.decode().rstrip()
            cmd_package = {'sender': addr, 'cmd': decoded_message}

            print(f'Received [{decoded_message}] from {addr[0]}:{addr[1]}')
            # queue it up
            self.queue.put(cmd_package)

            # if it happened to be exit...exit
            if decoded_message == 'exit':
                print('Committed to Shutdown Process...')
                return
            # other wise continue to listen

        print(f'Ending: {threading.current_thread().name}')

    @staticmethod
    def make_socket(port, ip_add='127.0.0.1'):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((ip_add, port))
        print("Listening on port %d" % port)
        return sock

class DispatcherControlThread(threading.Thread):
    """ Dispatcher Control Thread """

    def __init__(self, config, queue, producer_threads, **kwargs):
        self.__dict__.update(**kwargs)

        threading.Thread.__init__(self)
        self.queue     = queue
        self.shutdown_flag  = False
        self.producer_threads = producer_threads

    def toggle_shutdown(self):
        self.shutdown_flag = True

    def shutdown_threads(self):
        for thrd in self.producer_threads:
            thrd.shutdown_flag = True

    def run(self):
        print(f'Starting: {threading.current_thread().name}')

        while not self.shutdown_flag:
            cmd_package = self.queue.get()
            self.handle_function(cmd_package)

        print(f'Ending: {threading.current_thread().name}')

    def handle_function(self, cmd_package):
        
        if cmd_package['cmd'] == 'exit':
            self.shutdown_threads()
            self.toggle_shutdown()

        if cmd_package['cmd'] == 'data':
            
            #msg = '\n'.join([f'{thrd.name} {thrd.counter}' for thrd in self.producer_threads])
            msg = 'data?'
            message_package = {
                'ip': cmd_package['sender'][0], 
                'port': cmd_package['sender'][1], 
                'message': msg
            }
            self.outbox_queue.put(message_package)

if __name__ == '__main__':
    # nc -u localhost 8787

    cmdqueue = queue.Queue(maxsize=-1)
    # load configuration file
    config = configparser.ConfigParser()
    configfile = 'testconfig.ini'
    config.read(configfile)
    
    test_S826Board = S826Board(config)
    test_S826Board.configure()

    test_ListenerThread = ListenerControlThread(config, cmdqueue, queue.Queue(maxsize=-1))
    test_ListenerThread.start()
    test_ResponderThread = ResponderControlThread(config, cmdqueue, queue.Queue(maxsize=-1))
    test_ResponderThread.start()
   
    test_ListenerThread.join()
    test_ResponderThread.join()

    # close thread
    test_S826Board.close()
