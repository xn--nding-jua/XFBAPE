/*
  The MackieMCU protocol is used to communicate with the X32. The X32 can be set to MackieMCU-DAW-Control
  so that the right 8 faders are used to display channel information and set volumes of each channel

  Documentation of MackieProtocol:
  https://github.com/NicoG60/TouchMCU/blob/main/doc/mackie_control_protocol.md

  emagic Logic Control Users Manual Version 2.0, November 2002
  https://images.thomann.de/pics/prod/151261_manual.pdf

  ----------------------------------------------------------------------------------------------
  | MESSAGE         | StatusByte | Data1      | Data2      | Description                       |
  |                 | D7..D0     | D7..D0     | D7..D0     |                                   |
  |-----------------|------------|------------|------------|-----------------------------------|
  | Note Off        | 0b1000cccc | 0b0nnnnnnn | 0b0vvvvvvv | c: Channel n: Note v: Velocity    |
  | Note On         | 0b1001cccc | 0b0nnnnnnn | 0b0vvvvvvv | c: Channel n: Note v: Velocity    |
  | AfterTouch Key  | 0b1010cccc | 0b0nnnnnnn | 0b0vvvvvvv | c: Channel n: Note v: Velocity    |
  | ControlChange   | 0b1011cccc | 0b0nnnnnnn | 0b0vvvvvvv | c: Channel n: CC number v: Value  |
  | ProgramChange   | 0b1100cccc | 0b0ppppppp |            | c: Channel p: Program             |
  | AfterTouch Chan | 0b1101cccc | 0b0vvvvvvv |            | c: Channel v: Value               |
  | PitchBend       | 0b1110cccc | 0b0lllllll | 0b0mmmmmmm | c: Channel l: LSB m: MSB          |
  | SysEx Start     | 0b11110000 |            |            | 0xF0                              |
  | SysEx End       | 0b11110111 |            |            | 0xF7                              |
  |---------------------------------------------------------------------------------------------

*/

#if USE_MACKIE_MCU == 1
  // =========================== RECEIVING MIDI DATA ==========================
  void MackieMCU_handleNoteOn(uint8_t midiChannel, uint8_t note, uint8_t velocity) {
    if (mackieDmxMode) {
      // DMX512 MODE
      if ( (note >= 0x00) && (note <= 0x07) ) {
        // rec-button
        uint16_t channel = (uint16_t)note + MackieMCU.channelOffsetDmx;
        if (MackieMCU.channelDmx[channel].rec > 0) {
          MackieMCU.channelDmx[channel].rec = 0;
        }else{
          MackieMCU.channelDmx[channel].rec = 127;
        }
      }else if ( (note >= 0x08) && (note <= 0x0f) ) {
        // solo-button
        uint16_t channel = (uint16_t)(note - 0x08) + MackieMCU.channelOffsetDmx;
        if (MackieMCU.channelDmx[channel].solo > 0) {
          MackieMCU.channelDmx[channel].solo = 0;
        }else{
          MackieMCU.channelDmx[channel].solo = 127;
        }
      }else if ( (note >= 0x10) && (note <= 0x17) ) {
        // mute-button
        uint16_t channel = (uint16_t)(note - 0x10) + MackieMCU.channelOffsetDmx;
        if (MackieMCU.channelDmx[channel].mute > 0) {
          // turn mute off
          MackieMCU.channelDmx[channel].mute = 0;
        }else{
          // mute this channel
          MackieMCU.channelDmx[channel].mute = 127;
        }
      }else if ( (note >= 0x18) && (note <= 0x1f) ) {
        // select-button

        // disable all select-channels
        for (uint8_t i=0; i<32; i++) {
          MackieMCU.channelDmx[i].select = 0;
        }
        uint16_t channel = (uint16_t)(note - 0x18) + MackieMCU.channelOffsetDmx;
        MackieMCU.channelDmx[channel].select = 127;
      }else if ( (note >= 0x20) && (note <= 0x27) ) {
        // encoder-button
        // reset panning to 50%
        uint16_t channel = (uint16_t)(note - 0x20) + MackieMCU.channelOffsetDmx;
        MackieMCU.channelDmx[channel].encoderValue = 128;
      }else if (note == 0x2e) {
        // bank left
        if (MackieMCU.channelOffsetDmx >= 32) {
          MackieMCU.channelOffsetDmx -= 32;
        }else{
          MackieMCU.channelOffsetDmx = 0;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x2f) {
        // bank right
        if (MackieMCU.channelOffsetDmx <= 472) {
          MackieMCU.channelOffsetDmx += 32;
        }else{
          MackieMCU.channelOffsetDmx = 504;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x30) {
        // channel left
        if (MackieMCU.channelOffsetDmx >= 8) {
          MackieMCU.channelOffsetDmx -= 8;
        }else{
          MackieMCU.channelOffsetDmx = 0;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x31) {
        // channel right
        if (MackieMCU.channelOffsetDmx <= 496) {
          MackieMCU.channelOffsetDmx += 8;
        }else{
          MackieMCU.channelOffsetDmx = 504;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x32) {
        // flip (switches between Audio and DMX)
        mackieDmxMode != mackieDmxMode;
        MackieMCU.forceUpdate = true;
      }else if (note == 0x5b) {
        // rewind
        SerialNina.println("player:prev");
      }else if (note == 0x5c) {
        // forward
        SerialNina.println("player:next");
      }else if (note == 0x5d) {
        // stop
        SerialNina.println("player:stop");
      }else if (note == 0x5e) {
        // play
        SerialNina.println("player:pause");
      }else if (note == 0x5f) {
        // record (switches between Audio and DMX)
        mackieDmxMode = !mackieDmxMode;
        MackieMCU.forceUpdate = true;
      }else if (note == 0x60) {
        // up
      }else if (note == 0x61) {
        // down
      }else if (note == 0x62) {
        // left
      }else if (note == 0x63) {
        // right
      }else if ((note >= 0x68) && (note <= 0x6f)) {
        // Fadertouch 1-8
        MackieMCU.hardwareChannel[note - 0x68].faderTouched = true;
      }else if (note == 0x70) {
        // Masterfadertouch
        MackieMCU.hardwareMainfader.faderTouched = true;
      }else if (note == 0x3c) {
        // jogDialValue
        uint8_t value = velocity & 0b00111111;
        if (velocity & 0b01000000) {
          // turn left
          if (MackieMCU.jogDialValueDmx - value > 0) {
            MackieMCU.jogDialValueDmx -= value;
          }else{
            MackieMCU.jogDialValueDmx = 0;
          }
        }else{
          // turn right
          if (MackieMCU.jogDialValueDmx + value < 255) {
            MackieMCU.jogDialValueDmx += value;
          }else{
            MackieMCU.jogDialValueDmx = 255;
          }
        }
      }else{
        // some other buttons
      }
    }else{
      // AUDIO MODE

      if ( (note >= 0x00) && (note <= 0x07) ) {
        // rec-button
        uint8_t channel = note + MackieMCU.channelOffset;
        if (MackieMCU.channel[channel].rec > 0) {
          MackieMCU.channel[channel].rec = 0;
        }else{
          MackieMCU.channel[channel].rec = 127;
        }
      }else if ( (note >= 0x08) && (note <= 0x0f) ) {
        // solo-button
        uint8_t channel = note - 0x08 + MackieMCU.channelOffset;
        if (MackieMCU.channel[channel].solo > 0) {
          MackieMCU.channel[channel].solo = 0;
        }else{
          MackieMCU.channel[channel].solo = 127;
        }

        // send value to NINA
        SerialNina.println("mixer:solo:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].solo > 0));
      }else if ( (note >= 0x10) && (note <= 0x17) ) {
        // mute-button
        uint8_t channel = note - 0x10 + MackieMCU.channelOffset;
        if (MackieMCU.channel[channel].mute > 0) {
          MackieMCU.channel[channel].mute = 0;
        }else{
          MackieMCU.channel[channel].mute = 127;
        }

        // send value to NINA
        SerialNina.println("mixer:mute:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].mute > 0));
      }else if ( (note >= 0x18) && (note <= 0x1f) ) {
        // select-button

        // disable all select-channels
        for (uint8_t i=0; i<32; i++) {
          MackieMCU.channel[i].select = 0;
        }
        MackieMCU.channel[note - 0x18 + MackieMCU.channelOffset].select = 127;
      }else if ( (note >= 0x20) && (note <= 0x27) ) {
        // encoder-button
        // reset panning to 50%
        uint8_t channel = (note - 0x20) + MackieMCU.channelOffset;
        MackieMCU.channel[channel].encoderValue = 128;
        playerinfo.balanceCh[channel] = 128; // we are receiving this value from NINA with a bit delay again

        // send value to NINA
        SerialNina.println("mixer:balance:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].encoderValue / 2.55f));
      }else if (note == 0x2e) {
        // bank left
        if (MackieMCU.channelOffset >= 8) {
          MackieMCU.channelOffset -= 8;
        }else{
          MackieMCU.channelOffset = 0;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x2f) {
        // bank right
        if (MackieMCU.channelOffset <= 16) {
          MackieMCU.channelOffset += 8;
        }else{
          MackieMCU.channelOffset = 24;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x30) {
        // channel left
        if (MackieMCU.channelOffset >= 1) {
          MackieMCU.channelOffset -= 1;
        }else{
          MackieMCU.channelOffset = 0;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x31) {
        // channel right
        if (MackieMCU.channelOffset <= 23) {
          MackieMCU.channelOffset += 1;
        }else{
          MackieMCU.channelOffset = 24;
        }
        MackieMCU.forceUpdate = true;
      }else if (note == 0x32) {
        // flip (switches between Audio and DMX)
        mackieDmxMode != mackieDmxMode;
        MackieMCU.forceUpdate = true;
      }else if (note == 0x5b) {
        // rewind
        SerialNina.println("player:prev");
      }else if (note == 0x5c) {
        // forward
        SerialNina.println("player:next");
      }else if (note == 0x5d) {
        // stop
        SerialNina.println("player:stop");
      }else if (note == 0x5e) {
        // play
        SerialNina.println("player:pause");
      }else if (note == 0x5f) {
        // record (switches between Audio and DMX)
        mackieDmxMode = !mackieDmxMode;
        MackieMCU.forceUpdate = true;
      }else if (note == 0x60) {
        // up
      }else if (note == 0x61) {
        // down
      }else if (note == 0x62) {
        // left
      }else if (note == 0x63) {
        // right
      }else if ((note >= 0x68) && (note <= 0x6f)) {
        // Fadertouch 1-8
        MackieMCU.hardwareChannel[note - 0x68].faderTouched = true;
      }else if (note == 0x70) {
        // Masterfadertouch
        MackieMCU.hardwareMainfader.faderTouched = true;
      }else if (note == 0x3c) {
        // jogDialValue
        uint8_t value = velocity & 0b00111111;
        if (velocity & 0b01000000) {
          // turn left
          if (MackieMCU.jogDialValue - value > 0) {
            MackieMCU.jogDialValue -= value;
          }else{
            MackieMCU.jogDialValue = 0;
          }
        }else{
          // turn right
          if (MackieMCU.jogDialValue + value < 255) {
            MackieMCU.jogDialValue += value;
          }else{
            MackieMCU.jogDialValue = 255;
          }
        }
      }else{
        // some other buttons
      }
    }
  }

  void MackieMCU_handleNoteOff(uint8_t midiChannel, uint8_t note, uint8_t velocity) {
    if (mackieDmxMode) {
      if ((note >= 0x68) && (note <= 0x6f)) {
        // Faderuntouch 1-8
        MackieMCU.hardwareChannel[note - 0x68].faderTouched = false;
      }else if (note == 0x70) {
        // Masterfaderuntouch
        MackieMCU.hardwareMainfader.faderTouched = false;
      }
    }else{
      if ((note >= 0x68) && (note <= 0x6f)) {
        // Faderuntouch 1-8
        MackieMCU.hardwareChannel[note - 0x68].faderTouched = false;
      }else if (note == 0x70) {
        // Masterfaderuntouch
        MackieMCU.hardwareMainfader.faderTouched = false;
      }
    }
  }

  void MackieMCU_handleControlChange(uint8_t midiChannel, uint8_t data1, uint8_t data2) {
    if (mackieDmxMode) {
      if (midiChannel == 1) {
        if ((data1 >= 0x10) && (data1 <= 0x17)) {
          // encoder-value
          uint16_t channel = (uint16_t)(data1 - 0x10) + MackieMCU.channelOffsetDmx;
          uint8_t value = data2 & 0b00111111;

          if (data2 & 0b01000000) {
            // turn left
            if (MackieMCU.channelDmx[channel].encoderValue - value > 0) {
              MackieMCU.channelDmx[channel].encoderValue -= value;
            }else{
              MackieMCU.channelDmx[channel].encoderValue = 0;
            }
          }else{
            // turn right
            if (MackieMCU.channelDmx[channel].encoderValue + value < 255) {
              MackieMCU.channelDmx[channel].encoderValue += value;
            }else{
              MackieMCU.channelDmx[channel].encoderValue = 255;
            }
          }
          // TODO: do something with this value
        }
      }
    }else{
      if (midiChannel == 1) {
        if ((data1 >= 0x10) && (data1 <= 0x17)) {
          // encoder-value
          uint8_t channel = data1 - 0x10 + MackieMCU.channelOffset;
          uint8_t value = data2 & 0b00111111;

          if (data2 & 0b01000000) {
            // turn left
            if (MackieMCU.channel[channel].encoderValue - value > 0) {
              MackieMCU.channel[channel].encoderValue -= value;
            }else{
              MackieMCU.channel[channel].encoderValue = 0;
            }
          }else{
            // turn right
            if (MackieMCU.channel[channel].encoderValue + value < 255) {
              MackieMCU.channel[channel].encoderValue += value;
            }else{
              MackieMCU.channel[channel].encoderValue = 255;
            }
          }
          // set balance
          playerinfo.balanceCh[channel] = MackieMCU.channel[channel].encoderValue; // we are receiving this value from NINA with a bit delay again

          // send value to NINA
          SerialNina.println("mixer:balance:ch" + String(channel + 1) + "@" + String(MackieMCU.channel[channel].encoderValue / 2.55f));
        }
      }
    }
  }

  void MackieMCU_handlePitchBendChange(uint8_t midiChannel, int value) {
    if (mackieDmxMode) {
      if ((midiChannel >= 1) && (midiChannel <= 8)) {
        uint8_t hardwareFader = midiChannel - 1;
        uint16_t channel = (uint16_t)hardwareFader + MackieMCU.channelOffsetDmx;

        MackieMCU.hardwareChannel[hardwareFader].faderPositionHW = value - MIDI_PITCHBEND_MIN;
        if (MackieMCU.hardwareChannel[hardwareFader].faderTouched) {
          MackieMCU.channelDmx[channel].faderPosition = value - MIDI_PITCHBEND_MIN; // copy value to faderPosition
          uint8_t newValue = MackieMCU.channelDmx[channel].faderPosition/64.2470588f;

          // send new value to NINA
          SerialNina.println("dmx512:output:ch" + String(channel + 1) + "@" + String(newValue));

          // reset nameCounter to display current value in displays instead of names for some time
          MackieMCU.hardwareChannel[hardwareFader].showValueCounter = 20; // show value for 2 seconds as counter is at 100ms
        }
      }else if (midiChannel == 9) {
        MackieMCU.hardwareMainfader.faderPositionHW = value - MIDI_PITCHBEND_MIN;
        if (MackieMCU.hardwareMainfader.faderTouched) {
          MackieMCU.channelDmx[512].faderPosition = value - MIDI_PITCHBEND_MIN; // copy value to faderPosition
          uint8_t newValue = MackieMCU.channelDmx[512].faderPosition/64.2470588f;
          // we are not using Masterfader at the moment
        }
      }
    }else{
      if ((midiChannel >= 1) && (midiChannel <= 8)) {
        uint8_t channel = midiChannel - 1 + MackieMCU.channelOffset;

        uint8_t hardwareFader = channel - MackieMCU.channelOffset;
        MackieMCU.hardwareChannel[hardwareFader].faderPositionHW = value - MIDI_PITCHBEND_MIN;
        if (MackieMCU.hardwareChannel[hardwareFader].faderTouched) {
          MackieMCU.channel[channel].faderPosition = value - MIDI_PITCHBEND_MIN;
          float newVolume = ((MackieMCU.channel[channel].faderPosition/16383.0f) * 54.0f) - 48.0f;
          playerinfo.volumeCh[channel] = newVolume; // we are receiving this value from NINA with a bit delay again

          // send new value to NINA
          SerialNina.println("mixer:volume:ch" + String(channel + 1) + "@" + String(newVolume, 2));

          // reset nameCounter to display current value in displays instead of names for some time
          MackieMCU.hardwareChannel[hardwareFader].showValueCounter = 20; // show value for 2 seconds as counter is at 100ms
        }
      }else if (midiChannel == 9) {
        MackieMCU.hardwareMainfader.faderPositionHW = value - MIDI_PITCHBEND_MIN;
        if (MackieMCU.hardwareMainfader.faderTouched) {
          MackieMCU.channel[32].faderPosition = value - MIDI_PITCHBEND_MIN;
          float newVolume = ((MackieMCU.channel[32].faderPosition/16383.0f) * 54.0f) - 48.0f;
          playerinfo.volumeMain = newVolume;

          // send new main-value to NINA
          SerialNina.println("mixer:volume:main@" + String(newVolume, 2));
        }
      }
    }
  }

  void MackieMCU_handleProgramChange(uint8_t midiChannel, uint8_t data1) {
    // nothing to do here at the moment
  }

  void MackieMCU_handleSysEx(unsigned char* sysExArray, unsigned int sysExSize) {
    uint8_t data[20];

    if ((sysExArray[0] == 0xF0) && (sysExArray[3] == 0x66) && (sysExArray[sysExSize - 1] == 0xF7)) {
      if (sysExArray[5] == 0x01) {
        // we received a host connection query
        // send host connection reply
        data[0] = 0xF0; // SysEx startcode
        data[1] = 0x00; // SysEx header
        data[2] = 0x00; // SysEx header
        data[3] = 0x66; // SysEx header
        data[4] = 0x14; // SysEx header. DeviceID = Mackie Control
        data[5] = 0x02; // command
        data[6] = sysExArray[6];
        data[7] = sysExArray[7];
        data[8] = sysExArray[8];
        data[9] = sysExArray[9];
        data[10] = sysExArray[10];
        data[11] = sysExArray[11];
        data[12] = sysExArray[12];
        uint8_t l1 = sysExArray[13];
        uint8_t l2 = sysExArray[14];
        uint8_t l3 = sysExArray[15];
        uint8_t l4 = sysExArray[16];
        data[13] = 0x7F & (l1 + (l2 ^ 0x0A) - l4);
        data[14] = 0x7F & (l3 >> 4) ^ (l1+l4);
        data[15] = 0x7F & (l4 - (l3 << 2) ^ (l1 | l2));
        data[16] = 0x7F & (l2 - l3 + (0xF0 ^(l4 << 4)));
        data[17] = 0xF7;
        MIDI.sendSysEx(18, data);
      }else if (sysExArray[5] == 0x03) {
        // received host connection confirmation
      }else if (sysExArray[5] == 0x04) {
        // received host connection error
      }else if (sysExArray[5] == 0x14) {
        // received firmware-version
        String firmwareVersion = String((char)sysExArray[6]) + String((char)sysExArray[7]) + String((char)sysExArray[8]) + String((char)sysExArray[9]) + String((char)sysExArray[10]);
      }else{
        // unused or unknown command
      }
    }else{
      // error in SysEx-command
    }
  }

  void MackieMCU_init() {
    // set default names for all 32 channels. Can be changed via UART lateron
    for (uint8_t i_ch=0; i_ch<32; i_ch++) {
      MackieMCU.channel[i_ch].name = "Ch " + String(i_ch + 1) + "   ";
    }

    MIDI.setHandleNoteOn(MackieMCU_handleNoteOn);
    MIDI.setHandleNoteOff(MackieMCU_handleNoteOff);
    MIDI.setHandleControlChange(MackieMCU_handleControlChange);
    MIDI.setHandlePitchBend(MackieMCU_handlePitchBendChange);
    MIDI.setHandleProgramChange(MackieMCU_handleProgramChange);
    MIDI.setHandleSystemExclusive(MackieMCU_handleSysEx);
    MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages

    pinPeripheral(4, PIO_SERCOM_ALT); //Assign TX function to pin 4
    pinPeripheral(5, PIO_SERCOM_ALT); //Assign RX function to pin 5
  }

  void MackieMCU_sendData() {
    if (mackieDmxMode) {
      // update small LC-displays
      MackieMCU_updateTimecodeDisplay("------------"); // send all 12 LC-displays at once

      // update all faders and buttons for the current channel-selection
      uint32_t newFaderValue;
      for (uint16_t i_ch=MackieMCU.channelOffsetDmx; i_ch<(8+MackieMCU.channelOffsetDmx); i_ch++) {
        uint8_t hardwareFader = i_ch - MackieMCU.channelOffsetDmx;
        MackieMCU.hardwareChannel[hardwareFader].faderNeedsUpdate = (MackieMCU.channelDmx[i_ch].faderPosition != MackieMCU.hardwareChannel[hardwareFader].faderPositionHW) && (!MackieMCU.hardwareChannel[hardwareFader].faderTouched);
        MackieMCU.hardwareChannel[hardwareFader].meterLevel = MackieMCU.channelDmx[i_ch].faderPosition/1260; // 0..11

        // send values to device
        if ((MackieMCU.hardwareChannel[hardwareFader].faderNeedsUpdate || MackieMCU.forceUpdate) && (!MackieMCU.hardwareChannel[hardwareFader].faderTouched)) {
          MackieMCU.hardwareChannel[hardwareFader].faderNeedsUpdate = false;
          MackieMCU_setFader(hardwareFader, MackieMCU.channelDmx[i_ch].faderPosition);
        }
        MackieMCU_setMeter(hardwareFader, MackieMCU.hardwareChannel[hardwareFader].meterLevel);
        if (MackieMCU.hardwareChannel[hardwareFader].showValueCounter > 0) {
          // displaying current audio-level in second line
          MackieMCU_updateLCDs(hardwareFader, "DMX " + String(i_ch + 1), String(MackieMCU.channelDmx[i_ch].faderPosition/64.24705882352941f, 0) + "/" + String(MackieMCU.channelDmx[i_ch].faderPosition/163.83f, 0) + "%  ");
        }else{
          // display channel-name in second line
          MackieMCU_updateLCDs(hardwareFader, "DMX " + String(i_ch + 1), MackieMCU.channelDmx[i_ch].name);
        }
        MackieMCU_setLedRing(hardwareFader, MackieMCU.channelDmx[i_ch].encoderValue);
      }
/*
      // we are not using the masterfader in DMX512-mode at the moment

      // MasterFader
      newFaderValue = ...;
      MackieMCU.hmainfader.faderNeedsUpdate = newFaderValue != MackieMCU.channelDmx[512].faderPosition;
      MackieMCU.channelDmx[512].faderPosition = newFaderValue; // convert volumeMain from dBfs to 0...16388 but keep logarithmic scale
      if ((MackieMCU.hardwareMainfader.faderNeedsUpdate || MackieMCU.forceUpdate) && (!MackieMCU.hardwareMainfader.faderTouched)) {
        MackieMCU.hardwareMainfader.faderNeedsUpdate = false;
        MackieMCU_setFader(512, MackieMCU.channelDmx[512].faderPosition);
      }
*/
      MackieMCU.forceUpdate = false;

      // set button-leds
      MackieMCU_updateButtons(0x5b, 0); // rewind - we are not using this function in DMX-mode yet
      MackieMCU_updateButtons(0x5c, 0); // forward - we are not using this function in DMX-mode yet
      MackieMCU_updateButtons(0x5d, 0); // stop - we are not using this function in DMX-mode yet
      MackieMCU_updateButtons(0x5e, 0); // play - we are not using this function in DMX-mode yet
      MackieMCU_updateButtons(0x32, 127); // flip on = we are in DMX-Mode
      MackieMCU_updateButtons(0x5f, 127); // record on = we are in DMX-Mode

      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        uint16_t channel = (uint16_t)(i_ch) + MackieMCU.channelOffsetDmx;
        MackieMCU_updateButtons(i_ch, MackieMCU.channelDmx[channel].rec);
        MackieMCU_updateButtons(i_ch + 0x08, MackieMCU.channelDmx[channel].solo);
        MackieMCU_updateButtons(i_ch + 0x10, MackieMCU.channelDmx[channel].mute);
        MackieMCU_updateButtons(i_ch + 0x18, MackieMCU.channelDmx[channel].select);
      }
    }else{
      // update small LC-displays
      String playtime = secondsToHMS_B(playerinfo.time, false); // 00:00:00
      String valueString;
      if (MackieMCU.jogDialValue < 10) {
        valueString = "  " + String(MackieMCU.jogDialValue);
      }else if (MackieMCU.jogDialValue < 100) {
        valueString = " " + String(MackieMCU.jogDialValue);
      }else{
        valueString = String(MackieMCU.jogDialValue);
      }
      MackieMCU_updateTimecodeDisplay("10  " + playtime[0] + valueString); // send all 12 LC-displays at once

      // update all faders and buttons for the current channel-selection
      uint32_t newFaderValue;
      for (uint8_t i_ch=MackieMCU.channelOffset; i_ch<(8+MackieMCU.channelOffset); i_ch++) {
        newFaderValue = ((playerinfo.volumeCh[i_ch] + 48.0f)/54.0f) * 16383.0f;
        uint8_t hardwareFader = i_ch - MackieMCU.channelOffset;
        MackieMCU.hardwareChannel[hardwareFader].faderNeedsUpdate = newFaderValue != MackieMCU.hardwareChannel[hardwareFader].faderPositionHW;
        MackieMCU.channel[i_ch].faderPosition = newFaderValue; // 0..16383
        MackieMCU.hardwareChannel[hardwareFader].meterLevel = ((uint16_t)playerinfo.vuMeterCh[i_ch] * 12) / 255; // scale 0..255 -> 0..12, with CLIP-LED

        // send values to device
        if ((MackieMCU.hardwareChannel[hardwareFader].faderNeedsUpdate || MackieMCU.forceUpdate) && (!MackieMCU.hardwareChannel[i_ch - MackieMCU.channelOffset].faderTouched)) {
          MackieMCU.hardwareChannel[hardwareFader].faderNeedsUpdate = false;
          MackieMCU_setFader(hardwareFader, MackieMCU.channel[i_ch].faderPosition);
        }
        MackieMCU_setMeter(hardwareFader, MackieMCU.hardwareChannel[hardwareFader].meterLevel);
        if (MackieMCU.hardwareChannel[hardwareFader].showValueCounter > 0) {
          // displaying current audio-level in second line
          MackieMCU_updateLCDs(hardwareFader, "Ch" + String(i_ch+1) + MackieMCU_panString(MackieMCU.channel[i_ch].encoderValue), String(playerinfo.volumeCh[i_ch], 2) + "dB    ");
        }else{
          // display channel-name in second line
          MackieMCU_updateLCDs(hardwareFader, "Ch" + String(i_ch+1) + MackieMCU_panString(MackieMCU.channel[i_ch].encoderValue), MackieMCU.channel[i_ch].name);
        }
        MackieMCU_setLedRing(hardwareFader, MackieMCU.channel[i_ch].encoderValue);
      }
      // MasterFader
      newFaderValue = ((playerinfo.volumeMain + 48.0f)/54.0f) * 16383.0f;
      MackieMCU.hardwareMainfader.faderNeedsUpdate = newFaderValue != MackieMCU.channel[32].faderPosition;
      MackieMCU.channel[32].faderPosition = newFaderValue; // convert volumeMain from dBfs to 0...16388 but keep logarithmic scale
      if ((MackieMCU.hardwareMainfader.faderNeedsUpdate || MackieMCU.forceUpdate) && (!MackieMCU.hardwareMainfader.faderTouched)) {
        MackieMCU.hardwareMainfader.faderNeedsUpdate = false;
        MackieMCU_setFader(32, MackieMCU.channel[32].faderPosition);
      }
      MackieMCU.forceUpdate = false;

      // set button-leds
      MackieMCU_updateButtons(0x5b, (playerinfo.currentTrackNumber > 0) ? 127 : 0); // rewind
      MackieMCU_updateButtons(0x5c, (playerinfo.currentTrackNumber < (tocEntries - 1)) ? 127 : 0); // forward
      MackieMCU_updateButtons(0x5d, 127); // stop
      MackieMCU_updateButtons(0x5e, 127); // play
      MackieMCU_updateButtons(0x32, 0); // flip off
      MackieMCU_updateButtons(0x5f, 0); // record off

      for (uint8_t i_ch=0; i_ch<8; i_ch++) {
        MackieMCU_updateButtons(i_ch, MackieMCU.channel[i_ch + MackieMCU.channelOffset].rec);
        MackieMCU_updateButtons(i_ch + 0x08, MackieMCU.channel[i_ch + MackieMCU.channelOffset].solo);
        MackieMCU_updateButtons(i_ch + 0x10, MackieMCU.channel[i_ch + MackieMCU.channelOffset].mute);
        MackieMCU_updateButtons(i_ch + 0x18, MackieMCU.channel[i_ch + MackieMCU.channelOffset].select);
      }
    }
  }



  // =========================== SERVICE FUNCTIONS ==========================
  void handleMackieMCUCommunication() {
    // Read incoming messages
    MIDI.read();
  }

  void MackieMCU_updateTimecodeDisplay(uint8_t position, char c) {
    /*
      ASSIGNMENT          HOURS             MINUTES         SECONDS          FRAMES
      0x4b 0x4a       0x49 0x48 0x47       0x46 0x45       0x44 0x43       0x42 0x41 0x40
      0    1          2    3    4           5    6          7   8          9    10   11     <- position
    */  
    // send ControlChange = 0xB0
    MIDI.send(midi::MidiType::ControlChange, 0x40 + (11-position), c, 1); // type, data1, data2, channel
  }
  
  void MackieMCU_updateTimecodeDisplay(String text) {
    uint8_t data[18];
    data[0] = 0xF0; // SysEx startcode
    data[1] = 0x00; // SysEx header
    data[2] = 0x00; // SysEx header
    data[3] = 0x66; // SysEx header
    data[4] = 0x14; // SysEx header. DeviceID = Mackie Control
    data[5] = 0x10; // command for updating timecode-displays

    for (uint8_t i=0; i<11; i++) {
      data[i + 6] = text[i]; // copy data
    }
    data[17] = 0xF7; // SysEx endcode
    MIDI.sendSysEx(18, data);
  }

  void MackieMCU_updateLCDs(uint8_t channel, String top, String bot) {
    /*
      Writing to LCD:
      F0 <HDR> 12 pp cc cc .. cc F7
      with <HDR>: 0x00 0x00 0x66 0xdd
      with pp = location between 0x00 and 0x6f (0...111)
      with cc = characters
      with dd = DeviceIDs:
        0x05 = MackieHUI
        0x10 = Logic Control
        0x11 = Logic Control XT
        0x14 = Mackie Control
        0x15 = Mackie Control XT
    */  

    uint8_t data[15]; // 7 chars per channel
    data[0] = 0xF0; // SysEx startcode
    data[1] = 0x00; // SysEx header
    data[2] = 0x00; // SysEx header
    data[3] = 0x66; // SysEx header
    data[4] = 0x14; // SysEx header. DeviceID = Mackie Control
    data[5] = 0x12; // command for updating LCD
    data[14] = 0xF7; // SysEx endcode

    // update data-array for top-line
    data[6] = channel * 7; // position of line 1
    for (uint8_t i=0; i<7; i++) {
      data[i + 7] = top[i]; // copy data
    }
    MIDI.sendSysEx(15, data);

    // update data-array for bottom-line
    data[6] = (channel * 7) + 56; // position of line 2
    for (uint8_t i=0; i<7; i++) {
      data[i + 7] = bot[i]; // copy data
    }
    MIDI.sendSysEx(15, data);
  }

  void MackieMCU_updateButtons(uint8_t button, uint8_t value) {
    /*
      Velocity:
      0x00 = OFF
      0x01 = BLINK (only LEDs)
      0x7f = ON
    */

    // send NoteOn = 0x90
    MIDI.send(midi::MidiType::NoteOn, button, value, 1);
  }

  void MackieMCU_setLedRing(uint8_t channel, uint8_t value) {
    // value 0..6..11 -> 0=off, 1=left, 6=Center, 11=right
    uint8_t mode = 1; // 0=single led, 1=pan, 2=incrementing, 3=spread

    uint8_t valueRaw = (mode << 5) + value; // b7=0, b6=led, b5..b4=mode, b3...b0 = value

    // send ControlChange = 0xB0
    MIDI.send(midi::MidiType::ControlChange, 0x30 + channel, valueRaw, 1); // value between 0 and 16383
  }

  void MackieMCU_setFader(uint8_t channel, int16_t value) {
    // send PitchBend = 0xE0
    if (!MackieMCU.hardwareChannel[channel].faderTouched) {
      MackieMCU.hardwareChannel[channel].faderPositionHW = value; // X32 has no TouchFader, so we force the position everytime
    }
    MIDI.send(midi::MidiType::PitchBend, (value & 0x7f), (value >> 7) & 0x7f, channel+1); // value between 0 and 16383
  }

  void MackieMCU_setMeter(uint8_t channel, uint8_t value) {
    // send AfterTouchChannel = 0xD0
    MIDI.send(midi::MidiType::AfterTouchChannel, (channel << 4) + value, 0, 1); // type, data1, data2, channel
  }

  void MackieMCU_requestFirmwareVersion() {
    uint8_t data[8]; // 7 chars per channel
    data[0] = 0xF0; // SysEx startcode
    data[1] = 0x00; // SysEx header
    data[2] = 0x00; // SysEx header
    data[3] = 0x66; // SysEx header
    data[4] = 0x14; // SysEx header. DeviceID = Mackie Control
    data[5] = 0x13; // command for requesting firmware-version
    data[6] = 0x00; // zero
    data[7] = 0xF7; // SysEx endcode
    MIDI.sendSysEx(8, data);
  }
  
  String MackieMCU_panString(uint8_t value) {
    if (value == 128) {
      return "<C>";
    }else if (value <128) {
      return "L" + String(50 - (value/2.55), 0);
    }else{
      return "R" + String((value/2.55) - 50, 0);
    }
  }

  // Interrupt handler for SERCOM4
  void SERCOM4_Handler()
  {
    Serial3.IrqHandler();
  }
#endif
