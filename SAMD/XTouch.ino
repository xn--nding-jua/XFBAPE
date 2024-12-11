#if USE_XTOUCH == 1
  EthernetUDP XCtlUdp;

  uint8_t XCtl_Probe[8] = {0xF0, 0x00, 0x20, 0x32, 0x58, 0x54, 0x00, 0xF7};
  uint8_t XCtl_ProbeResponse[8] = {0xF0, 0x00, 0x20, 0x32, 0x58, 0x54, 0x01, 0xF7};
  uint8_t XCtl_ProbeB[18] = {0xF0, 0x00, 0x00, 0x66, 0x58, 0x01, 0x30, 0x31, 0x35, 0x36, 0x34, 0x30, 0x36, 0x36, 0x37, 0x34, 0x30, 0xF7};
  uint8_t XCtl_ProbeC[18] = {0xF0, 0x00, 0x00, 0x66, 0x58, 0x01, 0x30, 0x31, 0x35, 0x36, 0x34, 0x30, 0x36, 0x33, 0x44, 0x35, 0x36, 0xF7};
  uint8_t XCtl_IdlePacket[7] = {0xF0, 0x00, 0x00, 0x66, 0x14, 0x00, 0xF7};

  struct sXctlScribblePad{
    String topText = "TopText";
    String botText = "BotText";
    uint8_t color = 7; // 0=BLACK, 1=RED, 2=GREEN, 3=YELLOW, 4=BLUE, 5=PINK, 6=CYAN, 7=WHITE
    bool inverted = false;
  };

  struct sXCtlChannel{
    bool channelNeedsUpdate = true;
    uint16_t faderPosition = 0; // 0...16383
    uint16_t faderPositionHW = 0; // 0...16383
    uint8_t meterLevel = 0; // 0...8
    uint8_t dialLevel = 0; // 0..255 ->0..12

    uint8_t rec = 0; // 0=OFF, 1=ON, 2=FLASHING
    uint8_t solo = 0; // 0=OFF, 1=ON, 2=FLASHING
    uint8_t mute = 0; // 0=OFF, 1=ON, 2=FLASHING
    uint8_t select = 0; // 0=OFF, 1=ON, 2=FLASHING
  };

  struct sXCtl{
    IPAddress ip;
    uint8_t channelOffset = 0;
    sXctlScribblePad scribblePad[8];
    sXCtlChannel channel[8];
    bool faderNeedsUpdate = true;
    uint16_t masterFader = 0;
    uint16_t masterFaderHW = 0;
    uint8_t jogDialValue = 0;
    char segmentDisplay[12];
    uint8_t buttonLightOn[103];
  }XCtl;

  uint8_t XCtl_getSegmentBitmap(char c) {
    switch (c) {
      case '1': return 0x06; break;
      case '2': return 0x5b; break;
      case '3': return 0x4f; break;
      case '4': return 0x66; break;
      case '5': return 0x6d; break;
      case '6': return 0x7d; break;
      case '7': return 0x07; break;
      case '8': return 0x7f; break;
      case '9': return 0x6f; break;
      case '0': return 0x3f; break;
      case '-': return 0x40; break;
      case 'a': return 0x77; break;
    }
  }

  void XCtl_sendUdpPacket(const uint8_t *buffer, uint16_t size) {
    XCtlUdp.begin(10111);
    XCtlUdp.beginPacket(XCtl.ip, 10111);
    XCtlUdp.write(buffer, size);
    XCtlUdp.endPacket();
  }

  void XCtl_sendWatchDogMessage() {
    XCtl_sendUdpPacket(XCtl_IdlePacket, 7);
  }

  void XCtl_sendGeneralData() {
    uint8_t XCtl_TxMessage[300]; // we are using at least 78 bytes. The other bytes are for button-updates depending on button state
    uint8_t buttonCounter = 0;


    // handle button-light-counter
    for (uint8_t i=0; i<103; i++) {
      if ((XCtl.buttonLightOn[i] < 254) && (XCtl.buttonLightOn[i] > 1)) {
        XCtl.buttonLightOn[i] -= 1;
      }
    }


    // update scribble-strips
    XCtl_TxMessage[0]=0xf0; // begin of message
    XCtl_TxMessage[1]=0x00;
    XCtl_TxMessage[2]=0x00;
    XCtl_TxMessage[3]=0x66;
    XCtl_TxMessage[4]=0x58;
    XCtl_TxMessage[21] = 0xf7; // end of message
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      XCtl_TxMessage[5]=0x20 + i_ch;

      // prepare color
      if (XCtl.scribblePad[i_ch].inverted) {
        XCtl_TxMessage[6] = 0x40 + XCtl.scribblePad[i_ch].color;
      }else{
        XCtl_TxMessage[6] = XCtl.scribblePad[i_ch].color;
      }
      
      // prepare text
      for (uint8_t i=0; i<7; i++) {
        XCtl_TxMessage[7+i] = XCtl.scribblePad[i_ch].topText[i];
        XCtl_TxMessage[14+i] = XCtl.scribblePad[i_ch].botText[i];
      }
      // send data
      XCtl_sendUdpPacket(XCtl_TxMessage, 22); // send 22 bytes (Ctl_TxMessage[0..21]) to port 10111
    }

    // update all buttons
        // Sets the state of a button light (OFF, FLASHING, ON)
        // 0 to 7 - Rec buttons
        // 8 to 15 - Solo buttons
        // 16 to 23 - Mute buttons
        // 24 to 31 - Select buttons
        // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
        // 46 to 47 - Fader bank left / right
        // 48 to 49 - Channel left / right
        // 50 Flip
        // 51 Global view
        // 52 Display NAME/VALUE
        // 53 Button SMPTE/BEATS
        // 54 to 61 - Function buttons F1 to F8
        // 62 to 69 - Buttons under 7-seg displays
        // 70 to 73 - Modify buttons (shift, option, control, alt)
        // 74 to 79 - Automation buttons (read, write, trim, touch, latch, group)
        // 80 to 83 - Utility buttons (save, undo, cacel, enter)
        // 84 to 90 - Transport buttons (marker, nudge, cycle, drop, replace, click, solo)
        // 91 to 95 - Playback control (rewind, fast-forward, stop, play, record)
        // 96 to 100 - Cursor keys (up, down, left, right, middle)
        // 101 Scrub
        // 113 Smpte
        // 114 Beats
        // 115 Solo - on 7-seg display

    XCtl_TxMessage[0] = 0x90;
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      XCtl_TxMessage[1+i_ch*2] = i_ch; // rec buttons 0...7
      XCtl_TxMessage[2+i_ch*2] = XCtl.channel[i_ch].rec;

      XCtl_TxMessage[8*2+1+i_ch*2] = 8+i_ch; // solo buttons 8...15
      XCtl_TxMessage[8*2+2+i_ch*2] = XCtl.channel[i_ch].solo;

      XCtl_TxMessage[16*2+1+i_ch*2] = 16+i_ch; // mute buttons 16..23
      XCtl_TxMessage[16*2+2+i_ch*2] = XCtl.channel[i_ch].mute;

      XCtl_TxMessage[24*2+1+i_ch*2] = 24+i_ch; // select buttons 24..31
      XCtl_TxMessage[24*2+2+i_ch*2] = XCtl.channel[i_ch].select;
    }

    // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
    XCtl_TxMessage[67] = 40;
    XCtl_TxMessage[68] = 0;
    XCtl_TxMessage[69] = 42;
    XCtl_TxMessage[70] = 0;
    XCtl_TxMessage[71] = 44;
    XCtl_TxMessage[72] = 0;
    XCtl_TxMessage[73] = 41;
    XCtl_TxMessage[74] = 0;
    XCtl_TxMessage[75] = 43;
    XCtl_TxMessage[76] = 0;
    XCtl_TxMessage[77] = 45;
    XCtl_TxMessage[78] = 2;

    buttonCounter = 0;
    for (uint8_t i=0; i<103; i++) {
      if (XCtl.buttonLightOn[i] == 255) {
        // turn button on
        buttonCounter += 1;
        XCtl.buttonLightOn[i] = 0; // disable this channel
        XCtl_TxMessage[79 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[80 + (buttonCounter-1)*2] = 2;
      }else if (XCtl.buttonLightOn[i] == 254) {
        // button with auto-turnOff
        buttonCounter += 1;
        XCtl.buttonLightOn[i] = 4; // preload timer-value to 4 (=400ms)
        XCtl_TxMessage[79 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[80 + (buttonCounter-1)*2] = 2;
      }else if (XCtl.buttonLightOn[i] == 1) {
        // turn button off
        buttonCounter += 1;
        XCtl.buttonLightOn[i] = 0; // disable this channel
        XCtl_TxMessage[79 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[80 + (buttonCounter-1)*2] = 0;
      }
    }
    XCtl_sendUdpPacket(XCtl_TxMessage, 78+(buttonCounter*2)); //send 78+(buttonCounter*2) bytes (Ctl_TxMessage[0..78+(buttonCounter*2)]) to port 10111

    // Update DialLevels around PanKnob
    XCtl_TxMessage[0] = 0xB0;
    uint16_t dialLevelRaw = 0;
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      dialLevelRaw = 0;
      for (uint16_t i=0; i<=(uint16_t)(XCtl.channel[i_ch].dialLevel/21.25f); i++){
        dialLevelRaw += (1 << i);
      }
      XCtl_TxMessage[1 + i_ch*4] = 0x30 + i_ch;
      XCtl_TxMessage[2 + i_ch*4] = dialLevelRaw & 0x7F;
      XCtl_TxMessage[3 + i_ch*4] = 0x38 + i_ch;
      XCtl_TxMessage[4 + i_ch*4] = (dialLevelRaw >> 7) & 0x7F;
    }
    XCtl_sendUdpPacket(XCtl_TxMessage, 34); //send 34 bytes (Ctl_TxMessage[0..33]) to port 10111

    // Set 7-Segment-Displays
    XCtl_TxMessage[0] = 0xB0;
    for (uint8_t i=0; i<12; i++) {
      XCtl_TxMessage[1 + i*2] = i + 0x60;

      // 0x30 to 0x37 - Left hand sides of knobs
      // 0x38 to 0x3F - Right hand sides of knobs
      // 0x60 - Left hand assignment digit
      // 0x61 - Right hand assignment digit
      // 0x62-0x64 - Bars digits
      // 0x65-0x66 - Beats digits
      // 0x67-0x68 - Sub division digits
      // 0x69-0x6B - Ticks digits
      // 0x70-0x7B - same as above but with . also lit
      // Value: 7-bit bitmap of segments to illuminate
      XCtl_TxMessage[2 + i*2] = XCtl_getSegmentBitmap(XCtl.segmentDisplay[i]);
    }
    XCtl_sendUdpPacket(XCtl_TxMessage, 25); //send 25 bytes (Ctl_TxMessage[0..24]) to port 10111
  }

  void XCtl_sendFaderData() {
    uint8_t XCtl_TxMessage[9]; // we are using at least 78 bytes. The other bytes are for button-updates depending on button state

    // update channel-faders
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      if (XCtl.channel[i_ch].channelNeedsUpdate) {
        XCtl.channel[i_ch].channelNeedsUpdate = false;
        XCtl_TxMessage[0] = 0xE0 + i_ch;
        XCtl_TxMessage[1] = XCtl.channel[i_ch].faderPosition & 0x7F; // MIDI-Values between 0 and 127
        XCtl_TxMessage[2] = (XCtl.channel[i_ch].faderPosition >> 7) & 0x7F;
        XCtl_sendUdpPacket(XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
      }
    }

    // update meter-levels
    XCtl_TxMessage[0] = 0xD0;
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      XCtl_TxMessage[1 + i_ch] = (i_ch << 4) + XCtl.channel[i_ch].meterLevel;
    }
    XCtl_sendUdpPacket(XCtl_TxMessage, 9); //send 9 bytes (Ctl_TxMessage[0..8]) to port 10111
    
    // update masterfader
    if (XCtl.faderNeedsUpdate) {
      XCtl.faderNeedsUpdate = false;
      XCtl_TxMessage[0] = 0xE8; // E8=Masterfader
      XCtl_TxMessage[1] = XCtl.masterFader & 0x7F; // MIDI-Values between 0 and 127
      XCtl_TxMessage[2] = (XCtl.masterFader >> 7) & 0x7F;
      XCtl_sendUdpPacket(XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
    }
  }

  void handleXCtlMessages() {
    // message start: F0
    // message terminator: F7

    // every 2 seconds XTouch sends 00 20 32 58 54 00
    // we have to send 00 00 66 14 00

    uint8_t rxData[18]; //buffer to hold incoming packet,
    int packetSize = XCtlUdp.parsePacket();
    uint8_t fader = 0;
    int16_t value = 0;

    if (packetSize) {
      uint8_t len = XCtlUdp.read(rxData, 18);

      // check if packet is valid
      if ((len>0) && (rxData[0] == 0xF0) && (rxData[len-1] == 0xF7)) {
        // Message has expected start- and end-byte
        // check the received data

        if ((len == 8) && (memcmp(rxData, XCtl_Probe, 8))) {
          // we received a Probe-Message
          XCtl_sendUdpPacket(XCtl_ProbeResponse, 8);
        }else if ((len == 18) && (memcmp(rxData, XCtl_ProbeB, 18))) {
          // Ignore ProbeB MSG
        }else if ((len == 18) && (memcmp(rxData, XCtl_ProbeC, 18))) {
          // Ignore ProbeC MSG
        }else{
          // we received a unknown message between F0 and F7
        }
      }else{
        // we received values to parse

        // check for correct length
        if (len == 3) {
          // check for touched fader
          if ((rxData[0] == 0x90) && (rxData[1] >= 0x68) && (rxData[1] <= 0x70)) {
            // fader = rxData[1] - 0x68;
            // touched = rxData[2] != 0;
          }

          // read faderlevel
          if ((rxData[0] & 0xF0) == 0xE0) {
            // fader = rxData[0] & 0x0F;
            // value = rxData[1] + (rxData[2] << 7);

            if ((rxData[0] & 0x0F) <= 7) {
              // values of faders 1 to 8
              fader = rxData[0] & 0x0F;
              value = rxData[1] + (rxData[2] << 7); // 0...16383

              XCtl.channel[fader].faderPositionHW = value;
            }else if ((rxData[0] & 0x0F) == 8){
              // masterfader = fader 9
              value = rxData[1] + (rxData[2] << 7); // 0...16383
              XCtl.masterFaderHW = value;
            }
          }

          // read rotation
          if (rxData[0] == 0xB0) {
            if ((rxData[2] & 0x40) == 0x40) {
              value = (0 - (rxData[2] & 0x0F));
            }else{
              value = (rxData[2] & 0x0F);
            }

            if ((rxData[1] >= 16) && (rxData[1] <= 23)) {
              // channelDials
              if (value>0) {
                // turn right

                // TODO: do something with this information
              }else{
                // turn left

                // TODO: do something with this information
              }
            }else if (rxData[1] == 60) {
              // large jog-dial
              if (value>0) {
                // turn right

                if ((int16_t)XCtl.jogDialValue + (int16_t)value > 255) {
                  XCtl.jogDialValue = 255;
                }else{
                  XCtl.jogDialValue += value;
                }
              }else{
                // turn left

                if ((int16_t)XCtl.jogDialValue - (int16_t)value < 0) {
                  XCtl.jogDialValue = 0;
                }else{
                  XCtl.jogDialValue -= value;
                }
              }
            }
          }

          // read button
          if (rxData[0] == 0x90) {
            uint8_t button = rxData[1];
            uint8_t buttonState = rxData[2] != 0;

              // 0 to 7 - Rec buttons
              // 8 to 15 - Solo buttons
              // 16 to 23 - Mute buttons
              // 24 to 31 - Select buttons
              // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
              // 46 to 47 - Fader bank left / right
              // 48 to 49 - Channel left / right
              // 50 Flip
              // 51 Global view
              // 52 Display Name/Value
              // 53 Button SMTPE/BEATS 
              // 54 to 61 - Function buttons F1 to F8
              // 62 to 69 - Buttons under 7-seg displays
              // 70 to 73 - Modify buttons (shift, option, control, alt)
              // 74 to 79 - Automation buttons (read, write, trim, touch, latch, group)
              // 80 to 83 - Utility buttons (save, undo, cacel, enter)
              // 84 to 90 - Transport buttons (marker, nudge, cycle, drop, replace, click, solo)
              // 91 to 95 - Playback control (rewind, fast-forward, stop, play, record)
              // 96 to 100 - Cursor keys (up, down, left, right, middle)
              // 101 Scrub
              // 113 Smpte
              // 114 Beats
              // 115 Solo - on 7-seg display

            // turn on LED for the common buttons on button-press
            if (((button >= 32) && (button<102)) && !((button>=40) && (button<=45))) { // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
              if (buttonState) {
                XCtl.buttonLightOn[button] = 255;
              }else{
                XCtl.buttonLightOn[button] = 1;
              }
            }


            // TODO: do something with this information

            if ((button >= 0) && (button<=7)) {
              // rec-buttons
              if (buttonState) {
                // pressed
              }else{
                // released
              }
            }

            if ((button >= 8) && (button<=15)) {
              // solo-buttons
              if (buttonState) {
                // pressed
              }else{
                // released
              }
            }

            if ((button >= 16) && (button<=23)) {
              // mute-buttons
              if (buttonState) {
                // pressed
              }else{
                // released
              }
            }

            if ((button >= 24) && (button<=31)) {
              // select-buttons
              if (buttonState) {
                // pressed
              }else{
                // released
              }
            }

            if ((button == 46) && (buttonState)) {
              // fader bank left
              if (XCtl.channelOffset >= 8) {
                XCtl.channelOffset -= 8;
              }else{
                XCtl.channelOffset = 0;
              }
            }

            if ((button == 47) && (buttonState)) {
              // fader bank right
              if (XCtl.channelOffset <= 16) {
                XCtl.channelOffset += 8;
              }else{
                XCtl.channelOffset = 24;
              }
            }

            if ((button == 48) && (buttonState)) {
              // channel left
              if (XCtl.channelOffset >= 1) {
                XCtl.channelOffset -= 1;
              }else{
                XCtl.channelOffset = 0;
              }
            }

            if ((button == 49) && (buttonState)) {
              // channel right
              if (XCtl.channelOffset <= 23) {
                XCtl.channelOffset += 1;
              }else{
                XCtl.channelOffset = 24;
              }
            }
          }
        }
      }
    }
  }

  void XCtlPrepareData() {
    // Update Segment Display
    String playtime = secondsToHMS_B(playerinfo.time); // 00:00:00
    XCtl.segmentDisplay[0] = 49;
    XCtl.segmentDisplay[1] = ' ';
    XCtl.segmentDisplay[2] = ' ';
    XCtl.segmentDisplay[3] = playtime[0]; // h
    XCtl.segmentDisplay[4] = playtime[1]; // h
    XCtl.segmentDisplay[5] = playtime[3]; // min
    XCtl.segmentDisplay[6] = playtime[4]; // min
    XCtl.segmentDisplay[7] = playtime[6]; // s
    XCtl.segmentDisplay[8] = playtime[7]; // s

    String valueString = String(XCtl.jogDialValue);
    if (XCtl.jogDialValue < 10) {
      XCtl.segmentDisplay[9] = 0;
      XCtl.segmentDisplay[10] = 0;
      XCtl.segmentDisplay[11] = valueString[0];
    }else if (XCtl.jogDialValue < 100) {
      XCtl.segmentDisplay[9] = 0;
      XCtl.segmentDisplay[10] = valueString[1];
      XCtl.segmentDisplay[11] = valueString[0];
    }else{
      XCtl.segmentDisplay[9] = valueString[2];
      XCtl.segmentDisplay[10] = valueString[1];
      XCtl.segmentDisplay[11] = valueString[0];
    }

    // update Masterfader
    XCtl.faderNeedsUpdate = false; // TODO: check if we have to update
    XCtl.masterFader = 0; // TODO: set new value. convert playerinfo.volumeMain from dBfs to 0...16388

    // update channel dial-Levels
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      XCtl.channel[i_ch].dialLevel = 0; // 0..255
    }

    // update all other faders and buttons
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      XCtl.channel[i_ch].channelNeedsUpdate = false;
      XCtl.channel[i_ch].faderPosition = 0; // 0..16383
      XCtl.channel[i_ch].meterLevel = 0; // 0..8
      XCtl.scribblePad[i_ch].topText = "Ch" + String(i_ch) + "      ";
      XCtl.scribblePad[i_ch].botText = String(XCtl.channel[i_ch].faderPosition/64) + "      ";
      XCtl.scribblePad[i_ch].color = i_ch - 1; // 0=BLACK, 1=RED, 2=GREEN, 3=YELLOW, 4=BLUE, 5=PINK, 6=CYAN, 7=WHITE
      XCtl.scribblePad[i_ch].inverted = false;
    }
  }
#endif