using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Runtime.InteropServices;


static class mid826 {

    // CONSTANTS ================================================================================================

    // Middleware library name
    // Change to "s826_64.dll" for 64-bit system
    public const string S826_DLL = "lib826_64.so";

    // Resource limits
    // Maximum number of boards supported by driver and middleware
    public const Int32 S826_MAX_BOARDS = 16;
    // Number of counter channels
    public const Int32 S826_NUM_COUNT = 6;
    // Number of application-accessible analog input slots
    public const Int32 S826_NUM_ADC = 16;
    // Number of analog output channels
    public const Int32 S826_NUM_DAC = 8;
    // Number of general-purpose digital I/O channels
    public const Int32 S826_NUM_DIO = 48;

    // Error codes
    // No error
    public const Int32 S826_ERR_OK = 0;
    // Illegal board number
    public const Int32 S826_ERR_BOARD = (-1);
    // Illegal argument value
    public const Int32 S826_ERR_VALUE = (-2);
    // Device not ready or timeout waiting for device
    public const Int32 S826_ERR_NOTREADY = (-3);
    // Wait cancelled
    public const Int32 S826_ERR_CANCELLED = (-4);
    // Driver call failed
    public const Int32 S826_ERR_DRIVER = (-5);
    // Missed adc trigger
    public const Int32 S826_ERR_MISSEDTRIG = (-6);
    // Adc data buffer overflowed
    public const Int32 S826_ERR_BUFOVERFLOW = (-7);
    // Two boards set to same board number
    public const Int32 S826_ERR_DUPADDR = (-9);
    // Board is not open
    public const Int32 S826_ERR_BOARDCLOSED = (-10);
    // Can't create mutex
    public const Int32 S826_ERR_CREATEMUTEX = (-11);
    // Can't map board to memory address
    public const Int32 S826_ERR_MEMORYMAP = (-12);
    // Can't allocate memory
    public const Int32 S826_ERR_MALLOC = (-13);
    // Write protected
    public const Int32 S826_ERR_PROTECTED = (-14);
    // Port-specific error (base error number)
    public const Int32 S826_ERR_OSSPECIFIC = (-100);

    // Analog input range codes
    // -10V to +10V
    public const Int32 S826_ADC_GAIN_1 = 0;
    // -5V to +5V
    public const Int32 S826_ADC_GAIN_2 = 1;
    // -2V to +2V
    public const Int32 S826_ADC_GAIN_5 = 2;
    // -1V to +1V
    public const Int32 S826_ADC_GAIN_10 = 3;

    // Analog output range codes
    // 0 to +5V
    public const Int32 S826_DAC_SPAN_0_5 = 0;
    // 0 to +10V
    public const Int32 S826_DAC_SPAN_0_10 = 1;
    // -5V to +5V
    public const Int32 S826_DAC_SPAN_5_5 = 2;
    // -10V to +10V
    public const Int32 S826_DAC_SPAN_10_10 = 3;

    // Wait types
    // Wait for all listed events
    public const Int32 S826_WAIT_ALL = 0;
    // Wait for any listed event
    public const Int32 S826_WAIT_ANY = 1;

    // Counter snapshot reason flags
    public const Int32 S826_SSR_QUADERR = 256;
    public const Int32 S826_SSR_SOFT = 128;
    public const Int32 S826_SSR_EXTIN_RISE = 64;
    public const Int32 S826_SSR_EXTIN_FALL = 32;
    public const Int32 S826_SSR_IX_RISE = 16;
    public const Int32 S826_SSR_IX_FALL = 8;
    public const Int32 S826_SSR_ZEROCOUNT = 4;
    public const Int32 S826_SSR_MATCH1 = 2;

    public const Int32 S826_SSR_MATCH0 = 1;
    // Other
    // Size of dio data arrays
    public const Int32 S826_NUM_DIO_QUADLETS = 2;

    public static Int32 SystemOpen() {
        return S826_SystemOpen();

    }

    public static Int32 SystemClose() {
        return S826_SystemClose();
    }

    public static Int32 VersionRead(UInt32 board, ref UInt32 api, ref UInt32 driver, ref UInt32 bdrev, ref UInt32 fpgarev) {
        return S826_VersionRead(board, ref api, ref driver, ref bdrev, ref fpgarev);
    }

    public static Int32 TimestampRead(UInt32 board, ref UInt32 timestamp) {
        return S826_TimestampRead(board, ref timestamp);
    }

    //Safemode ================================================================================================================================

    public static Int32 SafeControlRead(UInt32 board, ref UInt32 settings) {
        return S826_SafeControlRead(board, ref settings);
    }

    public static Int32 SafeControlWrite(UInt32 board, UInt32 settings, UInt32 mode) {
        return S826_SafeControlWrite(board, settings, mode);
    }

    public static Int32 SafeWrenRead(UInt32 board, ref UInt32 enable) {
        return S826_SafeWrenRead(board, ref enable);
    }

    public static Int32 SafeWrenWrite(UInt32 board, UInt32 data) {
        return S826_SafeWrenWrite(board, data);
    }

    //ADC ================================================================================================================================

    public static Int32 AdcCalRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
double[] slope, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
Int32[] offset, ref Int32 valid) {
        return S826_AdcCalRead(board, slope, offset, ref valid);
    }

    public static Int32 AdcCalWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
double[] slope, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
Int32[] offset) {
        return S826_AdcCalWrite(board, slope, offset);
    }

    public static Int32 AdcSlotConfigWrite(UInt32 board, UInt32 slot, UInt32 chan, UInt32 tsettle, Int32 range) {
        return S826_AdcSlotConfigWrite(board, slot, chan, tsettle, range);
    }

    public static Int32 AdcSlotConfigRead(UInt32 board, UInt32 slot, ref UInt32 chan, ref UInt32 tsettle, ref UInt32 range) {
        return S826_AdcSlotConfigRead(board, slot, ref chan, ref tsettle, ref range);
    }

    public static Int32 AdcSlotlistWrite(UInt32 board, UInt32 slotlist, UInt32 mode) {
        return S826_AdcSlotlistWrite(board, slotlist, mode);
    }
    public static Int32 AdcSlotlistRead(UInt32 board, ref UInt32 slotlist) {
        return S826_AdcSlotlistRead(board, ref slotlist);
    }

    public static Int32 AdcTrigModeWrite(UInt32 board, Int32 trigmode) {
        return S826_AdcTrigModeWrite(board, trigmode);
    }

    public static Int32 AdcTrigModeRead(UInt32 board, ref UInt32 trigmode) {
        return S826_AdcTrigModeRead(board, ref trigmode);
    }

    public static Int32 AdcEnableWrite(UInt32 board, UInt32 enable) {
        return S826_AdcEnableWrite(board, enable);
    }

    public static Int32 AdcEnableRead(UInt32 board, ref UInt32 enable) {
        return S826_AdcEnableRead(board, ref enable);
    }

    public static Int32 AdcStatusRead(UInt32 board, ref UInt32 status) {
        return S826_AdcStatusRead(board, ref status);
    }

    public static Int32 AdcRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
Int32[] data, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
UInt32[] timestamps, ref UInt32 slotlist, UInt32 tmax) {
        return S826_AdcRead(board, data, timestamps, ref slotlist, tmax);
    }

    public static Int32 AdcRawRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
Int32[] data, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
UInt32[] timestamps, ref UInt32 slotlist, UInt32 tmax) {
        return S826_AdcRawRead(board, data, timestamps, ref slotlist, tmax);
    }

    //DAC ================================================================================================================================
    public static Int32 DacRangeWrite(UInt32 board, UInt32 chan, UInt32 range, UInt32 safemode) {
        return S826_DacRangeWrite(board, chan, range, safemode);
    }

    public static Int32 DacDataWrite(UInt32 board, UInt32 chan, UInt32 setpoint, UInt32 safemode) {
        return S826_DacDataWrite(board, chan, setpoint, safemode);
    }

    public static Int32 DacRead(UInt32 board, UInt32 chan, ref Int32 range, ref UInt32 setpoint, UInt32 safemode) {
        return S826_DacRead(board, chan, ref range, ref setpoint, safemode);
    }

    public static Int32 DacRawRead(UInt32 board, UInt32 chan, ref UInt32 setpoint) {
        return S826_DacRawRead(board, chan, ref setpoint);
    }

    public static Int32 DacRawWrite(UInt32 board, UInt32 chan, UInt32 setpoint) {
        return S826_DacRawWrite(board, chan, setpoint);
    }

    public static Int32 DacCalRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
double[] slope, ref Int32 valid) {
        return S826_DacCalRead(board, slope, ref valid);
    }

    public static Int32 DacCalWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
double[] slope) {
        return S826_DacCalWrite(board, slope);
    }

    //Counters ================================================================================================================================
    
    public static Int32 CounterExtInRoutingRead(UInt32 board, UInt32 chan, ref UInt32 route) {
        return S826_CounterExtInRoutingRead(board, chan, ref route);
    }
    
    public static Int32 CounterExtInRoutingWrite(UInt32 board, UInt32 chan, UInt32 route) {
        return S826_CounterExtInRoutingWrite(board, chan, route);
    }

    public static Int32 CounterFilterRead(UInt32 board, UInt32 chan, ref UInt32 cfg) {
        return S826_CounterFilterRead(board, chan, ref cfg);
    }

    public static Int32 CounterFilterWrite(UInt32 board, UInt32 chan, UInt32 cfg) {
        return S826_CounterFilterWrite(board, chan, cfg);
    }

    public static Int32 CounterSnapshotRead(UInt32 board, UInt32 chan, ref UInt32 counts, ref UInt32 tstamp, ref UInt32 reason, UInt32 tmax) {
        return S826_CounterSnapshotRead(board, chan, ref counts, ref tstamp, ref reason, tmax);
    }

    public static Int32 CounterRead(UInt32 board, UInt32 chan, ref UInt32 counts) {
        return S826_CounterRead(board, chan, ref counts);
    }

    public static Int32 CounterCompareRead(UInt32 board, UInt32 chan, UInt32 regid, ref UInt32 counts) {
        return S826_CounterCompareRead(board, chan, regid, ref counts);
    }

    public static Int32 CounterSnapshotConfigRead(UInt32 board, UInt32 chan, ref UInt32 ctrl) {
        return S826_CounterSnapshotConfigRead(board, chan, ref ctrl);
    }

    public static Int32 CounterModeRead(UInt32 board, UInt32 chan, ref UInt32 mode) {
        return S826_CounterModeRead(board, chan, ref mode);
    }

    public static Int32 CounterStatusRead(UInt32 board, UInt32 chan, ref UInt32 status) {
        return S826_CounterStatusRead(board, chan, ref status);
    }

    public static Int32 CounterPreloadRead(UInt32 board, UInt32 chan, UInt32 regid, ref UInt32 counts) {
        return S826_CounterPreloadRead(board, chan, regid, ref counts);
    }

    public static Int32 CounterSnapshot(UInt32 board, UInt32 chan) {
        return S826_CounterSnapshot(board, chan);
    }
        
    public static Int32 CounterStateWrite(UInt32 board, UInt32 chan, UInt32 run) {
        return S826_CounterStateWrite(board, chan, run);
    }
        
    public static Int32 CounterCompareWrite(UInt32 board, UInt32 chan, UInt32 regid, UInt32 counts) {
        return S826_CounterCompareWrite(board, chan, regid, counts);
    }
        
    public static Int32 CounterSnapshotConfigWrite(UInt32 board, UInt32 chan, UInt32 enables, UInt32 mode) {
        return S826_CounterSnapshotConfigWrite(board, chan, enables, mode);
    }
    
    public static Int32 CounterModeWrite(UInt32 board, UInt32 chan, UInt32 mode) {
        return S826_CounterModeWrite(board, chan, mode);
    }
        
    public static Int32 CounterPreloadWrite(UInt32 board, UInt32 chan, UInt32 reg, UInt32 counts) {
        return S826_CounterPreloadWrite(board, chan, reg, counts);
    }
        
    public static Int32 CounterPreload(UInt32 board, UInt32 chan, UInt32 level, UInt32 sticky) {
        return S826_CounterPreload(board, chan, level, sticky);
    }
    
    // Virtual Digital Outputs ================================================================================================================================

    
    public static Int32 VirtualWrite(UInt32 board, UInt32 data, UInt32 mode) {
        return S826_VirtualWrite(board, data, mode);   
    }
    public static Int32 VirtualRead(UInt32 board, ref UInt32 data) {
        return S826_VirtualRead(board, ref data);
    }
    public static Int32 VirtualSafeRead(UInt32 board, ref UInt32 data) {
        return S826_VirtualSafeRead(board, ref data);
    }
    public static int VirtualSafeWrite(UInt32 board, UInt32 data, UInt32 mode) {
        return S826_VirtualSafeWrite(board, data, mode);
    }
    public static Int32 VirtualSafeEnablesRead(UInt32 board, ref UInt32 data) {
        return S826_VirtualSafeEnablesRead(board, ref data);
    }
    public static int VirtualSafeEnablesWrite(UInt32 board, UInt32 data) {
        return S826_VirtualSafeEnablesWrite(board, data);
    }



    // Digital I/O ================================================================================================================================

    public static Int32 DioOutputWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data, UInt32 mode) {
        return S826_DioOutputWrite(board, data, mode);
    }

    public static Int32 DioOutputRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data) {
        return S826_DioOutputRead(board, data);
    }

    public static Int32 DioOutputSourceRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data) {
        return S826_DioOutputSourceRead(board, data);
    }

    public static Int32 DioCapEnablesWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] rising, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] falling, UInt32 mode) {
        return S826_DioCapEnablesWrite(board, rising, falling, mode);
    }

    public static Int32 DioCapEnablesRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] rising, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] falling) {
        return S826_DioCapEnablesRead(board, rising, falling);
    }

    public static Int32 DioInputRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data) {
        return S826_DioInputRead(board, data);
    }

    public static Int32 DioCapRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data, UInt32 waitall, UInt32 tmax) {
        return S826_DioCapRead(board, data, waitall, tmax);
    }

    public static Int32 DioOutputSourceWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data) {
        return S826_DioOutputSourceWrite(board, data);
    }

    public static Int32 DioSafeEnablesRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data) {
        return S826_DioSafeEnablesRead(board, data);
    }

    public static int DioSafeEnablesWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data) {
        return S826_DioSafeEnablesWrite(board, data);
    }

    public static Int32 DioSafeRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data) {
        return S826_DioSafeRead(board, data);
    }

    public static int DioSafeWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data, UInt32 mode) {
        return S826_DioSafeWrite(board, data, mode);
    }

    public static Int32 DioFilterRead(UInt32 board, ref UInt32 interval, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] enables) {
        return S826_DioFilterRead(board, ref interval, enables);
    }

    public static int DioFilterWrite(UInt32 board, UInt32 interval, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] enables) {
        return S826_DioFilterWrite(board, interval, enables);
    }
    
    // Watchdog ================================================================================================================================


    public static int WatchdogEnableRead(UInt32 board, ref UInt32 enable) {
        return S826_WatchdogEnableRead(board, ref enable);
    }


    public static int WatchdogEnableWrite(UInt32 board, UInt32 enable) {
        return S826_WatchdogEnableWrite(board, enable);
    }
    
    public static int WatchdogConfigRead(UInt32 board, ref UInt32 config, [MarshalAs(UnmanagedType.LPArray, SizeConst = 5), In()]
UInt32[] times) {
            return S826_WatchdogConfigRead(board, ref config, times);
    }
        
    public static int WatchdogConfigWrite(UInt32 board, UInt32 config, [MarshalAs(UnmanagedType.LPArray, SizeConst = 5), Out()]
UInt32[] times) {
        return S826_WatchdogConfigWrite(board, config, times);
     }
    

    public static int WatchdogStatusRead(UInt32 board, ref UInt32 status) {
        return S826_WatchdogStatusRead(board, ref status);
    }


    public static int WatchdogKick(UInt32 board, UInt32 data) {
        return S826_WatchdogKick(board, data);
    }


    public static int WatchdogEventWait(UInt32 board) {
        return S826_WatchdogEventWait(board);
    }

    public static int WatchdogWaitCancel(UInt32 board, UInt32 tmax) {
        return S826_WatchdogWaitCancel(board, tmax);
    }

    // ===================================================================================================================================
    // ==================================================  826 API LIBRARY FUNCTIONS  ====================================================
    // ===================================================================================================================================

    [DllImport(S826_DLL, CallingConvention=CallingConvention.StdCall, SetLastError=true)]
    static extern Int32 S826_SystemOpen();

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_SystemClose();

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_VersionRead(UInt32 board, ref UInt32 api, ref UInt32 driver, ref UInt32 bdrev, ref UInt32 fpgarev);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_TimestampRead(UInt32 board, ref UInt32 timestamp);

    //Safemode ================================================================================================================================

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_SafeControlRead(UInt32 board, ref UInt32 settings);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_SafeControlWrite(UInt32 board, UInt32 settings, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_SafeWrenRead(UInt32 board, ref UInt32 enable);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_SafeWrenWrite(UInt32 board, UInt32 data);

    //ADC ================================================================================================================================

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcCalRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
double[] slope, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
Int32[] offset, ref Int32 valid);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcCalWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
double[] slope, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
Int32[] offset);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcSlotConfigWrite(UInt32 board, UInt32 slot, UInt32 chan, UInt32 tsettle, Int32 range);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcSlotConfigRead(UInt32 board, UInt32 slot, ref UInt32 chan, ref UInt32 tsettle, ref UInt32 range);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcSlotlistWrite(UInt32 board, UInt32 slotlist, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcSlotlistRead(UInt32 board, ref UInt32 slotlist);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcTrigModeWrite(UInt32 board, Int32 trigmode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcTrigModeRead(UInt32 board, ref UInt32 trigmode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcEnableWrite(UInt32 board, UInt32 enable);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcEnableRead(UInt32 board, ref UInt32 enable);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcStatusRead(UInt32 board, ref UInt32 status);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
Int32[] data, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
UInt32[] timestamps, ref UInt32 slotlist, UInt32 tmax);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_AdcRawRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
Int32[] data, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_ADC), Out()]
UInt32[] timestamps, ref UInt32 slotlist, UInt32 tmax);

    //DAC ================================================================================================================================

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DacRangeWrite(UInt32 board, UInt32 chan, UInt32 range, UInt32 safemode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DacDataWrite(UInt32 board, UInt32 chan, UInt32 setpoint, UInt32 safemode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DacRead(UInt32 board, UInt32 chan, ref Int32 range, ref UInt32 setpoint, UInt32 safemode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DacRawRead(UInt32 board, UInt32 chan, ref UInt32 setpoint);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DacRawWrite(UInt32 board, UInt32 chan, UInt32 setpoint);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DacCalRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
double[] slope, ref Int32 valid);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DacCalWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
double[] slope);

    //Counters ================================================================================================================================

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterExtInRoutingRead(UInt32 board, UInt32 chan, ref UInt32 route);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterExtInRoutingWrite(UInt32 board, UInt32 chan, UInt32 route);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterFilterRead(UInt32 board, UInt32 chan, ref UInt32 cfg);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterFilterWrite(UInt32 board, UInt32 chan, UInt32 cfg);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterSnapshotRead(UInt32 board, UInt32 chan, ref UInt32 counts, ref UInt32 tstamp, ref UInt32 reason, UInt32 tmax);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterRead(UInt32 board, UInt32 chan, ref UInt32 counts);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterCompareRead(UInt32 board, UInt32 chan, UInt32 regid, ref UInt32 counts);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterSnapshotConfigRead(UInt32 board, UInt32 chan, ref UInt32 ctrl);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterModeRead(UInt32 board, UInt32 chan, ref UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterStatusRead(UInt32 board, UInt32 chan, ref UInt32 status);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterPreloadRead(UInt32 board, UInt32 chan, UInt32 regid, ref UInt32 counts);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterSnapshot(UInt32 board, UInt32 chan);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterStateWrite(UInt32 board, UInt32 chan, UInt32 run);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterCompareWrite(UInt32 board, UInt32 chan, UInt32 regid, UInt32 counts);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterSnapshotConfigWrite(UInt32 board, UInt32 chan, UInt32 enables, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterModeWrite(UInt32 board, UInt32 chan, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterPreloadWrite(UInt32 board, UInt32 chan, UInt32 reg, UInt32 counts);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_CounterPreload(UInt32 board, UInt32 chan, UInt32 level, UInt32 sticky);

    // Virtual Digital Outputs ================================================================================================================================

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_VirtualWrite(UInt32 board, UInt32 data, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_VirtualRead(UInt32 board, ref UInt32 data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_VirtualSafeRead(UInt32 board, ref UInt32 data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_VirtualSafeWrite(UInt32 board, UInt32 data, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_VirtualSafeEnablesRead(UInt32 board, ref UInt32 data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_VirtualSafeEnablesWrite(UInt32 board, UInt32 data);

    // Digital I/O ================================================================================================================================

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioOutputWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioOutputRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioOutputSourceRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioCapEnablesWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] rising, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] falling, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioCapEnablesRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] rising, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] falling);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioInputRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioCapRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data, UInt32 waitall, UInt32 tmax);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioOutputSourceWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioSafeEnablesRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_DioSafeEnablesWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioSafeRead(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_DioSafeWrite(UInt32 board, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] data, UInt32 mode);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern Int32 S826_DioFilterRead(UInt32 board, ref UInt32 interval, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), In()]
UInt32[] enables);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_DioFilterWrite(UInt32 board, UInt32 interval, [MarshalAs(UnmanagedType.LPArray, SizeConst = S826_NUM_DIO_QUADLETS), Out()]
UInt32[] enables);

    // Watchdog ================================================================================================================================

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogEnableRead(UInt32 board, ref UInt32 enable);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogEnableWrite(UInt32 board, UInt32 enable);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogConfigRead(UInt32 board, ref UInt32 config, [MarshalAs(UnmanagedType.LPArray, SizeConst = 5), In()]
UInt32[] times);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogConfigWrite(UInt32 board, UInt32 config, [MarshalAs(UnmanagedType.LPArray, SizeConst = 5), Out()] UInt32[] times);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogStatusRead(UInt32 board, ref UInt32 status);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogKick(UInt32 board, UInt32 data);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogEventWait(UInt32 board);

    [DllImport(S826_DLL, CallingConvention = CallingConvention.StdCall, SetLastError = true)]
    static extern int S826_WatchdogWaitCancel(UInt32 board, UInt32 tmax);

}