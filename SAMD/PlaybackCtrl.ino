/*
  As the X32-functions are located at the SAMD21 we have to place the playback-controls in the SAMD21
  at the moment. This is not very nice, but works for now. In a later release we can pack this functions
  to the NINA module again to keep all playback-functions in one controller
*/

void playbackSelectTitle(uint8_t number) {
  // decode desired TrackNumber from Timecode (we are using minutes as storage)
  playerinfo.currentTrackNumber = number;
  // now get the fileName from TOC
  String filename = split(TOC, '|', playerinfo.currentTrackNumber); // name of title0
  // play the file
  SerialNina.println("player:file@" + filename); // load file (and file will be played immediatyl)
  SerialNina.println("player:pause"); // stop file

  //SerialNina.println("player:select@" + String(number));
}

void playbackNextTitle() {
  if (playerinfo.currentTrackNumber < (tocEntries - 1)) {
    playerinfo.currentTrackNumber += 1;
  }
  // now get the fileName from TOC
  String filename = split(TOC, '|', playerinfo.currentTrackNumber); // name of title0
  // play the file
  SerialNina.println("player:file@" + filename); // load file (and file will be played immediatyl)

  //SerialNina.println("player:next");
}

void playbackPrevTitle() {
  if (playerinfo.currentTrackNumber > 0) {
    playerinfo.currentTrackNumber -= 1;
  }
  // now get the fileName from TOC
  String filename = split(TOC, '|', playerinfo.currentTrackNumber); // name of title0
  // play the file
  SerialNina.println("player:file@" + filename); // load file (and file will be played immediatyl)

  //SerialNina.println("player:prev");
}

void playbackStop() {
  SerialNina.println("player:stop");
}

void playbackPlayPause() {
  SerialNina.println("player:pause");
}