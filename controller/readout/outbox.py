import time
import ctypes
import threading
import configparser
import numpy as np
import queue
import socket

# nc -kluvw 1 50773
# nc -kluvw 1 localhost 50773
def shutdown_outbox_thread(outbox_queue, token):
    message_package = {'ip': '', 'port': '', 'message': token}
    outbox_queue.put(message_package)

class OutboxControlThread(threading.Thread):

    def __init__(self, config, outbox_queue, token, data_queues):
        threading.Thread.__init__(self)
        self.outbox_queue = outbox_queue
        self.logging_ip   = config['logging']['ip']
        self.logging_port = 5464#int(config['logging']['port'])

        self.shutdown_flag = False
        self.token = token
        quad_queue, pps_queue, zeropt_queue, sensor_queue = data_queues
        self.quad_queue = quad_queue
        self.pps_queue = pps_queue
        self.zeropt_queue = zeropt_queue
        self.sensor_queue = sensor_queue

    def run(self):
        
        while not self.shutdown_flag:
            try:
                msg_package = self.outbox_queue.get(timeout=0.5)
                
                print(msg_package)

                # get message
                ip   = msg_package['ip']
                port = msg_package['port']
                msg  = msg_package['message']

                if msg == self.token:
                    self.shutdown_flag = True
                    self.send_message('Shutdown Sequence Completed. Goodbye.', self.logging_ip, self.logging_port)
                    return 
                self.send_message(msg, self.logging_ip, self.logging_port)
                self.send_message(msg, ip, port)
            
            except queue.Empty:
                # continuous logging:
                msg = f'time:{int(time.time())} quad_queue:{self.quad_queue.qsize()} pps_queue:{self.pps_queue.qsize()} zeropt_queue:{self.zeropt_queue.qsize()} sensor_queue:{self.sensor_queue.qsize()}'
                self.send_message(msg, self.logging_ip, self.logging_port)
        
    def send_message(self, message, ip='127.0.0.1', port=5555):
        sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) # UDP
        sock.sendto(f'{message}\n'.encode('UTF-8'), (ip, port))
        sock.close()

if __name__ == '__main__':
    pass
