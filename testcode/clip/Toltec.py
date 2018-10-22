##
# Toltec.py
##

from string import atoi
from thread import *
from contextlib import closing
from socket import *
import time 
import re
import select
import sys
import os
import shutil
import sys
import string
import numpy as np
import traceback
from time import gmtime, strftime, sleep

try:
  import casperfpga
except Exception as ex:
  print ex
from GbE_init import Gbe
import threading

from functools import partial
import multiprocessing

def toR(index, msg):
    return Toltec.toRoach(index, msg)

udp_iface = "eth0" # Ethernet interface for binding receive socket
udp_iface = "enp4s0f0"
udp_host = "0.0.0.0"
firmware = "./toltec_test_2017_Jul_11_1535.fpg" # Test firmware image
ppc_ip = "192.168.151.55" # IP address of Roach PowerPC

class Toltec():
  bufsize    = 8192
  firstPortNum = 8880
  managerPortNum = 8898

  def __init__ (self):
    try:
      arg = int(sys.argv[1])
    except Exception as e:
      arg = Toltec.managerPortNum

    if arg >= 0 and arg <= 8:
      self.numRoach = arg
      port = Toltec.managerPortNum
    else:
      port = arg
      self.numRoach = 0
        
    self.listenPort = port
    self.thisHost       = '0.0.0.0'
    self.debug = 0x0
    print 'listen at %s:%d'%(self.thisHost, self.listenPort)
    if port == Toltec.managerPortNum:
      print 'manage %d roaches'%(self.numRoach)
            

    #open listen port 
    self.tcpServerSock = socket(AF_INET, SOCK_STREAM)
    self.tcpServerSock.setsockopt(SOL_SOCKET, SO_REUSEADDR,1)
    self.tcpServerSock.bind((self.thisHost, self.listenPort))
    self.tcpServerSock.listen(10)

  def __del__ (self) :
    self.tcpServerSock.close()

  @staticmethod
  def toRoach(index, msg):
    try:
      s = socket(AF_INET, SOCK_STREAM)
      s.settimeout(10)
      sport = Toltec.firstPortNum+index
      s.connect(('localhost', sport))
      s.send(msg)
      data = ''
      while True:
        data1 = s.recv(Toltec.bufsize)
        data += data1.strip()
        if not data1 or data1[len(data1)-1] == '\n': break
      print 'response from %d = %s' % (sport, data)
      s.close()
      return str(index) +' ' +data
    except Exception as e:
      print e
      return str(index) +' refused'
        
  def loop(self) :
    if self.listenPort != Toltec.managerPortNum:
      if not self.init(index=self.listenPort-Toltec.firstPortNum):
        return
    else:
      pool = multiprocessing.Pool()

    self.done = False
      
    while not self.done :
      conn, addres = self.tcpServerSock.accept()
      if(self.debug & 0x1):
        print("accepted client")
      while not self.done:
        msg = conn.recv(self.bufsize).strip()
        if len(msg) == 0 :
          #print('connection is closed by client, closing socket.')
          break
                
        print(msg)

        reply = ''
        status = True

        args = re.sub(r'[\n\r]+', '', msg).split()
        print 'received: ', args
        
        if(self.listenPort == Toltec.managerPortNum):
          if 'manage' in args:
            status = self.manageCommand(args)
            if type(status) == tuple :
              reply_data = status[1]
              status = status[0]
            else:
              reply_data = []
            conn_reply = ''
            if status:
              conn_reply += 'done'
            else:
              conn_reply += 'failed'
            if len(reply_data) > 0:
              reply_data = ','.join(map(str, reply_data))
              conn_reply = '('+conn_reply+',['+reply_data+'])'
            conn_reply += '\n'
            conn.send(conn_reply)
            
          d = {'msg': msg}
          partialToRoach = partial(toR, **d)
          reply = pool.map(partialToRoach, range(self.numRoach))
          print reply
          if status : conn.send(str(reply)+'\n')
          else      : conn.send(msg +' failed\n')

        else:
          status = self.manageCommand(args)
          if type(status) == tuple :
            reply_data = status[1]
            status = status[0]
          else:
            reply_data = []
          conn_reply = ''
          if status:
            conn_reply += 'done'
          else:
            conn_reply += 'failed'
          if len(reply_data) > 0:
            if 'dump' in args:
                conn_reply = reply_data
                if len(reply_data) != 2048:
                    break
                packet_count = (np.fromstring(reply_data[-1],dtype = '>i'))
                print self.listenPort, 'packet_count', packet_count
            else:
                reply_data = ','.join(map(str, reply_data))
                conn_reply = '('+conn_reply+',['+reply_data+'])'
                conn_reply += '\n'
          else:
                conn_reply += '\n'
          conn.send(conn_reply)

        if('quit' in args or 'exit' in args):
          self.done = True

      print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name+' close connection'
      conn.close()

  def _exec(self, cmd, shmemUser):
    print self.getObjectName()+".exec(): "+cmd
    self.manageCommand(cmd.split())
    self.setStatus(2)
    return True

  def manageCommand(self,args) :

    arg_com = {
      'manage' :  [self.manage,1],
      'config' :  [self.config,0],
      'snap' :  [self.snap,1],
      'open' :  [self.open_nc,1],
      'close':  [self.close_nc,0],
      'start' :  [self.start,0],
      'stop':  [self.stop,0],
      'dump':  [self.dump,0],
      'quit':  [self.quit,0],
      'exit':  [self.quit,0],
    }

    if not args[0] in arg_com :
      return False

    commset = arg_com[args[0]]
    argNum  = commset[1]
    if argNum == 0 :
      return commset[0]()
    elif argNum == -1 :
      return commset[0](' '.join(args[1:]))
    else :
      if len(args) < 2:
        return False
      subArg = args[1:commset[1]+1]
      return commset[0](*subArg)
    
  def init(self, index):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    try:
      self.sim = sys.argv[3]
      print 'sim =', self.sim
    except Exception as e:
      self.sim = None

    try:
      udp_port = sys.argv[2]
    except Exception as e:
      pass
    
    roach = None
    if self.sim == None:
      try:
        roach = casperfpga.katcp_fpga.KatcpFpga(ppc_ip, timeout=5.)
      except Exception as ex:
        print 'casperfpga.katcp_fpga.KatcpFpga ', ex
        try:
          roach = casperfpga.CasperFpga(ppc_ip, timeout=5.)
        except Exception as e:
          print 'CasperFpga', e
          return False
        else:
          print 'using casperfpga.CasperFpga'
      else:
        print 'using casperfpga.latcp_fpga.KatcpFpga'
      print 'udp_iface =', udp_iface
    print 'udp_port =', udp_port
    
    self.gbe = Gbe(int(udp_port))
    self.gbe.upload_firmware(roach, ppc_ip, firmware)
    self.gbe.init_reg(roach)
    self.gbe.init()
    self.out_data = [[0]]

    toltecTask = threading.Thread(target=self.gbe.stream_UDP,
                                  args=(roach, udp_iface, 511, self.out_data))
    toltecTask.daemon = True
    toltecTask.start()

    return True

  def manage(self, args):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    self.numRoach = int(args)
    print 'manage', self.numRoach
    return True

  def config(self):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    return True

  def start(self):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    return self.gbe.start_obs()

  def stop(self):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    return self.gbe.stop_obs()

  def open_nc(self, args):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    return self.gbe.open_nc(args)

  def close_nc(self):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    return self.gbe.close_nc()

  def snap(self, args):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    if self.open_nc(1) == False:
      return False
    if self.start() == False:
      return False
    time.sleep(int(args))
    if self.stop() == False:
      return False
    return self.close_nc()

  def dump(self):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    return True,self.out_data[0]
    
  def quit(self):
    print os.path.basename(__file__)+'::'+sys._getframe().f_code.co_name
    self.gbe.done_obs()
    self.done = True
    return True

def main():
  toltec = Toltec()
  try:
    toltec.loop()
  except Exception as e:
    print e
  

if __name__ == '__main__':
  if(len(sys.argv) < 2):
    print "usage: ", sys.argv[0], "num_roaches_to_manage"
    print "usage: ", sys.argv[0], "tcp_port_starting_at_%d udp_port"%Toltec.firstPortNum
    exit()
  main()
  
      

