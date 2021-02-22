
import time
import ctypes
import threading
import configparser
import numpy as np
import queue
import logging
from distutils.util import strtobool

# everything is in MHz (microseconds)

from s826board import S826Board, errhandle

class ZeroPointProducerThread(threading.Thread):
    """ Zero Point Producer Thread """
    
    def __init__(self, board, config, queue, shutdown_queue, **kwargs):
        self.__dict__.update(**kwargs)
        threading.Thread.__init__(self)

        self.board = board
        self.countzeropoint  = int(config['counter.zeropt']['counter_num'])
        self.queue = queue
        self.shutdown_flag = False

        self.local_dir = config['readout.local']['zeroptdir']
        self.datalog(self.local_dir)

    def datalog(self, dir):
        self.zeroptlog = logging.getLogger('ZeroPointLogger')

        self.zeroopthandler = logging.handlers.TimedRotatingFileHandler(f'{dir}/data', 
            when='midnight', backupCount=5)
        self.zeroptlog.addHandler(self.zeroopthandler)
        self.zeroptlog.setLevel(logging.INFO)


    def run(self):
        print(f'Starting: {threading.current_thread().name}')
        counts = ctypes.c_uint() 
        tstamp = ctypes.c_uint() 
        reason = ctypes.c_uint()

        counts_ptr = ctypes.pointer(counts)
        tstamp_ptr = ctypes.pointer(tstamp)
        reason_ptr = ctypes.pointer(reason)

        # read again
        errcode = self.board.s826_dll.S826_CounterSnapshotRead(
            self.board.board_id, self.countzeropoint,
            counts_ptr, tstamp_ptr, 
            reason_ptr, 0
        )

        self.counter = 0
        self.errors = 0
        while not self.shutdown_flag:

            # read 
            errcode = self.board.s826_dll.S826_CounterSnapshotRead(
                self.board.board_id, self.countzeropoint ,
                counts_ptr, tstamp_ptr, 
                reason_ptr, 0
            )

            while errcode in (0, -15):
                # insert value into queue
                
                self.queue.put((tstamp.value, counts.value, int(time.time())))
                self.zeroptlog.info(f'{tstamp.value}\t{counts.value}\t{int(time.time())}')
                self.counter += 1
                if self.counter > 3:
                    #prev_tstamp, prev_counts, _ = self.queue.queue[-2]
                    #curr_tstamp, curr_counts, _  = self.queue.queue[-1]
                    #diff = curr_tstamp - prev_tstamp 
                    pass
                    #print(f"{tstamp.value} , {counts.value}, Diff: {diff}")
                
                
                # if an overflow occurred, record it
                if errcode == -15:
                    self.errors += 1

                # continue reading until there is a S826_ERR_NOTREADY
                # Device was busy or unavailable, or blocking function timed out.
                errcode = self.board.s826_dll.S826_CounterSnapshotRead(
                    self.board.board_id, self.countzeropoint ,
                    counts_ptr, tstamp_ptr, 
                    reason_ptr, 0
                )
            # pause when there is blocking
            time.sleep(0.2)

        print(f'Ending: {threading.current_thread().name}')

if __name__ == '__main__':

    # load configuration file
    config = configparser.ConfigParser()
    configfile = 'testconfig.ini'
    config.read(configfile)
    
    test_S826Board = S826Board(config)
    test_S826Board.configure()

    test_ZeroPointThread = ZeroPointProducerThread(test_S826Board, config, queue.Queue(maxsize=-1), queue.Queue(maxsize=-1))
    test_ZeroPointThread.setName('TestZeroPointThread-1')
    test_ZeroPointThread.start()
    test_ZeroPointThread.join()

    # close thread
    test_S826Board.close()
