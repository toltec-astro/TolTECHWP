
import sys
import time
import ctypes
import queue
import copy
import collections
import configparser
import threading
import socket
import pickle
import struct
import logging
from distutils.util import strtobool

import numpy as np

class UDPPackagingConsumerThread(threading.Thread):
    """ UDP Packaging Consumer Thread """

    def __init__(self, config, quad_queue, pps_queue, zeropt_queue, sensor_queue, shutdown_queue):
        threading.Thread.__init__(self)

        self.quad_queue = quad_queue
        self.pps_queue = pps_queue
        self.zeropt_queue = zeropt_queue
        self.sensor_queue = sensor_queue
        
        self.dest_ip  = config['readout.destination']['ip']
        self.dest_port = int(config['readout.destination']['port'])

        self.shutdown_flag = False

        self.local_mode = False
        self.local_dir = config['readout.local']['quaddir']
        self.rotating_save(self.local_dir)

    def rotating_save(self, dir):
        self.quadlog = logging.getLogger('QuadLogger')
        if self.local_mode:
            self.quadhandler = logging.FileHandler(f'{dir}/debug_data.log', 'w')
        else:
            self.quadhandler = logging.handlers.TimedRotatingFileHandler(f'{dir}/data', 
                when='midnight', backupCount=5)
        self.quadlog.handlers = []
        self.quadlog.addHandler(self.quadhandler)
        self.quadlog.setLevel(logging.INFO)


    def send_message(self, message, ip='127.0.0.1', port=8798):
        sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) # UDP
        sock.sendto(message, (ip, port))
        sock.close()

    def run(self):
        print(f'Starting: {threading.current_thread().name}')

        # Keep track of packets sent
        self.packets_sent = 0

        # initialize running queue
        running_queue_len = 200
        r_pps_queue     = collections.deque(maxlen=64)
        r_zeropt_queue  = collections.deque(maxlen=64)
        r_sensors_queue = collections.deque(maxlen=64)

        for que in [r_pps_queue, r_zeropt_queue, r_sensors_queue]:
            _ = [que.append((0, 0, 0)) for _ in range(running_queue_len)]

        # keep sending until both shutdown is received AND there
        # are no more values in the queue.
        while not self.shutdown_flag or not self.quad_queue.empty():

            # get the quadrature data
            quad_data_list = []
            quad_packet_allocation = 800
            while len(quad_data_list) < quad_packet_allocation:
                try:
                    quad_data = self.quad_queue.get(timeout=1)
                except queue.Empty:
                    break
                quad_data_list.append(quad_data)
                #self.quadlog.info(f'{quad_data[0]}\t{quad_data[1]}\t{quad_data[2]}')
            quad_export = np.array(quad_data_list, dtype=np.uint32).flatten()

            # get the zeropt data (grab all)
            # update the running queue to most recent
            # 4 bytes * 3 * 200 (last 200 seconds)
            while True:
                try:
                    zeropt_data = self.zeropt_queue.get(timeout=0.05)
                    r_zeropt_queue.append(zeropt_data)
                except queue.Empty:
                    break
            zeroopt_export = np.array(list(r_zeropt_queue), dtype=np.uint32).flatten()

            # get the pps data 
            # update the running queue to most recent
            # 4 bytes * 3 * 200 (last 200 seconds)
            while True:
                try:
                    pps_data = self.pps_queue.get(timeout=0.05)
                    r_pps_queue.append(pps_data)
                except queue.Empty:
                    break
            pps_export = np.array(list(r_pps_queue), dtype=np.uint32).flatten()

            # get the sensor data (grab all) 
            # 4 bytes * 3 * 200 (last 20 seconds)
            while True: 
                try:
                    sensor_data = self.sensor_queue.get(timeout=0.05)
                    r_sensors_queue.append(sensor_data)
                except queue.Empty:
                    break
            sensor_export = np.array(list(r_sensors_queue), dtype=np.uint32).flatten()

            packet = list(quad_export) + list(zeroopt_export) + list(pps_export) + list(sensor_export)
            bin_packet = [struct.pack("=I", value) for value in packet]
            binarypackage = b"".join(bin_packet)
            
            # binarypackage = struct.pack("=I", *packet)
            # data = [2,25,0,99,0,23,18,188]
            # format = ">BBBHBBBB"
            # struct.pack(format, *packet)
            self.send_message(binarypackage, port=23423)
            self.send_message(binarypackage, ip='10.120.230.32', port=23423)
            self.packets_sent += 1


            #print(self.quad_queue.qsize())
            # get the total size
            #packet_size = np.sum([sys.getsizeof(i) for i in [quad_export,zeroopt_export,pps_export,sensor_export]])            
            #print(f'Quadrature Queue Size: {self.quad_queue.qsize()}\tPacket Size: {packet_size} \tQuad Len: {quad_export.size}')
            #binarypackage = f'{int(time.time())} Packet Size Sent: {packet_size} Packets Sent: {self.packets_sent}\n'.encode('UTF-8')
            #self.send_message(binarypackage, port=self.dest_port)

        print(f'Ending: {threading.current_thread().name}')
        print(f'Quadrature Queue Size: {self.quad_queue.qsize()}')
        


if __name__ == '__main__':

    # load configuration file
    config = configparser.ConfigParser()
    configfile = 'testconfig.ini'
    config.read(configfile)

    # queues
    quad_queue   = queue.Queue(maxsize=-1) 
    pps_queue    = queue.Queue(maxsize=-1) 
    sensor_queue = queue.Queue(maxsize=-1)
    zeropt_queue = queue.Queue(maxsize=-1)

    test_UDPPackagingThread = UDPPackagingConsumerThread(config, quad_queue, pps_queue, zeropt_queue, sensor_queue)
    test_UDPPackagingThread.setName('TestUDPPackagingThread-1')
    test_UDPPackagingThread.start()

    for _ in range(10000):
        quad_queue.put((83452, 4435, int(time.time())))
        time.sleep(0.001)

    test_UDPPackagingThread.join()
