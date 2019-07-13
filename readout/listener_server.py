import sys
import socket


def listen_UDP(ip='127.0.0.1', rec_port=3141):
    """ 
    send and recieve UDP message

    rec_port = port recieving data
    """

    # bind listening port
    rec_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rec_sock.bind((ip, rec_port))

    # recieve and print
    while True:
        data, addr = rec_sock.recvfrom(4096)
        print('from:', str(addr))
        print('response:', str(data.decode("utf-8") ))

if __name__ == '__main__':
    listen_UDP()