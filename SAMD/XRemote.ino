/*
  The XRemote-protocol is used to communicate with the XEdit software.
  The communication is based on OSC-protocol.

  This file uses some parts of the MackieMCU-struct to keep names and meters in sync. The X32Edit displays
  all 32 channels at once. At the moment only volume-fader, pan, mute and solo is supported.

  Parts of this file are based on the "Unofficial X32/M32 OSC Remote Protocol" v4.02 by Patrick-Gilles Maillot.
  Thank you very much for sharing your work!
*/

#if USE_XREMOTE == 1
  void xremoteInit() {
    xremoteUdp.begin(10023);
  }

  void xremoteUpdateAll() {
    for(uint8_t i=0; i<32; i++) {
      /*
      // some test-data
      MackieMCU.channel[i].faderPosition = random(0, 16383);
      MackieMCU.channel[i].encoderValue = random(0, 255);
      MackieMCU.channel[i].color = random(0, 15);
      MackieMCU.channel[i].mute = random(0, 2);
      MackieMCU.channel[i].solo = random(0, 2);
      playerinfo.vuMeterCh[i] = random(0, 255);
      */
      xremoteSetFader(i+1, MackieMCU.channel[i].faderPosition/16383.0f);
      xremoteSetPan(i+1, MackieMCU.channel[i].encoderValue/255.0f);
      xremoteSetMute(i+1, MackieMCU.channel[i].mute);
      xremoteSetSolo(i+1, MackieMCU.channel[i].solo);
      xremoteSetColor(i+1, MackieMCU.channel[i].color);
      xremoteSetName(i+1, MackieMCU.channel[i].name);
    }
    xremoteUpdateMeter();
  }

  void xremoteHandleCommunication() {
    uint16_t udpsize = xremoteUdp.parsePacket();
    if (udpsize > 0) {
      char rxData[udpsize];
      uint8_t len = xremoteUdp.read(rxData, udpsize);
      uint8_t channel;
      data_32b value32bit;

      /*
      Serial.print("Received: ");
      for (uint8_t i=0; i<len; i++){
        Serial.print(rxData[i]);
      }
      Serial.println("end");
      */

      if (len > 0) {
        // parse the incoming packet
        if (memcmp(rxData, xremote_cmd_info, 4) == 0) {
          // info
          //Serial.println("/info");
          xremoteAnswerInfo();
        }else if (memcmp(rxData, xremote_cmd_xinfo, 4) == 0) {
          // xinfo
          //Serial.println("/xinfo");
          xremoteAnswerXInfo();
        }else if (memcmp(rxData, xremote_cmd_status, 4) == 0) {
          // status
          //Serial.println("/status");
          xremoteAnswerStatus();
        }else if (memcmp(rxData, xremote_cmd_xremote, 4) == 0) {
          // xremote
          // Optional: read and store IP-Address of client
          //Serial.println("/xremote");

          // send routing, names and colors
          for (uint8_t i=0; i<32; i++) {
            xremoteSetName(i+1, MackieMCU.channel[i].name);
            if (MackieMCU.channel[i].color > 63) {
              xremoteSetColor(i+1, MackieMCU.channel[i].color - 64 + 8);
            }else{
              xremoteSetColor(i+1, MackieMCU.channel[i].color);
            }
            
            xremoteSetSource(i+1, i+1);
          }
          xremoteSetCard(10); // X-LIVE
        }else if (memcmp(rxData, xremote_cmd_unsubscribe, 4) == 0) {
          // unsubscribe
          // Optional: remove xremote client
          //Serial.println("/unsubscribe");
        }else if (memcmp(rxData, xremote_cmd_channel, 4) == 0) {
          // channel

          // /ch/xx/mix/fader~~~~,f~~[float]
          // /ch/xx/mix/pan~~,f~~[float]
          // /ch/xx/mix/on~~~,i~~[int]
          channel = ((rxData[4]-48)*10 + (rxData[5]-48)) - 1;
          if (len > 13) {
            if ((rxData[7] == 'm') && (rxData[8] == 'i') && (rxData[9] == 'x')) {
              if ((rxData[11] == 'f') && (rxData[12] == 'a') && (rxData[13] == 'd')) {
                // get fader-value
                value32bit.u8[0] = rxData[27];
                value32bit.u8[1] = rxData[26];
                value32bit.u8[2] = rxData[25];
                value32bit.u8[3] = rxData[24];

                float newVolume = (value32bit.f * 54.0f) - 48.0f;
                mixerSetVolume(channel, newVolume);
              }else if ((rxData[11] == 'p') && (rxData[12] == 'a') && (rxData[13] == 'n')) {
                // get pan-value
                value32bit.u8[0] = rxData[23];
                value32bit.u8[1] = rxData[22];
                value32bit.u8[2] = rxData[21];
                value32bit.u8[3] = rxData[20];

                MackieMCU.channel[channel].encoderValue = value32bit.f * 255.0f;
                mixerSetBalance(channel,  value32bit.f * 100.0f);
              }else if ((rxData[11] == 'o') && (rxData[12] == 'n')) {
                // get mute-state (caution: here it is "mixer-on"-state)
                mixerSetMute(channel, (rxData[20+3] == 0));
              }
            }else if ((rxData[7] == 'c') && (rxData[8] == 'o') && (rxData[9] == 'n')) {
              // config
              if  ((rxData[14] == 'c') && (rxData[15] == 'o') && (rxData[16] == 'l')) {
                // color
                value32bit.u8[0] = rxData[27];
                value32bit.u8[1] = rxData[26];
                value32bit.u8[2] = rxData[25];
                value32bit.u8[3] = rxData[24];

                if (value32bit.u32 < 8) {
                  MackieMCU.channel[channel].color = value32bit.u32;
                }else{
                  MackieMCU.channel[channel].color = value32bit.u32 - 8 + 64;
                }
              }else if  ((rxData[14] == 'n') && (rxData[15] == 'a') && (rxData[16] == 'm')) {
                // name
                String name;
                charToString(&rxData[24], name);

                MackieMCU.channel[channel].name = name;
              }else if  ((rxData[14] == 'i') && (rxData[15] == 'c') && (rxData[16] == 'o')) {
                // icon
                value32bit.u8[0] = rxData[27];
                value32bit.u8[1] = rxData[26];
                value32bit.u8[2] = rxData[25];
                value32bit.u8[3] = rxData[24];

                // do something with channel and value32bit.f
                //Serial.println("/ch/" + String(channel) + "/config/icon " + String(value32bit.u32));
              }
            }
          }
        }else if (memcmp(rxData, xremote_cmd_main, 4) == 0) {
          // main

          // /main/st/mix/fader~~,f~~[float]
          // /main/st/mix/pan~~~~,f~~[float]
          // /main/st/mix/on~,i~~[int]
          if (len > 12) {
            if ((rxData[6] == 's') && (rxData[7] == 't') && (rxData[9] == 'm') && (rxData[10] == 'i') && (rxData[11] == 'x')) {
              if ((rxData[13] == 'f') && (rxData[14] == 'a') && (rxData[15] == 'd')) {
                // get fader-value
                value32bit.u8[0] = rxData[27];
                value32bit.u8[1] = rxData[26];
                value32bit.u8[2] = rxData[25];
                value32bit.u8[3] = rxData[24];

                float newVolume = (value32bit.f * 54.0f) - 48.0f;
                mixerSetMainVolume(newVolume);
              }else if ((rxData[13] == 'p') && (rxData[14] == 'a') && (rxData[15] == 'n')) {
                // get pan-value
                value32bit.u8[0] = rxData[27];
                value32bit.u8[1] = rxData[26];
                value32bit.u8[2] = rxData[25];
                value32bit.u8[3] = rxData[24];

                mixerSetMainBalance(value32bit.f * 100);
              }else if ((rxData[13] == 'o') && (rxData[14] == 'n')) {
                // get mute-state
                // /main/st/mix/on~,i~~~
                // do something with channel and (rxData[20+3]) // 0 = mute off, 31 = mute on
              }
            }
          }
        }else if (memcmp(rxData, xremote_cmd_stat, 4) == 0) {
          // stat

          if ((rxData[7] == 's') && (rxData[8] == 'o') && (rxData[9] == 'l') && (rxData[10] == 'o') && (rxData[11] == 's') && (rxData[12] == 'w')) {
            // /-stat/solosw/xx~~~~,i~~[integer]
            channel = ((rxData[14]-48)*10 + (rxData[15]-48)) - 1;
            value32bit.u8[0] = rxData[27];
            value32bit.u8[1] = rxData[26];
            value32bit.u8[2] = rxData[25];
            value32bit.u8[3] = rxData[24];

            // we receive solo-values for 80 channels
            if ((channel>=0) && (channel<32)) {
              mixerSetSolo(channel, (value32bit.u32 == 1));
            }
          }else if ((rxData[7] == 'u') && (rxData[8] == 'r') && (rxData[9] == 'e') && (rxData[10] == 'c')) {
            value32bit.u8[0] = rxData[27];
            value32bit.u8[1] = rxData[26];
            value32bit.u8[2] = rxData[25];
            value32bit.u8[3] = rxData[24];

            // /-stat/urec/state~~~,i~~[integer]
            if (value32bit.u32 == 0) {
              // stop
              SerialNina.println("player:stop");
            }else if (value32bit.u32 == 1) {
              // pause
              SerialNina.println("player:pause");
            }else if (value32bit.u32 == 2) {
              // play
              SerialNina.println("player:pause");
            }else if (value32bit.u32 == 3) {
              // record
            }
          }
        }else{
          // ignore unused commands for now
        }
      }
    }
  }

  void xremoteAnswerInfo() {
    uint16_t len = xremotesprint(xremote_TxMessage, 0, 's', "/info");
    len = xremotesprint(xremote_TxMessage, len, 's', ",ssss");
    len = xremotesprint(xremote_TxMessage, len, 's', "V2.07");
    len = xremotesprint(xremote_TxMessage, len, 's', "X-FBAPE");
    len = xremotesprint(xremote_TxMessage, len, 's', XREMOTE_DEVICE);
    len = xremotesprint(xremote_TxMessage, len, 's', XREMOTE_FW_VER);

    xremoteSendUdpPacket(xremote_TxMessage, len);
  }

  void xremoteAnswerXInfo() {
    uint16_t len = xremotesprint(xremote_TxMessage, 0, 's', "/xinfo");
    len = xremotesprint(xremote_TxMessage, len, 's', ",ssss");
    len = xremotesprint(xremote_TxMessage, len, 's', IpAddress2String(eeprom_config.ip).c_str());
    len = xremotesprint(xremote_TxMessage, len, 's', "X-FBAPE");
    len = xremotesprint(xremote_TxMessage, len, 's', XREMOTE_DEVICE);
    len = xremotesprint(xremote_TxMessage, len, 's', XREMOTE_FW_VER);

    xremoteSendUdpPacket(xremote_TxMessage, len);
  }

  void xremoteAnswerStatus() {
    uint16_t len = xremotesprint(xremote_TxMessage, 0, 's', "/status");
    len = xremotesprint(xremote_TxMessage, len, 's', ",sss");
    len = xremotesprint(xremote_TxMessage, len, 's', "active");
    len = xremotesprint(xremote_TxMessage, len, 's', IpAddress2String(eeprom_config.ip).c_str());
    len = xremotesprint(xremote_TxMessage, len, 's', "X-FBAPE");

    xremoteSendUdpPacket(xremote_TxMessage, len);
  }

  void xremoteSetFader(uint8_t ch, float value_pu) {
    char cmd[32];
    sprintf(cmd, "/ch/%02i/mix/fader", ch);
    xremoteSendBasicMessage(cmd, 'f', 'b', (char*)&value_pu);
  }

  void xremoteSetPan(uint8_t ch, float value_pu) {
    char cmd[32];
    sprintf(cmd, "/ch/%02i/mix/pan", ch);
    xremoteSendBasicMessage(cmd, 'f', 'b', (char*)&value_pu);
  }

  void xremoteSetMainFader(float value_pu) {
    xremoteSendBasicMessage("/main/st/mix/fader", 'f', 'b', (char*)&value_pu);
  }

  void xremoteSetMainPan(float value_pu) {
    xremoteSendBasicMessage("/main/st/mix/pan", 'f', 'b', (char*)&value_pu);
  }

  void xremoteSetName(uint8_t ch, String name) {
    char nameArray[12];
    char cmd[50];
    name.toCharArray(nameArray, 12);
    sprintf(cmd, "/ch/%02i/config/name", ch);
    xremoteSendBasicMessage(cmd, 's', 's', nameArray);
  }

  void xremoteSetColor(uint8_t ch, int32_t color) {
    char cmd[32];
    sprintf(cmd, "/ch/%02i/config/color", ch);
    xremoteSendBasicMessage(cmd, 'i', 'b', (char*)&color);
  }

  void xremoteSetSource(uint8_t ch, int32_t source) {
    char cmd[32];
    sprintf(cmd, "/ch/%02i/config/source", ch);
    xremoteSendBasicMessage(cmd, 'i', 'b', (char*)&source);
  }

  void xremoteSetIcon(uint8_t ch, int32_t icon) {
    char cmd[32];
    sprintf(cmd, "/ch/%02i/config/icon", ch);
    xremoteSendBasicMessage(cmd, 'i', 'b', (char*)&icon);
  }

  void xremoteSetCard(uint8_t card) {
    char cmd[32];
    String scmd = "-stat/xcardtype " + String(card);
    scmd.toCharArray(cmd, scmd.length() + 1);
    cmd[scmd.length()] = 0x10; // add linefeed
    xremoteSendBasicMessage("node", 's', 's', cmd);
  }

  void xremoteSetMute(uint8_t ch, uint8_t muted) {
    char cmd[32];
    int32_t online;
    if (muted == 0) {
      online = 1;
    }else{
      online = 0;
    }
    sprintf(cmd, "/ch/%02i/mix/on", ch);
    xremoteSendBasicMessage(cmd, 'i', 'b', (char*)&online);
  }

  void xremoteSetSolo(uint8_t ch, uint8_t solo) {
    char cmd[20];
    String channel;
    String state;
    if (ch < 10) {
      channel = "0" + String(ch);
    }else{
      channel = String(ch);
    }
    if (solo > 0) {
      state = "ON";
    }else{
      state = "OFF";
    }
    String scmd = "-stat/solosw/" + channel + " " + state;
    scmd.toCharArray(cmd, 20);
    cmd[scmd.length()] = 0x10; // add linefeed
    xremoteSendBasicMessage("node", 's', 's', cmd);
  }

  void xremoteUpdateMeter() {
    int32_t value;

    uint16_t len = xremotesprint(xremote_TxMessage, 0, 's', "meters/0");
    len = xremotesprint(xremote_TxMessage, len, 's', ",b"); // 4 chars
    value = (70 + 1)*4; // number of bytes
    len = xremotesprint(xremote_TxMessage, len, 'b', (char*)&value); // big-endian
    value = 70; // number of floats
    len = xremotesprint(xremote_TxMessage, len, 'l', (char*)&value); // little endian

    float f;
    for (uint16_t i=0; i<70; i++) {
      f = playerinfo.vuMeterCh[i]/255.0f; // 32 channels, 8 aux, 8 FX returns, 16 busse, 6 matrix
      len = xremotesprint(xremote_TxMessage, len, 'l', (char*)&f); // little endian
    }

    xremoteSendUdpPacket(xremote_TxMessage, len);
  }

  ////////////////////////////////////
  // low-level communication functions
  ////////////////////////////////////

  void xremoteSendUdpPacket(char *buffer, uint16_t size) {
    bool connectionOK = xremoteUdp.beginPacket(xremoteUdp.remoteIP(), xremoteUdp.remotePort());

    if (connectionOK) {
      uint16_t writtenBytes = xremoteUdp.write(buffer, size);
  /*
      Serial.print("Sent: ");
      for (uint8_t i=0; i<size; i++){
        Serial.print(buffer[i]);
      }
      Serial.println("end");
  */

      if (writtenBytes > 0) {
        xremoteUdp.endPacket();
      }else{
        //Serial.println("Error: No bytes written");
      }
    }else{
      //Serial.println("Error: Connection not OK");
    }
  }

  void xremoteSendBasicMessage(char *cmd, char type, char format, char *value) {
    char tmp[2];
    tmp[0] = ',';
    tmp[1] = type;

    uint16_t len = xremotesprint(xremote_TxMessage, 0, 's', cmd);
    len = xremotesprint(xremote_TxMessage, len, 's', tmp);
    len = xremotesprint(xremote_TxMessage, len, format, value);
    xremoteSendUdpPacket(xremote_TxMessage, len);
  }

  uint16_t xremotesprint(char *bd, uint16_t index, char format, const char *bs) {
    /*
      Based on the work of Patrick-Gilles Maillot
      https://github.com/pmaillot/X32-Behringer/blob/master/X32.c
    */

    int i;
    // check format
    switch (format) {
      case 's': // string : copy characters one at a time until a 0 is found
        if (bs) {
          strcpy(bd+index, bs);
          index += (int)strlen(bs) + 1;
        } else {
          bd[index++] = 0;
        }
        // align to 4 bytes boundary if needed
        while (index & 3) bd[index++] = 0;
        break;
      case 'b': // float or int : copy the 4 bytes of float or int in big-endian order
        i = 4;
        while (i > 0)
          bd[index++] = (char)(bs[--i]);
        break;
      case 'l': // float or int : copy the 4 bytes of float or int in little-endian order
        i = 0;
        while (i < 4)
          bd[index++] = (char)(bs[i++]);
        break;
      default:
        // don't copy anything
        break;
    }
    return index;
  }

  uint16_t xremotefprint(char *bd, uint16_t index, char* text, char format, char *bs)
  {
    /*
      Based on the work of Patrick-Gilles Maillot
      https://github.com/pmaillot/X32-Behringer/blob/master/X32.c
    */

    // first copy text
    strcpy (bd+index, text);
    index += (int)strlen(text) + 1;
    // align to 4 bytes boundary if needed
    while (index & 3) bd[index++] = 0;
    // then set format, keeping #4 alignment
    bd[index++] = ',';
    bd[index++] = format;
    bd[index++] = 0;
    bd[index++] = 0;
    // based on format, set value
    return xremotesprint(bd, index, format, bs);
  }
#endif