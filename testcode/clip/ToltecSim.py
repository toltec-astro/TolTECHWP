import socket
import sys
import os
import numpy as np
import time
import struct
import socket as sock
from GbE_init import Gbe
from Toltec import Toltec

class ToltecSim():
    def __init__ (self):
        pass
    
    def init(self, dest_host, dest_port):
        print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name, dest_host, dest_host, dest_port)
        self.index = dest_port-100*(int)(dest_port/100)
        self.dest_host = dest_host
        self.dest_port = dest_port
        self.header =  bytearray([1]*(Gbe.header_len))
        self.data = np.zeros(int(Gbe.data_len/4), dtype='<i')
        for i in range(len(self.data)):
            self.data[i] = i
            
        try :
            self.beg = 1
            if self.beg == 0:
                self.s = sock.socket(sock.AF_PACKET, sock.SOCK_RAW, 3)
                self.s.bind(('eth0', 3))
            else:
                self.beg = Gbe.header_len # will always be this way
                self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            
            print('sim socket created to', self.dest_host, self.dest_port)
        except socket.error as msg :
            print('Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
            self.s = None
            return False

        self.sndbuflen = len(self.header) +  len(self.data.tobytes())
        self.s.setsockopt(socket.SOL_SOCKET,socket.SO_SNDBUF,self.sndbuflen)
        print(self.sndbuflen, len(self.data.tobytes()), Gbe.data_rate, 1./Gbe.data_rate)

        return True

    def loop(self):
        sec = np.zeros((1), dtype='>I')
        msec = np.zeros((1), dtype='>I')
        count = np.zeros((1), dtype='>I')
        status = np.zeros((1), dtype='>I')
        status[0] = 123
        self.done = False
        xy = 45
        ij = 45
        ii = 0
        while not self.done:
            t = time.time()
            x, y = np.meshgrid(np.linspace(-1,1,xy), np.linspace(-1,1,xy))
            d = np.sqrt(x*x+y*y/.75)
            #sigma, mu = 1.0*np.mod(t,10)/10, 0.0
            sigma, mu = 1.0*0.2, 0.0
            g = np.exp(-( (d-mu)**2 / ( 2.0 * sigma**2 ) ) )

            if False or np.mod(count, int(Gbe.data_rate)) == 0:
                for i in range(len(self.data)):
                    self.data[i] = 100+self.index*50

                self.data[ii] = 1000
                ii += 1
                if ii >= 300:
                    ii = 0

                if False:
                    ii = int(np.mod(t,80))-ij
                    #ii = 0
                    for j in range(ij):
                        for i in range(ij):
                            iii = i+ii
                            if iii >= 0 and iii < xy:
                                self.data[i+j*ij] = g[iii,j]*1000

            #gg = g.flatten()
            #self.data[0:len(gg)] = gg*1000
            sec[0] = int(t)
            ms = int(t*1000)-sec[0]*1000
            msec[0] = ms*Gbe.f_fpga/1000
            self.header[26:30] = sock.inet_aton(Gbe.roach_saddr)
            self.header[30:34] = sock.inet_aton(sock.gethostbyname(self.dest_host))
            self.header[34:36] = struct.pack('!1H',self.dest_port)
            self.header[36:38] = struct.pack('!1H',self.dest_port)
            packet_data = self.header+self.data.tostring()
            # Put time at the end of data (only important for KIDS)
            packet_data[-16:-12] = sec.tostring()
            packet_data[-12:-8] = msec.tostring()
            packet_data[-8:-4] = count.tostring()
            packet_data[-4:] = status.tostring()
            if self.beg == 0: # is never true
                self.s.send(packet_data[self.beg:])
            else: # only data is sent not header #######
                self.s.sendto(packet_data[self.beg:], (self.dest_host, self.dest_port))
            count[0] += 1
            #print t, count, sec, msec
            time.sleep(1./Gbe.data_rate/2.)

        print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name+' done')
        self.s.close()

    def done_sim(self):
        self.done = True

def main(udp_host, udp_port):
  toltecSim = ToltecSim()
  if toltecSim.init(udp_host, udp_port):
    toltecSim.loop()
  print('ToltecSim done')

if __name__ == '__main__':
    if(len(sys.argv) < 3):
        print("usage: ", sys.argv[0], "udp_host udp_port")
        exit()
    main((sys.argv[1]), int(sys.argv[2]))
