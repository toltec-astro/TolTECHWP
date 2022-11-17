
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

import numpy as np

# sudo sysctl -w net.inet.udp.maxdgram=65535'

def send_message(message, ip='127.0.0.1', port=8798):
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) # UDP
    sock.sendto(message, (ip, port))
    sock.close()
    #print(f"sent to {ip}:{port}")


if __name__ == '__main__':

    DESTINATION_IP   = '127.0.0.1'
    DESTINATION_PORT = 6969

    packets_sent = 0
    while True:
        #
        # generate fake quadrature data
        #
        quad_packet_allocation = 800
        fake_quad_data_ts       = np.arange(0, quad_packet_allocation, 1) +  np.random.randint(1000, 854895)
        fake_quad_data_cnter    = np.arange(0, quad_packet_allocation, 1) +  np.random.randint(10, 100)
        fake_quad_data_unixtime = np.arange(0, quad_packet_allocation, 1) + int(time.time())
        
        fake_quad_data = [fake_quad_data_ts, fake_quad_data_cnter, fake_quad_data_unixtime]
        quad_export = np.array(fake_quad_data, dtype=np.uint32).T.flatten()
        

        running_queue_len = 64
        r_pps_queue     = collections.deque(maxlen=running_queue_len)
        r_zeropt_queue  = collections.deque(maxlen=running_queue_len)
        r_sensors_queue = collections.deque(maxlen=running_queue_len)

            
        _ = [r_zeropt_queue.append((650, 690, 420)) for _ in range(running_queue_len)]
        zeroopt_export = np.array(list(r_zeropt_queue), dtype=np.uint32).flatten()

        _ = [r_pps_queue.append((1, 5, 9)) for _ in range(running_queue_len)]
        pps_export = np.array(list(r_pps_queue), dtype=np.uint32).flatten()

        _ = [r_sensors_queue.append((1342, 5454, 5454)) for _ in range(running_queue_len)]
        sensor_export = np.array(list(r_sensors_queue), dtype=np.uint32).flatten()

        # make the packet
        packet = list(quad_export) + list(zeroopt_export) + list(pps_export) + list(sensor_export)
        bin_packet = [struct.pack("=I", value) for value in packet]
        binarypackage = b"".join(bin_packet)    
        packet_size = np.sum([sys.getsizeof(i) for i in [quad_export,zeroopt_export,pps_export,sensor_export]])            
        
        try: 
            # send the packet
            send_message(binarypackage, port=6969)
            msg = f'{int(time.time())} Packet Size Sent: {packet_size} Packets Sent: {packets_sent}'
            print(msg)
            packets_sent += 1 
        except OSError as error:
            print(error)

        # do not spam them
        time.sleep(5)

        
    