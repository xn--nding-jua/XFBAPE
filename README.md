# X-f/bape - A DIY expansion-card for the Behringer X32
This repository contains the firmware for the X-f/bape DIY-ExpansionCard for the Behringer X32. This project
is based on the f/bape Audioplayer-project. Three individual devices are part of this repository:

* Atmel/Microchip SAMD21 ATSAMD21G18A Cortex-M0+ 32-Bit Microcontroller
* Intel Cyclone 10LP 10CL016 FPGA
* uBlox NINA W102 (ESP32 D0WDQ6 Dual-Core)

![alt text](Documentation/Images/System.jpg)

## Main-Features of the system
- [x] receive, mix and send 32 audio-channels with 48kHz and 24bit
- [x] MP3-Playback via SD-Card
- [x] Parametric-Equalizer
- [x] 24dB/oct Linkwitz-Riley Crossover for tweeter and subwoofer
- [x] Noise-Gate and dynamic Audio-Compression
- [x] Webserver for Controlling EQing and other parameters
- [x] Control via ASCII-based commands (or optional MQTT)
- [x] Analog Audio-Output via PDM
- [x] DMX512-output

## Overview
The SAMD21 is used as an USB-2-UART converter for controlling and updating the individual devices. Furthermore, it contains the bitstream for the Cyclone 10 FPGA as we are not using the original flash-chip for an easier update of the whole system. The device also controls the connected I2C-display, external GPIO-buffers and external ADC. It presents a simple GUI to the user to allow adjusting the most important functions without the webserver.
The FPGA receives audio-data via four TDM connections and control data via an UART-connection from the NINA W102. It also implements a frequency crossover for the two tweeters and the subwoofer as well as parametric equalizers and volume-controls.
The uBlox NINA W102 (ESP32) acts as a WiFi Accesspoint with webserver. It can be used to adjust audio-volume, select MP3-tracks adjust the equalizer and other functions of the system. It feeds the audio-data via I2S to the FPGA. It can be used to act as a Bluetooth-A2DP-sink as well.

As the original routing-functions of the Behringer X32 can still be used to route the individual channels, we are gaining more degree of freedom:
Using the 32 regular channels of the X32 and routing additional 32 channels from the AES50-connection to the X-f/bape-card, we can mix up to 64 channels in realtime with the X32. The X-f/bape will feed a submix to two of the 32 return-channels, that can be routed to two of the 6 AUX-Channels.
As the Cyclone 10LP is not the largest FPGA we are a bit limited with the EQing, but a 5-band PEQ with Compression can be added to the sum.

## Used Device-Ressources
* Atmel SAMD21 ATSAMD21G18A: 26% of program storage space
* Intel Cyclone 10LP 10CL016: 95% logic elements used / 79% Embedded Multipliers used
* uBlox NINA W102 (ESP32 D0WDQ6): 73% of program storage space

## Connections
USB -> SAMD21 <-> FPGA <-> ESP32

## Uploading/Updating the firmware(s)

As this system uses several controllers and ICs, a total of three different firmwares have to be compiled and uploaded. The USB-port is connected to the SAMD21. So this is the first place for updates. As the SAMD21 acts as a JTAG-uploader for the FPGA, too, it will upload the bitstream for the FPGA together with its firmware. Finally, the SAMD21 can be set to an upload-mode, to update the NINA W102-module (ESP32).

### SAMD21 with bitstream for Intel Cyclone 10-FPGA
Start Intel Quartus Prime Lite and compile the logic. If you alreade have a file "bitstream.h" this is not nescessary. As the FPGA bitstream is part of the SAMD21 firmware, you have to update the FPGA-bitstream using this command before uploading the code to SAMD21:
    update_fpga_bitstream.bat

This batch-file will call the following command, to convert the TabularTextFile (TTF) into an Arduino-Header:

    ..\FPGA\Tools\vidorcvt\vidorcvt.exe < ..\FPGA\output_files\Audioplayer.ttf > bitstream.h
	
Now start the Arduino IDE in version 2.x. Connect the SAMD via USB and connect Arduino IDE to the designated port. Select "Arduino MKR Vidor 4000" as board and upload the software using the regular upload-functions of Arduino IDE. Next to the regular Arduino upload-function, the software bossac.exe can be used to upload the firmware without Arduino IDE:

* connect SAMD21 via USB
* initialize the bootloader of the SAMD21:
`mode COM5: BAUD=1200 parity=N data=8 stop=1 > nul`
upload the firmware
`bossac.exe --port=COM21 -I -U true -e -w Audioplayer.ino.bin -R`

Caution: As the power-demand of this board is quite high, use a sufficient power-supply for your USB-hub and a good cable!

### NINA WiFi-module
A firmware-update to the ESP32 is only possible, when SAMD21-controller is set to "passthrough-mode" so that an UART communication to the ESP32 is possible directly through SAMD21 and the FPGA. So first, open a terminal and send the following command to the SAMD21:

    samd:update:nina
Alternatively you can open the updatemode using windows-command-line (in this example the Vidor is connected to COM5):

    echo samd:update:nina > COM5
Then use the Arduino IDE to upload the new software using the built-in uploading-functions for the ESP32 uBlox NINA W102. An upload is possible using the esptool.exe, directly:

    Tools\esptool.exe --chip esp32 --port COM5 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 2MB 0x1000 Update\NINA.bootloader.bin 0x8000 Update\NINA.partitions.bin 0xe000 Tools\boot_app0.bin 0x10000 Update\NINA.bin
After uploading the new firmware, please reset the board using the RESET-button.


## Graphical User Interfaces

There are several options to control the system:
* WiFi via integrated Webinterface
* UART via FBAPE.EXE
* via ASCII-commands via UART and Ethernet

As the Ethernet-Contoller is connected only to the SAMD21, the webinterface of the NINA-W102 cannot be accessed. Maybe we can change this in a later design. But for now the ethernet-port is limited to plain ASCII-commands.