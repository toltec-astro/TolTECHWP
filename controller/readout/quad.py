
import os
import time
import ctypes
import threading
import configparser
import numpy as np
import queue
import logging
from distutils.util import strtobool

from .s826board import S826Board, errhandle

class QuadratureProducerThread(threading.Thread):
    """ Quadrature Producer Thread """
    
    def __init__(self, board, config, queue, shutdown_queue):
        threading.Thread.__init__(self)
        
        # board information
        self.board = board
        # configuration information
        self.countquad       = int(config['counter.quad']['counter_num'])
        self.sleep_interval  = float(config['intervals']['quad_intervals'])
        # data queue
        self.queue = queue
        # shutdown flag
        self.shutdown_flag = False

        # this doesn't really do anything
        if 1 == 0:
            try:
                cmd = f'sudo renice -n -19 -p {os.getpid()}'
                result = os.system(cmd)
                print(f'renice: {os.system(cmd)}')
            except:
                print('Failed to elevate (probably permissions)')

    def run(self):
        print(f'Starting (Quad): {threading.current_thread().name}')
        counts = ctypes.c_uint() 
        tstamp = ctypes.c_uint() 
        reason = ctypes.c_uint()

        counts_ptr = ctypes.pointer(counts)
        tstamp_ptr = ctypes.pointer(tstamp)
        reason_ptr = ctypes.pointer(reason)

        zero_ctypes = ctypes.c_uint(0)

    
        # initalize by reading once
        errcode = self.board.s826_dll.S826_CounterSnapshotRead(
            self.board.board_id, self.countquad,
            counts_ptr, tstamp_ptr, reason_ptr, zero_ctypes
        )

        # enter the reading loop
        self.errors = 0
        self.counter = 0
        while not self.shutdown_flag:

            # read again
            errcode = self.board.s826_dll.S826_CounterSnapshotRead(
                self.board.board_id, self.countquad,
                counts_ptr, tstamp_ptr, reason_ptr, zero_ctypes
            )

            while errcode in (0, -15):
                # insert value into queue
                self.queue.put((tstamp.value, counts.value, int(time.time())))

                # if an overflow occurred, record it
                if errcode == -15:
                    self.errors += 1
                    print(f'Overflow Detected {self.errors} {int(time.time())}')

                errcode = self.board.s826_dll.S826_CounterSnapshotRead(
                    self.board.board_id, self.countquad,
                    counts_ptr, tstamp_ptr, reason_ptr, zero_ctypes
                )

            # pause when there is blocking
            time.sleep(self.sleep_interval)

        print(f'Ending: {threading.current_thread().name}')

if __name__ == '__main__':

    # load configuration file
    config = configparser.ConfigParser()
    configfile = 'testconfig.ini'
    config.read(configfile)
    
    # initialize board
    test_S826Board = S826Board(config)
    test_S826Board.configure()

    test_QuadThread = QuadratureProducerThread(test_S826Board, config, queue.Queue(maxsize=-1), queue.Queue(maxsize=-1))
    test_QuadThread.setName('TestQuadThread-1')
    test_QuadThread.start()
   
    test_QuadThread.shutdown_flag = True 
    test_QuadThread.join()

    # close board
    test_S826Board.close()

                # continue reading until there is a S826_ERR_NOTREADY
                # Device was busy or unavailable, or blocking function timed out.
                # errcode = self.board.s826_dll.S826_CounterSnapshotRead(
                #     self.board.board_id, self.countquad ,
                #     ctypes.pointer(counts), ctypes.pointer(tstamp), 
                #     ctypes.pointer(reason), 0
                # )
