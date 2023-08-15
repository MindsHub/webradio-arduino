/**
  A simple stream handler to play web radio stations using ESP8266

  Copyright (C) 2018 Vince Gellár (github.com/vincegellar)
  Licensed under GNU GPL v3

  Wiring:
  --------------------------------
  | VS1053  | ESP8266 |  ESP32   |
  --------------------------------
  |   SCK   |   D5    |   IO18   |
  |   MISO  |   D6    |   IO19   |
  |   MOSI  |   D7    |   IO23   |
  |   XRST  |   RST   |   EN     |
  |   CS    |   D1    |   IO5    |
  |   DCS   |   D0    |   IO16   |
  |   DREQ  |   D3    |   IO4    |
  |   5V    |   5V    |   5V     |
  |   GND   |   GND   |   GND    |
  --------------------------------

  Dependencies:
  -VS1053 library by baldram (https://github.com/baldram/ESP_VS1053_Library)
  -ESP8266Wifi/WiFi

  To run this example define the platformio.ini as below.

  [env:nodemcuv2]
  platform = espressif8266
  board = nodemcuv2
  framework = arduino
  build_flags = -D PIO_FRAMEWORK_ARDUINO_LWIP2_HIGHER_BANDWIDTH
  lib_deps =
    ESP_VS1053_Library

  [env:esp32dev]
  platform = espressif32
  board = esp32dev
  framework = arduino
  lib_deps =
    ESP_VS1053_Library

  Instructions:
  -Build the hardware
    (please find an additional description and Fritzing's schematic here:
     https://github.com/vincegellar/Simple-Radio-Node#wiring)
  -Set the station in this file
  -Upload the program

  IDE Settings (Tools):
  -IwIP Variant: v1.4 Higher Bandwidth
  -CPU Frequency: 160Hz
*/

#include <VS1053.h>
#include <EEPROM.h>

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#define VS1053_CS     D1
#define VS1053_DCS    D0
#define VS1053_DREQ   D3
#endif

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#define VS1053_CS     5
#define VS1053_DCS    16
#define VS1053_DREQ   4
#endif

// Default volume
#define VOLUME  100

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
WiFiClient client;

// WiFi settings example, substitute your own
// const char *ssid = "SSID";
// const char *password = "PASSWORD";

//  http://comet.shoutca.st:8563/1
const char *host = "cast.radioala.it";
const char *path = "/stream";
int httpPort = 8000;
// const char *host = "velluto.radioala.it";
// const char *path = "/listen/radio_ala/radio.mp3";
// int httpPort = 80;
// const char *host = "icestreaming.rai.it";
// const char *path = "/1.mp3";
// int httpPort = 80;

constexpr size_t buf_size = 32768;
// The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
constexpr size_t chunk_size = 64;
size_t buf_start = 0;
size_t buf_count = 0;
uint8_t buf_mp3[buf_size];

constexpr size_t eeprom_buf_size = 128;
char buf_ssid[eeprom_buf_size];
char buf_password[eeprom_buf_size];

void connect() {
    if (client.connect(host, httpPort)) {
        client.print(String("GET ") + path + " HTTP/1.0\r\n" +
                        "Host: " + host + "\r\n" +
                        "Connection: close\r\n\r\n");
    } else {
        Serial.println("Connection failed");
    }
}

void setup() {
    Serial.begin(115200);

    // Wait for VS1053 and PAM8403 to power up
    // otherwise the system might not start up correctly
    delay(3000);

    // This can be set in the IDE no need for ext library
    // system_update_cpu_freq(160);

    Serial.println("\n\nSimple Radio Node WiFi Radio");

    SPI.begin();

    player.begin();
    Serial.print("Loading patches... ");
    player.loadDefaultVs1053Patches();
    Serial.println("Done!");
    player.switchToMp3Mode();
    player.setVolume(VOLUME);

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Connecting to ");
    Serial.println(host);

    connect();

    Serial.print("Requested stream: ");
    Serial.println(path);
}

int i=0;
void loop() {
    if (!client.connected()) {
        Serial.println("Reconnecting...");
        connect();
    }

    if (buf_count < buf_size && client.available() > 0) {
        size_t bytestoread = (buf_start + buf_count >= buf_size) ? (buf_size - buf_count) : (buf_size - buf_start - buf_count);
        size_t bytesread = client.read(buf_mp3 + ((buf_start + buf_count) % buf_size), bytestoread);
        buf_count += bytesread;
        if (buf_count > buf_size) {
            Serial.println("Impossible situation, client read more bytes than requested");
        }
    }

    if (++i % 1024 == 0)
        Serial.println(buf_count);

    if (buf_count >= chunk_size) {
        // chunk_size divides buf_size, so we don't need to check for circular buffer bounds
        player.playChunk(buf_mp3 + buf_start, chunk_size);
        buf_count -= chunk_size;
        buf_start += chunk_size;
        buf_start %= buf_size;
    }
}