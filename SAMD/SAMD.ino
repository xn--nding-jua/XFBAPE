/*
  X-f/bape USBCtrl for Arduino MKR Vidor4000 Device
  v3.1.0 built on 21.12.2024
  Infos: https://www.github.com/xn--nding-jua/xfbape
  Copyright (c) 2023-2024 Dr.-Ing. Christian Nöding

  Target-Board: Arduino MKR Vidor 4000 (Atmel SAMD21 32-bit CortexM0+ controller)
  Board-package: http://downloads.arduino.cc/Hourly/samd/package_samd-hourly-build_index.json

  Description:
  =====================================================================
  This Arduino-Project loads a bitstream to the Intel Cyclone10LP FPGA and
  communicates with USB-Host and NINA-W102-Module, which implements a full
  featured MP3, WAVE and AAC SD-Card-Player with WiFi-Webserver and A2DP-
  Bluetooth-Receiver.

  Used libraries:
  =====================================================================
  only when using OLED display:
  - Adafruit SSD1306 v2.5.10 by Adafruit (for I2C Display)

  License information:
  =====================================================================
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  Pin information:
  =====================================================================
  +------------+------------------+--------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | Pin number |  MKR  Board pin  |  PIN   |  FPGA  | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H | Usage for Audioplayer                                     |
  |            |                  |        |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |                                                           |
  |            |                  |        |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 00         | D0               |  PA22  |  G1    |                 |  *06   |     |     | X10 |     |   3/00  |   5/00  |* TC4/0 | TCC0/4 |          | GCLK_IO6 | -> Serial2 TxD -> FPGA G1 -> VHDL -> FPGA T6 -> NINA RxD  |
  | 01         | D1               |  PA23  |  N3    |                 |  *07   |     |     | X11 |     |   3/01  |   5/01  |* TC4/1 | TCC0/5 | USB/SOF  | GCLK_IO7 | <- Serial2 RxD <- FPGA N3 <- VHDL <- FPGA E15 <- NINA TxD |
  | 02         | D2               |  PA10  |  P3    |                 |   10   | *18 |     | X02 |     |   0/02  |   2/02  |*TCC1/0 | TCC0/2 | I2S/SCK0 | GCLK_IO4 | -> Expansion Header X202.3                                |
  | 03         | D3               |  PA11  |  R3    |                 |   11   | *19 |     | X03 |     |   0/03  |   2/03  |*TCC1/1 | TCC0/3 | I2S/FS0  | GCLK_IO5 | -> Expansion Header X205.4                                |
  | 04         | D4               |  PB10  |  T3    |                 |  *10   |     |     |     |     |         |   4/02  |* TC5/0 | TCC0/4 | I2S/MCK1 | GCLK_IO4 | -> Expansion Header X205.3 (MIDI Tx)                      |
  | 05         | D5               |  PB11  |  T2    |                 |  *11   |     |     |     |     |         |   4/03  |* TC5/1 | TCC0/5 | I2S/SCK1 | GCLK_IO5 | -> Expansion Header X205.2 (MIDI Rx)                      |
  | 06         | D6               |  PA20  |  G16   |                 |  *04   |     |     | X08 |     |   5/02  |   3/02  |        |*TCC0/6 | I2S/SCK0 | GCLK_IO4 | -> n/a                                                    |
  | 07         | D7               |  PA21  |  G15   |                 |  *05   |     |     | X09 |     |   5/03  |   3/03  |        |*TCC0/7 | I2S/FS0  | GCLK_IO5 | -> SPI-CS W5500 EthernetShield                            |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |       SPI        |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 08         | MOSI             |  PA16  |  F16   |                 |  *00   |     |     | X04 |     |  *1/00  |   3/00  |*TCC2/0 | TCC0/6 |          | GCLK_IO2 | -> SPI-MOSI W5500 EthernetShield                          |
  | 09         | SCK              |  PA17  |  F15   |                 |  *01   |     |     | X05 |     |  *1/01  |   3/01  | TCC2/1 | TCC0/7 |          | GCLK_IO3 | -> SPI-SCK W5500 EthernetShield                           |
  | 10         | MISO             |  PA19  |  C16   |                 |   03   |     |     | X07 |     |  *1/03  |   3/03  |* TC3/1 | TCC0/3 | I2S/SD0  | AC/CMP1  | -> SPI-MISO W5500 EthernetShield                          |
  +------------+------------------+--------+--------+-----------------+--------------------+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |       Wire       |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           | 
  | 11         | SDA              |  PA08  |  C15   |                 |   NMI  | *16 |     | X00 |     |  *0/00  |   2/00  | TCC0/0 | TCC1/2 | I2S/SD1  |          | -> I2C SDA (SSD1308, EEPROM) / NINA PIO27 (Bootmode-Sel.) |
  | 12         | SCL              |  PA09  |  B16   |                 |   09   | *17 |     | X01 |     |  *0/01  |   2/01  | TCC0/1 | TCC1/3 | I2S/MCK0 |          | -> I2C SCL (SSD1308, EEPROM)                              |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |      Serial1     |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 13         | RX               |  PB23  |  C11   |                 |   07   |     |     |     |     |         |  *5/03  |        |        |          | GCLK_IO1 | <- FPGA RxD <- Behringer X32 Main-Control                 |
  | 14         | TX               |  PB22  |  A13   |                 |   06   |     |     |     |     |         |  *5/02  |        |        |          | GCLK_IO0 | -> FPGA TxD -> Behringer X32 Main-Control                 |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 15         | A0 / DAC0        |  PA02  |  C2    |                 |   02   | *00 |     | Y00 | OUT |         |         |        |        |          |          | -> Expansion Header X202.10                               |
  | 16         | A1               |  PB02  |  C3    |                 |  *02   | *10 |     | Y08 |     |         |   5/00  |        |        |          |          | -> Expansion Header X202.9                                |
  | 17         | A2               |  PB03  |  C6    |                 |  *03   | *11 |     | Y09 |     |         |   5/01  |        |        |          |          | -> Expansion Header X202.8                                |
  | 18         | A3               |  PA04  |  D1    |                 |   04   | *04 |  00 | Y02 |     |         |   0/00  |*TCC0/0 |        |          |          | -> Expansion Header X202.7                                |
  | 19         | A4               |  PA05  |  D3    |                 |   05   | *05 |  01 | Y03 |     |         |   0/01  |*TCC0/1 |        |          |          | -> Expansion Header X202.6                                |
  | 20         | A5               |  PA06  |  F3    |                 |   06   | *06 |  02 | Y04 |     |         |   0/02  | TCC1/0 |        |          |          | -> Expansion Header X202.5                                |
  | 21         | A6               |  PA07  |  G2    |                 |   07   | *07 |  03 | Y05 |     |         |   0/03  | TCC1/1 |        | I2S/SD0  |          | -> Expansion Header X202.4                                |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            |       USB        |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 22         |                  |  PA24  |        | USB N           |   12   |     |     |     |     |   3/02  |   5/02  |  TC5/0 | TCC1/2 | USB/DM   |          | -> Serial Rx/Tx via USB                                   |
  | 23         |                  |  PA25  |        | USB P           |   13   |     |     |     |     |   3/03  |   5/03  |  TC5/1 | TCC1/3 | USB/DP   |          | -> Serial Rx/Tx via USB                                   |
  | 24         |                  |  PA18  |        | USB ID          |   02   |     |     | X06 |     |   1/02  |   3/02  |  TC3/0 | TCC0/2 |          | AC/CMP0  |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 25         | AREF             |  PA03  |  B1    |                 |   03   |  01 |     | Y01 |     |         |         |        |        |          |          | -> n/a                                                    |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            | FPGA             |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 26         |                  |  PA12  |  H4    | FPGA TDI        |   12   |     |     |     |     |  *2/00  |   4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  | -> [JTAG]                                                 |
  | 27         |                  |  PA13  |  H3    | FPGA TCK        |   13   |     |     |     |     |  *2/01  |   4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  | -> [JTAG]                                                 |
  | 28         |                  |  PA14  |  J5    | FPGA TMS        |   14   |     |     |     |     |   2/02  |   4/02  |  TC3/0 | TCC0/4 |          | GCLK_IO0 | -> [JTAG]                                                 |
  | 29         |                  |  PA15  |  J4    | FPGA TDO        |   15   |     |     |     |     |  *2/03  |   4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 | -> [JTAG]                                                 |
  | 30         |                  |  PA27  |  E2    | FPGA CLK        |   15   |     |     |     |     |         |         |        |        |          | GCLK_IO0 | -> 48 MHz Clock -> FPGA                                   |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  | 31         |                  |  PA28  |  L16   | FPGA MB INT     |   08   |     |     |     |     |         |         |        |        |          | GCLK_IO0 | -> NINA RESET_N                                           |
  | 32         |                  |  PB08  |        | LED_RED_BUILTIN |   08   |  02 |     | Y14 |     |         |   4/00  |  TC4/0 |        |          |          | -> LED                                                    |
  | 33         |                  |  PB09  |        | SAM_INT_OUT     |  *09   |  03 |     | Y15 |     |         |   4/01  |  TC4/1 |        |          |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
  |            | 32768Hz Crystal  |        |        |                 |        |     |     |     |     |         |         |        |        |          |          |                                                           |
  | 34         |                  |  PA00  |        | XIN32           |   00   |     |     |     |     |         |   1/00  | TCC2/0 |        |          |          |                                                           |
  | 35         |                  |  PA01  |        | XOUT32          |   01   |     |     |     |     |         |   1/01  | TCC2/1 |        |          |          |                                                           |
  +------------+------------------+--------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+-----------------------------------------------------------+
*/

#include "SAMD.h"

void ticker100msFcn() {
  #if USE_DISPLAY == 1
    // do menu/GUI-relevant stuff
    displayDrawMenu();
  #endif

  #if USE_XTOUCH == 1
    if (Ethernet.linkStatus() != LinkOFF) {
      if (XCtlConnectionTimeout > 0) {
        XCtlConnectionTimeout -= 1;
      }else{
        XCtlWatchdogCounter -= 1;
        if (XCtlWatchdogCounter == 0) {
          XCtlWatchdogCounter = 20;

          for (uint8_t i_xtouch=0; i_xtouch<XTOUCH_COUNT; i_xtouch++) {
            XCtl_sendWatchDogMessage(i_xtouch);
          }
        }

        for (uint8_t i_xtouch=0; i_xtouch<XTOUCH_COUNT; i_xtouch++) {
          // count down channel-name-counter to display channel-name after changing a valud
          for (uint8_t i_ch=0; i_ch<8; i_ch++) {
            if (XCtl[i_xtouch].hardwareChannel[i_ch].nameCounter > 0) {
              XCtl[i_xtouch].hardwareChannel[i_ch].nameCounter -= 1;
            }
          }

          // send data
          XCtl_prepareData(i_xtouch); // prepare values to send to device
          XCtl_sendGeneralData(i_xtouch); // update buttons and displays
          XCtl_sendFaderData(i_xtouch); // update fader
        }
      }
    }
  #endif
  
  #if USE_MACKIE_MCU == 1
    mackieUpdateCounter -= 1;

    // count down channel-name-counter to display channel-name after changing a valud
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      if (MackieMCU.hardwareChannel[i_ch].nameCounter > 0) {
        MackieMCU.hardwareChannel[i_ch].nameCounter -= 1;
      }
    }

    if (mackieUpdateCounter == 0) {
      mackieUpdateCounter = 2; // 200ms update-rate
      MackieMCU_sendData();
    }
  #endif
}
Ticker ticker100ms(ticker100msFcn, 100, 0, MILLIS);

void ticker85msFcn() {
  x32AliveCounter--;
  if (x32AliveCounter == 0) {
    x32AliveCounter = 59; // preload to 5 seconds

    SerialX32.print(x32AliveCommand);
  }

  if (x32StartupCounter > 0) {
    x32StartupCounter--;

    if (x32StartupCounter == 0) {
      // tell the X32, that we have a card
      x32UpdateCard();
    }
  }

  if (x32Playback) {
    // send current sample-position every 85ms if playing
    uint32_t x32PlaybackPosition = playerinfo.duration * playerinfo.progress * (48000.0f/100.0f); // calculate sample-position based on 48kHz, duration and current process
    SerialX32.print("*9N22" + intToHex(x32PlaybackPosition, 8) + "#");
  }
}
Ticker ticker85ms(ticker85msFcn, 85, 0, MILLIS);

void setup() {
  // initialize peripherals
  pinMode(LED_BUILTIN, OUTPUT); // red color on RGB-LED of Vidor4000

  // initialize communication
  // Serial for communication via USB
  Serial.begin(115200); // the baud-rate will be set dynamically on connecting a computer
  Serial.setTimeout(1000); // Timeout for commands

  Serial.println("X-f/bape USBCtrl " + String(versionstring) + " | " + String(compile_date)); // send to USB

  // initialize FPGA
  Serial.println(F("Init FPGA..."));
  setup_fpga();
  delay(1000); // give FPGA a second to startup

  // init I2C (is used at least by EEPROM)
  Serial.println(F("Init I2C..."));
  Wire.begin();
  Wire.setClock(100000); // max. 400000Hz

  #if USE_DISPLAY == 1
    Serial.println(F("Init Display..."));
    initDisplay(); // init display again after uploading data to FPGA
    displayShowLogo(1); // show main logo again
    //displayText(5, F("Init MainCtrl..."));
  #endif

  // SerialX32 for communication with Behringer X32 MainControl
  Serial.println(F("Init UART-ports..."));
  SerialX32.begin(38400); // X32 uses regular 38400 8N1 for communication
  SerialX32.setTimeout(1000); // Timeout for commands

  // Serial2 for communication with NINA-module
  SerialNina.begin(921600); // SAMD21 supports maximum 3Mbps: f_baud <= 48MHz/16. But everything above 921600 baud seems to be instable
  //SerialNina.begin(1228800); // NINA can be programmed, but communication between SAMD21 and computer is not stable
  SerialNina.setTimeout(1000); // Timeout for commands
  pinPeripheral(0, PIO_SERCOM); //Assign TX function to pin 0
  pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1
  SerialNina.flush();
  delay(1000); // give NINA some time to startup

/*
  // Serial4
  Serial4.begin(115200);
  Serial4.setTimeout(1000); // Timeout for commands
  pinPeripheral(16, PIO_SERCOM_ALT); //Assign TX function to pin 16
  pinPeripheral(17, PIO_SERCOM_ALT); //Assign RX function to pin 17
*/

  // initialize eeprom and ethernet via W5500
  Serial.println(F("Init EEPROM..."));
  initEeprom();
  Serial.println(F("Init Ethernet..."));
  initEthernet();

  // initialize the X32
  Serial.println(F("Init X32..."));
  x32Init();

  // start mainsystem
  Serial.println(F("Init MainCtrl..."));
  SerialNina.println(F("system:init")); // initialize main-system

  #if USE_DISPLAY == 1
    // show main-menu
    displayDrawMenu();
  #endif

  #if USE_XTOUCH == 1
    Serial.println(F("Init X-TOUCH-devices..."));

    for (uint8_t i_xtouch=0; i_xtouch<XTOUCH_COUNT; i_xtouch++) {
      XCtl[i_xtouch].ip = eeprom_config.xtouchip[i_xtouch];
      XCtl_init(i_xtouch);
    }
  #endif

  #if USE_MACKIE_MCU == 1
    Serial.println(F("Init MackieMCU..."));
    MackieMCU_init();
  #endif

  // start ticker
  Serial.println(F("Init timers..."));
  ticker100ms.start();
  ticker85ms.start();
  Serial.println(F("Ready."));
}

void loop() {
  // main loop
  refreshCounter += 1;

  // handle communication
  if (firmwareUpdateMode) {
    if (refreshCounter == 12000) { // fast blinking to indicate upload-readyness
      refreshCounter = 0;
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    handleNINAUpdate();
  }else{
    if (refreshCounter == 24000) { // medium-fast blinking to indicate standby
      refreshCounter = 0;
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    // handle ethernet clients
    handleHTTPClients();
    handleCMDClients();

    // handle serial communication
    handleUSBCommunication(); // communication through Serial (USB) with connected computer
    handleX32Communication(); // communication through Serial1 with Behringer X32 MainControl
    handleNINACommunication(); // communication through Serial2 with NINA W102
    #if USE_XTOUCH == 1
      if (XCtlConnectionTimeout == 0) {
        for (uint8_t i_xtouch=0; i_xtouch<XTOUCH_COUNT; i_xtouch++) {
          handleXCtlMessages(i_xtouch); // communication via ethernet-jack (UDP)
        }
      }
    #endif
    #if USE_MACKIE_MCU == 1
      handleMackieMCUCommunication(); // communication via Serial/MIDI
    #endif

    ticker100ms.update();
    ticker85ms.update();
  }
}
