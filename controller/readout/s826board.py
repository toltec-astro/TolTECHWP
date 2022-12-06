
import ctypes
import numpy as np

SHARED_OBJECT_PATH = '/home/poluser/TolTECHWP/vend/sdk_826_linux_3.3.12/middleware/lib826_64.so'

def errhandle(errcode, name=None):
    # it doesn't really matter since you died anyway
    if errcode < 0:
        if not name:
            name = 'Unspecified Command'
        if errcode < -15:
            errname = 'S826_ERR_OSSPECIFIC'
            descrip = 'Error specific to the operating system. Contact Sensoray.'
            raise RuntimeError(f'{name} Error Code: {errcode} \n {errname} \n {descrip}')
        raise RuntimeError(f'{name} Error Code: {errcode}')    

class S826Board(object):
    """ Sensoray S826 Board Object """

    def __init__(self, config):

        self.load_config(config)
        self.s826_dll = ctypes.cdll.LoadLibrary(SHARED_OBJECT_PATH)
        self.open()

    def load_config(self, config):

        # load power for sensors
        snsrpwr = config['sensors.power']
        self.voltage_range     = snsrpwr['output_voltage_range']
        self.voltage_setpoint  = snsrpwr['output_voltage_setpoint'] 

        # load counterids
        self.countpps        = config['counter.pps']['counter_num']
        self.counttime       = config['counter.timer']['counter_num'] 
        self.countzeropoint  = config['counter.zeropt']['counter_num']
        self.countquad       = config['counter.quad']['counter_num']

        # load card save interval (in microseconds)
        self.datausec = config['intervals']['carddata_intervals']


    def open(self):
        """ open connection to s826 board """
        errcode = self.s826_dll.S826_SystemOpen()
        if errcode != 1:
            self.close()
            
            # specifically because typically this happens
            if errcode == 0:
                raise RuntimeError(f'No Boards Detected - Check Driver - S826_SystemOpen Error Code: {errcode}')    
            
            raise RuntimeError(f'S826_SystemOpen Error Code: {errcode}')

        self.board_id = np.int(np.log(errcode) / np.log(2))
        print(f'System Open - Board ID: {self.board_id}')

    def close(self):
        """ close connection to s826 board """
        self.s826_dll.S826_SystemClose()
        print('System Closed')

    def configure(self):
        """ run all on configurations """

        self._configure_power_sensors()
        self._configure_pps()
        self._configure_zero_point()
        self._configure_timer()
        self._configure_quadrature()
        self._configure_sensors()
        

    def _configure_power_sensors(self):
        # hardcoded
        runmode = 0
        analog_out_cnt = 8

        # configuration # voltage_setpoint = 0xFFFF # 5V
        output_range     = int(self.voltage_range)
        voltage_setpoint = int(self.voltage_setpoint, 16)

        #print(voltage_setpoint)
        for channel in range(analog_out_cnt):
            pwr = self.s826_dll.S826_DacRangeWrite(self.board_id,
                channel, output_range, runmode)
            errhandle(pwr, name=f'S826_DacRangeWrite [{channel}]')

            pwr_set = self.s826_dll.S826_DacDataWrite(self.board_id, 
                channel, voltage_setpoint, runmode)
            errhandle(pwr_set, name=f'S826_DacDataWrite [{channel}]')

    def _configure_sensors(self):

        # hardcoded 
        analog_in_cnt       = 16
        S826_ADC_GAIN_1     = 0
        S826_BITWRITE       = 0

        for sens in range(analog_in_cnt):
            sensor_config = self.s826_dll.S826_AdcSlotConfigWrite(
                self.board_id, sens, sens, 5000, S826_ADC_GAIN_1
            )
            errhandle(sensor_config, name=f'S826_AdcSlotConfigWrite [{sens}]')

        err = self.s826_dll.S826_AdcSlotlistWrite(self.board_id, 0xFFFF, S826_BITWRITE)
        errhandle(err, name=f'S826_AdcSlotlistWrite')

        err = self.s826_dll.S826_AdcTrigModeWrite(self.board_id, 0) 
        errhandle(err, name=f'S826_AdcTrigModeWrite')

        err = self.s826_dll.S826_AdcEnableWrite(self.board_id, 1)  
        errhandle(err, name=f'S826_AdcEnableWrite')


    def _configure_pps(self):

        # harcoded
        S826_CM_K_AFALL = (1 << 4)
        S826_SSRMASK_IXRISE = (1 << 4)
        S826_BITWRITE = 0

        countpps = int(self.countpps)

        pps_flags = self.s826_dll.S826_CounterModeWrite(
            self.board_id, countpps, S826_CM_K_AFALL 
        )   
        errhandle(pps_flags, name=f'S826_CounterModeWrite')
    
        pps_flags = self.s826_dll.S826_CounterFilterWrite(
            self.board_id, countpps,        # Filter Configuration:
            (1 << 31) | (1 << 30) | # Apply to IX and clock
            0                     # 20 * (F * 1) Not true any more >>100x20ns = 2us filter constant
        )                 
        errhandle(pps_flags, name=f'S826_CounterFilterWrite')

        pps_flags = self.s826_dll.S826_CounterSnapshotConfigWrite(
            self.board_id, countpps,  
            S826_SSRMASK_IXRISE, # Acquire counts upon tick rising edge. 
            S826_BITWRITE
        )
        errhandle(pps_flags, name=f'S826_CounterSnapshotConfigWrite')

        pps_flags = self.s826_dll.S826_CounterStateWrite(
            self.board_id, countpps, 1 # start the counter
        )
        errhandle(pps_flags, name=f'S826_CounterStateWrite')

    def _configure_zero_point(self):

        S826_CM_K_AFALL = (1 << 4)
        S826_CM_K_ARISE = (0 << 4)
        S826_SSRMASK_IXRISE = (1 << 4)
        S826_BITWRITE = 0

        countzeropoint = int(self.countzeropoint)

        zeropoint_flags = self.s826_dll.S826_CounterModeWrite(
            self.board_id, 
            countzeropoint,      # Configure counter:
            S826_CM_K_AFALL 
        )
        errhandle(zeropoint_flags, name=f'S826_CounterModeWrite')

        zeropoint_flags = self.s826_dll.S826_CounterFilterWrite(
            self.board_id, countzeropoint,  # Filter Configuration:
            (1 << 31) | (1 << 30) |         # Apply to IX and clock
            10                             # 10x20ns = 2us filter constant
        )
        errhandle(zeropoint_flags, name=f'S826_CounterFilterWrite')

        zeropoint_flags = self.s826_dll.S826_CounterSnapshotConfigWrite(
            self.board_id, 
            countzeropoint,  
            S826_SSRMASK_IXRISE, # Acquire counts upon tick rising edge. 
            #(1 << 6),
            S826_BITWRITE
        )
        errhandle(zeropoint_flags, name=f'S826_CounterSnapshotConfigWrite')

        zeropoint_flags = self.s826_dll.S826_CounterStateWrite(
            self.board_id, countzeropoint, 1); # start the counter
        errhandle(zeropoint_flags, name=f'S826_CounterStateWrite')
        
    def _configure_timer(self):
        """ Configure Timer """    
        S826_CM_K_1MHZ     = (2 << 4)   
        S826_CM_PX_START   = (1 << 24)
        S826_CM_PX_ZERO    = (1 << 13)
        S826_CM_UD_REVERSE = (1 << 22)
        S826_CM_OM_NOTZERO = (3 << 18)

        counttime = int(self.counttime)
        datausec  = int(self.datausec)
        #print(datausec)

        tmr = self.s826_dll.S826_CounterModeWrite(
            self.board_id, counttime,
            S826_CM_K_1MHZ | 
            S826_CM_PX_START | S826_CM_PX_ZERO | 
            S826_CM_UD_REVERSE | S826_CM_OM_NOTZERO
            )
        errhandle(tmr, name='S826_CounterModeWrite')

        # preload the data
        timer_flags = self.s826_dll.S826_CounterPreloadWrite(
            self.board_id, counttime, 0, datausec)
        errhandle(timer_flags, name='S826_CounterPreloadWrite')

        # start the timer running.
        timer_flags = self.s826_dll.S826_CounterStateWrite(
            self.board_id, counttime, 1);    
        errhandle(timer_flags, name='S826_CounterStateWrite')

    def _configure_quadrature(self):

        S826_CM_K_QUADX4 = (7 << 4)
        S826_CM_XS_CH0   = 2
        S826_SSRMASK_IXRISE = (1 << 4)
        S826_BITWRITE       = 0

        counttime = int(self.counttime)
        countquad = int(self.countquad)

        quaderrcode = self.s826_dll.S826_CounterModeWrite(
            self.board_id, countquad,        # Configure counter:
            S826_CM_K_QUADX4 |               # Quadrature x1/x2/x4 multiplier
            #S826_CM_K_ARISE |               # clock = ClkA (external digital signal)
            #S826_XS_100HKZ |                # route tick generator to index input
            (counttime + S826_CM_XS_CH0)     # route CH1 to Index input
        )   
        errhandle(quaderrcode, name='S826_CounterModeWrite')

        # # preload data
        quaderrcode = self.s826_dll.S826_CounterPreloadWrite(
            self.board_id, countquad, 0, 0        
        )
        errhandle(quaderrcode, name='S826_CounterPreloadWrite')

        quaderrcode = self.s826_dll.S826_CounterPreload(
            self.board_id, countquad, 0, 1        
        )
        errhandle(quaderrcode, name='S826_CounterPreload')
        # #

        quaderrcode = self.s826_dll.S826_CounterSnapshotConfigWrite(
            self.board_id, countquad, # Acquire counts upon tick rising edge.
            S826_SSRMASK_IXRISE, 
            S826_BITWRITE
        )
        errhandle(quaderrcode, name='S826_CounterSnapshotConfigWrite')
  
        quaderrcode = self.s826_dll.S826_CounterStateWrite(
            self.board_id, countquad, 1) # start the counter
        errhandle(quaderrcode, name='S826_CounterStateWrite')

if __name__ == '__main__':
    
    import configparser
    import time
    import ctypes
    import numpy as np

    config = configparser.ConfigParser()
    configfile = 'testconfig.ini'
    config.read(configfile)
    
    test_S826Board = S826Board(config)
    test_S826Board.configure()

    S826_WAIT_INFINITE = 0xFFFFFFFF
    
    slotpylist = [0 for i in range(16)]
    slotval = (ctypes.c_int32 * 16)(*slotpylist)
    slotlist = ctypes.c_uint(0xFFFF)

    sensor_interval = float(config['intervals']['sensor_intervals'])
         
    for _ in range(2):

        timestamp = int(time.time())
        errcode = test_S826Board.s826_dll.S826_AdcRead(
            test_S826Board.board_id, slotval, #ctypes.addressof(slotval), 
            None, ctypes.pointer(slotlist), S826_WAIT_INFINITE
        )
        errhandle(errcode, 'S826_AdcRead')

        result = np.ctypeslib.as_array(slotval)
        adcdata  = result & 0xFFFF
        #burstnum = np.uint8(result >> 24)
        voltage  = (adcdata / (0x7FFF)) * 10
        
        temp_V_OUT = voltage[0]
        humi_V_OUT = voltage[1]
        T = (temp_V_OUT / 0.01) # degrees C
        sensor_RH =  ((humi_V_OUT / 5) - 0.16) / 0.0062
        true_RH   = (sensor_RH) / (1.0546 - (0.00216 * T))

        print(f'Temperature: {T:0.1f} C ({((T * 9/5) + 32):0.1f} F) \t Humidity: {true_RH:0.1f} %')
        time.sleep(1)

    for _ in range(1000):
        
        counts = ctypes.c_uint() 
        tstamp = ctypes.c_uint() 
        reason = ctypes.c_uint()

        counts_ptr = ctypes.pointer(counts)
        tstamp_ptr = ctypes.pointer(tstamp)
        reason_ptr = ctypes.pointer(reason)

        # read again
        errcode = test_S826Board.s826_dll.S826_CounterSnapshotRead(
            test_S826Board.board_id, 1,
            counts_ptr, tstamp_ptr, 
            reason_ptr, 0
        )

        print((errcode, tstamp.value, counts.value, int(time.time())))
        time.sleep(1)
    test_S826Board.close()
