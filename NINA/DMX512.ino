#if USE_DMX512 == 1
  void initDmx512() {
    xTaskCreatePinnedToCore(
      Dmx512TaskFcn, // Function to implement the task
      "Dmx512Task", // Name of the task
      10000,  // Stack size in words
      NULL,  // Task input parameter
      tskIDLE_PRIORITY,  // Priority of the task: tskIDLE_PRIORITY, 1, 2
      &Dmx512Task,  // Task handle.
      1  // Core where the task should run
    );
  }

  void Dmx512TaskFcn(void *pvParameters) {
    // init DMX512
    dmx_config_t config = DMX_CONFIG_DEFAULT;
    dmx_personality_t personalities[] = {};
    int personality_count = 0;
    dmx_driver_install(dmxPort, &config, personalities, personality_count);
    dmx_set_pin(dmxPort, DMX512_TX_PIN, DMX512_RX_PIN, DMX512_EN_PIN);

    while(true) {
      // send buffer to hardware
      dmx_send(dmxPort);
      //dmx_send_num(dmxPort, DMX_PACKET_SIZE); // send limited number of channels

      // block task until dmx has been sent
      dmx_wait_sent(dmxPort, DMX_TIMEOUT_TICK);

      // write data-array to buffer
      dmx_write(dmxPort, dmxData, DMX_PACKET_SIZE);
    }
  }
#endif

#if USE_DMX512_RX == 1
  void initDmx512Receiver() {
    xTaskCreatePinnedToCore(
      Dmx512ReceiverTaskFcn, // Function to implement the task
      "Dmx512ReceiverTask", // Name of the task
      10000,  // Stack size in words
      NULL,  // Task input parameter
      tskIDLE_PRIORITY,  // Priority of the task: tskIDLE_PRIORITY, 1, 2
      &Dmx512ReceiverTask,  // Task handle.
      1  // Core where the task should run
    );
  }

  void Dmx512ReceiverTaskFcn(void *pvParameters) {
    // init DMX512 Receiver

    while(true) {
      if (dmx_receive(dmxPort, &DmxReceiverPacket, DMX_TIMEOUT_TICK)) {
        if (DmxReceiverPacket.err == DMX_OK) {
          dmx_read(dmxPort, dmxRxData, DmxReceiverPacket.size);
        }
      }
    }
  }
#endif
