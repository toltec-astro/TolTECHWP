
import time
import ctypes
import threading
import configparser
import numpy as np
import queue
import logging
from distutils.util import strtobool

# everything is in MHz (microseconds)

try:
    from .s826board import S826Board, errhandle
except:
    from s826board import S826Board, errhandle

class PulsePerSecProducerThread(threading.Thread):
    """Pulse Per Sec Producer Thread """

    def __init__(self, board, config, queue, shutdown_queue, debug=False):
        threading.Thread.__init__(self)

        self.board = board
        self.countpps  = int(config['counter.pps']['counter_num'])
        self.queue = queue
        self.shutdown_flag = False

        self.local_dir = config['readout.local']['ppsdir']
        self.datalog(self.local_dir)
        self.debug = debug

    def datalog(self, dir):
        self.ppslog = logging.getLogger('PPSLogger')

        #self.ppshandler = logging.handlers.TimedRotatingFileHandler(f'{dir}/data', 
        #    when='midnight', backupCount=5)
        #self.ppslog.addHandler(self.ppshandler)
        #self.ppslog.setLevel(logging.INFO)

    def run(self):
        print(f'Starting: {threading.current_thread().name}')
        counts = ctypes.c_uint() 
        tstamp = ctypes.c_uint() 
        reason = ctypes.c_uint()

        counts_ptr = ctypes.pointer(counts)
        tstamp_ptr = ctypes.pointer(tstamp)
        reason_ptr = ctypes.pointer(reason)

        if self.debug == True:
            print('reason: ', reason.value)

        # read again
        errcode = self.board.s826_dll.S826_CounterSnapshotRead(
            self.board.board_id, self.countpps,
            counts_ptr, tstamp_ptr, 
            reason_ptr, 0
        )

        self.counter = 0
        self.errors = 0
        while not self.shutdown_flag:

            # read
            errcode = self.board.s826_dll.S826_CounterSnapshotRead(
                self.board.board_id, self.countpps ,
                counts_ptr, tstamp_ptr, 
                reason_ptr, 0
            )
            # print(f"errcode: {errcode}")

            while errcode in (0, -15):

                # insert value into queue
                if self.debug == True:
                    print((tstamp.value, counts.value, int(time.time()), reason.value))
                self.queue.put((tstamp.value, counts.value, int(time.time())))
                self.counter += 1
                #self.ppslog.info(f'{tstamp.value}\t{counts.value}\t{int(time.time())}')          
                
                # if an overflow occurred, record it
                if errcode == -15:
                    self.errors += 1

                # continue reading until there is a S826_ERR_NOTREADY
                # Device was busy or unavailable, or blocking function timed out.
                errcode = self.board.s826_dll.S826_CounterSnapshotRead(
                    self.board.board_id, self.countpps,
                    counts_ptr, tstamp_ptr, 
                    reason_ptr, 0
                )
            # pause when there is blocking
            time.sleep(0.1)

        print(f'Ending (PPS): {threading.current_thread().name}')

if __name__ == '__main__':

    # load configuration file
    config = configparser.ConfigParser()
    #configfile = 'testconfig.ini'
    configfile = '/home/poluser/TolTECHWP/config/masterconfig.ini'
    config.read(configfile)
    
    test_S826Board = S826Board(config)
    test_S826Board.configure()

    test_PulsePerSecThread = PulsePerSecProducerThread(test_S826Board, config, queue.Queue(maxsize=-1), queue.Queue(maxsize=-1), debug=True)
    test_PulsePerSecThread.setName('TestPulsePerSecThread-1')
    test_PulsePerSecThread.start()
    test_PulsePerSecThread.join()

    # close thread
    test_S826Board.close()
