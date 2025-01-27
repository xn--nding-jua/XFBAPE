void mixerSetVolume(uint8_t channel, float volume) {
  playerinfo.volumeCh[channel] = volume; // we are receiving this value from NINA with a bit delay again

  // send new value to NINA
  SerialNina.println("mixer:volume:ch" + String(channel + 1) + "@" + String(volume, 2));

  #if USE_XREMOTE == 1
    xremoteSetFader(channel+1, (volume + 48.0f)/54.0f);
  #endif
}

void mixerSetBalance(uint8_t channel, uint8_t balance) {
  playerinfo.balanceCh[channel] = balance; // we are receiving this value from NINA with a bit delay again

  // send value to NINA
  SerialNina.println("mixer:balance:ch" + String(channel + 1) + "@" + String(balance));

  #if USE_XREMOTE == 1
    xremoteSetPan(channel+1, balance / 100.0f);
  #endif
}

void mixerSetMainVolume(float volume) {
  playerinfo.volumeMain = volume; // we are receiving this value from NINA with a bit delay again

  // send new value to NINA
  SerialNina.println("mixer:volume:main@" + String(volume, 2));

  #if USE_XREMOTE == 1
    xremoteSetMainFader((volume + 48.0f)/54.0f);
  #endif
}

void mixerSetMainBalance(uint8_t balance) {
  playerinfo.balanceMain = balance; // we are receiving this value from NINA with a bit delay again

  // send value to NINA
  SerialNina.println("mixer:balance:main@" + String(balance));

  #if USE_XREMOTE == 1
    xremoteSetMainPan(balance / 100.0f);
  #endif
}

void mixerSetMute(uint8_t channel, bool mute) {
  if (mute) {
    MackieMCU.channel[channel].mute = 127;
  }else{
    MackieMCU.channel[channel].mute = 0;
  }

  // send value to NINA
  SerialNina.println("mixer:mute:ch" + String(channel+1) + "@" + String(mute));

  // update X32Edit
  #if USE_XREMOTE == 1
    xremoteSetMute(channel+1, MackieMCU.channel[channel].mute);
  #endif
}

void mixerSetSolo(uint8_t channel, bool solo) {
  if (solo) {
    MackieMCU.channel[channel].solo = 127;
  }else{
    MackieMCU.channel[channel].solo = 0;
  }

  // send value to NINA
  SerialNina.println("mixer:solo:ch" + String(channel+1) + "@" + String(solo));

  // update X32Edit
  #if USE_XREMOTE == 1
    xremoteSetSolo(channel+1, MackieMCU.channel[channel].solo);
  #endif
}
