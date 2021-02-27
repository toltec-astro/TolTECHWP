
import time
import ctypes
import queue
import logging
import logging.handlers 
import configparser
import secrets
import numpy as np
import multiprocessing

from .s826board import S826Board, errhandle

from .pps     import PulsePerSecProducerThread
from .quad    import QuadratureProducerThread
from .zeropt  import ZeroPointProducerThread
from .packet  import UDPPackagingConsumerThread
from .sensors import SensorProducerThread

from .control import DispatcherControlThread, ListenerControlThread
from .outbox  import OutboxControlThread, shutdown_outbox_thread

'''
Send Commands: nc -u localhost 8787
View Logs : nc -kluvw 1 localhost 50773
'''

def mp_quad(thread):
        """convert thread into a process """
        thread.start()
        thread.join()

def readout(config):

    # load configuration file
    # config = configparser.ConfigParser()
    # configfile = 'testconfig.ini'
    # config.read(configfile)

    # outbox queue (primarily for logging)
    outbox_queue = queue.Queue(maxsize=-1)

    # configure board
    test_S826Board = S826Board(config)
    test_S826Board.configure()

    # data queues
    quad_queue   = multiprocessing.Queue(maxsize=-1) 
    pps_queue    = queue.Queue(maxsize=-1) 
    sensor_queue = queue.Queue(maxsize=-1)
    zeropt_queue = queue.Queue(maxsize=-1)

    data_queues = quad_queue, pps_queue, zeropt_queue, sensor_queue

    # spawn the messaging thread for logging
    final_shutdown_token = secrets.token_hex(64)
    OutboxThread = OutboxControlThread(config, outbox_queue, final_shutdown_token, data_queues)
    outbox_threads = [OutboxThread]
    for idx, obj_thread in enumerate(outbox_threads):
        obj_thread.setName(f'{idx}_{type(obj_thread).__name__}')
        obj_thread.log = logging.getLogger('readout.'+ obj_thread.name)
        obj_thread.start()


    # command queue
    command_queue  = queue.Queue(maxsize=-1)

    # shutdown message queue
    producer_shutdown_queue = queue.Queue(maxsize=-1)
    consumer_shutdown_queue = queue.Queue(maxsize=-1)

    # producer threads
    SensorThread = SensorProducerThread(
        board  = test_S826Board, 
        config = config, 
        queue  = sensor_queue, 
        shutdown_queue = producer_shutdown_queue
    )
    QuadratureThread = QuadratureProducerThread(
        board  = test_S826Board, 
        config = config, 
        queue  = quad_queue, 
        shutdown_queue = producer_shutdown_queue
    )
    QuadratureThreadProcess = multiprocessing.Process(target=mp_quad, args=(QuadratureThread,))
    
    ZeroPointThread = ZeroPointProducerThread(
        board  = test_S826Board, 
        config = config, 
        queue  = zeropt_queue, 
        shutdown_queue = producer_shutdown_queue
    )
    PulsePerSecThread = PulsePerSecProducerThread(
        board  = test_S826Board, 
        config = config, 
        queue  = pps_queue, 
        shutdown_queue = producer_shutdown_queue
    )
    producers_list = [SensorThread, QuadratureThread, ZeroPointThread, PulsePerSecThread]
    
    # consumer thread
    UDPPacketThread = UDPPackagingConsumerThread(
        config, quad_queue, pps_queue, zeropt_queue, sensor_queue, consumer_shutdown_queue
    )
    consumers_list = [UDPPacketThread]

    # general control thread
    DispatcherThread = DispatcherControlThread(
        config, command_queue, producers_list, outbox_queue=outbox_queue
    )
    ListenerThread = ListenerControlThread(
        config, command_queue, queue.Queue(maxsize=-1)
    )
    
    producers_list = [SensorThread, ZeroPointThread, PulsePerSecThread]
    control_threads = [DispatcherThread, ListenerThread]

    
    # start the quadrature readout process
    QuadratureThreadProcess.start()
    # start thread
    threads_list = producers_list + consumers_list + control_threads
    for idx, obj_thread in enumerate(threads_list):
        #obj_thread.setName(f'{idx}_{type(obj_thread).__name__}')
        obj_thread.log = logging.getLogger('readout.'+ obj_thread.name)
        obj_thread.start()    

    # primary control
    main_control_shutdown_flag = False
    while not main_control_shutdown_flag:

        # if all the producers are stopped:
        if np.sum([int(producer_thread.is_alive()) for producer_thread in producers_list]) == 0:
            # stop the consumers
            for thrd in consumers_list:
                thrd.shutdown_flag = True
            # if all the consumers have stopped
            if np.sum([int(thrd.is_alive()) for thrd in (consumers_list + control_threads)]) == 0:
                main_control_shutdown_flag = True
        time.sleep(0.1)
    
    # join all threads
    QuadratureThreadProcess.join()
    for obj_thread in threads_list:
        obj_thread.join()
    
    # close connection to board
    test_S826Board.close()

    # shutdown outbox thread
    shutdown_outbox_thread(outbox_queue, final_shutdown_token)
    for obj_thread in outbox_threads:
        obj_thread.join()

if __name__ == '__main__':
    readout()
    
    
