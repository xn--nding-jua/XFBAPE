const char* versionstring = "v3.1.0";
const char compile_date[] = __DATE__ " " __TIME__;

String hostname = "xfbape";
#define FTP_USER "xfbape"
#define FTP_PASSWORD "xfbape"

// Setup the WiFi
// Accesspoint-Mode
bool WiFiAP = true;
String ssid     = "xfbape";
String password = "xf/bapePASSWORD"; // at least 8 characters
/*
// Client-Mode
bool WiFiAP = false; // WiFi-mode can be changed via USB or Ethernet during operation
String ssid     = "YourVeryOriginalSSID"; // SSID can be changed via USB or Ethernet during operation
String password = "YourVerySafeWiFiPassword"; // password can be changed via USB or Ethernet during operation
*/

// configure the system
#define MAX_AUDIO_CHANNELS  32  // the number of audio-channels must match the number of channels in the FPGA
#define MAX_EQUALIZERS      5   // number of PEQ must match number of PEQ in SAMD21 and FPGA
#define MAX_ADCS            1
#define MAX_NOISEGATES      1
#define MAX_COMPRESSORS     2
#define USE_STORAGE         0   // 0 = SD, 1 = SD_MMC, 2 = FATFS, 3 = LITTLEFS
#define USE_SDPLAYER        1   // enable or disable SD-Card-Player-functions as the ESP32 has not enough space for both SD-Card-Playback and Bluetooth
#define USE_BLUETOOTH       0   // enable the bluetooth-A2DP-receiver (takes ~ 741,104 bytes)
#define USE_STATICIP        0   // only for client-mode. In AccessPoint-Mode it will use static-IP and ignores this options
#define USE_TCPSERVER       1   // enable TCP-Server (takes ~ 10029 bytes)
#define USE_MQTT            0   // enable MQTT (takes ~ 8360 bytes)
#define USE_FTP_SERVER      0   // enable FtpServer for uploading/deleting/editing files
#define USE_DISPLAY         1   // enable functions for I2C display connected to SAMD21 (takes ~ 384 bytes)
#define USE_DMX512          0   // enable outputting DMX512 via UART2
#define USE_DMX512_RX       0   // enable DMX512-receiver via UART2

#define AUDIO_INIT_VOLUME   21          // 0...21, so set to max on initialization
#define AUDIO_SAMPLERATE    48000       // if using X32 this is fixed to 48kHz

#define FPGA_IDX_MAIN_VOL   0   // main-volume
#define FPGA_IDX_CH_VOL     3   // first channel-volume
#define FPGA_IDX_XOVER      70	// cross-over
#define FPGA_IDX_PEQ        88	// parametric equalizers
#define FPGA_IDX_GATE       150	// noisegates
#define FPGA_IDX_COMP       175	// compressors
#define FPGA_IDX_AUX_CMD    200	// auxiliary commands
#define FPGA_IDX_ADC_GAIN   220	// adc gains

#include "NINA_Pinmap.h"        // define the pins for this project

// setup bluetooth
#if USE_BLUETOOTH == 1
  #define BT_I2S_MCK        NINA_PIO34 // will not be used but we have to define it
  #define BT_I2S_DOUT       NINA_PIO36
  #define BT_I2S_BCLK       NINA_PIO32
  #define BT_I2S_LRC        NINA_PIO35
  #define BT_BITS           16  // select 16 or 32 bit. Please change receiver-block in FPGA to this bitrate
#endif

// here the interface to the SD-Card can be setup
#if USE_STORAGE == 0
  #define SPI_CLOCK           16000000    // ESP32 supports SPI-clock of up to 80 MHz (on good cables). 20MHz is working

  // defines for SD-SPI-mode
  #define SD_CS               NINA_PIO28  // connected to PCIE-Adapter T9/P23 <- is read-only. So we are using N12/P20
  #define SPI_MOSI            NINA_PIO1   // connected to PCIE-Adapter R9/P25 <- is read-only. So we are using R8/P6
  #define SPI_MISO            NINA_PIO21  // connected to PCIE-Adapter R12/P30
  #define SPI_SCK             NINA_PIO29  // connected to PCIE-Adapter T13/P28

  // include official Arduino-headers
  #include "SD.h"
  #include "SPI.h"
  #include "FS.h"

  #define FileSystem SD
#elif  USE_STORAGE == 1
  // defines for SDMMC-mode
  #define SDMMC_DAT0          NINA_PIO2  // needs to be bidirectional so we cannot pass through FPGA!
  #define SDMMC_DAT1          NINA_PIO3  // needs to be bidirectional so we cannot pass through FPGA!
  #define SDMMC_DAT2          NINA_PIO4  // needs to be bidirectional so we cannot pass through FPGA!
  #define SDMMC_DAT3          NINA_PIO24 // needs to be bidirectional so we cannot pass through FPGA!
  #define SDMMC_CMD           NINA_PIO25 // needs to be bidirectional so we cannot pass through FPGA!
  #define SDMMC_CLK           NINA_PIO29 // only unidirectional

  // include official Arduino-headers
  #include "SD_MMC.h"
  #include "SPI.h"
  #include "FS.h"

  #define FileSystem SD_MMC
#elif USE_STORAGE == 2
  // setup FatFS (store files within the FLASH)
  #include "FFat.h"

  #define FileSystem FFat
#elif USE_STORAGE == 3
  // setup LittleFS (store files within the FLASH)
  #include "LittleFS.h"

  #define FileSystem LittleFS
#endif

String currentAudioFile;
uint32_t currentAudioPosition;

#if USE_SDPLAYER == 1
  #define I2S_DOUT            NINA_PIO8
  #define I2S_BCLK            NINA_PIO5
  #define I2S_LRC             NINA_PIO7
#endif

// include official Arduino-headers
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <Ticker.h> // timer-functions
#include <stdlib.h> // atoi()
#include <ArduinoJson.h>

// IP-Address of Access-Point
IPAddress ip(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 0, 1);
IPAddress secondaryDNS(1, 1, 1, 1);

#if USE_BLUETOOTH == 1
  // setup bluetooth
  #include "AudioTools.h"
  #include "BluetoothA2DPSink.h" // bluetooth-functions
  I2SStream i2s;
  BluetoothA2DPSink a2dp_sink(i2s);
#endif

#if USE_SDPLAYER == 1
  // includes for Audiosystem and SD-Card-Playback
  #include "Audio.h"
  Audio audio;
  TaskHandle_t AudioTask;
#endif

// initializing individual tools
WebServer webserver(80); // Webserver
#if USE_TCPSERVER == 1
  WiFiServer cmdserver(5025); // command-server
  WiFiClient cmdclient;
#endif
File configFile;

#if USE_MQTT == 1
  #define mqtt_id "xfbape"
  IPAddress mqtt_server(192, 168, 0, 24); // can be changed via USB or Ethernet
  #define mqtt_serverport 1883

  #include <PubSubClient.h>

  WiFiClient mqttnetworkclient;
  PubSubClient mqttclient(mqttnetworkclient);
#endif

#if USE_FTP_SERVER == 1
  #include <FtpServerKey.h>
  // redefine the SD-Card Settings
  #define DEFAULT_STORAGE_TYPE_ESP32 STORAGE_SD
  // include the library
  #include <SimpleFTPServer.h>

  FtpServer ftp;
#elif USE_FTP_SERVER == 2
  #include "ESP-FTP-Server-Lib.h"
  #include "FTPFilesystem.h"

  FTPServer ftp;
#endif

#if USE_DMX512 == 1
  #include <esp_dmx.h>
  #define DMX512_TX_PIN NINA_PIO2
  #define DMX512_RX_PIN NINA_PIO3
  #define DMX512_EN_PIN NINA_PIO4
  #define DMX_PACKET_SIZE 513
  dmx_port_t dmxPort = 2; // Serial0 = Comm with SAMD21, Serial1 = Comm with FPGA, Serial2 = DMX512 output
  uint8_t dmxData[DMX_PACKET_SIZE];
  TaskHandle_t Dmx512Task;
#endif
#if USE_DMX512_RX == 1
  uint8_t dmxRxData[DMX_PACKET_SIZE];
  dmx_packet_t DmxReceiverPacket;
  TaskHandle_t Dmx512ReceiverTask;
#endif

// includes for Ticker
#include <Ticker.h>
Ticker TimerSeconds;

// general variables
bool systemOnline = false;
String USBCtrlIDN = "0";
String FPGA_Version = "0";
uint8_t gateStatusInfo = 0; // info about individual gates (at the moment we are not using this here)
uint8_t compStatusInfo = 0; // info about individual gates (at the moment we are using only 2 compressors for left/right and sub)
uint8_t clipStatusInfo = 0; // info about individual gates (at the moment we are using only 3 clip-channels for left, right and sub)
uint8_t audioStatusInfo = 0; // most important infos in one byte for SAMD21
bool debugRedirectFpgaSerialToUSB = false;
#define SCI_PAYLOAD_LEN 7 // 7 bytes as payload
#define SCI_RINGBUF_LEN 15
#define SCI_CMD_LEN (SCI_PAYLOAD_LEN + 4) // payload + "A", "C0", "C1", "E"
uint8_t fpgaRingBuffer[SCI_RINGBUF_LEN];
uint16_t fpgaRingBufferPointer = 0;

// vu-meter
#define vumeter_halfLife 0.25f // seconds
#define vumeter_sampleRate 20.0f // effective samplerate for VuMeter (FPGA sends 20 messages per second)
float vumeter_decay = pow(0.5f, 1.0f / (vumeter_halfLife * vumeter_sampleRate));
float vu_meter_value[3]; // vu-meter for left, right and sub

// define own datatypes
typedef union 
{
    uint64_t u64;
    int64_t s64;
    uint32_t u32[2];
    int32_t s32[2];
    uint16_t u16[4];
    int16_t s16[4];
    uint8_t u8[8];
    int8_t s8[8];
    double d;
}data_64b;

typedef union 
{
    uint32_t u32;
    int32_t s32;
    uint16_t u16[2];
    int16_t s16[2];
    uint8_t u8[4];
    int8_t s8[4];
    float   f;
}data_32b;

typedef union 
{
    uint16_t u16;
    int16_t s16;
    uint8_t u8[2];
    int8_t s8[2];
}data_16b;

// variables
struct sPEQ {
  // user-settings
  uint8_t type = 0; // 0=allpass, 1=peak, 2=low-shelf, 3=high-shelf, 4=bandpass, 5=notch, 6=lowpass, 7=highpass
  float fc = 400; // center-frequency of PEQ
  float Q = 2; // Quality of PEQ (bandwidth)
  float gain = 1; // gain of PEQ

  data_32b a[3];
  data_32b b[3];
};

struct sLR12 {
  // user-settings
  float fc = 100; // cutoff-frequency for high- or lowpass
  bool isHighpass = false; // choose if Highpass or Lowpass

  // filter-coefficients
  data_64b a[3];
  data_64b b[3];
};

struct sLR24 {
  // user-settings
  float fc = 100; // cutoff-frequency for high- or lowpass
  bool isHighpass = false; // choose if Highpass or Lowpass

  // filter-coefficients
  data_64b a[5];
  data_64b b[5];
};

struct sNoisegate {
  // user-settings
  float threshold = -80; // value between -80 dBfs (no gate) and 0 dBfs (full gate) -> 2^23..2^13.33
  float range = 48; // value between 48dB (full range) and 3dB (minimum effect) 
  float attackTime_ms = 10;
  float holdTime_ms = 50;
  float releaseTime_ms = 258;

  // filter-data
  const float audio_bitwidth = 24; // depends on implementation of FPGA
  data_32b value_threshold;
  const float gainmin_bitwidth = 8; // depends on implementation of FPGA
  data_16b value_gainmin;
  data_16b value_coeff_attack;
  data_16b value_hold_ticks;
  data_16b value_coeff_release;
};

struct sCompressor {
  // user-settings
  float threshold = 0; // value between 0 dBfs (no compression) and -80 dBfs (full compression) -> 2^23..2^13.33
  uint8_t ratio = 1; // value between 0=oo:1, 1=1:1, 2=2:1, 4=4:1, 8=8:1, 16=16:1, 32=32:1, 64=64:1
  uint8_t makeup = 0; // value between 0dB, 6dB, 12dB, 18dB, 24dB, 30dB, 36dB, 42dB, 48dB
  float attackTime_ms = 10;
  float holdTime_ms = 10;
  float releaseTime_ms = 151;

  // filter-data
  const float audio_bitwidth = 24; // depends on implementation of FPGA
  data_32b value_threshold;
  data_16b value_ratio;
  data_16b value_makeup;
  data_16b value_coeff_attack;
  data_16b value_hold_ticks;
  data_16b value_coeff_release;
};

// struct for audiomixer
struct {
  float mainVolume = -20.0; // dBfs
  uint8_t mainBalance = 50; // center L/R
  float mainVolumeSub = -20.0; // dBfs
  float cardVolume = -20.0; // dBfs
  float btVolume = -20.0; // dBfs

  float chVolume[MAX_AUDIO_CHANNELS];
  uint8_t chBalance[MAX_AUDIO_CHANNELS];

  // equalizers
  sPEQ peq[MAX_EQUALIZERS];

  // crossover
  sLR24 LR24_LP_Sub; // <=100Hz
  sLR24 LR24_HP_LR; // >=100Hz

  uint16_t resetFlags = 0; // b7..b4=free, b3=Upsampler, b2=Crossover, b1=Compressor, b0=EQs
  uint16_t bypassFlags = 0; // b7=Crossover LR, b6=Crossover Sub, b5=free, b4..b0=eq5..eq1

  uint16_t adcGain[MAX_ADCS];
  sNoisegate gates[MAX_NOISEGATES];
  sCompressor compressors[MAX_COMPRESSORS]; // left/right, sub
}audiomixer;
