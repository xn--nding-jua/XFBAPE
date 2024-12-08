/*
  X32 sends commands starting with a "*" and ending with a "#"
  This code implements a buffer for incoming characters at 38400 baud
  and scans for * and # and executes desired functions based on
  reverse engineered commands of the X32 using a X-LIVE card

  Supported original functions:
  - Play/Pause/Record/Delete file
  - Seeking
  - Set and recall markers
  - Format
  - Change USB-configuration
  - Switch between SD-Card and USB
  - Send current play-position

  Missing commands:
  - Firmwareupdate

  Todo:
  - Test commands

  These original functions are used to control the X-FBAPE MP3-player
  display TOC of the SD-Card and some other nice functions
*/

// Serial-CMD-Receiver
void handleX32Communication() {
  if (SerialX32.available() > 0) {
    // read all characters. The individual commands can be distributed over several chunks.
    // so we have to fill everything into a buffer and scan the buffer for "*" and "#"

    uint8_t rxChar = SerialX32.read();

    x32RingBuffer[x32RingBufferPointer++] = rxChar;

    if (x32RingBufferPointer >= X32_RINGBUF_LEN) {
      x32RingBufferPointer = 0;
    }

    // check for end of command (== '#')
    if (rxChar == 35) {
      x32SearchCmd();
    }
  }
}

// function to read around the end of the ring-buffer
uint16_t x32RingBufferPointerOverflow(uint16_t bufPointer) {
  if (bufPointer >= X32_RINGBUF_LEN) {
    return (bufPointer - X32_RINGBUF_LEN);
  }else{
    return bufPointer;
  }
}

// search for a valid command starting with "*" and ending with "#" and execute the command
void x32SearchCmd() {
  uint8_t i;
  uint8_t j;
  uint8_t k;
  uint8_t commandBeginIndex;
  uint8_t commandEndIndex;
  uint8_t commandLength;
  uint8_t commandArray[23]; // maximum command-length + 1
  String command;

  for (i=0; i<(X32_RINGBUF_LEN + 22); i++) { // increment i beyond the limit of the buffer to catch commands around the border
    if (x32RingBuffer[x32RingBufferPointerOverflow(i)] == 42) { // search for "*"
      // we found the begin of a command
      commandBeginIndex = x32RingBufferPointerOverflow(i);

      // now search for the end of the command from the current index
      for (j=commandBeginIndex; j<(X32_RINGBUF_LEN + 22); j++) {
        if (x32RingBuffer[x32RingBufferPointerOverflow(j)] == 35) {
          // we found a possible end of command. Lets copy the data and validate the command
          commandEndIndex = x32RingBufferPointerOverflow(j);

          if (commandBeginIndex < commandEndIndex) {
            // we are in the same direction -> just copy chars
            // ____*xxxxxx#_____
            commandLength = commandEndIndex - commandBeginIndex + 1;
            memcpy(&commandArray[0], &x32RingBuffer[commandBeginIndex], commandLength);
          }else if (commandBeginIndex > commandEndIndex) {
            // we have to copy the chars around the corner
            // xxx#_________*xxx
            commandLength = commandEndIndex + (X32_RINGBUF_LEN - commandBeginIndex) + 1;
            memcpy(&commandArray[0], &x32RingBuffer[commandBeginIndex], X32_RINGBUF_LEN - commandBeginIndex); // copy first part from the end
            memcpy(&commandArray[X32_RINGBUF_LEN - commandBeginIndex], &x32RingBuffer[0], commandEndIndex + 1); // copy last part from the beginning
          }

          // convert to String, execute the command and output the Answer
          command = String((char*)commandArray);
          String Answer = x32ExecCmd(command);
          if (Answer.length() > 0) {
            SerialX32.print(Answer);
          }
          if (x32Debug) {
            Serial.println("X32: " + command + " | X-FBAPE: " + Answer);
          }

          // delete used chars from ring-buffer
          for (k=i; k<(i+commandLength); k++) {
            x32RingBuffer[x32RingBufferPointerOverflow(k)] = 0;
          }
          // clear commandArray
          for (k=0; k<32; k++) {
            commandArray[k] = 0;
          }

          break;
        }
      }
    }
  }
}

// execute the command
String x32ExecCmd(String command) {
  String Answer = "";

  if (command.length()>2) {
    if (command.indexOf("*8I#")==0) {
      // I = IDENTIFY
      // X32 wants to know who we are
      Answer = "*8X-UREC:A:12#"; // we pretend to be an X-LIVE card with firmware-version A12 :)
    }else if (command.indexOf("*8R#")==0) {
      // X32 wants to know who we are - dont ask me, whats the difference between *8I# and *8R#
      Answer = "*8X-UREC:A:12#"; // we pretend to be an X-LIVE card with firmware-version A12 :)
    }else if (command.indexOf("*8C8")==0) {
      // switch between card and USB: *8C8od#
      uint8_t channelOption = command[4] - 48; // options between 0 and 5
      uint8_t deviceOption;
      if (command[5] == 85) {
        deviceOption = 0; // USB
      }else{
        deviceOption = 1; // SDCARD
      }
      Answer = "*8Y#";
    }else if (command.indexOf("*9X")==0) {
      // delete file
      String filename = command.substring(3, command.indexOf("#"));
      Answer = "*9Y00#";
    }else if (command.indexOf("*9H")==0) {
      // start recording
      String filename = command.substring(3, command.indexOf("#")-3);
      uint8_t channelCount = command.substring(command.indexOf("#")-3, command.indexOf("#")-1).toInt();
      Answer = "*9Y00#";
    }else if (command.indexOf("*9F#")==0) {
      // stop recording

      Answer = "*9Y00#";

      /*
        Card now Answers with a bunch of results
        *9N24003F38D0# -> probably info-command for new entries?

        *9N0003A6DD80#
        *9N0003A6D300#

        *9N24003F47D0#

        *9N0003A6D300#
        *9N24003F50A0#

        *9N24003F56D0#
      */
    }else if (command.indexOf("*9D#")==0) {
      // play file
      Answer = "*9D00#";

      // during playback, the card should send current position as samples every 85ms:
      //SerialX32.print("*9N22xxxxxxxx#");
      // this is done via the 85ms-timer in the SAMD.ino
    }else if (command.indexOf("*9M")==0) {
      // seek to specific sample based on 48kHz
      uint32_t sampleIndex = hexToInt(command.substring(3, command.indexOf("#")));

      Answer = "*9M00#";
    }else if (command.indexOf("*9Q~#")==0) { // TODO: check command. Maybe it is *9Q0# for card1 and *9Q1# for card2?
      // format SD-Card

      Answer = "*9Y00#*9N0700000000#"; // TODO: check response. Maybe it should be *9Y00# for card1 and *9Y01# for card2?
    }else if (command.indexOf("*9I")==0) {
      // create new Marker
      uint32_t sampleIndex = hexToInt(command.substring(3, command.indexOf("#")));

      Answer = "*9Y00#";
    }else if (command.indexOf("*9B")==0) {
      // select file
      String filename = command.substring(3, command.indexOf("#")-3);

      // prepare Answer
      uint32_t markerCount = 0; // X32 will request markers even when this is set to 0
      uint32_t channelCount = 32;
      // *9B0000232000F4E900000000000#
      // *9B000 MARKERCOUNT CHANNELCOUNT NUMBEROFTOTALSAMPLES(?)
      Answer = "*9B000" + intToHex(markerCount, 2) + String(channelCount) + "000000000000000000#";
    }else if (command.indexOf("*9C")==0) {
      // request marker
      uint32_t markerIndex = hexToInt(command.substring(3, command.indexOf("#")));

      // here we can look up desired marker, return timeIndex and wait for the next command
      // maybe MP3-TOC can be used for this if file-handling is not working well
      uint32_t timeIndex = 0; // if timeIndex == 0 X32 will handle this as "no marker"

      Answer = "*9C00" + intToHex(markerIndex, 2) + intToHex(timeIndex, 8) + "#";
    }else if (command.indexOf("*9R")==0) {
      // select SD-Card
      x32currentCardSelection = command[3] - 48; // either 0 or 1

      Answer = "*9R00#";
    }else if (command.indexOf("*9AF#")==0) {
      // request first entry of TOC
      tocCounter = 0; // reset tocCounter as we are requesting first entry

      String TOCtest = "5977A2A0|5977A2A4|5977A52B Part 1|Test|";
      String title = split(TOCtest, '|', tocCounter); // name of title0

      Answer = "*9ASF" + title + "#";
    }else if (command.indexOf("*9AN#")==0) {
      // request followup-titles of TOC
      tocCounter++;
      if (tocCounter<tocEntries) {
        String TOCtest = "5977A2A0|5977A2A4|5977A52B Part 1|Test|";
        String title = split(TOCtest, '|', tocCounter); // name of titleX

        Answer = "*9ASN" + title + "#";
      }else{
        // no more entries available
        Answer = "*9AEN00#";
      }
    }else if (command.indexOf("*9N")==0) {
      // N for german "NOCH" = remaining?
      // X32 requests size of SD-card
      uint8_t cardNumber = command[3] - 48; // either 0 or 1
      uint32_t cardSize = x32CardSize[cardNumber];

      uint32_t usedSpace = 2097152; // 2GB

      Answer = "*9N" + String(cardNumber) + "0" + intToHex((cardSize-usedSpace)*2, 8) + intToHex(usedSpace*2, 8) + "#";
    }else if (command.indexOf("*9G")==0) {
      // G for german "GESAMT" = total?
      uint8_t cardNumber = command[3] - 48; // either 0 or 1
      uint32_t cardSize = x32CardSize[cardNumber];// "03B70600" for a 32GB card!? TODO: check the calculation. seems to be 2*32GB

      Answer = "*9G" + String(cardNumber) + "0" + intToHex(cardSize*2, 8) + "#";
    }else if (command.indexOf("G")==2) {
      // received one of the initialization-commands *0G00000# ... *3G70000#
      // the usage is unclear so far
      uint8_t index = command[1] - 48; // index between 0 and 3
      uint8_t value = command[3] - 48; // value between 0 and 7

      // the X-LIVE is not responding to these commands so we either

      Answer = ""; // TODO: check if we have to Answer this command
    }else{
      // Error: unknown command -> help!
    }
  }else{
    // command to short
  }

  return Answer;
}

void x32Init() {
  SerialX32.print("*8BE#");
  delay(1000);
  SerialX32.print("*9Z00#*9N010000000000000000#*9N110000000000000000#"); // no card present
  delay(67);
  SerialX32.print("*8BE#");
  delay(500);
}

void x32NewCard(uint8_t cardIndex) {
  SerialX32.print("*9N" + String(cardIndex) + "000007F4000000000#"); // 32576 could be MegaBytes?
}