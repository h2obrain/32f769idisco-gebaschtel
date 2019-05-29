# DSI host (see AN4860)


Main subsystems
- DSI host
- DSI wrapper


## STM32 MIPI-DSI implementation overview

### DSI command???
- Display command set (DCS)


### DSI command mode (DBI)
- Display bus interface (MIPI-DBI)
    - Type A which supports the parallel Motorola 6800 bus.
    - Type B which supports the parallel Intel 8080 bus.
    - Type C which supports the SPI interface.
- MIPI-DBI Type A and Type B are supported by the STM32 embedding a F(S)MC
  (flexible synchronous memory controller).
  See AN2790 'TFT LCD interfacing with the high-density STM32F10xxx FSMC'.
- Interfacing directly with APB


### DSI video mode (DPI)
- Display pixel interface (MIPI-DPI)
- Interfacing with LTDC
- Pixel stream to display. No GRAM in display needed.


### STM32 DSI host modes
- Video mode
    - Implements DPI
- Adapted command mode
    - Update GRAM of display only when requested through DSI host
- APB command mode
    - Send generic commands to the display. Init and control display.


## DSI host config
```
Global initialization
- DSI host 1.2V voltage regulator
    - Enable:                                             DSI_WRPCR.REGEN
    - Wait ready:                                         DSI_WISR.RRS
    - D-PHY is now powered ON
- Clocks configuration (HS, TX, secondary)
    - Internal PLL
        - FVCO = (HSE / IDF) x 2 x NDIV
        - Constraints: 500MHz <= FVCO <= 1GHz
        - Input div:                                      DSI_WRPCR.IDF
        - Loop div:                                       DSI_WRPCR.NDIV
    - HS clock using DSI Host internal PLL
        - HS   = FVCO / (2 x ODF)
        - Constraints: 80MHz <= HS <= 500MHz
        - Output div:                                     DSI_WRPCR.ODF
    - Lane_Byte_CLK (lane_byte_clock, LBCLK)
        - LBCLK = HS / 8 (or PLLR as secondary clock)
        - Constraints: LBCLK <= 62.5MHz
    - TX escape clock for LP communication
        - TX = LBCLK / TXECKDIV
        - Constraints: TX <= 20MHz
    - Secondary clock when DSI host PLL off
        - To use PLLR set DCKCFGR2.DSISEL bit
    - Enable PLL
        - Enable:                                         DSI_WRPCR.PLLEN
        - Wait lock:                                      DSI_WISR.PLLS
- DSI PHY (D-PHY) configuration
    - Lane count:                                         DSI_PCONFR.NL
    - D-PHY enable:                                       DSI_PCTLR.DEN
    - D-PHY clock lane:                                   DSI_PCTLR.CKE
    - Clock lane control
        - Automatic Clock lane control:                   DSI_CLCR.ACR
        - D-PHY Clock Control:                            DSI_CLCR.DPCC
        - Constraints: DSI_CLCR.DPCC|=DSI_CLCR.ACR
    - DSI wrapper configuration (??? formula ???)
        - UIX4 = floor(1/HS/0.25ns)
        - HS bit period (/0.25ns):                        DSI_WPCR0.UIX4
    - Protocol flow control
        - See AN2790 for details
        - EoTp transmission enable:                       DSI_PCR.ETTXE
        - EoTp reception enable:                          DSI_PCR.ETRXE
        - Bus turnaround enable:                          DSI_PCR.BTAE
        - ECC reception enable:                           DSI_PCR.ECCRXE
        - CRC reception enable:                           DSI_PCR.CRCRXE
- DSI Host LTDC interface configuration
    - Color coding configuration
        - Wrapper color coding:                           DSI_WCFGR. COLMUX
        - Host color coding:                              DSI_LCOLCR.COLC
        - Loosely packed variant to 18-bit:               DSI_LCOLCR.LPE
    - Video control signal polarity
        - HSYNC polarity:                                 DSI_LPCR.HSP
        - VSYNC polarity:                                 DSI_LPCR.VSP
        - DATA ENABLE polarity:                           DSI_LPCR.DEP

DSI operational mode: Video mode over LTDC interface
- Video mode selection
    - Video mode type:                                    DSI_VMCR.VMT
- LP state in video mode
    - At least once per frame, better once per line.
    - Enables (provides?) PHY synchronization.
- PHY transition timing
    - See AN2790 for details
    - xxxxx_TIME = time / LBCLK
    - Clock lane timer (maximum)
        - High-speed to LP time:                          DSI_CLTCR.HS2LP_TIME
        - LP time to high-speed:                          DSI_CLTCR.LP2HS_TIME
    - Data lane timer (maximum)
        - High-speed to LP time:                          DSI_DLTCR.HS2LP_TIME
        - LP time to high-speed:                          DSI_DLTCR.LP2HS_TIME
            - Constraints: Only needed if DSI_CLCR.ACR
- LP (Low-power) transitions
    - See AN2790 for details
    - Horizontal front-porch enable:                      DSI_VMCR.LPHFPE
    - Horizontal back-porch enable:                       DSI_VMCR.LPHBPE
    - Vertical active enable:                             DSI_VMCR.LPVAE
    - Vertical front-porch enable:                        DSI_VMCR.LPVFPE
    - Vertical back-porch enable:                         DSI_VMCR.LPVBPE
    - Vertical sync active enable:                        DSI_VMCR.LPVSAE
- LTDC settings
    - See AN2790 for an example
    - Constraints:
        - Displays supported timings
        - Horizontal timing should allow entering LP mode
        - Vertical refresh timing shall allow entering LP mode
    - Set LTDC pixel clock (PLLSAI out, pixel_clock, PCLK)
        - PCLK = (VSA+VBP+VACT+VFP) x (HSA+HBP+HACT+HFP) x frame rate
        - rcc_pllsai_config(..)
        - rcc_pllsai_postscalers(..)
        - rcc_pllsai_enable(), while (!rcc_pllsai_ready());
    - Use ltdc_set_tft_sync_timings(..) to set timings
        - Timing unit = time / PCLK
        - >>>??? this can't be true.. >> seems to be for DSI
        - HSA  = sync_width    /PCLK*LBCLK
        - HBP  = h_back_porch  /PCLK*LBCLK
        - HACT = active_width  /PCLK*LBCLK
        - HFP  = h_front_porch /PCLK*LBCLK
        - VSA  = sync_height
        - VBP  = v_back_porch
        - VACT = active_height
        - VFP  = v_front_porch
- DSI Host video timing
    - See AN2790 for an example
    - Timing unit = time / LBCLK
    - ?? correct ?? >>
    - Horizontal sync active (HSA):                       DSI_VHSACR.HSA
    - Horizontal back porch (HBP):                        DSI_VHBPCR.HBP
    - Horizontal line (HSA+HBP+HACT+HFP):                 DSI_VLCR.HLINE
    - Vertical active (VSA):                              DSI_VVSACR.VSA
    - Vertical back porch (VBP):                          DSI_VVBPCR.VBP
    - Vertical front porch (VFP):                         DSI_VVFPCR.VFP
    - Vertical active (VACT):                             DSI_VVACR.VA
- DSI clock setting
    - See AN2790 for an example
    - Burst mode
        - Ideally set LBCLK to max (62.5MHz)
    - Non-burst mode
        - See DSI video packet parameters
        - Set LBCLK that DSI output pixel ratio = LTDC input pixel ratio
- DSI video packet parameters
    - See AN2790 for an example
    - Video packet size in pixels:                        DSI_VPCR.VPSIZE
    - Number of chunks:                                   DSI_VCCR.NUMC
    - Null packet size in bytes:                          DSI_VNPCR.NPSIZE
    - Burst mode
        - VPSIZE = DSI_VLCR.HLINE ???
        - NUMC   = 0
        - NPSIZE = 0
    - Non-burst mode
        - Null packets enabled
            - lanebyteclkperiod * NUMC
                * (VPSIZE * bytes_per_pixel + 12 + NPSIZE)
                / number_of_lanes
              = pixels_per_line * PCLK
        - Null packets disabled
            - lanebyteclkperiod * NUMC
                * (VPSIZE x bytes_per_pixel + 6)
                / number_of_lanes
              = pixels_per_line * PCLK
- Command transmission in video mode
    - Note: Some displays require commands in LP mode during initialisation
    - Transmit commands in LP mode:                       DSI_VMCR.LPCE
    - Largest packet size:                                DSI_LPMCR.LPSIZE
    - VACT largest packet size:                           DSI_LPMCR.VLPSIZE
    - LPSIZE in non-burst mode
        - LPSIZE = floor_and_margin(
                      tL - (tH1 + tHSLP + tLPDT + tESCCLK*2 + tLPHS)
                      / (2*8*tESCCLK)
                  )
        - tL = (HSA + HBP + HACT + HFP) / PCLK
        - With sync pulses
            - tH1 = HSA / PCLK
        - With sync events
            - tH1 = 4 / (Number_Lanes * LBCLK)
        - tHSLP = DSI_CLTCR.HS2LP_TIME / LBCLK = 291ns (this is D-PHY specific) ???
        - tLPHS = DSI_CLTCR.LP2HS_TIME / LBCLK = 264ns (this is D-PHY specific) ???
        - tLPDT = 22 x tESCCLK
        - tESCCLK = TXECKDIV / LBCLK
    - VLPSIZE in non-burst mode
        - VLPSIZE = floor_and_margin(
                      (tL - (tHSA + tHBP + tHACT + tHSLP + tLPDT + tLPHS))
                      / (2*8*tESCCLK)
                  )
        - tL = (HSA + HBP + HACT + HFP) / PCLK
        - tHSA = HSA / PCLK
        - tHBP = HBP / PCLK
        - Non-burst mode
            - tHACT = HACT / PCLK
        - Burst mode
            - tHACT = (VPSIZE x Bytes_per_Pixel) / (Number_Lanes * LBCLK)
        - See LPSIZE for tHSLP, tLPHS, tLPDT and tESCCLK
- Frame acknowledge
    - Frame bus-turnaround ack enable:                    DSI_VMCR.FBTAAE
    - Constraints: DSI_PCR.BTAE |= FBTAAE

DSI operational mode: Adapted command mode over LTDC interface
- DSI command mode
    - Enable adapted command mode:                        DSI_WCFGR.DSIM
    - Enable DSI Host command mode:                       DSI_MCR.CMDM
- Stop wait time configuration
    - time = 1/LBCLK
    - Minimum stop state to high-speed transmission time request delay:
                                                          DSI_PCONFR.SW_TIME
    - Constraints for DSI host: SW_TIME >= 10
    - Constraints for display may also apply
- Command size (CMDSIZE)
    - Maximum LTDC write memory size in pixels:           DSI_LCCR.CMDSIZE
    - Constraints
        - DSI Host pixel FIFO size is 960 32bit words
        - 24bpp mode: CMDSIZE = 1280
        - 16bpp mode: CMDSIZE = 1920
- LTDC halt polarity
    - DSI Wrapper VSYNC polarity:                         DSI_WCFGR.VSPOL
    - Constraints: VSPOL = LTDC halt polarity
- Tearing effect settings
    - TE source:                                          DSI_WCFGR.TESRC
    - TE polarity:                                        DSI_WCFGR.TEPOL
    - TE acknowledge request enable:                      DSI_CMCR.TEARE
    - Constraints if !TESRC: TEARE=1; TEPOL=x (no effect)
- Refresh mode
    - Automatic Refresh:                                  DSI_WCFGR.AR
    - If !AR the software has to set DSI_WCR.LTDCEN after TE event
- LTDC settings
    - Pixel clock (PCLK)
        - PCLK is mostly independent
        - Constraints: display(GRAM) refresh rate < LTDC refresh < max ???
- LTDC video timing
    - Timing can be set to minimum, HACT/VACT need to be correct
    - HSA = HBP = HFP = VSA = VBP = VFP = 1
    - HACT = width, VACT = height
- Command transmission mode
    - Set which commands are transmitted in LP mode
    - Note: Some displays require commands in LP mode during initialisation
    - Maximum read packet size:                           DSI_CMCR.MRDPS
    - DCS long write transmission:                        DSI_CMCR.DLWTX
    - DCS short read zero parameter transmission:         DSI_CMCR.DSR0TX
    - DCS short read one parameter transmission:          DSI_CMCR.DSW1TX
    - DCS short write zero parameter transmission:        DSI_CMCR.DSW0TX
    - Generic long write transmission:                    DSI_CMCR.GLWTX
    - Generic short read two parameters transmission:     DSI_CMCR.GSR2TX
    - Generic short read one parameters transmission:     DSI_CMCR.GSR1TX
    - Generic short read zero parameters transmission:    DSI_CMCR.SR0TX
    - Generic short write two parameters transmission:    DSI_CMCR.GSW2TX
    - Generic short write one parameters transmission:    DSI_CMCR.GSW1TX
    - Generic short write zero parameters transmission:   DSI_CMCR.GSW0TX
- Acknowledge request
    - Note: Should be avoided in adapted command mode (adds a lot of overhead)
    - Enable acknowledge request after each packet:       DSI_CMCR.ARE


```





