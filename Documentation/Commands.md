# Overview of X-FBAPE Command-System
The X-FBAPE supports multiple ways to control the system:
1. USB-connection
2. TCP-Connection
3. WiFi
4. X32-Surface

In this document the ASCII-commands are described using the connection types 1 to 3. Here both microcontrollers, the SAMD21 and the NINA W102, can be accessed individually. The SAMD21 owns the USB- and TCP-Connection and passes all commands to the NINA, that not start with a "samd:".

## Commands related to SAMD
The SAMD controls the X-Touch, the MackieMCU emulation and the X32-Surface. So it controls the color of X-Touch displays, the channel-names and more:

* samd:ver?
> Returns a short version-information as String
* samd:*IDN?
> Returns a detailed information about the system as String
* samd:info?
> Returns a multi-line device information about the system as String
* samd:update:nina
> Sets the NINA W102 into an serial-update-mode and tunnels all UART-communication between USB and NINA-W102. So the regular "Upload firmware"-option in Arduino IDE can be used to upload a new NINA-firmware.
* samd:passthrough:nina@0 / samd:passthrough:nina@1
> Passthrough all serial-data from NINA not starting with "samd:" to the USB-port.
* samd:debug:nina@0 / samd:debug:nina@1
> Passthrough all serial-data from NINA to USB-port, even if it contains "samd:"
* samd:debug:x32@0 / samd:debug:x32@1
> Passthough all communication from X32 to the USB-port.
* samd:config:mac?
> Returns the current MAC-address read from the EEPROM as String.
* samd:config:ip?
> Returns the current IP-address of the SAMD21
* samd:config:set:ip@192.168.0.42
> Sets the IP-address of SAMD21 to the given address
* samd:config:set:xtouchipX@192.168.0.43
> Sets the IP-address of X-Touch X to the given address. X ranges from 1 to XTOUCH_COUNT with SAMD.h
* samd:setname:chX@Y
> Sets the channel-name of channel X to String Y. X ranges from 1 to 32
* samd:setcolor:chX@Y
> Sets the channel-colorof channel X to Y. X ranges from 1 to 32
> Valid values for Y are 0=black, 1=red, 2=green, 3=yellow, 4=blue, 5=pink, 6=cyan and 7=white
> Adding 64 to these values inverts the second line of display.
* samd:setname:dmxchX@Y
> Sets the channel-name of channel X to String Y. X ranges from 1 to 512
* samd:setcolor:dmxchX@Y
> Sets the channel-colorof channel X to Y. X ranges from 1 to 512
> Valid values for Y are 0=black, 1=red, 2=green, 3=yellow, 4=blue, 5=pink, 6=cyan and 7=white.
> Adding 64 to these values inverts the second line of display.
* samd:config:save
> Saves IP-addresses of device and X-Touch to EEPROM


## Commands related to NINA W102
The NINA-W102 (ESP32) is the main-control of the system and has more options compared to the SAMD21:

### Commands related to WiFi
* wifi:ssid@X
> sets the WiFi SSID to the String X
* wifi:password@X
> sets the WiFi password to the String X
* wifi:ip?
> Returns the current IP-address as String
* wifi:ip@192.168.0.42
> Sets the IP-address to the given address (but does not restart the WiFi!)
* wifi:gateway@192.168.0.1
> Sets the IP-gateway to the given address (but does not restart the WiFi!)
* wifi:subnet@255.255.255.0
> Sets the IP-subnet to the given subnet (but does not restart the WiFi!)
* wifi:mode@0 / wifi:mode@1
> Sets the WiFi to either 0=Client or 1=Access-Point (but does not restart the WiFi!)
* wifi:enabled@0 / wifi:enabled@1
> Turns the WiFi off (if 0) or (Re-)Initializes the WiFi (if 1)
* wifi:mqttserver@192.168.0.33
> Sets the mqtt-server to the given IP-address

### Commands related to playback-control
* player:filesize?
> Returns the current filesize in bytes as String
* player:file@filename.mp3
> Sets the given filename as new file
* player:stream@http://play.mp3.com
> Connects to the given host to play mp3 or m3u
* player:currentfile?
> Returns the current audio-file as String
* player:toc?
> Returns the content of the SD-file as CSV-String
* player:play
> Plays the current audiofile
* player:loop@0 / player:loop@1
> Enables or disables the loop of an audio-file
* player:pause
> Pauses or resumes the current audio-file
* player:stop
> Stops the current audio-file
* player:duration?
> Returns the duration of the current audiofile in seconds as String
* player:position?
> Returns the current position in seconds as String
* player:position@X
> Sets the current play-position to the given position X as String in seconds
* player:percent@X
> Sets the current play-position to the given position X as String in percent
* player:select@X
> Selects the track number X, but does not start the playback
* player:next
> Plays the next track on the SD-card
* player:prev
> Plays the previous track on the SD-card
* player:volume@X
> Sets the internal volume of the player in dBfs. X ranges from -140 to 0

### Commands related to bluetooth
Several commands are prepared but due to the lack of internal flash-space in the NINA-W102 these functions cannot be used at the moment. When using a larger/more modern ESP32, you can enable the bluetooth features. The FPGA is already prepared to play the I2S-stream of an A2DP-source.

### Commands related to audio-mixing
* mixer:volume:main@X
> Sets the volume of the main-channel to X in dBfs. X ranges from -48 to 6
* mixer:balance:main@X
> Sets the balance of the main-channel to X in percent. X ranges from 0 to 100 with 50 as center.
* mixer:volume:sub
> Sets the volume of the sub-channel to X in dBfs. X ranges from -48 to 6
* mixer:volume:chX@Y
> Sets the volume of channel X to Y in dBfs. X ranges from 1 to 32 and Y from -48 to 6
* mixer:mute :chX@Y
> Mutes channel X if Y is 1 or unmutes it if Y is 0. Multiple channels can be muted at the same time.
* mixer:solo:ch
> Enables solo for channel X if Y is 1 or disables solo it if Y is 0. Multiple channels can be solo-enabled at the same time.
* mixer:volume:card@X
> Sets the volume of the sd-card-channel to X in dBfs. X ranges from -48 to 6
* mixer:balance:chX@Y
> Sets the balance of channel X to Y in percent. X ranges from 1 o 32 and Y from 0 to 100 with 50 as center.
* mixer:reset
> Initializes all channels and effects (including EQs and compressors) to the default values

### Commands related to EQing and effects
* mixer:eq:reset
> Resets all EQs to the default values
* mixer:dynamics:reset
> Resets all compressors and gates to the default values
* mixer:eq:peqX@A,B,C,D
> Sets EQ X to type A, frequency B, quality C and gain D.
> X ranges from 1 to 5
> A ranges from 0=Allpass, 1=peak, 2=LowShelf, 3=HighShelf, 4=Bandpass, 5=Notch, 6=LowPass, 7=HighPass
> B ranges from 10 to 20000
> C ranges from 0.2 to 10
> D ranges from -20 to 20
* mixer:eq:bypassX@Y
> Bypasses EQ X if Y is 1
* mixer:eq:lp@X
> Sets frequency of low-pass to X Hz
* mixer:eq:hp
> Sets frequency of high-pass to X Hz
* mixer:adc:gainX@Y
> Sets gain for adc X to Y dB.
* mixer:gateX@A,B,C,D,E
> Sets Noisegate X to Threshold A, Range B, Attack C, Hold D and Release E
> A and B ranges from -60 to 0
> C, D and E ranges from 0 to 2000 ms
* mixer:compX@A,B,C,DE,F
> Sets dynamic compressor X to Threshold A, Ratio B, Makeup C, Attack D, Hold E, Release F
> A ranges from -60 to 0
> B ranges from 0=Limiter, over 1=1:1 to 8=1:64
> C ranges from 0 to 48
> D, E and F ranges from 0 to 2000 ms

### Commands related to DMX512
* dmx512:output:chX@Y
> Sets DMX512 channel X to value Y.
> X ranges from 0 (Startbyte, leave it value 0!), 1=channel 1 to 512
> Y ranges from 0 to 255

### Additional Commands

* ver?
> Returns a short version-information of the main-control as String
* *IDN?
> Returns a detailed information about the main-control as String
* info?
> Returns a multi-line device information about the main-control system as String
* fpga:ver?
> Returns a short version-information of the FPGA-bitstream as String
* fpga:*IDN?
> Returns a detailed information about the FPGA-bitstream system as String
* system:debug:fpga@0 | system:debug:fpga@1
> Enables or disables direct passthrough of the FPGA-communication to the SAMD21 and then the USB-connection
* system:init
> Initializes the main-controller. This function is called automatically at boot
* system:stop
> Stops the main-controller and disables all timers
* system:card:init
> Reinitializes the SD-Card. Useful if you remove and reinserts the card
* system:config:read@file.cfg
> Read the given file from SD and executes commands line-by-line. You can use all commands you find in this file
* system:config:write@file.cfg
> Writes the current configuration of the main-controller to the given file
* system:wificonfig:write
> Writes the current WiFi-configuration to the file "wifi.cfg". Caution: the file uses plain-text-passwords!!!
