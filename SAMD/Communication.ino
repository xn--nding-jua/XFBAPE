// USB-CMD-Receiver
void usbHandleCommunication() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // we are using both CR/LF but we have to read until LF
    command.trim();

    if (command.indexOf("samd:") == 0) {
      // command begins with "samd:" so we have to interprete it
      Serial.println(executeCommand(command));
    }else{
      // we received a command for NINA-module or FPGA -> passthrough command via Serial2 to NINA-Module
      SerialNina.println(command); // "\n" has been truncated from command, so we have to use println() again
      Serial.println(SerialNina.readStringUntil('\n')); // receive answer
    }
  }
}

// NINA-CMD-Receiver
void ninaHandleCommunication() {
  // passthrough all incoming data to USB-serial
  if (SerialNina.available() > 0) {
    String command = SerialNina.readStringUntil('\n');
    command.trim();

    if (command.indexOf(F("samd:")) == 0) {
      // command begins with "samd:" so we have to interprete it
      SerialNina.println(executeCommand(command));
      
      if (passthroughNINADebug) {
        Serial.println(command); // regular commands to samd will be filtered by default
      }
    }else{
      // received some status-infos
      if (passthroughNINADebug || (passthroughNINA && ((command.indexOf("OK") == -1) && (command.indexOf("ERROR") == -1) && (command.indexOf("UNKNOWN_CMD") == -1)))) {
        // passthrough to USB
        Serial.println(command);
      }
    }
  }
}

// command-interpreter
String executeCommand(String Command) {
  String Answer;

  if (Command.length()>2){
    // we got a new command. Lets find out what we have to do today...

    if (Command.indexOf(F("samd:update:info")) > -1){
      String ParameterString = Command.substring(Command.indexOf("@")+1);

      playerinfo.title = split(ParameterString, ',', 0);
      playerinfo.currentTrackNumber = split(ParameterString, ',', 1).toInt();
      playerinfo.time = split(ParameterString, ',', 2).toInt();
      playerinfo.duration = split(ParameterString, ',', 3).toInt();
      playerinfo.progress = split(ParameterString, ',', 4).toInt();
      playerinfo.volumeMain = split(ParameterString, ',', 5).toFloat();
      playerinfo.balanceMain = split(ParameterString, ',', 6).toInt();
      playerinfo.volumeSub = split(ParameterString, ',', 7).toFloat();
      playerinfo.volumeCard = split(ParameterString, ',', 8).toFloat();
      playerinfo.volumeBt = split(ParameterString, ',', 9).toFloat();
      playerinfo.audioStatusInfo = split(ParameterString, ',', 10).toInt(); // contains infos about noisegate, clip (L/R/S) and compression (LR/S)
      TOC = split(ParameterString, ',', 11);
      tocEntries = getNumberOfTocEntries('|');

      // send time to X32 when progress is above 0
      x32Playback = (playerinfo.progress > 0);

      Answer = "SAMD: OK";
    }else if (Command.indexOf(F("samd:update:ch")) > -1){
      String ParameterString = Command.substring(Command.indexOf("@")+1);

      String vuMeterInfo = split(ParameterString, ',', 0); // 64-char HEX-String containing 32 values for vuMeter
      for (uint8_t i=0; i<32; i++) {
        // receive vuMeterCh[] as parameter 0 as concatenated HEX-Strings
        playerinfo.vuMeterCh[i] = hexToInt((String)vuMeterInfo[i * 2] + (String)vuMeterInfo[i * 2 + 1]); // 64 bytes
      }

/*
      String balanceInfo = split(ParameterString, ',', 1); // 64-char HEX-String containing 32 values for balance
      for (uint8_t i=0; i<32; i++) {
        // receive balance[] as parameter 1 as concatenated HEX-Strings
        playerinfo.balanceCh[i] = hexToInt((String)balanceInfo[i * 2] + (String)balanceInfo[i * 2 + 1]); // 64 bytes
      }

      // receive volumeCh[] as parameters 2 to 33 as float-values
      for (uint8_t i=0; i<32; i++) {
        playerinfo.volumeCh[i] = split(ParameterString, ',', 2 + i).toFloat(); // max. 192 bytes
      }
*/
      Answer = "SAMD: OK";
    #if USE_DISPLAY == 1
      }else if (Command.indexOf(F("samd:player:updatedisplay")) > -1){
        displayDrawMenu();
        Answer = "SAMD: OK";
    #endif
    }else if (Command.indexOf("samd:ver?") > -1){
      // return version of controller
      Answer = versionstring;
    }else if (Command.indexOf("samd:*IDN?") > -1){
      // return some short info-text for the GUI
      Answer = "X-f/bape USBCtrl " + String(versionstring) + " built on " + String(compile_date);
    }else if (Command.indexOf("samd:info?") > -1){
      // return some info-text for the GUI
      Answer = "X-f/bape USB-Controller " + String(versionstring) + " built on " + String(compile_date) + "\n(c) Dr.-Ing. Christian Noeding\nhttps://www.github.com/xn--nding-jua/xfbape";
    }else if (Command.indexOf("samd:version:main") > -1){
      playerinfo.MainCtrlVersion = Command.substring(Command.indexOf("@")+1);
      Answer = "SAMD: OK";
    }else if (Command.indexOf("samd:version:fpga") > -1){
      playerinfo.FPGAVersion = Command.substring(Command.indexOf("@")+1);
      Answer = "SAMD: OK";
    }else if (Command.indexOf("samd:reset:fpga") > -1){
      setup_fpga();
      Answer = "SAMD: OK";
    }else if (Command.indexOf(F("samd:update:nina")) > -1){
      ninaEnterUpdateMode();
      Answer = F("Entered NINA-Update-Mode...\nPlease close serial-port and upload new firmware via Arduino or esptool.py.\n\nReboot system to return to normal mode.");
    }else if (Command.indexOf("samd:reset:nina") > -1){
      ninaReset();
      Answer = "SAMD: OK";
    }else if (Command.indexOf("samd:passthrough:nina") > -1){
      passthroughNINA = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
      Answer = "SAMD: OK";
    }else if (Command.indexOf("samd:debug:nina") > -1){
      passthroughNINADebug = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
      Answer = "SAMD: OK";
    }else if (Command.indexOf(F("samd:debug:x32")) > -1) {
      x32Debug = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
      Answer = "SAMD: OK";
    }else if (Command.indexOf(F("samd:config:mac?")) > -1) {
      // requesting the MAC-Address from EEPROM
      Answer = mac2String(config.mac);
    }else if (Command.indexOf(F("samd:config:ip?")) > -1) {
      // requesting the current IP-address
      Answer = IpAddress2String(eeprom_config.ip);
    }else if (Command.indexOf("samd:config:set:ip") > -1){
      //samd:config:set:ip@000.000.000.000
	    String ipString = Command.substring(Command.indexOf("@")+1) + '.'; // adding leading dot at the end to ensure function of split()
      uint8_t ip0 = split(ipString, '.', 0).toInt();
      uint8_t ip1 = split(ipString, '.', 1).toInt();
      uint8_t ip2 = split(ipString, '.', 2).toInt();
      uint8_t ip3 = split(ipString, '.', 3).toInt();

      eeprom_config.ip = IPAddress(ip0, ip1, ip2, ip3);
      ethernetInit();
      Answer = "SAMD: OK";
    #if USE_XTOUCH == 1
      }else if (Command.indexOf(F("samd:config:xtouchip?")) > -1) {
        // samd:config:xtouchip?
        // requesting the current IP-address of specific xtouch
        String ips = IpAddress2String(XCtl[0].ip);
        for (uint8_t i=1; i<XTOUCH_COUNT; i++) {
          ips = ips + ", " + IpAddress2String(XCtl[i].ip);
        }
        Answer = ips;
      }else if (Command.indexOf("samd:config:set:xtouchip") > -1){
        //samd:config:set:xtouchip1@000.000.000.000
        uint8_t i_xtouch = Command.substring(24, Command.indexOf("@")).toInt() - 1;
        if ((i_xtouch >=0 ) && (i_xtouch < XTOUCH_COUNT)) {
          String ipString = Command.substring(Command.indexOf("@")+1) + '.'; // adding leading dot at the end to ensure function of split()
          uint8_t ip0 = split(ipString, '.', 0).toInt();
          uint8_t ip1 = split(ipString, '.', 1).toInt();
          uint8_t ip2 = split(ipString, '.', 2).toInt();
          uint8_t ip3 = split(ipString, '.', 3).toInt();
          IPAddress newIp = IPAddress(ip0, ip1, ip2, ip3);

          eeprom_config.xtouchip[i_xtouch] = newIp;
          XCtlUdp[i_xtouch].stop();
          delay(10);
          XCtl[i_xtouch].ip = newIp;
          xctlInit(i_xtouch);
          Answer = "SAMD: OK";
        }else{
          Answer = "SAMD: ERROR - OUT OF RANGE";
        }
      }else if (Command.indexOf("samd:xtouch:reconnect") > -1){
        for (uint8_t i_xtouch=0; i_xtouch<XTOUCH_COUNT; i_xtouch++) {
          XCtlWatchdogCounter[i_xtouch] = 5; // 500ms to resend watchdog-message
          XCtl[i_xtouch].online = true; // enable device-communication
        }
        Answer = "SAMD: OK";
    #endif
    #if USE_MACKIE_MCU == 1 || USE_XTOUCH == 1
      }else if (Command.indexOf("samd:setname:ch") > -1){
        //samd:setname:ch1@Ch 1
        uint8_t channel = Command.substring(15, Command.indexOf("@")).toInt() - 1;
        if ((channel >= 0) && (channel < 32)) {
          String name = Command.substring(Command.indexOf("@")+1);
          MackieMCU.channel[channel].name = name;
          #if USE_XREMOTE == 1
            xremoteSetName(channel+1, MackieMCU.channel[channel].name);
          #endif

          Answer = "SAMD: OK";
        }else{
          Answer = "SAMD: ERROR - OUT OF RANGE";
        }
      }else if (Command.indexOf("samd:setcolor:ch") > -1){
        //samd:setcolor:ch1@7
        uint8_t channel = Command.substring(16, Command.indexOf("@")).toInt() - 1;
        if ((channel >= 0) && (channel < 32)) {
          uint8_t color = Command.substring(Command.indexOf("@")+1).toInt();
          MackieMCU.channel[channel].color = color;
          #if USE_XREMOTE == 1
            xremoteSetColor(channel+1, MackieMCU.channel[channel].color);
          #endif

          Answer = "SAMD: OK";
        }else{
          Answer = "SAMD: ERROR - OUT OF RANGE";
        }
      }else if (Command.indexOf("samd:setname:dmxch") > -1){
        //samd:setname:dmxch1@Ch 1
        uint16_t dmxChannel = Command.substring(18, Command.indexOf("@")).toInt() - 1;
        if ((dmxChannel >= 0) && (dmxChannel < 512)) {
          String name = Command.substring(Command.indexOf("@")+1);
          MackieMCU.channelDmx[dmxChannel].name = name;

          Answer = "SAMD: OK";
        }else{
          Answer = "SAMD: ERROR - OUT OF RANGE";
        }
      }else if (Command.indexOf("samd:setcolor:dmxch") > -1){
        //samd:setcolor:dmxch1@7
        uint16_t dmxChannel = Command.substring(19, Command.indexOf("@")).toInt() - 1;
        if ((dmxChannel >= 0) && (dmxChannel < 512)) {
          uint8_t color = Command.substring(Command.indexOf("@")+1).toInt();
          MackieMCU.channelDmx[dmxChannel].color = color;

          Answer = "SAMD: OK";
        }else{
          Answer = "SAMD: ERROR - OUT OF RANGE";
        }
    #endif
    }else if (Command.indexOf("samd:config:save") > -1){
      saveConfig();
      Answer = "SAMD: OK";
    }else{
      // unknown command
      Answer = "SAMD: UNKNOWN_CMD";
    }
  }else{
    Answer = "SAMD: ERROR";
  }

  return Answer;
}

// Interrupt handler for SERCOM3
void SERCOM3_Handler()
{
  SerialNina.IrqHandler();
}

/*
// Interrupt handler for SERCOM3
void SERCOM5_Handler()
{
  Serial4.IrqHandler();
}
*/

void ninaReset() {
  pinMode(NINA_RESET_N, OUTPUT);

  digitalWrite(NINA_RESET_N, HIGH);
  delay(100);
  digitalWrite(NINA_RESET_N, LOW);
  delay(100);
  digitalWrite(NINA_RESET_N, HIGH);
  delay(100);

  //pinMode(NINA_RESET_N, INPUT);
}

void ninaEnterUpdateMode() {
  SerialNina.println(F("system:stop"));

  // switch back to 115200 baud to communicate with the ESP32 bootloader
  SerialNina.flush();
  SerialNina.begin(115200);
  pinPeripheral(0, PIO_SERCOM); //Assign TX function to pin 0
  pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1

  #if USE_DISPLAY == 1
    // disable ticker that access I2C
    ticker100ms.stop();

    // show update-information on display
    display.clearDisplay();
    currentDisplayLine = 0;
    displayPrintLn(F("< NINA Updatemode >"));
    displayPrintLn("");
    displayPrintLn(F("Please upload new"));
    displayPrintLn(F("Firmware and reboot"));
    displayPrintLn(F("the system..."));

    // stop I2C as SDA is connected to Bootselect-Pin of NINA-W102
    Wire.end();
    delay(1000);
  #endif
  
  // set reset- and bootmode-pins into output-mode
  pinMode(NINA_PIO27, OUTPUT);
  pinMode(NINA_RESET_N, OUTPUT);
  digitalWrite(NINA_PIO27, LOW); // PIO27 = LOW = Bootstrap pin for NINA-W102 -> put into Bootloader-Mode (Factory Boot)
  digitalWrite(NINA_RESET_N, HIGH);

  // perform a reset of the NINA-module
  delay(100);
  digitalWrite(NINA_RESET_N, LOW);
  delay(100);
  digitalWrite(NINA_RESET_N, HIGH);
  delay(100);

  firmwareUpdateMode = true; // entering NINA-Update-Mode
}

void ninaHandleUpdate() {
  // use RTS to RESET the NINA-module
  if (rts != Serial.rts()) {
    digitalWrite(NINA_RESET_N, (Serial.rts() == 1) ? LOW : HIGH);
    rts = Serial.rts();
  }

  // use DTR to select/deselect Bootloader-Mode
  if (dtr != Serial.dtr()) {
    digitalWrite(NINA_PIO27, (Serial.dtr() == 1) ? LOW : HIGH);
    dtr = Serial.dtr();
  }

  // copy bytes from USB to NINA
  if (Serial.available()) {
    SerialNina.write(Serial.read());
  }
  // copy bytes from NINA to USB
  if (SerialNina.available()) {
    Serial.write(SerialNina.read());
  }

  // check if the USB virtual serial wants a new baud rate
  if (Serial.baud() != baud) {
    rts = -1;
    dtr = -1;

    baud = Serial.baud();
    SerialNina.flush();
    SerialNina.begin(baud);
    pinPeripheral(0, PIO_SERCOM); //Assign TX function to pin 0
    pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1
  }
}