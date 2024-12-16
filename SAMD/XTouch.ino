#if USE_XTOUCH == 1
  uint8_t XCtl_getSegmentBitmap(char c) {
    switch (c) {
      case ' ': return 0x00; break;
      case '0': return 0x3f; break;
      case '1': return 0x06; break;
      case '2': return 0x5b; break;
      case '3': return 0x4f; break;
      case '4': return 0x66; break;
      case '5': return 0x6d; break;
      case '6': return 0x7d; break;
      case '7': return 0x07; break;
      case '8': return 0x7f; break;
      case '9': return 0x6f; break;
      case '-': return 0x40; break;
      default: return 0x00; break;
    }
  }

  void XCtl_init(uint8_t i_xtouch) {
    XCtlUdp[i_xtouch].begin(10111);
  }

  void XCtl_sendUdpPacket(uint8_t i_xtouch, const uint8_t *buffer, uint16_t size) {
    XCtlUdp[i_xtouch].beginPacket(XCtl[i_xtouch].ip, 10111);
    XCtlUdp[i_xtouch].write(buffer, size);
    XCtlUdp[i_xtouch].endPacket();
  }

  void XCtl_sendWatchDogMessage(uint8_t i_xtouch) {
    XCtl_sendUdpPacket(i_xtouch, XCtl_IdlePacket, 7);
  }

  void XCtl_sendGeneralData(uint8_t i_xtouch) {
    uint8_t XCtl_TxMessage[300]; // we are using at least 78 bytes. The other bytes are for button-updates depending on button state
    uint16_t buttonCounter = 0;


    // handle button-light-counter
    for (uint8_t i=0; i<103; i++) {
      if ((XCtl[i_xtouch].buttonLightOn[i] < 254) && (XCtl[i_xtouch].buttonLightOn[i] > 1)) {
        XCtl[i_xtouch].buttonLightOn[i] -= 1;
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
      if (XCtl[i_xtouch].scribblePad[i_ch + XCtl[i_xtouch].channelOffset].inverted) {
        XCtl_TxMessage[6] = 0x40 + XCtl[i_xtouch].scribblePad[i_ch + XCtl[i_xtouch].channelOffset].color;
      }else{
        XCtl_TxMessage[6] = XCtl[i_xtouch].scribblePad[i_ch + XCtl[i_xtouch].channelOffset].color;
      }
      
      // prepare text
      for (uint8_t i=0; i<7; i++) {
        XCtl_TxMessage[7+i] = XCtl[i_xtouch].scribblePad[i_ch + XCtl[i_xtouch].channelOffset].topText[i];
        XCtl_TxMessage[14+i] = XCtl[i_xtouch].scribblePad[i_ch + XCtl[i_xtouch].channelOffset].botText[i];
      }
      // send data
      XCtl_sendUdpPacket(i_xtouch, XCtl_TxMessage, 22); // send 22 bytes (Ctl_TxMessage[0..21]) to port 10111
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
      XCtl_TxMessage[2+i_ch*2] = XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].rec;

      XCtl_TxMessage[8*2+1+i_ch*2] = 8+i_ch; // solo buttons 8...15
      XCtl_TxMessage[8*2+2+i_ch*2] = XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].solo;

      XCtl_TxMessage[16*2+1+i_ch*2] = 16+i_ch; // mute buttons 16..23
      XCtl_TxMessage[16*2+2+i_ch*2] = XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].mute;

      XCtl_TxMessage[24*2+1+i_ch*2] = 24+i_ch; // select buttons 24..31
      XCtl_TxMessage[24*2+2+i_ch*2] = XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].select;
    }

    // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
    XCtl_TxMessage[65] = 40;
    XCtl_TxMessage[66] = 2;
    XCtl_TxMessage[67] = 42;
    XCtl_TxMessage[68] = 0;
    XCtl_TxMessage[69] = 44;
    XCtl_TxMessage[70] = 0;
    XCtl_TxMessage[71] = 41;
    XCtl_TxMessage[72] = 0;
    XCtl_TxMessage[73] = 43;
    XCtl_TxMessage[74] = 0;
    XCtl_TxMessage[75] = 45;
    XCtl_TxMessage[76] = 0;

    buttonCounter = 0;
    for (uint16_t i=0; i<103; i++) {
      if (XCtl[i_xtouch].buttonLightOn[i] == 255) {
        // turn button on
        buttonCounter += 1;
        XCtl[i_xtouch].buttonLightOn[i] = 0; // disable this channel
        XCtl_TxMessage[77 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[78 + (buttonCounter-1)*2] = 2; // turnOn Button
      }else if (XCtl[i_xtouch].buttonLightOn[i] == 254) {
        // button with auto-turnOff
        buttonCounter += 1;
        XCtl[i_xtouch].buttonLightOn[i] = 40; // preload timer-value to 4 (=400ms)
        XCtl_TxMessage[77 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[78 + (buttonCounter-1)*2] = 2; // turnOn Button
      }else if (XCtl[i_xtouch].buttonLightOn[i] == 1) {
        // turn button off
        buttonCounter += 1;
        XCtl[i_xtouch].buttonLightOn[i] = 0; // disable this channel
        XCtl_TxMessage[77 + (buttonCounter-1)*2] = i;
        XCtl_TxMessage[78 + (buttonCounter-1)*2] = 0; // turnOff Button
      }
    }
    XCtl_sendUdpPacket(i_xtouch, XCtl_TxMessage, 77+(buttonCounter*2)); //send 77+(buttonCounter*2) bytes (Ctl_TxMessage[0..77+(buttonCounter*2)]) to port 10111

    // Update DialLevels around PanKnob
    XCtl_TxMessage[0] = 0xB0;
    uint16_t dialLevelRaw = 0;
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
  /*
      // render as volume-level = growing bar-level
      dialLevelRaw = 0;
      for (uint16_t i=0; i<=(uint16_t)(XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].dialLevel/21.25f); i++){
        dialLevelRaw += (1 << i);
      }
  */
      // render as pan-level = single-mark
      dialLevelRaw = (uint8_t)(XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].dialLevel/21.25f);
      dialLevelRaw = (1 << dialLevelRaw);

      XCtl_TxMessage[1 + i_ch*4] = 0x30 + i_ch;
      XCtl_TxMessage[2 + i_ch*4] = dialLevelRaw & 0x7F;
      XCtl_TxMessage[3 + i_ch*4] = 0x38 + i_ch;
      XCtl_TxMessage[4 + i_ch*4] = (dialLevelRaw >> 7) & 0x7F;
    }
    XCtl_sendUdpPacket(i_xtouch, XCtl_TxMessage, 34); //send 34 bytes (Ctl_TxMessage[0..33]) to port 10111

    // Set 7-Segment-Displays
    XCtl_TxMessage[0] = 0xB0;
    for (uint8_t i=0; i<12; i++) {
      XCtl_TxMessage[1 + i*2] = 0x60 + i;

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
      XCtl_TxMessage[2 + i*2] = XCtl_getSegmentBitmap(XCtl[i_xtouch].segmentDisplay[i]);
    }
    XCtl_sendUdpPacket(i_xtouch, XCtl_TxMessage, 25); //send 25 bytes (Ctl_TxMessage[0..24]) to port 10111
  }

  void XCtl_sendFaderData(uint8_t i_xtouch) {
    uint8_t XCtl_TxMessage[9]; // we are using at least 78 bytes. The other bytes are for button-updates depending on button state

    // update channel-faders
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      if (XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].faderNeedsUpdate || XCtl[i_xtouch].forceUpdate) {
        XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].faderNeedsUpdate = false;
        XCtl_TxMessage[0] = 0xE0 + i_ch;
        XCtl_TxMessage[1] = XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].faderPosition & 0x7F; // MIDI-Values between 0 and 127
        XCtl_TxMessage[2] = (XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].faderPosition >> 7) & 0x7F;
        XCtl_sendUdpPacket(i_xtouch, XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
      }
    }
    XCtl[i_xtouch].forceUpdate = false;

    // update meter-levels
    XCtl_TxMessage[0] = 0xD0;
    for (uint8_t i_ch=0; i_ch<8; i_ch++) {
      XCtl_TxMessage[1 + i_ch] = (i_ch << 4) + XCtl[i_xtouch].channel[i_ch + XCtl[i_xtouch].channelOffset].meterLevel;
    }
    XCtl_sendUdpPacket(i_xtouch, XCtl_TxMessage, 9); //send 9 bytes (Ctl_TxMessage[0..8]) to port 10111
    
    // update masterfader
    if (XCtl[i_xtouch].channel[32].faderNeedsUpdate) {
      XCtl[i_xtouch].channel[32].faderNeedsUpdate = false;
      XCtl_TxMessage[0] = 0xE8; // E8=Masterfader
      XCtl_TxMessage[1] = XCtl[i_xtouch].channel[32].faderPosition & 0x7F; // MIDI-Values between 0 and 127
      XCtl_TxMessage[2] = (XCtl[i_xtouch].channel[32].faderPosition >> 7) & 0x7F;
      XCtl_sendUdpPacket(i_xtouch, XCtl_TxMessage, 3); //send 3 bytes (Ctl_TxMessage[0..2]) to port 10111
    }
  }

  void handleXCtlMessages(uint8_t i_xtouch) {
    // message start: F0
    // message terminator: F7

    // every 2 seconds XTouch sends 00 20 32 58 54 00
    // we have to send 00 00 66 14 00

    if (XCtlUdp[i_xtouch].parsePacket() > 0) {
      uint8_t rxData[18]; //buffer to hold incoming packet,
      uint8_t len = XCtlUdp[i_xtouch].read(rxData, 18);
      uint8_t channel = 0;
      int16_t value = 0;

      // check if packet is valid
      if ((len>0) && (rxData[0] == 0xF0) && (rxData[len-1] == 0xF7)) {
        // Message has expected start- and end-byte
        // check the received data

        if ((len == 8) && (memcmp(rxData, XCtl_Probe, 8))) {
          // we received a Probe-Message
          XCtl_sendUdpPacket(i_xtouch, XCtl_ProbeResponse, 8);
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
            channel = (rxData[1] - 0x68) + XCtl[i_xtouch].channelOffset;
            XCtl[i_xtouch].channel[channel].faderTouched = rxData[2] != 0;
          }

          // read faderlevel
          if ((rxData[0] & 0xF0) == 0xE0) {
            // fader = rxData[0] & 0x0F;
            // value = rxData[1] + (rxData[2] << 7);

            if ((rxData[0] & 0x0F) <= 7) {
              // values of faders 1 to 8
              channel = (rxData[0] & 0x0F) + XCtl[i_xtouch].channelOffset;
              value = rxData[1] + (rxData[2] << 7); // 0...16383

              XCtl[i_xtouch].channel[channel].faderPositionHW = value;
              if (XCtl[i_xtouch].channel[channel].faderTouched) {
                // send new channel-volume
                float newVolume = ((value/16383.0f) * 54.0f) - 48.0f;
                playerinfo.volumeCh[channel] = newVolume;
                //Serial2.println("mixer:volume:ch" + String(channel + 1) + "@" + String(newVolume, 2));
              }
            }else if ((rxData[0] & 0x0F) == 8){
              // masterfader = fader 9
              value = rxData[1] + (rxData[2] << 7); // 0...16383
              
              XCtl[i_xtouch].channel[32].faderPositionHW = value;
              if (XCtl[i_xtouch].channel[32].faderTouched) {
                // send new main-volume
                float newVolume = ((value/16383.0f) * 54.0f) - 48.0f;
                playerinfo.volumeCh[channel] = newVolume;
                //Serial2.println("mixer:volume:main@" + String(newVolume, 2));
              }
            }
          }

          // read rotation
          if (rxData[0] == 0xB0) {
            if ((rxData[2] & 0x40) == 0x40) {
              value = (0 - (int16_t)(rxData[2] & 0x0F));
            }else{
              value = (rxData[2] & 0x0F);
            }

            if ((rxData[1] >= 16) && (rxData[1] <= 23)) {
              // channelDials
              if (value>0) {
                // turn right
                channel = rxData[1] - 16 + XCtl[i_xtouch].channelOffset;
                if (XCtl[i_xtouch].channel[channel].dialLevel + value > 255) {
                  XCtl[i_xtouch].channel[channel].dialLevel = 255;
                }else{
                  XCtl[i_xtouch].channel[channel].dialLevel += value;
                }
                // set balance
                //Serial2.println("mixer:balance:ch" + String(channel + 1) + "@" + String(XCtl[i_xtouch].channel[channel].dialLevel / 2.55f));
              }else{
                // turn left
                channel = rxData[1] - 16 + XCtl[i_xtouch].channelOffset;
                if ((int16_t)XCtl[i_xtouch].channel[channel].dialLevel + value < 0) {
                  XCtl[i_xtouch].channel[channel].dialLevel = 0;
                }else{
                  XCtl[i_xtouch].channel[channel].dialLevel += value;
                }
                // set balance
                //Serial2.println("mixer:balance:ch" + String(channel + 1) + "@" + String(XCtl[i_xtouch].channel[channel].dialLevel / 2.55f));
              }
            }else if (rxData[1] == 60) {
              // large jog-dial
              if (value>0) {
                // turn right

                if (XCtl[i_xtouch].jogDialValue + value > 255) {
                  XCtl[i_xtouch].jogDialValue = 255;
                }else{
                  XCtl[i_xtouch].jogDialValue += value;
                }
              }else{
                // turn left

                if ((int16_t)XCtl[i_xtouch].jogDialValue + value < 0) {
                  XCtl[i_xtouch].jogDialValue = 0;
                }else{
                  XCtl[i_xtouch].jogDialValue += value;
                }
              }
            }
          }

          // read button
          if (rxData[0] == 0x90) {
            uint8_t button = rxData[1];
            uint8_t buttonState = rxData[2] > 0;

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

  /*
            // turn on LED for the common buttons on button-press
            if (buttonState && ((button >= 32) && (button < 102))) {
              XCtl[i_xtouch].buttonLightOn[button] = 254; // 255=TurnOn, 254=enable with auto-TurnOff, 1=TurnOff
            }
  */
            // turn on LED for the common buttons on button-press
            if (((button >= 32) && (button<102)) && !((button>=40) && (button<=45))) { // 40 to 45 - encoder assign buttons (track, send, pan,  plugin, eq, inst)
              if (buttonState) {
                XCtl[i_xtouch].buttonLightOn[button] = 255; // on
              }else{
                XCtl[i_xtouch].buttonLightOn[button] = 1; // one step before off
              }
            }

            // individual buttons
            if ((button >= 0) && (button<=7)) {
              // rec-buttons
              if (buttonState) {
                // pressed
                if (XCtl[i_xtouch].channel[button + XCtl[i_xtouch].channelOffset].rec == 0) {
                  XCtl[i_xtouch].channel[button + XCtl[i_xtouch].channelOffset].rec = 2;
                }else{
                  XCtl[i_xtouch].channel[button + XCtl[i_xtouch].channelOffset].rec = 0;
                }
              }else{
                // released
              }
            }

            if ((button >= 8) && (button<=15)) {
              // solo-buttons
              if (buttonState) {
                // pressed
                if (XCtl[i_xtouch].channel[(button-8) + XCtl[i_xtouch].channelOffset].solo == 0) {
                  XCtl[i_xtouch].channel[(button-8) + XCtl[i_xtouch].channelOffset].solo = 2;
                }else{
                  XCtl[i_xtouch].channel[(button-8) + XCtl[i_xtouch].channelOffset].solo = 0;
                }
              }else{
                // released
              }
            }

            if ((button >= 16) && (button<=23)) {
              // mute-buttons
              if (buttonState) {
                // pressed
                if (XCtl[i_xtouch].channel[(button-16) + XCtl[i_xtouch].channelOffset].mute == 0) {
                  XCtl[i_xtouch].channel[(button-16) + XCtl[i_xtouch].channelOffset].mute = 2;
                }else{
                  XCtl[i_xtouch].channel[(button-16) + XCtl[i_xtouch].channelOffset].mute = 0;
                }
              }else{
                // released
              }
            }

            if ((button >= 24) && (button<=31)) {
              // select-buttons
              if (buttonState) {
                // pressed
                if (XCtl[i_xtouch].channel[(button-24) + XCtl[i_xtouch].channelOffset].select == 0) {
                  // disable all select-channels
                  for (uint8_t i=0; i<32; i++) {
                    XCtl[i_xtouch].channel[i].select = 0;
                  }

                  XCtl[i_xtouch].channel[(button-24) + XCtl[i_xtouch].channelOffset].select = 2;
                }else{
                  XCtl[i_xtouch].channel[(button-24) + XCtl[i_xtouch].channelOffset].select = 0;
                }
              }else{
                // released
              }
            }

            if ((button >= 32) && (button<=39)) {
              // encoder-buttons
              if (buttonState) {
                // pressed
                // reset panning to 50%
                XCtl[i_xtouch].channel[(button-32) + XCtl[i_xtouch].channelOffset].dialLevel = 128;
              }else{
                // released
              }
            }

            if ((button == 46) && (buttonState)) {
              // fader bank left
              if (XCtl[i_xtouch].channelOffset >= 8) {
                XCtl[i_xtouch].channelOffset -= 8;
              }else{
                XCtl[i_xtouch].channelOffset = 0;
              }
              XCtl[i_xtouch].forceUpdate = true;
            }

            if ((button == 47) && (buttonState)) {
              // fader bank right
              if (XCtl[i_xtouch].channelOffset <= 16) {
                XCtl[i_xtouch].channelOffset += 8;
              }else{
                XCtl[i_xtouch].channelOffset = 24;
              }
              XCtl[i_xtouch].forceUpdate = true;
            }

            if ((button == 48) && (buttonState)) {
              // channel left
              if (XCtl[i_xtouch].channelOffset >= 1) {
                XCtl[i_xtouch].channelOffset -= 1;
              }else{
                XCtl[i_xtouch].channelOffset = 0;
              }
              XCtl[i_xtouch].forceUpdate = true;
            }

            if ((button == 49) && (buttonState)) {
              // channel right
              if (XCtl[i_xtouch].channelOffset <= 23) {
                XCtl[i_xtouch].channelOffset += 1;
              }else{
                XCtl[i_xtouch].channelOffset = 24;
              }
              XCtl[i_xtouch].forceUpdate = true;
            }

            if ((button == 52) && (buttonState)) {
              XCtl[i_xtouch].options.showValues != XCtl[i_xtouch].options.showValues;
            }
          }
        }
      }
    }
  }

  String XCtl_panString(uint8_t value) {
    if (value == 128) {
      return "<C>";
    }else if (value <128) {
      return "L" + String(50 - (value/2.55), 0);
    }else{
      return "R" + String((value/2.55) - 50, 0);
    }
  }

  void XCtl_prepareData(uint8_t i_xtouch) {
    // Update Segment Display
    String playtime = secondsToHMS_B(playerinfo.time); // 00:00:00
    XCtl[i_xtouch].segmentDisplay[0] = 49; // 49 = ASCII '1'
    XCtl[i_xtouch].segmentDisplay[1] = ' ';
    XCtl[i_xtouch].segmentDisplay[2] = ' ';
    XCtl[i_xtouch].segmentDisplay[3] = playtime[0]; // h
    XCtl[i_xtouch].segmentDisplay[4] = playtime[1]; // h
    XCtl[i_xtouch].segmentDisplay[5] = playtime[3]; // min
    XCtl[i_xtouch].segmentDisplay[6] = playtime[4]; // min
    XCtl[i_xtouch].segmentDisplay[7] = playtime[6]; // s
    XCtl[i_xtouch].segmentDisplay[8] = playtime[7]; // s

    String valueString = String(XCtl[i_xtouch].jogDialValue);
    if (XCtl[i_xtouch].jogDialValue < 10) {
      XCtl[i_xtouch].segmentDisplay[9] = 0;
      XCtl[i_xtouch].segmentDisplay[10] = 0;
      XCtl[i_xtouch].segmentDisplay[11] = valueString[0];
    }else if (XCtl[i_xtouch].jogDialValue < 100) {
      XCtl[i_xtouch].segmentDisplay[9] = 0;
      XCtl[i_xtouch].segmentDisplay[10] = valueString[1];
      XCtl[i_xtouch].segmentDisplay[11] = valueString[0];
    }else{
      XCtl[i_xtouch].segmentDisplay[9] = valueString[2];
      XCtl[i_xtouch].segmentDisplay[10] = valueString[1];
      XCtl[i_xtouch].segmentDisplay[11] = valueString[0];
    }

    // update all faders and buttons for the current channel-selection
    uint32_t newFaderValue;
    for (uint8_t i_ch=XCtl[i_xtouch].channelOffset; i_ch<(8+XCtl[i_xtouch].channelOffset); i_ch++) {
      newFaderValue = ((playerinfo.volumeCh[i_ch] + 48.0f)/54.0f) * 16383.0f;
      XCtl[i_xtouch].channel[i_ch].faderNeedsUpdate = newFaderValue != XCtl[i_xtouch].channel[i_ch].faderPosition;
      XCtl[i_xtouch].channel[i_ch].faderPosition = newFaderValue; // 0..16383

      XCtl[i_xtouch].channel[i_ch].meterLevel = random(0, 8); // not used at the moment
      XCtl[i_xtouch].scribblePad[i_ch].topText = "Ch" + String(i_ch + 1) + " " + XCtl_panString(XCtl[i_xtouch].channel[i_ch].dialLevel);
      XCtl[i_xtouch].scribblePad[i_ch].botText = String(playerinfo.volumeCh[i_ch], 2) + "dB    ";
      XCtl[i_xtouch].scribblePad[i_ch].color = 7; // 0=BLACK, 1=RED, 2=GREEN, 3=YELLOW, 4=BLUE, 5=PINK, 6=CYAN, 7=WHITE // fixed to white at the moment
      XCtl[i_xtouch].scribblePad[i_ch].inverted = false; // not used at the moment
    }

    // update Masterfader
    newFaderValue = ((playerinfo.volumeMain + 48.0f)/54.0f) * 16383.0f;
    XCtl[i_xtouch].channel[32].faderNeedsUpdate = newFaderValue != XCtl[i_xtouch].channel[32].faderPosition;
    XCtl[i_xtouch].channel[32].faderPosition = newFaderValue; // convert volumeMain from dBfs to 0...16388 but keep logarithmic scale
  }
#endif