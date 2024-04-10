
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
import traceback

import numpy as np

# sudo sysctl -w net.inet.udp.maxdgram=65535'

def send_message(message, ip='192.168.0.207', port=64013):
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) # UDP
#    sock.bind(('192.168.0.205', port+1000))
    sock.sendto(message, (ip, port))
    sock.close()
    #print(f"sent to {ip}:{port}")


if __name__ == '__main__':

    packets_sent = 0
    while True:
        #
        # generate fake quadrature data
        #
        quad_packet_allocation = 500
        fake_quad_data_ts       = np.arange(0, quad_packet_allocation, 1) +  np.random.randint(1000, 854895)
        fake_quad_data_cnter    = np.arange(0, quad_packet_allocation, 1) +  np.random.randint(10, 100)
        fake_quad_data_unixtime = np.arange(0, quad_packet_allocation, 1) + int(time.time())
        
        running_queue_len = 500
        r_quad_queue     = collections.deque(maxlen=running_queue_len)
        _ = [r_quad_queue.append((fake_quad_data_ts[i], fake_quad_data_cnter[i], fake_quad_data_unixtime[i])) for i in range(running_queue_len)]
        quad_export = np.array(list(r_quad_queue), dtype=np.uint32).flatten()

        running_queue_len = 1
        r_pps_queue     = collections.deque(maxlen=4)
        r_zeropt_queue  = collections.deque(maxlen=10)
        r_sensors_queue = collections.deque(maxlen=32)

            
        _ = [r_zeropt_queue.append((650, 690, 420)) for _ in range(running_queue_len)]
        zeroopt_export = np.array(list(r_zeropt_queue), dtype=np.uint32).flatten()

        _ = [r_pps_queue.append((1, 5, 9)) for _ in range(running_queue_len)]
        pps_export = np.array(list(r_pps_queue), dtype=np.uint32).flatten()

        _ = [r_sensors_queue.append((1342, 5454, 5454)) for _ in range(running_queue_len)]
        sensor_export = np.array(list(r_sensors_queue), dtype=np.uint32).flatten()

        # make the packet
        packet = list(quad_export[0:166*3]) + [0]*14 + list(quad_export[166*3:332*3]) + [0]*14 + list(quad_export[332*3:498*3]) + [0]*14 + list(quad_export[498:500]) + list(zeroopt_export) + list(pps_export) + list(sensor_export)
        packet += [0]*(2048-len(packet))
        packet[1024*2-6+5] = socket.htonl(packets_sent)
        print(packet[1024*2-6])
        #packet[1024*2-6+2] = socket.htonl(latest_pps)
        bin_packet = [struct.pack("=I", value) for value in packet]
        binarypackage = b"".join(bin_packet)
        packet_size = len(packet)*np.dtype(np.uint32).itemsize
        
        try: 
            # send the packet
            send_message(binarypackage)
            msg = f'{int(time.time())} Packet Size Sent: {packet_size} Packets Sent: {packets_sent} {packet[2]} {packet[500+3*14+3+3+3-1]}'
            print(msg)
            packets_sent += 1 
        except OSError as error:
            print(error)
            traceback.print_exc()

        # do not spam them
        time.sleep(1.)

        
    
