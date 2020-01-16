import sys
import socket
import glob
import struct


def split_4(array):
    return [array[i:i+4] for i in range(0, len(array), 4)]    

def parse_sensordata(sensor_data):
    sensor_data = split_4(sensor_data)
    
    computer_time = [struct.unpack("=I", _pt)[0] for _pt in sensor_data[0::3]]
    sensor_id = [struct.unpack("=I", _pt)[0] for _pt in sensor_data[1::3]]
    voltage = [struct.unpack("=f", _pt)[0] for _pt in sensor_data[2::3]]
    
    return computer_time, sensor_id, voltage

def parse_pps(pps_data):
    pps_data = split_4(pps_data)
    pps_id = [struct.unpack("=I", _pt)[0] for _pt in pps_data[0::2]]
    pps_cnter = [struct.unpack("=I", _pt)[0] for _pt in pps_data[1::2]]

    return pps_id, pps_cnter

def parse_zeropt(zeropt_data):
    zeropt_data = split_4(zeropt_data)
    zeropt_id = [struct.unpack("=I", _pt)[0] for _pt in zeropt_data[0::2]]
    zeropt_cnter = [struct.unpack("=I", _pt)[0] for _pt in zeropt_data[1::2]]

    return zeropt_id, zeropt_cnter

def parse_quad(quadrature_data):
    quadrature_data = split_4(quadrature_data)
    quadrature_data_value = [struct.unpack("=I", _pt)[0] for _pt in quadrature_data[0::2]]
    quadrature_data_cnter = [struct.unpack("=I", _pt)[0] for _pt in quadrature_data[1::2]]

    return quadrature_data_value, quadrature_data_cnter

def listen_UDP(ip='127.0.0.1', rec_port=1213):
    """ 
    send and recieve UDP message
    rec_port = port recieving data
    """

    # bind listening port
    rec_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rec_sock.bind((ip, rec_port))

    # recieve and print
    packet_size = 8192    
    while True:
        data, addr = rec_sock.recvfrom(packet_size)

        # how data is stored
        unix_time = data[packet_size-4:]
        pps_data = data[packet_size-36:packet_size-4]
        zeropt_data = data[packet_size-68:packet_size-36]
        sensor_data = data[packet_size-212:packet_size-68]
        parse_pps(pps_data)


if __name__ == '__main__':
    
    packet_size = 8192    

    binarydumps = glob.glob('sample_data/*')
    
    for dump in binarydumps:

        with open(dump, 'rb') as packet_dump:
            data = packet_dump.read()
            txtfile = dump.replace('.data', '.txt').replace('sample_data', 'parsed_txt')

            if len(data) != 0:                
                with open(txtfile, 'w') as parsed_dump:
                    
                    # parse quardrature data
                    parsed_dump.write("QUADRATURE DATA \n")
                    quadrature_data = data[0 : packet_size - 836]
                    quadrature_data_value, quadrature_data_cnter = parse_quad(quadrature_data)
                    for val, cnter in zip(quadrature_data_value, quadrature_data_cnter):
                        parsed_dump.write(f'{val} {cnter}\n')

                    # parse sensor data
                    parsed_dump.write("SENSOR DATA \n")
                    sensor_data = data[packet_size-836:packet_size-68]
                    computer_time, sensor_id, voltage = parse_sensordata(sensor_data)
                    for time, id, volt in zip(computer_time, sensor_id, voltage):
                        parsed_dump.write(f'{time} {id} {volt}\n')


                    # parse zero point data
                    parsed_dump.write("ZERO POINT DATA \n")
                    zeropt_data = data[packet_size-68:packet_size-36]
                    zeropt_id, zeropt_cnter = parse_zeropt(zeropt_data)
                    for id, cnter in zip(zeropt_id, zeropt_cnter):
                        parsed_dump.write(f'{id} {cnter}\n')

                    # parse pulse per second data
                    parsed_dump.write("PULSE PER SECOND DATA \n")
                    pps_data = data[packet_size-36:packet_size-4]
                    pps_id, pps_cnter = parse_pps(pps_data)
                    for id, cnter in zip(pps_id, pps_cnter):
                        parsed_dump.write(f'{id} {cnter}\n')

                    parsed_dump.write("UNIX TIMESTAMP\n")
                    unix_time = data[packet_size - 4:]
                    parsed_dump.write(f'{struct.unpack("=I", unix_time)[0]}')
                    # pps_data = data[packet_size-36:packet_size-4]
                    # zeropt_data = data[packet_size-68:packet_size-36]
                    # sensor_data = data[packet_size-836:packet_size-68]
                    
                    
                    # computer_time, sensor_id, voltage = parse_sensordata(sensor_data)
                    # pps_id, pps_cnter = parse_pps(pps_data)
                    
                    


                    

