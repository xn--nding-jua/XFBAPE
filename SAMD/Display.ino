#if USE_DISPLAY == 1
  bool initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      return false;
    }

    display.setTextSize(1);
    display.setTextColor(WHITE);

    return true;
  }

  void displayShowLogo(uint8_t logo) {
    display.clearDisplay();
    if (logo == 0) {
      display.drawBitmap(0, 0, bitmap_card_logo, 128, 64, 1);
    }
    if (logo == 1) {
      display.drawBitmap(0, 0, fbape_logo, 128, 64, 1);
    }
    display.display();
  }

  void displayText(uint8_t line, String text) {
    display.setCursor(0, line*11);
    display.setTextColor(WHITE);
    display.print(text);
    display.display();
  }

  void displayPrintLn(String text) {
    if (currentDisplayLine==6) {
      display.clearDisplay();
      currentDisplayLine = 0;
    }

    display.setCursor(0, currentDisplayLine*11);
    display.print(text);
    display.display();

    // increment current display-line for next text-line
    currentDisplayLine += 1;
  }

  // Online-Display-Designer: https://github.com/rickkas7/DisplayGenerator
  // Online Bitmap-Converter for SD1308: https://javl.github.io/image2cpp
  void displayDrawMenu() {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    // main-menu (Title, Progress, Volume)
    display.print(F("X-f/bape>"));
    display.print(F("Main"));
    display.drawLine(0, 10, 128, 10, 1);
    display.setCursor(0, 15);
    display.print("T:" + playerinfo.title);
    display.setCursor(0, 27);
    display.print(secondsToHMS(playerinfo.time) + " / " + secondsToHMS(playerinfo.duration));
    display.fillRect(0, 40, playerinfo.progress, 7, 1);
    display.setCursor(104, 40);
    display.print(String(playerinfo.progress) + "%");
    display.drawLine(0, 50, 128, 50, 1);
    display.setCursor(0, 55);
    display.print("LR");
    display.print(String(playerinfo.volumeMain, 1) + "dB");
    display.setCursor(64, 55);
    display.print("Sub");
    display.print(String(playerinfo.volumeSub, 1) + "dB");

    display.display();
  }
#endif