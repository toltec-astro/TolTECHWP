import sys
import socket
import glob
import struct
import numpy as np

def split_4(array):
    return [array[i:i+4] for i in range(0, len(array), 4)]    

def parse_sensordata(sensor_data):
    sensor_data = split_4(sensor_data)
    
    computer_time = [struct.unpack("=I", _pt)[0] for _pt in sensor_data[0::3]]
    sensor_id = [struct.unpack("=I", _pt)[0] for _pt in sensor_data[1::3]]
    voltage = [struct.unpack("=I", _pt)[0] for _pt in sensor_data[2::3]]
    
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


# loop through
def listen_UDP(ip='127.0.0.1', rec_port=6969):
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
        #for i in split_4(data):
        #    print(struct.unpack("=I", i)[0])
        list_form = [struct.unpack("=I", i)[0] for i in split_4(data)]

        # first set of 166 values
        encoder_1 = list_form[0:166*3]
        print('encoder_1', len(encoder_1), encoder_1[0:3], encoder_1[-1])

        padding_1 = list_form[498:498+14]
        print('padding_1', len(padding_1), padding_1)

        encoder_2 = list_form[512:512 + 166 *3]
        print('encoder_2', len(encoder_2), encoder_2[0], encoder_2[-1])

        padding_2 = list_form[1010:1010+14]
        print('padding_2', len(padding_2), padding_2)

        encoder_3 = list_form[1024:1024 + 166*3]
        print('encoder_3', len(encoder_3), encoder_3[0], encoder_3[-1])

        padding_3 = list_form[1522:1522+14]
        print('padding_3', len(padding_3), padding_3)

        encoder_4 = list_form[1536:1536 + 2 * 3]
        print('encoder_4', len(encoder_4), encoder_4[0], encoder_4[-1])
        

        zeropt = list_form[1542:1542 + (10 * 3)]
        print('zerpot', len(zeropt), zeropt)

        pps = list_form[1572:1572 + (3 * 4)]
        print('pps', len(pps), pps)

        sensor_data = list_form[1584:1584+(32 * 3)]
        print('sensor_data', len(sensor_data), sensor_data)
        
        encoder_data = encoder_1 + encoder_2 + encoder_3 + encoder_4
        encoder_data_arr = np.reshape(np.array(encoder_data), (500,3))
        sensor_data_arr = np.reshape(np.array(sensor_data), (32,3))
        packet_number = socket.ntohl(list_form[2047])


        encoder_file = '/data/toltec/readout/packet_monitoring/encoder.txt'
        np.savetxt(encoder_file, encoder_data_arr, delimiter="\t")

        sensor_file = '/data/toltec/readout/packet_monitoring/sensor.txt'
        np.savetxt(sensor_file, sensor_data_arr, delimiter="\t")
        


        #import matplotlib.pyplot as plt
        #plt.plot(encoder_data_arr.T[0], encoder_data_arr.T[2])
        #plt.show()
        # zero point 10 
        # pps 4
        # sensor data 16


        #print(list_form[0:166*3])
        # padding of zeros
        #print(list_form[166*3 + 1:166 *3 +14])

        # second set of 166 values
        #print(list_form[(166*3+15):(166*3*2+14)])
        # padding of zeros
        #print(list_form[(166*3*2+15):(166*3*2+28)])
        


if __name__ == '__main__':

    
    listen_UDP()
   
    # return the data in arrays
    # append to array
    
    



    # packet_size = 8192    

    # binarydumps = glob.glob('*.data')
    
    # for dump in binarydumps:

    #     with open(dump, 'rb') as packet_dump:
    #         data = packet_dump.read()
    #         txtfile = dump.replace('.data', '.txt')

    #         if len(data) != 0:                
    #             with open(txtfile, 'w') as parsed_dump:
                    
    #                 # parse quardrature data
    #                 parsed_dump.write("QUADRATURE DATA \n")
    #                 quadrature_data = data[0 : packet_size - 836]
    #                 quadrature_data_value, quadrature_data_cnter = parse_quad(quadrature_data)
    #                 for val, cnter in zip(quadrature_data_value, quadrature_data_cnter):
    #                     parsed_dump.write(f'{val} {cnter}\n')

    #                 # parse sensor data
    #                 parsed_dump.write("SENSOR DATA \n")
    #                 sensor_data = data[packet_size-836:packet_size-68]
    #                 computer_time, sensor_id, voltage = parse_sensordata(sensor_data)
    #                 for time, id, volt in zip(computer_time, sensor_id, voltage):
    #                     parsed_dump.write(f'{time} {id} {volt}\n')


    #                 # parse zero point data
    #                 parsed_dump.write("ZERO POINT DATA \n")
    #                 zeropt_data = data[packet_size-68:packet_size-36]
    #                 zeropt_id, zeropt_cnter = parse_zeropt(zeropt_data)
    #                 for id, cnter in zip(zeropt_id, zeropt_cnter):
    #                     parsed_dump.write(f'{id} {cnter}\n')

    #                 # parse pulse per second data
    #                 parsed_dump.write("PULSE PER SECOND DATA \n")
    #                 pps_data = data[packet_size-36:packet_size-4]
    #                 pps_id, pps_cnter = parse_pps(pps_data)
    #                 for id, cnter in zip(pps_id, pps_cnter):
    #                     parsed_dump.write(f'{id} {cnter}\n')

    #                 parsed_dump.write("UNIX TIMESTAMP\n")
    #                 unix_time = data[packet_size - 4:]
    #                 parsed_dump.write(f'{struct.unpack("=I", unix_time)[0]}')
    #                 # pps_data = data[packet_size-36:packet_size-4]
    #                 # zeropt_data = data[packet_size-68:packet_size-36]
    #                 # sensor_data = data[packet_size-836:packet_size-68]
                    
                    
    #                 # computer_time, sensor_id, voltage = parse_sensordata(sensor_data)
    #                 # pps_id, pps_cnter = parse_pps(pps_data)
                    
                    
