
import time
import ctypes
import threading
import configparser
import numpy as np
import queue
import logging
import logging.handlers
from distutils.util import strtobool

from .s826board import S826Board, errhandle

S826_WAIT_INFINITE = 0xFFFFFFFF

class SensorProducerThread(threading.Thread):
    
    def __init__(self, board, config, queue, shutdown_queue):
        threading.Thread.__init__(self)

        self.board = board
        self.sensor_interval = float(config['intervals']['sensor_intervals'])
        self.queue = queue
        self.shutdown_flag = False

        self.local_dir = config['readout.local']['sensorsdir']
        self.datalog(self.local_dir)

    def datalog(self, dir):
        self.sensorlog = logging.getLogger('SensorLogger')

        self.sensorhandler = logging.handlers.TimedRotatingFileHandler(f'{dir}/data', 
            when='midnight', backupCount=5)
        self.sensorlog.addHandler(self.sensorhandler)
        self.sensorlog.setLevel(logging.INFO)

    def run(self):
        print(f'Starting: {threading.current_thread().name}')
        
        slotpylist = [i for i in range(16)]
        slotval = (ctypes.c_int32 * 16)(*slotpylist)
        slotlist = ctypes.c_uint(0xFFFF)
        slotlist_ptr = ctypes.pointer(slotlist)

        self.counter = 0
        iters = int(1.0 /  self.sensor_interval)
        while not self.shutdown_flag:
            
            avg_voltage_arr = []
            for _ in range(20):

                # read from card
                errcode = self.board.s826_dll.S826_AdcRead(
                    self.board.board_id, slotval, 
                    None, slotlist_ptr, S826_WAIT_INFINITE
                )
                errhandle(errcode, 'S826_AdcRead')

                # convert to volts
                result = np.ctypeslib.as_array(slotval)
                adcdata  = result & 0xFFFF    
                voltage  = (adcdata / (0x7FFF)) * 10

                # accumulate
                avg_voltage_arr.append(voltage) 
                time.sleep(self.sensor_interval)

            # average voltages over the last approx seconds
            avg_voltage_arr = np.mean(np.array(avg_voltage_arr), axis=0)
    
            # put in queue
            timestamp = int(time.time())
            for slot_num, volt in zip(slotpylist, avg_voltage_arr):
                self.queue.put((slot_num, volt, timestamp))
                #self.sensorlog.info(f'{slot_num}\t{volt:08.5f}\t{timestamp}')
            #print(timestamp, avg_voltage_arr)            

        print(f'Ending (Sensors): {threading.current_thread().name}')

if __name__ == '__main__':

    # load configuration file
    config = configparser.ConfigParser()
    configfile = 'testconfig.ini'
    config.read(configfile)
    
    test_S826Board = S826Board(config)
    test_S826Board.configure()

    sdqueue = queue.Queue(maxsize=-1)
    
    test_SensorThread = SensorProducerThread(test_S826Board, config, queue.Queue(maxsize=-1), sdqueue)
    test_SensorThread.setName('TestSensorThread-1')
    test_SensorThread.start()
    time.sleep(10)

    test_SensorThread.shutdown_flag = True
    test_SensorThread.join()
    
    # close thread
    test_S826Board.close()


#ctypes.addressof(slotval), 
    #burstnum = np.uint8(result >> 24)

                #mv = np.array(voltage) * 1000
                # for slot_num, volt in zip(slotpylist, voltage):
                #     self.sensorlog.info(f'{slot_num}\t{volt:0.5f}\t{timestamp}')
                #     self.counter += 1
            # temp_V_OUT = voltage[0]
            # humi_V_OUT = voltage[1]
            # T = (temp_V_OUT / 0.01) # degrees C
            # sensor_RH =  ((humi_V_OUT / 5) - 0.16) / 0.0062
            # true_RH   = (sensor_RH) / (1.0546 - (0.00216 * T))

            #print(f'Temperature: {T:0.1f} C ({((T * 9/5) + 32):0.1f} F) \t Humidity: {true_RH:0.1f} %')

            
