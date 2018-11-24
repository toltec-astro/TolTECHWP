using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
//using mid826;


namespace app_826_win_csharp {
    public partial class Form1 : Form {
        // Board number (as set by switches) of active board
        private UInt32 board;
		public TextBox[] txtAdcBurstNum = new TextBox[16];
		public TextBox[] txtAdcTimestamp = new TextBox[16];
		public TextBox[] txtAdcTSettle = new TextBox[16];
		public CheckBox[] caps = new CheckBox[48];
		public CheckBox[] dioi = new CheckBox[48];
		public CheckBox[] dioo = new CheckBox[48];
		public CheckBox[] osrc = new CheckBox[48];
		public ComboBox[] cmbAdcChan = new ComboBox[16];
		public ComboBox[] cmbAdcRange = new ComboBox[16];
		public Font m_font;
        public Form1() {
			int i;
			//this.SuspendLayout();

			// Initialize your components here
			//this.label1.Text = "Hello, World!";
			//this.ClientSize = new System.Drawing.Size(1057, 627);
			//this.Controls.Add(label1);
			//this.ResumeLayout();
			//this.Name = "MainForm Name.";
			//this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			//this.Text = "MainForm Title!";
			m_font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.SuspendLayout ();
			InitializeComponent();
// 
			for (i = 0; i < 16; i++) {
				txtAdcBurstNum[i] = new TextBox();
				txtAdcTimestamp[i] = new TextBox();
				txtAdcTSettle[i] = new TextBox();
				txtAdcBurstNum[i].Font = m_font;
				txtAdcTimestamp[i].Font = m_font;
				txtAdcBurstNum[i].ReadOnly = true;
				txtAdcTimestamp[i].ReadOnly = true;
				txtAdcBurstNum[i].Size = new System.Drawing.Size(50, 20);
				txtAdcTimestamp[i].Size = new System.Drawing.Size(107, 20);
				txtAdcBurstNum[i].Name = "txtAdcBurstNum_" + i.ToString();
				txtAdcTimestamp[i].Name = "txtAdcTimestamp_" + i.ToString();
				txtAdcTSettle[i].Name = "txtAdcTSettle_" + i.ToString();
				txtAdcBurstNum[i].TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
				txtAdcTimestamp[i].TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
				txtAdcBurstNum[i].TabIndex = 172 + i;
				txtAdcTimestamp[i].TabIndex = 172 + i;
				txtAdcTSettle[i].BackColor = System.Drawing.SystemColors.Window;
				txtAdcTSettle[i].Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
				txtAdcTSettle[i].Size = new System.Drawing.Size(61, 20);
				txtAdcTSettle[i].TabIndex = 170-15+i;
				txtAdcTSettle[i].TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
				cmbAdcChan[i] = new ComboBox();
				cmbAdcChan[i].FormattingEnabled = true;
				cmbAdcChan[i].Items.AddRange(new object[] {"4", "1", "2", "3",
				            	"4", "5", "6", "7",
				             	"8", "9", "10", "11",
				            	"12", "13", "14", "15"});
				cmbAdcChan[i].MaxDropDownItems = 16;
	    			cmbAdcChan[i].Name = "cmbAdcChan_" + i.ToString();
	    			cmbAdcChan[i].Size = new System.Drawing.Size(40, 21);
	    			cmbAdcChan[i].TabIndex = 88 + i;
				cmbAdcRange[i] = new ComboBox();
				this.cmbAdcRange[i].FormattingEnabled = true;
				this.cmbAdcRange[i].Items.AddRange(new object[] {
					"-10V to -10V",
					"  -5V to +5V",
					"  -2V to +2V",
					"  -1V to +1V"}); 

				this.cmbAdcRange[i].Name = "cmbAdcRange_" + i.ToString();
				this.cmbAdcRange[i].Size = new System.Drawing.Size(83, 21);
				this.cmbAdcRange[i].TabIndex = 33+i;
			}

			for (i = 0; i <= 15; i++) {
				this.cmbAdcChan[i].Location = new System.Drawing.Point(63, i*27+40);	
				this.cmbAdcRange[i].Location = new System.Drawing.Point(112, i*27+40);	
				this.txtAdcTSettle[i].Location = new System.Drawing.Point(202, i*27+40);
				this.txtAdcTimestamp[i].Location = new System.Drawing.Point(433, i*27+40);
				this.txtAdcBurstNum[i].Location = new System.Drawing.Point(376, i*27+40);
  		   	}

			for (i = 0; i < 16; i++) {
				this.Panel60.Controls.Add(this.txtAdcBurstNum[i]);
				this.Panel60.Controls.Add(this.txtAdcTimestamp[i]);
				this.Panel60.Controls.Add(this.cmbAdcRange[i]);	
				this.Panel60.Controls.Add(this.cmbAdcChan[i]);	
				this.Panel60.Controls.Add(this.txtAdcTSettle[i]);	
			}
			for (i = 0; i < 48; i++) {
				dioi[i] = new System.Windows.Forms.CheckBox();
				dioi[i].Size = new System.Drawing.Size(15, 14);
				dioo[i] = new System.Windows.Forms.CheckBox();
				dioo[i].Size = new System.Drawing.Size(15, 14);
				caps[i] = new System.Windows.Forms.CheckBox();
				caps[i].AutoSize = true;
				caps[i].Enabled = false;
				osrc[i] = new System.Windows.Forms.CheckBox();
				if (i > 10) {
					caps[i].Name = "caps_" + i.ToString();
					dioi[i].Name = "dioi_" + i.ToString();
					dioo[i].Name = "dioo_" + i.ToString();
					osrc[i].Name = "osrc_" + i.ToString();
				} else {
					caps[i].Name = "caps_0" + i.ToString();
					dioi[i].Name = "dioi_0" + i.ToString();
					dioo[i].Name = "dioo_0" + i.ToString();
					osrc[i].Name = "osrc_0" + i.ToString();
				}
				caps[i].Size = new System.Drawing.Size(15, 14);
				caps[i].UseVisualStyleBackColor = true;
				caps[i].TabIndex = i;
				dioi[i].TabIndex = i+38;
				dioi[i].AutoSize = true;
				dioi[i].Enabled = false;
				dioi[i].UseVisualStyleBackColor = true;
				dioo[i].AutoSize = true;
				dioo[i].Enabled = true;
				dioo[i].UseVisualStyleBackColor = true;
				dioo[i].TabIndex = i+3;
				osrc[i].TabIndex = i+30;
				osrc[i].UseVisualStyleBackColor = true;
				osrc[i].AutoSize = true;
				osrc[i].Size = new System.Drawing.Size(15, 14);
			}
			int j = -10;
			int k;
			i = 4;
			for (k = 15; k >= 0; k--) {
			    if ((k % 4) == 3)
			       j += 10;
			    this.dioi[k].Location = new System.Drawing.Point(i+j, 3);
			    this.dioi[k+16].Location = new System.Drawing.Point(i+j, 3);
			    this.dioi[k+32].Location = new System.Drawing.Point(i+j, 3);
			    this.dioo[k].Location = new System.Drawing.Point(i+j, 3);
			    this.dioo[k+16].Location = new System.Drawing.Point(i+j, 3);
			    this.dioo[k+32].Location = new System.Drawing.Point(i+j, 3);
			    this.caps[k].Location = new System.Drawing.Point(i+j, 3);
 			    this.caps[k+16].Location = new System.Drawing.Point(i+j, 3);
			    this.caps[k+32].Location = new System.Drawing.Point(i+j, 3);
			    this.osrc[k].Location = new System.Drawing.Point(i+j, 3);
 			    this.osrc[k+16].Location = new System.Drawing.Point(i+j, 3);
			    this.osrc[k+32].Location = new System.Drawing.Point(i+j, 3);
			    i += 18;
			}

			this.Panel21.Controls.Add(this.Panel23);
			this.Panel21.Controls.Add(this.Panel25);
			this.Panel21.Controls.Add(this.Panel27);

			this.Panel13.Controls.Add(this.Panel16);
			this.Panel13.Controls.Add(this.Panel15);
			this.Panel13.Controls.Add(this.Panel14);

			for (i = 0; i < 16; i++) {
				this.Panel13.Controls.Add(this.dioi[i]);
			}

			for (i = 16; i < 32; i++) {
				this.Panel17.Controls.Add(this.dioi[i]);
			}

			for (i = 32; i < 48; i++) {
				this.Panel21.Controls.Add(this.dioi[i]);
			}

			for (i = 32; i < 48; i++) {
				this.Panel51.Controls.Add(this.osrc[i]);
			}

			for (i = 0; i < 16; i++) {
				this.Panel43.Controls.Add(this.osrc[i]);
			}

			for (i = 16; i < 32; i++) {
				this.Panel47.Controls.Add(this.osrc[i]);
			}

			this.Panel17.Location = new System.Drawing.Point(338, 19);
			this.Panel17.Controls.Add(this.Panel18);
			this.Panel17.Controls.Add(this.Panel19);
			this.Panel17.Controls.Add(this.Panel20);
			this.Panel17.Name = "Panel17";
			this.Panel17.Size = new System.Drawing.Size(327, 22);
			this.Panel17.TabIndex = 30;

			for (i = 0; i <= 15; i++)
			    this.pnlDIO0.Controls.Add(this.dioo[i]);

			for (i = 16; i <= 31; i++)
			    this.pnlDIO1.Controls.Add(this.dioo[i]);

			for (i = 32; i <= 47; i++)
			    this.pnlDIO2.Controls.Add(this.dioo[i]);

			for (i = 32; i <= 47; i++)
			    this.Panel81.Controls.Add(this.caps[i]);

			for (i = 16; i <= 31; i++)
			    this.Panel77.Controls.Add(this.caps[i]);

			for (i = 0; i <= 15; i++)
			    this.Panel73.Controls.Add(this.caps[i]);

			this.ResumeLayout ();

        }
        // TODO.  is this necessary?
        public static ManualResetEvent EvtEndScan = new ManualResetEvent(false);


        //*************************************************************************************************************
        //********************************************************  COMMON  *******************************************
        //*************************************************************************************************************
        // Top-level form load
        private void Form1_Load(object sender, EventArgs e) {
            // Open all boards.
            Int32 boardflags = default(Int32);
            int low_board = -1;
			boardflags = mid826.SystemOpen();
            if ((boardflags < 0)) {
                MessageBox.Show("SystemOpen ERROR!" + boardflags);
            }
            // Count boards and populate board list.
            Int32 brdCount = 0;
            for (int i = 0; i <= mid826.S826_MAX_BOARDS - 1; i++) {
                if ((boardflags & (1 << i)) != 0) {
                    if (low_board == -1)
                        low_board = i;
                    listboxActiveBoard.Items.Add(i.ToString());
                    brdCount += 1;
                }
            }
            // Set visibilities based on board count.
            tabCtrl.Enabled = (brdCount > 0);
            listboxActiveBoard.Enabled = (brdCount > 1);
            if (brdCount == 0) {
                MessageBox.Show("No 826 boards found!");
                return;
            }
            // Initialize all tabs.
            InitDioTab();
            // Create dio checkbox proxies and install dio checkbox event handlers.
            InitCntrTab();
            InitDacTab();
            InitAdcTab();
            // Select board with lowest board number
            SelectBoard((uint)low_board);
            // Install and start master update timer
            UpdateTimer.Tick += this.UpdateTimer_Tick;
            UpdateTimer.Interval = 20;
            UpdateTimer.Start();
        }

        // Top-level form close
          private void Form1_FormClosing(object sender, FormClosingEventArgs e) {
                EvtEndScan.Set();
                // Cancels all scanning threads running in sub-forms
                //mid826.SystemClose();
                e.Cancel = false;
            }
            // Update the input info for the active tab.
            private void UpdateTimer_Tick(System.Object sender, System.EventArgs e) {
                switch (tabCtrl.SelectedTab.Text) {
                    case "General":
                        UpdateMainTab();
                        break;
                    case "DIO":
                        UpdateDio();
                        break;
                    case "DAC":
                        break;
                    case "ADC":
                        UpdateADC();
                        break;
                    case "Counter":
                        UpdateCounter();
                        break;
                }
            }
         
        private void ReportErr(int errcode) {
            if (((errcode != mid826.S826_ERR_OK) & (string.IsNullOrEmpty(lblLastError.Text)))) {
                switch (errcode) {
                    case mid826.S826_ERR_BOARD:
                        lblLastError.Text = "BAD BOARDNUM";
                        break;
                    case mid826.S826_ERR_VALUE:
                        lblLastError.Text = "BAD VALUE";
                        break;
                    case mid826.S826_ERR_NOTREADY:
                        lblLastError.Text = "NOT READY";
                        break;
                    case mid826.S826_ERR_CANCELLED:
                        lblLastError.Text = "CANCELLED";
                        break;
                    case mid826.S826_ERR_DRIVER:
                        lblLastError.Text = "DRIVER ERROR";
                        break;
                    case mid826.S826_ERR_MISSEDTRIG:
                        lblLastError.Text = "MISSED ADC TRIG";
                        break;
                    case mid826.S826_ERR_BUFOVERFLOW:
                        lblLastError.Text = "ADC OVERFLOW";
                        break;
                    case mid826.S826_ERR_DUPADDR:
                        lblLastError.Text = "DUP ADDR";
                        break;
                    case mid826.S826_ERR_BOARDCLOSED:
                        lblLastError.Text = "CLOSED";
                        break;
                    case mid826.S826_ERR_CREATEMUTEX:
                        lblLastError.Text = "CREATE MUTEX";
                        break;
                    case mid826.S826_ERR_MEMORYMAP:
                        lblLastError.Text = "MEMORY MAP";
                        break;
                    case mid826.S826_ERR_MALLOC:
                        lblLastError.Text = "MALLOC";
                        break;
                    case mid826.S826_ERR_PROTECTED:
                        lblLastError.Text = "PROTECTED";
                        break;
                    case mid826.S826_ERR_OSSPECIFIC:
                        lblLastError.Text = "Port-specific error (base error number)";
                        break;
                }
            }
        }

        // Error reporting
        private void lblLastError_Click(System.Object sender, System.EventArgs e) {
            lblLastError.Text = "";
            //clear last error
        }

        // Helper functions
        private int IndexOf(object obj) {
            // Extract numeric value from object name. Value must be delimited by leading underscore.
            string s;
            switch (obj.GetType().Name) {
                case "CheckBox":
                    CheckBox chk = (CheckBox)obj;
                    s = chk.Name;
                    break;
                case "TrackBar":
                    TrackBar tb = (TrackBar)obj;
                    s = tb.Name;
                    break;
                case "ComboBox":
                    ComboBox cmb = (ComboBox)obj;
                    s = cmb.Name;
                    break;
                case "TextBox":
                    TextBox txt = (TextBox)obj;
                    s = txt.Name;
                    break;
                default:
                    return -1;
            }
            
            
            int posn = s.LastIndexOf("_");
            if (posn <= 0)
                return -1;
            s = s.Substring(posn+1);
            return int.Parse(s);
        }
        private object FindControl(string ControlNamePrefix, int index) {
            String str;
            str = ControlNamePrefix + "_" + String.Format("{0}", index);
            return this.Controls.Find(str, true)[0];
        }


        //*******************************************************************************************************
        //**********************************************************  MAIN TAB  *********************************
        //*******************************************************************************************************
        private const UInt32 COMCMD_XSF = 8;
        private const UInt32 COMCMD_SAF = 2;
        private const UInt32 SAFEN_SWE = 2;

        private const UInt32 SAFEN_SWD = 1;


        private void UpdateMainTab() {
            UInt32 settings = default(UInt32);
            UInt32 safen = default(UInt32);
            mid826.SafeControlRead(board, ref settings);
            mid826.SafeWrenRead(board, ref safen);
            init += 1;
            //----------------
            chkConfigSWE.Checked = ((safen & SAFEN_SWE) != 0);
            chkConfigSAF.Checked = ((settings & COMCMD_SAF) != 0);
            chkConfigXSF.Checked = ((settings & COMCMD_XSF) != 0);
            
            chkConfigSAF.Enabled = chkConfigSWE.Checked;
            chkConfigXSF.Enabled = chkConfigSWE.Checked;
            chkWdogSEN.Enabled = chkConfigSWE.Checked;
            UpdateVirtualOut();
            UpdateWatchdog();
            // watchdog status
            init -= 1;
            //----------------
        }

        private string VersionToString(UInt32 ver) {
            return (ver >> 24).ToString() + "." + ((ver >> 16) & 0xff).ToString() + "." + (ver & 0xffff).ToString();
        }

        // Activate 826 board
        private void SelectBoard(UInt32 boardNum) {
            UInt32 api = default(UInt32);
            UInt32 driver = default(UInt32);
            UInt32 boardrev = default(UInt32);
            UInt32 fpgarev = default(UInt32);
            // Officially select the board
            board = boardNum;
            // Get version info
            mid826.VersionRead(board, ref api, ref driver, ref boardrev, ref fpgarev);
            lblFpgaVersion.Text = VersionToString(fpgarev);
            lblBoardRev.Text = "Rev " + String.Format("{0}", Convert.ToChar(boardrev + (char)'A'));
            lblApiVersion.Text = VersionToString(api);
            lblDriverVersion.Text = VersionToString(driver);
            WatchdogSettingsRead();
            //UpdateMainTab();
        }

        private void listboxActiveBoard_SelectedIndexChanged(object sender, EventArgs e) {
            SelectBoard((uint)listboxActiveBoard.SelectedIndex);
        }

        private void chkConfigSWE_CheckedChanged(object sender, EventArgs e) {
            if (init == 0)
                mid826.SafeWrenWrite(board, (chkConfigSWE.Checked ? SAFEN_SWE : SAFEN_SWD));
            chkConfigSAF.Enabled = chkConfigSWE.Checked;
            chkConfigXSF.Enabled = chkConfigSWE.Checked;
            chkWdogSEN.Enabled = chkConfigSWE.Checked;
        }

        private void chkConfigSAF_CheckedChanged(object sender, EventArgs e) {
            if (init == 0)
                ReportErr(mid826.SafeControlWrite(board, COMCMD_SAF,
                    (chkConfigSAF.Checked ? C_REG_BITSET : C_REG_BITCLR)));
        }

        private void chkOutputsDisable_CheckedChanged(System.Object sender, System.EventArgs e) {
            if (init == 0)
                ReportErr(mid826.SafeControlWrite(board, COMCMD_SAF,
                    (chkConfigSAF.Checked ? C_REG_BITSET : C_REG_BITCLR)));
        }

        private void chkEnableXSF_CheckedChanged(System.Object sender, System.EventArgs e) {
            if (init == 0)
                ReportErr(mid826.SafeControlWrite(board, COMCMD_XSF,
                    (chkConfigXSF.Checked ? C_REG_BITSET : C_REG_BITCLR)));
        }

        //***********************************************************************************************************
        //*************************************************  VIRTUAL DIGITAL OUTPUTS  ***********************************
        //***************************************************************************************************************

        private void UpdateVirtualOut() {
            uint vout = 0;
            mid826.VirtualRead(board, ref vout);
            init += 1;
            //-------------------
            chkVirtualOut5.Checked = ((vout & 32) != 0);
            chkVirtualOut4.Checked = ((vout & 16) != 0);
            chkVirtualOut3.Checked = ((vout & 8) != 0);
            chkVirtualOut2.Checked = ((vout & 4) != 0);
            chkVirtualOut1.Checked = ((vout & 2) != 0);
            chkVirtualOut0.Checked = ((vout & 1) != 0);
            init -= 1;
            //-------------------
        }

        private void chkVirtualOut0_CheckedChanged(System.Object sender, System.EventArgs e) {
            // Program virtual digital outputs
            if (init == 0) {
                uint vout = (uint)(((chkVirtualOut5.Checked & 1 > 0 ? 1 : 0) << 5) +
                    ((chkVirtualOut4.Checked & 1 > 0 ? 1 : 0) << 4) +
                    ((chkVirtualOut3.Checked & 1 > 0 ? 1 : 0) << 3) +
                    ((chkVirtualOut2.Checked & 1 > 0 ? 1 : 0) << 2) +
                    ((chkVirtualOut1.Checked & 1 > 0 ? 1 : 0) << 1) +
                    (chkVirtualOut0.Checked ? 1 : 0));
                mid826.VirtualWrite(board, vout, 0);
            }
        }

        //******************************************************************************************************
        //************************************************************  ADC  ***********************************
        //******************************************************************************************************

        // total channels in system, including application and internal
        private const int S826_NUM_ADC_SYSTEM = 32;

        // Oversampling
        // slot data accumulators
        private int[] accnum = new int[mid826.S826_NUM_ADC];
        // slot data sample counters
        private Int32[] accbuf = new Int32[mid826.S826_NUM_ADC];
        // oversampling target sample count
        private int adcOversampleCount = 1;

        // Controls
        private CheckBox[] adcChkEnable = new CheckBox[mid826.S826_NUM_ADC];
        private ComboBox[] adcCmbChan = new ComboBox[S826_NUM_ADC_SYSTEM];
        private ComboBox[] adcCmbRange = new ComboBox[mid826.S826_NUM_ADC];
        private TextBox[] adcTxtVolts = new TextBox[mid826.S826_NUM_ADC];
        private TextBox[] adcTxtBurst = new TextBox[mid826.S826_NUM_ADC];
        private TextBox[] adcTxtTstamp = new TextBox[mid826.S826_NUM_ADC];

        private TextBox[] adcTxtTSettle = new TextBox[mid826.S826_NUM_ADC];
        // Voltage scalars (constants)
        private double[] adcVoltScalar = {10.0 / 32768.0, 5.0 / 32768.0, 2.0 / 32768.0, 1.0 / 32768.0};
        // Changed oversampling count.
        private void tbOversample_Scroll(System.Object sender, System.EventArgs e) {
            lblAdcAvgSamples.Text = tbOversample.Value.ToString();
            adcOversampleCount = tbOversample.Value;
            // Reset slot data accumulators
            for (int slot = 0; slot <= mid826.S826_NUM_ADC - 1; slot++) {
                accnum[slot] = 0;
                accbuf[slot] = 0;
            }
        }

        // Fetch and display all adc data.
        private void UpdateADC() {
            int[] buf = new int[mid826.S826_NUM_ADC];
            int[] data = new int[mid826.S826_NUM_ADC];
            UInt32[] burstnum = new UInt32[mid826.S826_NUM_ADC];
            UInt32[] tstamp = new UInt32[mid826.S826_NUM_ADC];
            bool[] bufoverflow = new bool[mid826.S826_NUM_ADC];
            UInt32 slotlist = default(UInt32);
            int errcode = 0;
            // Read and accumulate adc data
            mid826.AdcSlotlistRead(board, ref slotlist);
            // Get list of enabled slots
            if (chkAdcRaw.Checked) {
                errcode = mid826.AdcRawRead(board, buf, tstamp, ref slotlist, 0);
                // Read raw adc data
            } else {
                errcode = mid826.AdcRead(board, buf, tstamp, ref slotlist, 0);
                // Read corrected adc data
            }
            if ((errcode != mid826.S826_ERR_OK) & (errcode != mid826.S826_ERR_NOTREADY)) {
                ReportErr(errcode);
            }
            // For each possible slot
            for (UInt32 slot = 0; (int)slot <= mid826.S826_NUM_ADC - 1; slot++) {
                //   If it's a slot of interest and data is available
                if ((((slotlist >> (int)slot) & 1) != 0)) {
                    // extract adcdata, burstnum, and bufoverflowflag from buf
                    int mask = (chkAdcRaw.Checked ? 0x3ffff : 0xffff);
                    int signmask = (chkAdcRaw.Checked ? 0x20000 : 0x8000);
                    data[slot] = ((buf[slot] & signmask) != 0 ? (buf[slot] | ~mask) : buf[slot] & mask);
                    burstnum[slot] = Convert.ToUInt32((buf[slot] >> 24) & 0xff);
                    bufoverflow[slot] = ((((int)buf[slot] & 0x800000) != 0) ? true : false);

                    // accumulate slot data
                    accbuf[slot] += data[slot];
                    accnum[slot] += 1;
                    //     If enough samples have been accumulated
                    if ((accnum[slot] >= adcOversampleCount)) {
                        double avg = accbuf[slot] / accnum[slot];
                        //       compute and display average sample value
                        if (chkAdcShowVolts.Checked) {
                            UInt32 chan = default(UInt32);
                            UInt32 tsettle = default(UInt32);
                            UInt32 range = default(UInt32);
                            mid826.AdcSlotConfigRead(board, slot, ref chan, ref tsettle, ref range);
                            // read channel gain setting
                            adcTxtVolts[slot].Text = (avg * adcVoltScalar[range]).ToString("+#0.0000;-#0.0000");
                            // compute & display volts based on gain
                        } else {
                            adcTxtVolts[slot].Text = "0x" + Convert.ToInt32(avg).ToString("X8");
                            // display raw adc value
                        }

                        adcTxtBurst[slot].Text = burstnum[slot].ToString();
                        adcTxtBurst[slot].BackColor = (bufoverflow[slot] ? Color.Pink : Color.White);


                        adcTxtTstamp[slot].Text = tstamp[slot].ToString();



                        accnum[slot] = 0;
                        //   Reset accumulator
                        accbuf[slot] = 0;
                    }
                }
            }
        }

        private void AdcTriggerRefresh() {
            chkAdcTrigPolarity.Enabled = chkAdcTrigEnable.Checked;
            cmbAdcTrigger.Enabled = chkAdcTrigEnable.Checked;
        }

        // Configure external trigger
        private void AdcConfigTrigger(System.Object sender, System.EventArgs e) {
            mid826.AdcTrigModeWrite(board, (chkAdcTrigEnable.Checked ? 128 : 0) +
                (chkAdcTrigPolarity.Checked ? 64 : 0) + cmbAdcTrigger.SelectedIndex);
            AdcTriggerRefresh();
        }

        private void chkAdcSysEnableChanged(System.Object sender, System.EventArgs e) {
            if (init == 0)
                mid826.AdcEnableWrite(board, (uint)(chkAdcSysEnable.Checked ? 1 : 0));
        }

        private void chkSlotEnableChanged(System.Object sender, System.EventArgs e) {
            CheckBox chk = (CheckBox)sender;
            UInt32 slot = (uint)IndexOf(chk);
            mid826.AdcSlotlistWrite(board, (uint)(1 << ((int)slot)), (chk.Checked ? C_REG_BITSET : C_REG_BITCLR));
            if (!chk.Checked) {
                adcTxtVolts[slot].Text = "";
                // blank adc data value if slot disabled
                adcTxtBurst[slot].Text = "";
                adcTxtTstamp[slot].Text = "";
            }
        }

        private void ChangeSlotConfig(UInt32 slot) {
            UInt32 tsettle = uint.Parse(adcTxtTSettle[slot].Text);
            adcTxtTSettle[slot].Text = tsettle.ToString();
            mid826.AdcSlotConfigWrite(board, slot, (uint)adcCmbChan[slot].SelectedIndex, tsettle,
                adcCmbRange[slot].SelectedIndex);
        }
        private void cmbAdcSlotConfigChanged(System.Object sender, System.EventArgs e) {
            ChangeSlotConfig((uint)IndexOf(sender));
        }
        private void btnAdcTSettleApply_Click(System.Object sender, System.EventArgs e) {
            for (int slot = 0; slot <= mid826.S826_NUM_ADC - 1; slot++) {
                adcTxtTSettle[slot].Text = txtDefaultSettlingTime.Text;
                ChangeSlotConfig((uint)slot);
            }
        }

        // Initialize ADC tab
        private void InitAdcTab() {
            UInt32 chan = default(UInt32);
            UInt32 tsettle = default(UInt32);
            UInt32 range = default(UInt32);
            UInt32 trigmode = default(UInt32);
            UInt32 slotlist = default(UInt32);
            UInt32 adcEnable = default(UInt32);

            init = 1;// true;

            mid826.AdcEnableRead(board, ref adcEnable);
            mid826.AdcSlotlistRead(board, ref slotlist);
            mid826.AdcTrigModeRead(board, ref trigmode);

            // Initialize adc system controls ------------------------

            // Populate trigger source list
            for (int i = 0; i <= mid826.S826_NUM_DIO - 1; i++) {
                cmbAdcTrigger.Items.Add("DIO " + i.ToString());
            }
            for (int i = 0; i <= mid826.S826_NUM_COUNT - 1; i++) {
                cmbAdcTrigger.Items.Add("CNT " + i.ToString());
            }
            for (int i = 0; i <= 5; i++) {
                cmbAdcTrigger.Items.Add("VOUT" + i.ToString());
            }


            chkAdcSysEnable.Checked = adcEnable != 0 ? true : false;
            // adc enable
            chkAdcTrigEnable.Checked = (trigmode & 0x80) != 0;
            // trigger enable
            chkAdcTrigPolarity.Checked = (trigmode & 0x40) != 0;
            // trigger polarity
            cmbAdcTrigger.SelectedIndex = (int)(trigmode & 0x3f);
            // trigger source

            AdcTriggerRefresh();

            // Install adc system control event handlers.
            chkAdcSysEnable.CheckedChanged += this.chkAdcSysEnableChanged;
            cmbAdcTrigger.SelectedIndexChanged += this.AdcConfigTrigger;
            chkAdcTrigEnable.CheckedChanged += this.AdcConfigTrigger;
            chkAdcTrigPolarity.CheckedChanged += this.AdcConfigTrigger;


            for (UInt32 slot = 0; (int)slot <= mid826.S826_NUM_ADC - 1; slot++) {
                // Create control proxies.
                adcChkEnable[slot] = (CheckBox)FindControl("chkAdcEnable", (int)slot);
                adcCmbChan[slot] = cmbAdcChan[slot];
                adcCmbRange[slot] = cmbAdcRange[slot];
                adcTxtVolts[slot] = (TextBox)this.Controls.Find("txtAdcVolts_" + String.Format("{0}", slot), true)[0];
                adcTxtBurst[slot] = txtAdcBurstNum[slot];//(TextBox)this.Controls.Find("txtAdcBurstNum_" + String.Format("{0}", slot), true)[0];
                adcTxtTstamp[slot] = txtAdcTimestamp[slot];//(TextBox)this.Controls.Find("txtAdcTimestamp_" + String.Format("{0}", slot), true)[0];
                adcTxtTSettle[slot] = txtAdcTSettle[slot];//(TextBox)this.Controls.Find("txtAdcTSettle_" + String.Format("{0}", slot), true)[0];

                // Populate channel number combos.
                adcCmbChan[slot].Items.Clear();
                for (int i = 0; i <= 15; i++) {
		    if (i < 16) {
                            adcCmbChan[slot].Items.Add(i);
		    } else {// future use
                    switch (i) {
                        case 17:
                            adcCmbChan[slot].Items.Add("0V");
                            break;
                        case 18:
                            adcCmbChan[slot].Items.Add("-5V");
                            break;
                        case 19:
                            adcCmbChan[slot].Items.Add("+5V");
                            break;
                        default:
                            adcCmbChan[slot].Items.Add("--");
                            break;
                    }
		    }
                }

                // Initialize slot controls.
                mid826.AdcSlotConfigRead(board, slot, ref chan, ref tsettle, ref range);
                // Read slot configuration.
                adcChkEnable[slot].Checked = (((int)slotlist >> (int)slot) & 1) != 0 ? true : false;
                // slot enable
                adcCmbChan[slot].SelectedIndex = (int)chan;
                // slot ain channel
                adcTxtTSettle[slot].Text = tsettle.ToString();
                adcCmbRange[slot].SelectedIndex = (int)range;
                // slot voltage range
                adcTxtVolts[slot].Text = "";
                adcTxtBurst[slot].Text = "";
                adcTxtTstamp[slot].Text = "";

                // Install slot configuration event handlers.
                adcChkEnable[slot].CheckedChanged += this.chkSlotEnableChanged;
                adcCmbChan[slot].SelectedIndexChanged += this.cmbAdcSlotConfigChanged;
                adcCmbRange[slot].SelectedIndexChanged += this.cmbAdcSlotConfigChanged;
                adcTxtTSettle[slot].TextChanged += this.cmbAdcSlotConfigChanged;

            }

            init = 0;// false;

        }

        //*******************************************************************************************************
        //**************************************  DAC  **********************************************************
        //*******************************************************************************************************

        // DAC constants
        private string[,] DacSliderLabels = {{"0", "+2.5", "+5"}, {"0", "+5", "+10"}, {"-5", "0", "+5"},
                                        {"-10", "0", "+10"}}; //range,min|mid|max
        private int[] DacOffset = { 0, 0, 0x8000, 0x8000 };
        private double[] DacScalar = { 13107.2, 6553.6, 6553.6, 3276.8 };
        // DAC controls
        private TrackBar[] dacTrackbar = new TrackBar[mid826.S826_NUM_DAC];
        private ComboBox[] dacCombobox = new ComboBox[mid826.S826_NUM_DAC];
        private void DacUpdate() {
            int range = 0;
            uint setpoint = 0;
            init += 1;
            //-----------
            for (int chan = 0; chan <= mid826.S826_NUM_DAC - 1; chan++) {
                mid826.DacRead(board, (uint)chan, ref range, ref setpoint, (uint)(chkDacShowSafe.Checked ? 1 : 0));
                dacCombobox[chan].SelectedIndex = range;
                dacTrackbar[chan].Value = (int)setpoint;
                DacShowSetpointValue(chan, range, setpoint);
                DacShowTrackbarLabels(chan, range);
            }
            init -= 1;
            //-----------
        }

        private void InitDacTab() {
            // Execute this once when form is loaded.
            for (int chan = 0; chan <= mid826.S826_NUM_DAC - 1; chan++) {
                // Create control proxies.
                dacTrackbar[chan] = (TrackBar)this.Controls.Find(String.Format("tbDac_{0}", chan), true)[0];
                dacCombobox[chan] = (ComboBox)this.Controls.Find(String.Format("cmbDac_{0}", chan), true)[0];
                // Install control event handlers.
                dacTrackbar[chan].Scroll += this.DacSetpointScroll;
                dacCombobox[chan].SelectedIndexChanged += this.DacRangeClick;
            }
            DacUpdate();
        }
        
        // Display SafeMode settings (vs. Runtime settings)
        private void chkDacShowSafe_CheckedChanged(System.Object sender, System.EventArgs e) {
            if (chkDacShowSafe.Checked)
                chkDacShowRaw.Checked = false;
            DacUpdate();
        }

        // Display output voltage based on dac range and setpoint
        private void DacShowSetpointValue(int chan, int range, uint setpoint) {
            String str;
            str = "lblDac_" + chan.ToString() + "value";
            this.Controls.Find(str, true)[0].Text = string.Format("{0:0.000}", (setpoint - DacOffset[range]) / DacScalar[range]);
        }

        private void DacSetOutput(int chan, uint setpoint) {
            int range = 0;
            if (chkDacShowRaw.Checked) {
                mid826.DacRawWrite(board, (uint)chan, (uint)setpoint);
            } else {
                ReportErr(mid826.DacDataWrite(board, (uint)chan, (uint)setpoint, (uint)(chkDacShowSafe.Checked ? 1 : 0)));
                // program output voltage
            }
            mid826.DacRead(board, (uint)chan, ref range, ref setpoint, (uint)(chkDacShowSafe.Checked ? 1 : 0));
            // get programmed range and setpoint
            DacShowSetpointValue(chan, range, setpoint);
            // display setpoint
        }

        private void DacShowTrackbarLabels(int chan, int range) {
            // Change trackbar labels to match range
            this.Controls.Find("lblDac_" + chan.ToString() + "min", true)[0].Text = DacSliderLabels[range, 0]; //min
            this.Controls.Find("lblDac_" + chan.ToString() + "mid", true)[0].Text = DacSliderLabels[range, 1]; //mid
            this.Controls.Find("lblDac_" + chan.ToString() + "max", true)[0].Text = DacSliderLabels[range, 2]; //max
        }

        private void DacSetRange(int chan, int range) {
            DacShowTrackbarLabels(chan, range);
            // Change trackbar labels to match range
            mid826.DacRangeWrite(board, (uint)chan, (uint)range, (uint)(chkDacShowSafe.Checked ? 1 : 0));
            // Program the range (runmode or safemode)
            // Program the output voltage
            dacTrackbar[chan].Value = DacOffset[range];
            // Move trackbar to 0V position
            dacTrackbar[chan].Refresh();
            DacSetOutput(chan, (uint)DacOffset[range]);
            // Program output to 0V and display volts
        }

        // Event handler for dac output range select comboboxes
        private void DacRangeClick(System.Object sender, System.EventArgs e) {
            if (init == 0) {
                ComboBox combo = (ComboBox)sender;
                DacSetRange(IndexOf(combo), combo.SelectedIndex);
                // Change slider labels to match range, program range and set output to 0V
            }
        }

        // Event handler for dac setpoint trackbars
        private void DacSetpointScroll(System.Object sender, System.EventArgs e) {
            if (init == 0) {
                TrackBar tb = (TrackBar)sender;
                DacSetOutput((int)IndexOf(tb), (uint)tb.Value);
                // program and display output voltage
            }
        }

        //******************************************************************************************************
        //********************************************  DIO  ***************************************************
        //******************************************************************************************************

        const UInt32 C_REG_BITSET = 2;

        const UInt32 C_REG_BITCLR = 1;

        private UInt32 DioFilterEnables;
        // DIO checkboxes

        private CheckBox[] chkDioo = new CheckBox[mid826.S826_NUM_DIO]; // output states
        private CheckBox[] chkDioi = new CheckBox[mid826.S826_NUM_DIO]; // input states
        private CheckBox[] chkCpos = new CheckBox[mid826.S826_NUM_DIO]; // enable rising edge capture
        private CheckBox[] chkCneg = new CheckBox[mid826.S826_NUM_DIO]; // enable falling edge capture
        private CheckBox[] chkOsrc = new CheckBox[mid826.S826_NUM_DIO]; // source select
        private CheckBox[] chkSafe = new CheckBox[mid826.S826_NUM_DIO]; // safemode states
        private CheckBox[] chkSfen = new CheckBox[mid826.S826_NUM_DIO]; // safemode enables
        private CheckBox[] chkCaps = new CheckBox[mid826.S826_NUM_DIO]; // capture status
        private CheckBox[] chkFilt = new CheckBox[mid826.S826_NUM_DIO]; // filter enables


        private void InitDioTab() {
            UInt32[] dioov = new UInt32[2];
            UInt32[] cpos = new UInt32[2];
            UInt32[] cneg = new UInt32[2];
            UInt32[] cape = new UInt32[2];
            UInt32[] osrcv = new UInt32[2];
            UInt32[] safe = new UInt32[2];
            UInt32[] sfen = new UInt32[2];
            UInt32[] filt = new UInt32[2];
            UInt32 filtdelay = default(UInt32);
            // Read current states
            mid826.DioOutputRead(board, dioov);
            mid826.DioSafeRead(board, safe);
            mid826.DioSafeEnablesRead(board, sfen);
            mid826.DioCapEnablesRead(board, cpos, cneg);
            mid826.DioOutputSourceRead(board, osrcv);
            mid826.DioFilterRead(board, ref filtdelay, filt);
            // For each channel ...
            for (int i = 0; i <= mid826.S826_NUM_DIO - 1; i++) {
                uint index = (uint)i / 24;
                // array index
                uint mask = (uint)(1 << ((int)i % 24));
                // dio channel bit mask
                // Create dio checkbox proxies.
                chkDioo[i] = dioo[i];
                //output state
                chkCpos[i] = (CheckBox)this.Controls.Find(String.Format("cpos_{0:00}", i), true)[0];
                //rising edge capture enable
                chkCneg[i] = (CheckBox)this.Controls.Find(String.Format("cneg_{0:00}", i), true)[0];
                //falling edge capture enable
                chkOsrc[i] = osrc[i];
                //output source
                chkSafe[i] = (CheckBox)this.Controls.Find(String.Format("safe_{0:00}", i), true)[0];
                //safemode states
                chkSfen[i] = (CheckBox)this.Controls.Find(string.Format("sfen_{0:00}", i), true)[0];
                //safemode enables
		chkDioi[i] = dioi[i];
                //input state
		chkCaps[i] = caps[i];
                //capture status
                chkFilt[i] = (CheckBox)this.Controls.Find(string.Format("filt_{0:00}", i), true)[0];
                //filter enables
                // Initialize checkbox states before installing event handlers.
                chkDioo[i].Checked = ((dioov[index] & mask) != 0);
                chkCpos[i].Checked = (cpos[index] & mask) != 0;
                chkCneg[i].Checked = (cneg[index] & mask) != 0;
                chkOsrc[i].Checked = (osrcv[index] & mask) != 0;
                chkSafe[i].Checked = (safe[index] & mask) != 0;
                chkSfen[i].Checked = (sfen[index] & mask) != 0;
                chkFilt[i].Checked = (filt[index] & mask) != 0;
                scrollDioFilterDelay.Value = (int)filtdelay;
                lblDioFilterDelay.Text = String.Format("{0}", filtdelay);
                chkCaps[i].Enabled = true;
                chkCaps[i].AutoCheck = false;
                // Install dio checkbox event handlers.
                chkDioo[i].CheckedChanged += this.ClickDio;
                chkCpos[i].CheckedChanged += this.ClickDio;
                chkCneg[i].CheckedChanged += this.ClickDio;
                chkOsrc[i].CheckedChanged += this.ClickDio;
                chkSafe[i].CheckedChanged += this.ClickDio;
                chkSfen[i].CheckedChanged += this.ClickDio;
                chkFilt[i].CheckedChanged += this.ClickDio;
                chkCaps[i].Click += this.ClickDio;
            }
        }

        private void ClickDio(System.Object sender, System.EventArgs e) {
            // Get dio checkbox name and channel number from sender, which is the clicked checkbox
            if (init == 0) {
                int errcode = 0;
                CheckBox chk = (CheckBox)sender;
                string chkname = chk.Name.Substring(0, 4);
                UInt32 chan = (uint)IndexOf(chk);
                UInt32 filtdelay = default(UInt32);

                UInt32[] data = {0, 0};
                UInt32[] rising = {0, 0};
                UInt32[] falling = {0, 0};

                UInt32 quadlet = chan / 24;
                UInt32 mask = (UInt32)(1 << ((int)chan % 24));
                // Decode the checkbox and take appropriate action
                switch (chkname) {
                    case "dioo":
                        data[quadlet] = mask;
                        mid826.DioOutputWrite(board, data, (chk.Checked ? C_REG_BITSET : C_REG_BITCLR));
                        // 1=set, 2=clr
                        break;
                    case "cpos":
                        rising[quadlet] = mask;
                        mid826.DioCapEnablesWrite(board, rising, falling, (chk.Checked ? C_REG_BITSET : C_REG_BITCLR));
                        break;
                    case "cneg":
                        falling[quadlet] = mask;
                        mid826.DioCapEnablesWrite(board, rising, falling, (chk.Checked ? C_REG_BITSET : C_REG_BITCLR));
                        break;
                    case "safe":
                        data[quadlet] = mask;
                        errcode = mid826.DioSafeWrite(board, data, (chk.Checked ? C_REG_BITSET : C_REG_BITCLR));
                        // Verify write occured; it will be blocked if SafeMode writes are prohibited
                        if (errcode != 0) {
                            ReportErr(errcode);
                            init += 1;
                            chk.Checked = !chk.Checked;
                            init -= 1;
                        }
                        break;
                    case "sfen":
                        mid826.DioSafeEnablesRead(board, data);
                        data[quadlet] = (chk.Checked ? data[quadlet] | mask : data[quadlet] & ~mask);
                        mid826.DioSafeEnablesWrite(board, data);
                        break;
                    case "filt":
                        mid826.DioFilterRead(board, ref filtdelay, data);
                        data[quadlet] = (chk.Checked ? data[quadlet] | mask : data[quadlet] & ~mask);
                        mid826.DioFilterWrite(board, filtdelay, data);
                        break;
                    case "osrc":
                        mid826.DioOutputSourceRead(board, data);
                        data[quadlet] = (chk.Checked ? data[quadlet] | mask : data[quadlet] & ~mask);
                        mid826.DioOutputSourceWrite(board, data);
                        break;
                    case "caps":
                        // uncheck capture indicator
                        chkCaps[chan].Checked = false;
                        break;
                }
            }
        }

        private void ProgramAllDios(bool bitval) {
            for (Int32 i = 0; i <= mid826.S826_NUM_DIO - 1; i++) {
                chkDioo[i].Checked = bitval;
            }
        }

        private void BtnSetAll_Click(System.Object sender, System.EventArgs e) {
            ProgramAllDios(true);
        }
        private void BtnResetAll_Click(System.Object sender, System.EventArgs e) {
            ProgramAllDios(false);
        }

        private void UpdateDio() {
            // Update DIO tab upon timer tick
            UInt32[] pinstate = new UInt32[mid826.S826_NUM_DIO_QUADLETS + 1];
            UInt32[] capstate = {
			0xffffff,
			0xffffff
		};


            mid826.DioInputRead(board, pinstate);
            mid826.DioCapRead(board, capstate, 0, 0);
            for (Int32 i = 0; i <= mid826.S826_NUM_DIO - 1; i++) {
                chkDioi[i].Checked = ((pinstate[i / 24] & (1 << (i % 24))) != 0);
                // pin physical state
                if (((capstate[i / 24] & (1 << (i % 24))) != 0)) {
                    chkCaps[i].Checked = true;
                    // capture status: check when capture detected
                }
            }
        }



        //****************************************************************************************************
        //*******************************************************  COUNTERS  *********************************
        //****************************************************************************************************


        // Currently selected counter channel
        private UInt32 ctr;

        private int init = 0;

        private bool[] autoSnapshot = new bool[7];

        // Mode register field positions.
        private const int CTRMODE_SHIFT_IP = 30;
        private const int CTRMODE_SHIFT_IM = 28;
        private const int CTRMODE_SHIFT_PS = 24;
        private const int CTRMODE_SHIFT_NR = 23;
        private const int CTRMODE_SHIFT_UD = 22;
        private const int CTRMODE_SHIFT_BP = 21;
        private const int CTRMODE_SHIFT_OM = 18;
        private const int CTRMODE_SHIFT_OP = 17;
        private const int CTRMODE_SHIFT_PXH = 16;
        private const int CTRMODE_SHIFT_PXR = 15;
        private const int CTRMODE_SHIFT_PXF = 14;
        private const int CTRMODE_SHIFT_PZ = 13;
        private const int CTRMODE_SHIFT_PM1 = 12;
        private const int CTRMODE_SHIFT_PM0 = 11;
        private const int CTRMODE_SHIFT_TE = 9;
        private const int CTRMODE_SHIFT_TD = 7;
        private const int CTRMODE_SHIFT_K = 4;
        private const int CTRMODE_SHIFT_XS = 0;

        // Mode register field masks.
        private const uint CTRMODE_MASK_IP = (1 << CTRMODE_SHIFT_IP);
        private const uint CTRMODE_MASK_IM = (3 << CTRMODE_SHIFT_IM);
        private const uint CTRMODE_MASK_PS = (1 << CTRMODE_SHIFT_PS);
        private const uint CTRMODE_MASK_NR = (1 << CTRMODE_SHIFT_NR);
        private const uint CTRMODE_MASK_UD = (1 << CTRMODE_SHIFT_UD);
        private const uint CTRMODE_MASK_BP = (1 << CTRMODE_SHIFT_BP);
        private const uint CTRMODE_MASK_OM = (7 << CTRMODE_SHIFT_OM);
        private const uint CTRMODE_MASK_OP = (1 << CTRMODE_SHIFT_OP);
        private const uint CTRMODE_MASK_PXH = (1 << CTRMODE_SHIFT_PXH);
        private const uint CTRMODE_MASK_PXR = (1 << CTRMODE_SHIFT_PXR);
        private const uint CTRMODE_MASK_PXF = (1 << CTRMODE_SHIFT_PXF);
        private const uint CTRMODE_MASK_PZ = (1 << CTRMODE_SHIFT_PZ);
        private const uint CTRMODE_MASK_PM1 = (1 << CTRMODE_SHIFT_PM1);
        private const uint CTRMODE_MASK_PM0 = (1 << CTRMODE_SHIFT_PM0);
        private const uint CTRMODE_MASK_TE = (3 << CTRMODE_SHIFT_TE);
        private const uint CTRMODE_MASK_TD = (3 << CTRMODE_SHIFT_TD);
        private const uint CTRMODE_MASK_K = (7 << CTRMODE_SHIFT_K);

        private const uint CTRMODE_MASK_XS = (15 << CTRMODE_SHIFT_XS);
        // Utility functions ==================================

        private string UintToString(UInt32 u) {
            string functionReturnValue = null;
            if (chkCtrSigned.Checked) {
                //if displaying as signed, cvt large val to negative
                functionReturnValue = Convert.ToString(Convert.ToInt32(u & 0x7fffffff)
                    + Convert.ToInt32((u > 0x7fffffff ? 0x80000000 : 0)));
            } else {
                functionReturnValue = Convert.ToString(u);
            }
            return functionReturnValue;
        }

        private uint TextboxToUint(ref TextBox t) {
            uint u = (UInt32)( (UInt64)(Int64.Parse(t.Text) & 0xffffffffL));
            t.Text = UintToString(u);
            return u;
        }

        // Counter functions =====================================
        // Populate list of ExtIn signal sources
        private void InitCntrTab() {
            for (int i = 0; i <= 47; i++) {
                cmbRouteCenSource.Items.Add("Dio" + i.ToString());
            }
            for (int i = 0; i <= 5; i++) {
                cmbRouteCenSource.Items.Add("Ctr" + i.ToString());
            }
            for (int i = 0; i <= 5; i++) {
                cmbRouteCenSource.Items.Add("Vout" + i.ToString());
            }
        }

        private void ShowCounterConfig() {
            // Read counter configuration and change the form controls to reflect the mode.

            init += 1;
            // Set form control states----------------

            chkAutoSnapshot.Checked = autoSnapshot[ctr];

            // Fetch and display run state
            UInt32 status = default(UInt32);
            mid826.CounterStatusRead(board, ctr, ref status);
            chkCtrEnable.Checked = (((int)status >> 29) & 1) != 0 ? true : false;

            // Fetch and display mode register
            UInt32 mode = default(UInt32);
			int rv;
            rv = mid826.CounterModeRead(board, (uint)ctr, ref mode);
			if (rv != 0)
				return;
            UpdateCounterMode(mode);
            chkCtrIP.Checked = ((mode & CTRMODE_MASK_IP) >> CTRMODE_SHIFT_IP) != 0 ? true : false;
			rv = (int)((mode & CTRMODE_MASK_IM) >> CTRMODE_SHIFT_IM);
			cmbCtrIM.SelectedIndex = rv;
            chkCtrPS.Checked = ((mode & CTRMODE_MASK_PS) >> CTRMODE_SHIFT_PS) != 0 ? true : false;
            cmbCtrNR.SelectedIndex = (int)((mode & CTRMODE_MASK_NR) >> CTRMODE_SHIFT_NR);
            cmbCtrUD.SelectedIndex = (int)((mode & CTRMODE_MASK_UD) >> CTRMODE_SHIFT_UD);
            chkCtrBP.Checked = ((mode & CTRMODE_MASK_BP) >> CTRMODE_SHIFT_BP) != 0 ? true : false;
			rv = (int)((mode & CTRMODE_MASK_OM) >> CTRMODE_SHIFT_OM);
			if (rv > 4) // invalid mode
				return;
			cmbCtrOM.SelectedIndex = rv;
            chkCtrOP.Checked = ((mode & CTRMODE_MASK_OP) >> CTRMODE_SHIFT_OP) != 0 ? true : false;
            chkCtrPXH.Checked = ((mode & CTRMODE_MASK_PXH) >> CTRMODE_SHIFT_PXH) != 0 ? true : false;
            chkCtrPXR.Checked = ((mode & CTRMODE_MASK_PXR) >> CTRMODE_SHIFT_PXR) != 0 ? true : false;
            chkCtrPXF.Checked = ((mode & CTRMODE_MASK_PXF) >> CTRMODE_SHIFT_PXF) != 0 ? true : false;
            chkCtrPZ.Checked = ((mode & CTRMODE_MASK_PZ) >> CTRMODE_SHIFT_PZ) != 0 ? true : false;
            chkCtrPM1.Checked = ((mode & CTRMODE_MASK_PM1) >> CTRMODE_SHIFT_PM1) != 0 ? true : false;
            chkCtrPM0.Checked = ((mode & CTRMODE_MASK_PM0) >> CTRMODE_SHIFT_PM0) != 0 ? true : false;
            cmbCtrTE.SelectedIndex = (int)(mode & CTRMODE_MASK_TE) >> CTRMODE_SHIFT_TE;
            cmbCtrTD.SelectedIndex = (int)(mode & CTRMODE_MASK_TD) >> CTRMODE_SHIFT_TD;
            cmbCtrK.SelectedIndex = (int)(mode & CTRMODE_MASK_K) >> CTRMODE_SHIFT_K;
            cmbCtrXS.SelectedIndex = (int)(mode & CTRMODE_MASK_XS) >> CTRMODE_SHIFT_XS;

            // Fetch and display signal routing
            UInt32 route = default(UInt32);
            mid826.CounterExtInRoutingRead(board, ctr, ref route);
            cmbRouteCenSource.SelectedIndex = (int)route;

            // Fetch and display input signal filters
            UInt32 flt = default(UInt32);
            mid826.CounterFilterRead(board, ctr, ref flt);
            chkCtrFiltClk.Checked = ((flt & 0x40000000) != 0);
            chkCtrFiltIX.Checked = ((flt & 0x80000000) != 0);
            txtCtrFilterDelay.Text = (flt & 0xffff).ToString();

            // Fetch and display preload and compare registers
            UInt32 val = default(UInt32);
            mid826.CounterCompareRead(board, ctr, 0, ref val);
            txtCtrCompare0.Text = UintToString(val);
            mid826.CounterCompareRead(board, ctr, 1, ref val);
            txtCtrCompare1.Text = UintToString(val);
            mid826.CounterPreloadRead(board, ctr, 0, ref val);
            txtCtrPreload0.Text = UintToString(val);
            mid826.CounterPreloadRead(board, ctr, 1, ref val);
            txtCtrPreload1.Text = UintToString(val);
            chkCtrStickyPreload.Checked = (((status >> 27) & 1) != 0);
            // sticky preload

            // Fetch and display SnapshotEnables and AutoresetSnapshotEnables
            UInt32 ssen = default(UInt32);
            mid826.CounterSnapshotConfigRead(board, ctr, ref ssen);
            UpdateSnapshotEnables(ssen);
            chkCtrSSENCLR_ER.Checked = ((ssen & 0x400000) != 0);
            chkCtrSSENCLR_EF.Checked = ((ssen & 0x200000) != 0);
            chkCtrSSENCLR_XR.Checked = ((ssen & 0x100000) != 0);
            chkCtrSSENCLR_XF.Checked = ((ssen & 0x80000) != 0);
            chkCtrSSENCLR_Z.Checked = ((ssen & 0x40000) != 0);
            chkCtrSSENCLR_M1.Checked = ((ssen & 0x20000) != 0);
            chkCtrSSENCLR_M0.Checked = ((ssen & 0x10000) != 0);

            init -= 1;
            //-----------------

        }

        private void UpdateSnapshotEnables(UInt32 ssen) {
            // assumed: init>0
            UpdateSnapshotControl(ssen);
            chkCtrSSEN_ER.Checked = ((ssen & 64) != 0);
            chkCtrSSEN_EF.Checked = ((ssen & 32) != 0);
            chkCtrSSEN_XR.Checked = ((ssen & 16) != 0);
            chkCtrSSEN_XF.Checked = ((ssen & 8) != 0);
            chkCtrSSEN_Z.Checked = ((ssen & 4) != 0);
            chkCtrSSEN_M1.Checked = ((ssen & 2) != 0);
            chkCtrSSEN_M0.Checked = ((ssen & 1) != 0);
        }

        // Convert snapshot reason flags to string
        private string ReasonText(UInt32 reason) {
            string t = "";
            if ((reason & mid826.S826_SSR_MATCH0) != 0)
                t += " M0";
            if ((reason & mid826.S826_SSR_MATCH1) != 0)
                t += " M1";
            if ((reason & mid826.S826_SSR_ZEROCOUNT) != 0)
                t += " ZER";
            if ((reason & mid826.S826_SSR_IX_RISE) != 0)
                t += " IXR";
            if ((reason & mid826.S826_SSR_IX_FALL) != 0)
                t += " IXF";
            if ((reason & mid826.S826_SSR_EXTIN_RISE) != 0)
                t += " EXR";
            if ((reason & mid826.S826_SSR_EXTIN_FALL) != 0)
                t += " EXF";
            if ((reason & mid826.S826_SSR_SOFT) != 0)
                t += " SOFT";
            if ((reason & mid826.S826_SSR_QUADERR) != 0)
                t += " QERR";
            return t;
        }

        // Timer event handler
        private void UpdateCounter() {
            UInt32 counts = default(UInt32);
            UInt32 timestamp = default(UInt32);
            UInt32 reason = default(UInt32);
            UInt32 ssen = default(UInt32);

            if (autoSnapshot[ctr])
                mid826.CounterSnapshot(board, ctr);
            // Invoke manual snapshot if required

            // If new snapshot is available ...

            if ((mid826.CounterSnapshotRead(board, ctr, ref counts, ref timestamp, ref reason, 0) == mid826.S826_ERR_OK)) {
                // Display snapshot values
                txtCtrCounts.Text = UintToString(counts);
                txtCtrTimestamp.Text = timestamp.ToString();
                lblSnapshotReason.Text = ReasonText(reason);

                // Update ShapshotEnables
                mid826.CounterSnapshotConfigRead(board, ctr, ref ssen);
                init += 1;
                //-------------------
                UpdateSnapshotEnables(ssen);
                init -= 1;
                //-------------------
            }
        }

        // Select counter tab
        private void TabCtrl_SelectedTab(object sender, System.EventArgs e) {
            cmbCtrChan.SelectedIndex = (int)ctr;
            ShowCounterConfig();
        }

        // Change counter channel
        private void cmbCtrChan_SelectedIndexChanged(System.Object sender, System.EventArgs e) {
            ctr = (uint)cmbCtrChan.SelectedIndex;
            lblSnapshotReason.Text = "";
            ShowCounterConfig();
        }

        // Invoke soft snapshot
        private void btnLatch_Click(System.Object sender, System.EventArgs e) {
            ReportErr(mid826.CounterSnapshot(board, ctr));
        }

        // Enable/disable counter channel
        private void chkCtrEnable_CheckedChanged(System.Object sender, System.EventArgs e) {
            if ((init == 0))
                mid826.CounterStateWrite(board, (uint)ctr, (uint)(chkCtrEnable.Checked ? 1 : 0));
        }

        // Route ExtIn signal
        private void ChangeCounterRoute(System.Object sender, System.EventArgs e) {
            if (init == 0)
                mid826.CounterExtInRoutingWrite(board, ctr, (uint)cmbRouteCenSource.SelectedIndex);
        }

        private void UpdateCounterMode(UInt32 mode) {
            lblCounterMode.Text = "0x" + mode.ToString("X8");
        }
        /*
    ' Program mode register
    Private Sub ChangeCounterMode(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles _
        chkCtrIP.CheckedChanged, cmbCtrIM.SelectedIndexChanged, _
        chkCtrPS.CheckedChanged, cmbCtrNR.SelectedIndexChanged, cmbCtrUD.SelectedIndexChanged, _
        chkCtrBP.CheckedChanged, cmbCtrOM.SelectedIndexChanged, chkCtrOP.CheckedChanged, _
        chkCtrPXH.CheckedChanged, chkCtrPXR.CheckedChanged, chkCtrPXF.CheckedChanged, _
        chkCtrPZ.CheckedChanged, chkCtrPM1.CheckedChanged, chkCtrPM0.CheckedChanged, _
        cmbCtrTE.SelectedIndexChanged, cmbCtrTD.SelectedIndexChanged, cmbCtrK.SelectedIndexChanged, cmbCtrXS.SelectedIndexChanged, cmbRouteCenSource.SelectedIndexChanged, chkCtrPS.CheckedChanged
        */
        private void ChangeCounterMode(System.Object sender, System.EventArgs e) {
            if (init == 0) {
                UInt32 mode = (uint)(((chkCtrIP.Checked ? 1 : 0 << CTRMODE_SHIFT_IP) & CTRMODE_MASK_IP) +
                    ((cmbCtrIM.SelectedIndex << CTRMODE_SHIFT_IM) & CTRMODE_MASK_IM) +
                    ((chkCtrPS.Checked ? 1 : 0 << CTRMODE_SHIFT_PS) & CTRMODE_MASK_PS) +
                    ((cmbCtrNR.SelectedIndex << CTRMODE_SHIFT_NR) & CTRMODE_MASK_NR) +
                    ((cmbCtrUD.SelectedIndex << CTRMODE_SHIFT_UD) & CTRMODE_MASK_UD) +
                    ((chkCtrBP.Checked ? 1 : 0 << CTRMODE_SHIFT_BP) & CTRMODE_MASK_BP) +
                    ((cmbCtrOM.SelectedIndex << CTRMODE_SHIFT_OM) & CTRMODE_MASK_OM) +
                    ((chkCtrOP.Checked ? 1 : 0 << CTRMODE_SHIFT_OP) & CTRMODE_MASK_OP) +
                    ((chkCtrPXH.Checked ? 1 : 0 << CTRMODE_SHIFT_PXH) & CTRMODE_MASK_PXH) +
                    ((chkCtrPXR.Checked ? 1 : 0 << CTRMODE_SHIFT_PXR) & CTRMODE_MASK_PXR) +
                    ((chkCtrPXF.Checked ? 1 : 0 << CTRMODE_SHIFT_PXF) & CTRMODE_MASK_PXF) +
                    ((chkCtrPZ.Checked ? 1 : 0 << CTRMODE_SHIFT_PZ) & CTRMODE_MASK_PZ) +
                    ((chkCtrPM1.Checked ? 1 : 0 << CTRMODE_SHIFT_PM1) & CTRMODE_MASK_PM1) +
                    ((chkCtrPM0.Checked ? 1 : 0 << CTRMODE_SHIFT_PM0) & CTRMODE_MASK_PM0) +
                    ((cmbCtrTE.SelectedIndex << CTRMODE_SHIFT_TE) & CTRMODE_MASK_TE) +
                    ((cmbCtrTD.SelectedIndex << CTRMODE_SHIFT_TD) & CTRMODE_MASK_TD) +
                    (uint) ((cmbCtrK.SelectedIndex << CTRMODE_SHIFT_K) & CTRMODE_MASK_K) +
                    (uint) ((cmbCtrXS.SelectedIndex << CTRMODE_SHIFT_XS) & CTRMODE_MASK_XS));

                mid826.CounterModeWrite(board, ctr, mode);
                UpdateCounterMode(mode);
            }

        }

        // Write to preload registers
        private void btnCtrWritePL0_Click(System.Object sender, System.EventArgs e) {
            mid826.CounterPreloadWrite(board, (uint)ctr, (uint)0, TextboxToUint(ref txtCtrPreload0));
        }
        private void btnCtrWritePL1_Click(System.Object sender, System.EventArgs e) {
            mid826.CounterPreloadWrite(board, (uint)ctr, (uint)1, TextboxToUint(ref txtCtrPreload1));
        }

        // Invoke soft preload
        private void btnCtrPreload0_Click(System.Object sender, System.EventArgs e) {
            if (((init == 0) & !chkCtrStickyPreload.Checked))
                mid826.CounterPreload(board, ctr, 0, 0);
        }
        private void chkCtrStickyPreload_CheckedChanged(System.Object sender, System.EventArgs e) {
            if ((init == 0))
                mid826.CounterPreload(board, ctr, (uint)(chkCtrStickyPreload.Checked ? 1 : 0),
                    (uint)(chkCtrStickyPreload.Checked ? 1 : 0));
        }

        // Write Compare registers
        private void CtrWriteCompare0(System.Object sender, System.EventArgs e) {
            if ((init == 0))
                mid826.CounterCompareWrite(board, ctr, 0, TextboxToUint(ref txtCtrCompare0));
        }
        private void CtrWriteCompare1(System.Object sender, System.EventArgs e) {
            if ((init == 0))
                mid826.CounterCompareWrite(board, ctr, 1, TextboxToUint(ref txtCtrCompare1));
        }

        // Update snapshot control reg value
        private void UpdateSnapshotControl(UInt32 ctrl) {
            lblCounterSSCtrlReg.Text = "0x" + ctrl.ToString("X8");
        }
        // Write snapshot control
        private void ConfigSnapshot(int enables, CheckBox cbox) {
            if ((init == 0))
                mid826.CounterSnapshotConfigWrite(board, ctr, (uint)enables,
                    (uint)(cbox.Checked ? C_REG_BITSET : C_REG_BITCLR));
            UInt32 ctrl = default(UInt32);
            mid826.CounterSnapshotConfigRead(board, ctr, ref ctrl);
            UpdateSnapshotControl(ctrl);
        }
        // Snapshot enables
        private void btnCtrSSENCLR_ER(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(0x400000, chkCtrSSENCLR_ER);
        }
        private void btnCtrSSENCLR_EF(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(0x200000, chkCtrSSENCLR_EF);
        }
        private void btnCtrSetSSENCLR_XR(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(0x100000, chkCtrSSENCLR_XR);
        }
        private void btnCtrSetSSENCLR_XF(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(0x80000, chkCtrSSENCLR_XF);
        }
        private void btnCtrSetSSENCLR_Z(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(0x40000, chkCtrSSENCLR_Z);
        }
        private void btnCtrSetSSENCLR_M1(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(0x20000, chkCtrSSENCLR_M1);
        }
        private void btnCtrSetSSENCLR_M0(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(0x10000, chkCtrSSENCLR_M0);
        }
        // Autoclear enables
        private void btnCtrSSEN_ER(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(64, chkCtrSSEN_ER);
        }
        private void btnCtrSSEN_EF(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(32, chkCtrSSEN_EF);
        }
        private void btnCtrSetSSEN_XR(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(16, chkCtrSSEN_XR);
        }
        private void btnCtrSetSSEN_XF(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(8, chkCtrSSEN_XF);
        }
        private void btnCtrSetSSEN_Z(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(4, chkCtrSSEN_Z);
        }
        private void btnCtrSetSSEN_M1(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(2, chkCtrSSEN_M1);
        }
        private void btnCtrSetSSEN_M0(System.Object sender, System.EventArgs e) {
            ConfigSnapshot(1, chkCtrSSEN_M0);
        }

        //Format Preload/Compare/counts values as signed or unsigned
        private void chkCtrSigned_CheckedChanged(System.Object sender, System.EventArgs e) {
            ShowCounterConfig();
        }

        private void chkAutoSnapshot_CheckedChanged(System.Object sender, System.EventArgs e) {
            if (init == 0)
                autoSnapshot[ctr] = chkAutoSnapshot.Checked;
        }

        private void btnAdcCalRead_Click(System.Object sender, System.EventArgs e) {
            double[] slope = new double[5];
            Int32[] offset = new Int32[5];
            Int32 valid = default(Int32);
            mid826.AdcCalRead(board, slope, offset, ref valid);
            txtAdcCalOffset0.Text = offset[0].ToString();
            txtAdcCalOffset1.Text = offset[1].ToString();
            txtAdcCalOffset2.Text = offset[2].ToString();
            txtAdcCalOffset3.Text = offset[3].ToString();
            txtAdcCalSlope0.Text = slope[0].ToString("0.00000");
            txtAdcCalSlope1.Text = slope[1].ToString("0.00000");
            txtAdcCalSlope2.Text = slope[2].ToString("0.00000");
            txtAdcCalSlope3.Text = slope[3].ToString("0.00000");
            lblAdcCalValid.Text = (valid != 0 ? "Valid" : "Not valid");
        }

        private void btnAdcCalWrite_Click(System.Object sender, System.EventArgs e) {
            double[] slope = new double[5];
            Int32[] offset = new Int32[5];
            offset[0] = Int32.Parse(txtAdcCalOffset0.Text);
            offset[1] = Int32.Parse(txtAdcCalOffset1.Text);
            offset[2] = Int32.Parse(txtAdcCalOffset2.Text);
            offset[3] = Int32.Parse(txtAdcCalOffset3.Text);
            slope[0] = double.Parse(txtAdcCalSlope0.Text);
            slope[1] = double.Parse(txtAdcCalSlope1.Text);
            slope[2] = double.Parse(txtAdcCalSlope2.Text);
            slope[3] = double.Parse(txtAdcCalSlope3.Text);
            ReportErr(mid826.AdcCalWrite(board, slope, offset));
        }

        private void chkAdcCalCorrect_CheckedChanged(System.Object sender, System.EventArgs e) {
            if (chkAdcRaw.Checked) {
                chkAdcShowVolts.Enabled = false;
                chkAdcShowVolts.Checked = false;
            } else {
                chkAdcShowVolts.Enabled = true;
            }
        }

        private void btnDacCalRead_Click(System.Object sender, System.EventArgs e) {
            double[] slope = new double[5];
            Int32 valid = default(Int32);
            mid826.DacCalRead(board, slope, ref valid);
            txtDacCalSlope0.Text = slope[0].ToString("0.000000");
            txtDacCalSlope1.Text = slope[1].ToString("0.000000");
            txtDacCalSlope2.Text = slope[2].ToString("0.000000");
            txtDacCalSlope3.Text = slope[3].ToString("0.000000");
            lblDacCalValid.Text = (valid != 0 ? "Valid" : "Not valid");
        }

        private void btnDacCalWrite_Click(System.Object sender, System.EventArgs e) {
            double[] slope = new double[5];
            slope[0] = double.Parse(txtDacCalSlope0.Text);
            slope[1] = double.Parse(txtDacCalSlope1.Text);
            slope[2] = double.Parse(txtDacCalSlope2.Text);
            slope[3] = double.Parse(txtDacCalSlope3.Text);
            ReportErr(mid826.DacCalWrite(board, slope));
        }

        //*********************************************************************************
        //*******************************************************  WATCHDOG  **************
        //*********************************************************************************

        //update watchdog status
        private void UpdateWatchdog() {
            uint status = 0;
            mid826.WatchdogStatusRead(board, ref status);
            chkWdogStatus0.Checked = ((status & 1) != 0);
            chkWdogStatus1.Checked = ((status & 2) != 0);
            chkWdogStatus2.Checked = ((status & 4) != 0);
        }
        private void WatchdogSettingsRead() {
            uint[] timers = new uint[6];
            uint cfg = 0;
            uint enab = 0;
            mid826.WatchdogEnableRead(board, ref enab);
            mid826.WatchdogConfigRead(board, ref cfg, timers);
            init += 1;
            //-------------------
            chkWdogEN.Checked = (enab != 0);
            chkWdogSEN.Checked = ((cfg & 0x10) != 0);
            chkWdogNIE.Checked = ((cfg & 8) != 0);
            chkWdogPEN.Checked = ((cfg & 4) != 0);
            chkWdogOEN.Checked = ((cfg & 1) != 0);

            txtWdogTimeStage0.Text = String.Format("{0}", timers[0]);
            txtWdogTimeStage1.Text = String.Format("{0}", timers[1]);
            txtWdogTimeStage2.Text = String.Format("{0}", timers[2]);
            txtWdogTimePulse.Text = String.Format("{0}", timers[3]);
            txtWdogTimeGap.Text = String.Format("{0}", timers[4]);

            init -= 1;
            //-------------------
        }


        private void WatchdogConfigChanged(object sender, EventArgs e) {
            if (init == 0) {
                // Write watchdog configuration
                UInt32[] timers = new UInt32[6];
                timers[0] = uint.Parse(txtWdogTimeStage0.Text);
                timers[1] = uint.Parse(txtWdogTimeStage1.Text);
                timers[2] = uint.Parse(txtWdogTimeStage2.Text);
                timers[3] = uint.Parse(txtWdogTimePulse.Text);
                timers[4] = uint.Parse(txtWdogTimeGap.Text);
                uint cfg = (uint)(((chkWdogSEN.Checked ? 1 : 0 & 1) << 4) +
                    ((chkWdogNIE.Checked ? 1 : 0 & 1) << 3) +
                    ((chkWdogPEN.Checked ? 1 : 0 & 1) << 2) +
                    (chkWdogOEN.Checked ? 1 : 0 & 1));
                ReportErr(mid826.WatchdogConfigWrite(board, cfg, timers));
            }
        }

        private void chkWdogEN_CheckedChanged(object sender, EventArgs e) {
            if (init == 0) {
                int errcode = mid826.WatchdogEnableWrite(board, (uint)(chkWdogEN.Checked ? 1 : 0));
                ReportErr(errcode);
            }
        }


        private void btnWdogKick_Click(object sender, EventArgs e) {
            mid826.WatchdogKick(board, 0x5a55aa5a);
        }

        private void scrollDioFilterDelay_Scroll(System.Object sender, System.Windows.Forms.ScrollEventArgs e) {
            UInt32[] enables = new UInt32[2];
            UInt32 filtdelay = default(UInt32);
            mid826.DioFilterRead(board, ref filtdelay, enables);
            mid826.DioFilterWrite(board, (uint) scrollDioFilterDelay.Value, enables);
            lblDioFilterDelay.Text = scrollDioFilterDelay.Value.ToString();

        }

        private void chkCtrFiltClk_CheckedChanged(System.Object sender, System.EventArgs e) {
            if ((init == 0)) {
                UInt32 cfg = default(UInt32);
                mid826.CounterFilterRead(board, ctr, ref cfg);
                mid826.CounterFilterWrite(board, ctr, (chkCtrFiltClk.Checked ? cfg | 0x40000000 : cfg & 0xbfffffff));
            }
        }

        private void chkCtrFiltIX_CheckedChanged(System.Object sender, System.EventArgs e) {
            if ((init == 0)) {
                UInt32 cfg = default(UInt32);
                mid826.CounterFilterRead(board, ctr, ref cfg);
                mid826.CounterFilterWrite(board, ctr, (chkCtrFiltIX.Checked ? cfg | 0x80000000 : cfg & 0x7fffffff));
            }
        }

        private void btnCtrFilterWrite_Click(System.Object sender, System.EventArgs e) {
            UInt32 cfg = default(UInt32);
            if ((int.Parse(txtCtrFilterDelay.Text) > 0xffff)) {
                txtCtrFilterDelay.Text = (0xffff).ToString();
            } else if ((int.Parse(txtCtrFilterDelay.Text) < 0)) {
                txtCtrFilterDelay.Text = (0).ToString();
            }
            mid826.CounterFilterRead(board, ctr, ref cfg);
            mid826.CounterFilterWrite(board, ctr, (cfg & 0xc0000000) | uint.Parse(txtCtrFilterDelay.Text));
        }

    }
}
    