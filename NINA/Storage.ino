bool initStorage() {
  #if USE_STORAGE == 0 // SD
    // setup the chip-select-pin
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    // init SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI); // (int8_t sck=-1, int8_t miso=-1, int8_t mosi=-1, int8_t ss=-1)
    SPI.setFrequency(SPI_CLOCK);

    // init SD-card
    if (SD.begin(SD_CS, SPI, SPI_CLOCK)) { //(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=4000000, const char * mountpoint="/sd", uint8_t max_files=5, bool format_if_empty=false);
      // init done successfully
      return true;
    }else{
      // an error occurred during SD-card-initialization
      return false;
    }
  #elif USE_STORAGE == 1 // SDMMC
    // configure pins
    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DAT0, SDMMC_DAT1, SDMMC_DAT2, SDMMC_DAT3);

    // initialize SDMMC-mode
    if (SD_MMC.begin()) {
      // card mounted successfully
      //uint8_t cardType = SD_MMC.cardType(); // cardType == CARD_NONE || CARD_MMC || CARD_SD || CARD_SDHC
      //uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024); // cardSize inMiB
      return true;
    }else{
      // card mount failed
      return false;
    }
  #elif USE_STORAGE == 2 // FatFS
    // initialize FFat
    if (FFat.begin(false, "/fatfs", 10, "ffat")) { // FFat.begin(bool formatOnFail = false, const char *basePath = "/ffat", uint8_t maxOpenFiles = 10, const char *partitionLabel = (char *)FFAT_PARTITION_LABEL);
      // init done successfully
      return true;
    }else{
      // an error occurred during SD-card-initialization
      return false;
    }
  #elif USE_STORAGE == 3 // LittleFS
    if (LittleFS.begin(false, "/littlefs", 10, "spiffs")) { // LittleFS.begin(bool formatOnFail = false, const char *basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char *partitionLabel = "spiffs");
      // init done successfully
      return true;
    }else{
      // an error occurred during SD-card-initialization
      return false;
    }
  #endif
}

uint16_t SD_getNumberOfFiles(File dir, String filter) {
  // at the moment we are supporting only files in the root-directory

  uint16_t fileCount = 0;

  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    if (!entry.isDirectory()) {
      if ((filter.length() == 0) || (String(entry.name()).indexOf(filter) > -1)) {
        fileCount += 1;
      }
    }
    entry.close();
  }
  
  return fileCount;
}

String SD_getFileName(File dir, String filter, uint16_t fileNumber) {
  // at the moment we are supporting only files in the root-directory
  uint16_t fileCount = 0;
  String s;

  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      return "ERROR";
    }
    if (!entry.isDirectory()) {
      s = entry.name();
      if ((filter.length() == 0) || (s.indexOf(filter) > -1)) {
        if (fileCount == fileNumber) {
          return s;
        }
        fileCount += 1;
      }
    }
    entry.close();
  }
}

String SD_getFileSize(File dir, String filter, uint16_t fileNumber) {
  // at the moment we are supporting only files in the root-directory
  uint16_t fileCount = 0;
  String s;

  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      return "ERROR";
    }
    if (!entry.isDirectory()) {
      s = entry.name();
      if ((filter.length() == 0) || (s.indexOf(filter) > -1)) {
        if (fileCount == fileNumber) {
          return formatSize(entry.size());
        }
        fileCount += 1;
      }
    }
    entry.close();
  }
}

String SD_getTOC(uint8_t format) { // 0=json, 1=csv, 2=psv
  // at the moment we are supporting only files in the root-directory
  String s;
  String TOC = "";
  File root = SD.open("/");
  bool running = true;

  while (running) {
    File entry = root.openNextFile();

    if (!entry) {
      running = false;
    }else{
      if (!entry.isDirectory()) {
        // we have a valid file. Check file-type

        s = entry.name();
        if ((s.indexOf(".mp3") > -1) || (s.indexOf(".wav") > -1)) {
          // we found a mp3 or wave-file
          if (format == 0) {
            // JSON
            if (TOC.length()>0) {
              TOC += ",\"";
            }else{
              TOC += "\"";
            }
            TOC += s;
            TOC += "\":\"";
            TOC += formatSize(entry.size());
            TOC += "\"";
          }else if (format == 1) {
            // CSV
            if (TOC.length()>0) {
              TOC += ",";
            }
            TOC += s;
          }else if (format == 2) {
            // PSV = "Pipe"SeparatedValue
            if (TOC.length()>0) {
              TOC += "|";
            }
            TOC += s;
          }
        }
      }
      entry.close();
    }
  }

  return TOC;
}

uint8_t getNumberOfTocEntries() {
  uint8_t entries = 0;
  String TOC = SD_getTOC(2);
  for (uint8_t i=0; i<TOC.length(); i++) {
    if (TOC[i] == '|') entries++;
  }
  return entries;
}

String listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  String s;
  s = dirname;

  File root = fs.open(dirname);
  if(!root){
      s += "OpenFailed";
      return s;
  }
  if(!root.isDirectory()){
      s += "NotADir";
      return s;
  }

  File file = root.openNextFile();
  while(file){
      if(file.isDirectory()){
          s += file.name();
          if(levels){
              s += listDir(fs, file.name(), levels -1);
          }
      } else {
          s += file.name();
          s += file.size();
      }
      file.close();
      file = root.openNextFile();
  }

  return s;
}
