from netCDF4 import Dataset
import os
import threading
import numpy as np
import socket as sock
import struct
import select
import sys
import time

### This script generates Roach2 test packets. Use with TolTEC test firmware.
### author = Sam Gordon (sbgordo1@asu.edu)

UDP_BUF_SIZE_TIMES = 600
DATA_LEN_DIVISOR = 1
USE_NC = True

class Gbe():

  # IP address corresponding to iface
  dest_ip  = 192*(2**24) + 168*(2**16) + 40*(2**8) + 51

  # source IP address of Roach packets, hardcoded in firmware
  # (source port is hardcoded as: 60000)
  roach_saddr = "192.168.40.71"

  data_len = 8192 # Data size in bytes, hardcoded in firmware
  nc_data_len = data_len/DATA_LEN_DIVISOR
  header_len = 42 # Header size in bytes, hardcoded in firmwmare
  buf_size = data_len + header_len

  f_fpga = 256.0e6 # FPGA clock frequency, 256 MHz

  # Number of data accumulations per output sample.
  # Used to set output packet data rate.
  accum_len = 2**19

  data_rate = f_fpga/accum_len # Output packet data rate

  def __init__(self, dest_port):
      self.dest_port = dest_port
      self.index = dest_port-100*(int)(dest_port/100)
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name, self.dest_port, self.index)

  ### Upload firmware
  def upload_firmware(self, roach, ppc_ip, firmware_file):
      if roach == None:
          print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name +': no roach')
          return
      print('Connecting...')
      katcp_port=7147
      t1 = time.time()
      timeout = 10
      while not roach.is_connected():
         if (time.time() - t1) > timeout:
             raise Exception("Connection timeout to roach")
      time.sleep(0.1)
      if (roach.is_connected() == True):
          print('Connected to the FPGA ')
          roach.upload_to_ram_and_program(str(firmware_file))
      else:
          print('Not connected to the FPGA')
      time.sleep(2)
      print('Connection established to', ppc_ip)
      print('Uploaded', firmware_file)
      return

  ### Initialize GbE parameters
  def init_reg(self, roach):
      if roach == None:
          print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name +': no roach')
          return
      roach.write_int('GbE_tx_destip', self.dest_ip)
      roach.write_int('GbE_tx_destport', self.dest_port)
      roach.write_int('sync_accum_len', self.accum_len - 1)
      roach.write_int('GbE_tx_rst',0)
      roach.write_int('GbE_tx_rst',1)
      roach.write_int('GbE_tx_rst',0)
      roach.write_int('start', 1)
      roach.write_int('sync_accum_reset', 0)
      roach.write_int('sync_accum_reset', 1)
      return

  ### Create a socket for receiving UDP data,
  # bind to eth_iface (raw socket is used here, could be datagram).
  def init_socket(self, eth_iface):
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name, eth_iface)
      sock_fd = sock.socket(sock.AF_PACKET, sock.SOCK_RAW, 3)
      # test gaps
      if UDP_BUF_SIZE_TIMES > 0:
        sock_fd.setsockopt(sock.SOL_SOCKET, sock.SO_RCVBUF, UDP_BUF_SIZE_TIMES*self.buf_size)
      sock_fd.bind((eth_iface, 3))
      return sock_fd 

  ### Create a socket for receiving simulated UDP data
  def sim_socket(self, iface):
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name, self.dest_port)
      sock_fd = sock.socket(sock.AF_INET, sock.SOCK_DGRAM)
      # test gaps
      if UDP_BUF_SIZE_TIMES > 0:
        sock_fd.setsockopt(sock.SOL_SOCKET, sock.SO_RCVBUF, UDP_BUF_SIZE_TIMES*self.buf_size)
      sock_fd.bind(('', self.dest_port))
      self.header_len = 0
      self.buf_size = self.data_len + self.header_len

      return sock_fd 

  ### sock = socket file descriptor
  def wait_for_data(self, sock_fd):
      read, write, error = select.select([sock_fd], [], [])
      while (1):
          for s in read:
              packet = s.recv(self.buf_size)
              if len(packet) == self.buf_size:
                  return packet
              else:
                  pass
      return

  ### Stream set number of packets (Npackets) on Eth interface, iface. 
  ### Prints header and data info for a single channel (0 < chan < 1016)    
  def stream_UDP(self, roach, iface, chan, out_data):
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name)
      old_dowrite = 0
      if roach == None:
          print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name +': no roach')
          sock_fd = self.sim_socket(iface)
      else:
          sock_fd = self.init_socket(iface)
          roach.write_int('GbE_pps_start', 0)
          roach.write_int('GbE_pps_start', 1)
      t0 = time.time()
      self.previous_count = 0
      self.dodoing = True
      while self.dodone == 0:
        if(old_dowrite != self.dowrite):
          print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name +' self.dowrite = ' +str(self.dowrite))
        old_dowrite = self.dowrite
        packet_data = self.wait_for_data(sock_fd)
        if USE_NC:
          self.write_packet(packet_data, out_data)
        else:
          if self.dowrite == 1:
            self.bin_file.write(packet_data)
            #for i in range(1):
              #self.bin_file.write(packet_data)
              #self.bin_file.write(packet_data[:self.header_len])
              #self.bin_file.write(packet_data[-self.nc_data_len:])
      sock_fd.close()
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name+' done')
      self.dodoing = False
      return

  def write_packet(self, packet_data, out_data):
      if self.header_len > 0:
        header = packet_data[:self.header_len]
        saddr = np.fromstring(header[26:30], dtype = "<I")
        saddr = sock.inet_ntoa(saddr) # source addr
        ### Filter on source IP ###
        if (False and saddr != self.roach_saddr):
          print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name +' saddr = ' +saddr +' roach_saddr = ' +self.roach_saddr)
          return
      else:
        saddr = '127.0.0.1'

      if USE_NC:
        if self.header_len > 0:
          ### Print header info ###
          daddr = np.fromstring(header[30:34], dtype = "<I")
          daddr = sock.inet_ntoa(daddr) # dest addr
          smac = np.fromstring(header[6:12], dtype = "<B")
          print(smac)
          smac = struct.unpack(b'BBBBBB', smac)
          src = np.fromstring(header[34:36], dtype = ">H")[0]
          dst = np.fromstring(header[36:38], dtype = ">H")[0]
        else:
          daddr = '127.0.0.1'
          smac = struct.unpack(b'BBBBBB', b'000000')
          src = 60001
          dst = 60001
        
        ### Parse packet data ###

        data = np.fromstring(packet_data[self.header_len:],dtype = '<i').astype('int32')
        roach_checksum = (np.fromstring(packet_data[-16:-12],dtype = '>I'))

        # seconds elapsed since 'pps_start'
        sec_ts = (np.fromstring(packet_data[-16:-12],dtype = '>I'))
        # milliseconds since last packet
        fine_ts = np.round((np.fromstring(packet_data[-12:-8],dtype = '>I').astype('float')/self.f_fpga)*1.0e3,3)
        
      # raw packet count since 'pps_start'
      packet_count = (np.fromstring(packet_data[-8:-4],dtype = '>I'))
      packet_status = (np.fromstring(packet_data[-4:],dtype = '>I'))
      #print sec_ts, packet_count, packet_status
      if self.dowrite == 1:
          if USE_NC:
            t1000 = time.time()*1000
            self.nc_lock.acquire()
            if self.nc_data_len > 0:
              self.nc_data[self.count,:] = data[:self.nc_data_len/4]
            self.nc_sec[self.count,:] = sec_ts
            self.nc_msec[self.count,:] = fine_ts
            self.nc_msec_clip[self.count,:] = t1000
            self.nc_packet_count[self.count,:] = packet_count
            # print t, packet_count, sec_ts, fine_ts
            self.nc_lock.release()
          else:
            for i in range(1):
              self.bin_file.write(packet_data)
              #self.bin_file.write(packet_data[:self.header_len])
              #self.bin_file.write(packet_data[-self.nc_data_len:])
          self.count += 1
      else:
          self.count = 0
      ### Check for missing packets ###
      if self.previous_count > 0:
        if (packet_count[0] - self.previous_count != 1):
          print(self.index, 'missing packets', self.previous_count, packet_count[0]-self.previous_count)
          #break
      self.previous_count = packet_count[0]
      if USE_NC:
        chan = 0
        if chan  < 512:
            I = data[0 + chan]
            Q = data[512 + chan]
        else:
            I = data[1024 + chan-512]
            Q = data[1536 + chan-512]
        phase = np.degrees(np.arctan2([Q],[I]))
        if out_data and np.mod(packet_count[0], int(Gbe.data_rate)) == 0:
          out_data[0] = data
          #self.doprint = True
        if self.doprint:
            self.doprint = False
            print()
            print("Roach chan =", chan)
            print("Packet length =", len(packet_data))
            print("src MAC = %x:%x:%x:%x:%x:%x" % smac)
            print("src IP : src port =", saddr,":", src)
            print("dst IP : dst port  =", daddr,":", dst)
            print("Roach chksum =", roach_checksum[0])
            print("Time sec =", time.time())
            print("PPS sec =", sec_ts[0])
            print("PPS msec =", fine_ts[0])
            print("Packet count =", packet_count[0])
            print("I (unscaled) =", I)
            print("Q (unscaled) =", Q)

  def init(self):
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name)
      self.nc_lock = threading.Lock()
      self.dowrite = 0
      self.dodone = 0
      self.doprint = True
      self.count = 0
      self.nc_file = None
      self.daemon = True
      print('data len', self.nc_data_len)

  def open_nc(self, obsNum):
      self.nc_lock.acquire()
      dataDir = "/data_toltec/toltec"
      dataDir = "."
      if USE_NC:
        zlib = False
        complevel = 1
        filename = '%s/r%d_%s.nc'%(dataDir, self.index, obsNum)
        print('open', filename)
        self.nc_file = Dataset(filename, 'w')
        self.nc_file.createDimension('data_len', self.nc_data_len/4)
        self.nc_file.createDimension('dim_one', 1)
        self.nc_file.createDimension('time', None)
        self.nc_obsnum = self.nc_file.createVariable('Header.Dcs.ObsNum',np.dtype('int32'),zlib=zlib,complevel=complevel)
        self.nc_obsnum[:] = obsNum
        self.nc_index = self.nc_file.createVariable('Header.Toltec.RoachIndex',np.dtype('int32'),zlib=zlib,complevel=complevel)
        self.nc_index[:] = self.index
        self.nc_data = self.nc_file.createVariable('Data.Toltec.Data',np.dtype('int32').char,('time', 'data_len'),zlib=zlib,complevel=complevel)
        self.nc_sec = self.nc_file.createVariable('Data.Toltec.Sec',np.dtype('int32').char,('time', 'dim_one'),zlib=zlib,complevel=complevel)
        self.nc_msec = self.nc_file.createVariable('Data.Toltec.Msec',np.dtype('float').char,('time', 'dim_one'),zlib=zlib,complevel=complevel)
        self.nc_msec_clip = self.nc_file.createVariable('Data.Toltec.MsecClip',np.dtype('float').char,('time', 'dim_one'),zlib=zlib,complevel=complevel)
        self.nc_packet_count = self.nc_file.createVariable('Data.Toltec.Count',np.dtype('int32').char,('time', 'dim_one'),zlib=zlib,complevel=complevel)
        print('opened nc file')
      else:
        self.bin_file = open('%s/r%d_%s.bin'%(dataDir, self.index, obsNum), 'w', 0)
      self.nc_lock.release()
      return True

  def close_nc(self):
      print('close nc file')
      if self.nc_file == None:
        return False
      self.nc_lock.acquire()
      if USE_NC:
        self.nc_file.close()
      else:
        self.bin_file.close()
      self.nc_lock.release()
      return True

  def start_obs(self):
      print('start obs')
      if self.nc_file == None:
        return False
      self.dowrite = 1
      return True

  def stop_obs(self):
      print('stop obs')
      if self.nc_file == None:
        return False
      self.dowrite = 0
      return True

  def done_obs(self):
      self.dodone = 1
      while self.dodoing == True:
          time.sleep(0.1)

  def run_test(self):
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name)
      self.doprint = True
      old_dowrite = 0
      self.count = 0
      t0 = time.time()
      self.dodoing = True
      while self.dodone == 0:
          if(old_dowrite != self.dowrite):
              print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name +' self.dowrite = ' +str(self.dowrite))
              old_dowrite = self.dowrite
          data = np.zeros((self.data_len/4), dtype='<i')
          iii = np.iinfo('<i')
          if self.dowrite == 1:
              t = time.time()
              self.nc_lock.acquire()
              data = np.random.randint(iii.max, size=self.data_len/4)
              if self.nc_data_len > 0:
                self.nc_data[self.count,:] = data[:self.nc_data_len/4]
              now = time.time()
              self.nc_sec[self.count,:] = now
              self.nc_msec[self.count,:] = now*1000
              self.nc_msec_clip[self.count,:] = now*1000
              self.nc_packet_count[self.count,:] = self.count
              # print t, packet_count, sec_ts, fine_ts
              self.nc_lock.release()
              self.count += 1
          else:
              self.count = 0
          if self.count > 29270:
            self.dodone = 1
          #time.sleep(0.5/Gbe.data_rate)
      print(os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name+' done')
      self.dodoing = False
      return



def main():
  gbe = Gbe(0)
  gbe.init()
  gbe.open_nc(1)
  gbe.start_obs()
  gbe.run_test()
  gbe.stop_obs()
  gbe.close_nc()
  

if __name__ == '__main__':
    main()
    
