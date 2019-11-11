
import sys
import socket


def sendreceive_UDP(message, ip='127.0.0.1', port=8787, rec_port=5555):
    """ 
    send and recieve UDP message
    """

    # bind listening port
    rec_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rec_sock.bind((ip, rec_port))

    # send message
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(bytes(message, 'utf-8'), (ip, port))

    # receive and print
    data, addr = sock.recvfrom(4096)
    print('from:', str(addr))
    print('response:', str(data.decode("utf-8") ))


if __name__ == '__main__':
    _, msg = sys.argv
    sendreceive_UDP(msg)