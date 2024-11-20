void initEthernet() {
  Ethernet.init(7);

  Ethernet.begin(config.mac, eeprom_config.ip);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    // Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    // Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  cmdserver.begin();
}

// Webserver
void HandleHTTPClients() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    if (client.available()) {
      char c = client.read();
      // if you've gotten to the end of the line (received a newline
      // character) and the line is blank, the HTTP request has ended,
      // so you can send a reply
      if (c == '\n' && currentLineIsBlank) {
        // send a standard HTTP response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println("Refresh: 5");  // refresh the page automatically every 5 sec
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<head><title>X-f/bape</title></head>");
        client.println("<html><center><font face=\"Arial\" size=\"6\">");
        client.println("<b>X-fbape<br>32-channel 48kHz/24bit Audio-Card for Behringer X32</b>");
        client.println("</font></center></html>");
      }
      if (c == '\n') {
        // you're starting a new line
        currentLineIsBlank = true;
      } else if (c != '\r') {
        // you've gotten a character on the current line
        currentLineIsBlank = false;
      }
    }

    // close the connection:
    client.stop();
  }
}

// Command-Server
void HandleCMDClients() {
  // listen for incoming clients
  EthernetClient client = cmdserver.available();
  if (client) {
    // we have an active connection
    if (client.available()) {
      // we have unread data
      client.println(executeCommand((client.readStringUntil('\n') + ","))); // add leading "," to the command
    }
  }
}
