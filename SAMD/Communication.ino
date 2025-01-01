// USB-CMD-Receiver
void handleUSBCommunication() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // we are using both CR/LF but we have to read until LF
    command.trim();

    if (command.indexOf("samd:") == 0) {
      // command begins with "samd:" so we have to interprete it
      Serial.println(executeCommand(command));
    }else{
      // we received a command for NINA-module or FPGA -> passthrough command via Serial2 to NINA-Module
      Serial2.println(command); // "\n" has been truncated from command, so we have to use println() again
      Serial.println(Serial2.readStringUntil('\n')); // receive answer
    }
  }
}

// NINA-CMD-Receiver
void handleNINACommunication() {
  // passthrough all incoming data to USB-serial
  if (Serial2.available() > 0) {
    if (passthroughNINA) {
      //Serial.write(Serial2.read()); // this prevents us from receiving commands from NINA
      String command = Serial2.readStringUntil('\n');
      command.trim();
      
      if (command.indexOf(F("samd:")) == 0) {
        // command begins with "samd:" so we have to interprete it
        Serial2.println(executeCommand(command));
      }else{
        // passthrough command to USB
        Serial.println(command);
      }
    }else{
      String command = Serial2.readStringUntil('\n');
      command.trim();

      if (command.indexOf("OK") > -1) {
        // received "OK" from NINA -> last command is OK
      }else if (command.indexOf("ERROR") > -1) {
        // received "ERROR" from NINA -> last command had an error
      }else if (command.indexOf("UNKNOWN_CMD") > -1) {
        // NINA received some garbage or communication-problem -> ignore it for the moment
      }else{
        // this is a command for our command-processor
        Serial2.println(executeCommand(command));
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
      playerinfo.time = split(ParameterString, ',', 1).toInt();
      playerinfo.duration = split(ParameterString, ',', 2).toInt();
      playerinfo.progress = split(ParameterString, ',', 3).toInt();
      playerinfo.volumeMain = split(ParameterString, ',', 4).toFloat();
      playerinfo.balanceMain = split(ParameterString, ',', 5).toInt();
      playerinfo.volumeSub = split(ParameterString, ',', 6).toFloat();
      playerinfo.volumeCard = split(ParameterString, ',', 7).toFloat();
      playerinfo.volumeBt = split(ParameterString, ',', 8).toFloat();
      playerinfo.audioStatusInfo = split(ParameterString, ',', 9).toInt(); // contains infos about noisegate, clip (L/R/S) and compression (LR/S)
      TOC = split(ParameterString, ',', 10);
      tocEntries = getNumberOfTocEntries('|');

      // receive volumeCh[] as parameters 11 to 42 as float-values
      for (uint8_t i=0; i<32; i++) {
        playerinfo.volumeCh[i] = split(ParameterString, ',', 11 + i).toFloat();
      }

      String balanceInfo = split(ParameterString, ',', 43); // 64-char HEX-String containing 32 values for balance
      String vuMeterInfo = split(ParameterString, ',', 44); // 64-char HEX-String containing 32 values for vuMeter
      for (uint8_t i=0; i<32; i++) {
        // receive balance[] as parameter 44 as concatenated HEX-Strings
        playerinfo.balanceCh[i] = hexToInt((String)balanceInfo[i * 2] + (String)balanceInfo[i * 2 + 1]);

        // receive vuMeterCh[] as parameter 45 as concatenated HEX-Strings
        playerinfo.vuMeterCh[i] = hexToInt((String)vuMeterInfo[i * 2] + (String)vuMeterInfo[i * 2 + 1]);
      }

      // send time to X32 when progress is above 0
      x32Playback = (playerinfo.progress > 0);

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
      enterNinaUpdateMode(true);
      Answer = F("Entered NINA-Update-Mode...\nPlease close serial-port and upload new firmware via Arduino or esptool.py.\n\nReboot system to return to normal mode.");
    }else if (Command.indexOf("samd:reset:nina") > -1){
      resetNina();
      Answer = "SAMD: OK";
    }else if (Command.indexOf("samd:passthrough:nina") > -1){
      passthroughNINA = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
      Answer = "SAMD: OK";
    }else if (Command.indexOf(F("samd:debug:x32")) > -1) {
      x32Debug = (Command.substring(Command.indexOf("@")+1).toInt() == 1);
      Answer = "SAMD: OK";
    }else if (Command.indexOf(F("samd:config:mac")) > -1) {
      // requesting the MAC-Address from EEPROM
      Answer = mac2String(config.mac);
    }else if (Command.indexOf(F("samd:config:ip")) > -1) {
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
      initEthernet();
      Answer = "SAMD: OK";
    #if USE_XTOUCH == 1
      }else if (Command.indexOf("samd:config:set:xtouchip") > -1){
        //samd:config:set:xtouchip1@000.000.000.000
        uint8_t i_xtouch = Command.substring(24, Command.indexOf("@")).toInt() - 1;
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
        XCtl_init(i_xtouch);
        Answer = "SAMD: OK";
    #endif
    #if USE_MACKIE_MCU == 1 || USE_XTOUCH == 1
      }else if (Command.indexOf("samd:setname:ch") > -1){
        //samd:setname:ch1@Ch 1
        uint16_t channel = Command.substring(15, Command.indexOf("@")).toInt();
        String name = Command.substring(Command.indexOf("@")+1);
        MackieMCU.channel[channel].name = name;

        Answer = "SAMD: OK";
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
  Serial2.IrqHandler();
}

/*
// Interrupt handler for SERCOM3
void SERCOM5_Handler()
{
  Serial4.IrqHandler();
}
*/

void resetNina() {
  pinMode(NINA_RESET_N, OUTPUT);

  digitalWrite(NINA_RESET_N, HIGH);
  delay(100);
  digitalWrite(NINA_RESET_N, LOW);
  delay(100);
  digitalWrite(NINA_RESET_N, HIGH);
  delay(100);

  //pinMode(NINA_RESET_N, INPUT);
}

void enterNinaUpdateMode(bool performReset) {
  if (!performReset) {
    // stop mainCtrl
    Serial2.println(F("system:stop"));
  }

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

  if (performReset) {
    // perform a reset of the NINA-module
    delay(100);
    digitalWrite(NINA_RESET_N, LOW);
    delay(100);
    digitalWrite(NINA_RESET_N, HIGH);
    delay(100);
  }

  firmwareUpdateMode = true; // entering NINA-Update-Mode
}

void handleNINAUpdate() {
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
    Serial2.write(Serial.read());
  }
  // copy bytes from NINA to USB
  if (Serial2.available()) {
    Serial.write(Serial2.read());
  }

  // check if the USB virtual serial wants a new baud rate
  if (Serial.baud() != baud) {
    rts = -1;
    dtr = -1;

    baud = Serial.baud();
    Serial2.flush();
    Serial2.begin(baud);
    pinPeripheral(0, PIO_SERCOM); //Assign TX function to pin 0
    pinPeripheral(1, PIO_SERCOM); //Assign RX function to pin 1
  }
}