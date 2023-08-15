#include <VS1053.h>
#include "wifi.hpp"
#include "circular_buffer.hpp"

#ifdef ARDUINO_ARCH_ESP8266
#define VS1053_CS     D1
#define VS1053_DCS    D0
#define VS1053_DREQ   D3
#else
#ifdef ARDUINO_ARCH_ESP32
#define VS1053_CS     5
#define VS1053_DCS    16
#define VS1053_DREQ   4
#else
#error Unsupported board
#endif
#endif

// Default volume
#define VOLUME  100

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

// WiFi settings example, substitute your own
const char *ssid = "SSID";
const char *password = "PASSWORD";

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

// The chunk size os 64 seems to be optimal. At 32 and 128 the sound might be brassy.
char buf_mp3_data[32768];
CircularBuffer buf_mp3{buf_mp3_data, sizeof(buf_mp3_data), 64};


void connectWifiOrStartHotspot() {
    if (connectWifi(ssid, password)) {
        connectWifiClient(host, httpPort, path);
        return;
    }

    // connecting to the Wifi network failed, open the hotspot
    // TODO
    while(true) delay(1000);
}

void setup() {
    int timeBeginning = millis();
    Serial.begin(115200);

    // This can be set in the IDE no need for ext library
    // system_update_cpu_freq(160);

    connectWifiOrStartHotspot();

    if (int remainingTime = 3000 - (millis() - timeBeginning); remainingTime > 0) {
        delay(remainingTime); // wait at least 3 seconds before initializing VS1053
    }

    SPI.begin();
    player.begin();
    Serial.print("Loading patches... ");
    player.loadDefaultVs1053Patches();
    Serial.println("Done!");
    player.switchToMp3Mode();
    player.setVolume(VOLUME);
}

int i=0;
void loop() {
    reconnectWifiClientIfNeeded(host, httpPort, path);

    if (buf_mp3.writeMaxCount() > 0 && availableInWifiClient() > 0) {
        readFromWifiClient(buf_mp3.writeAddress(), buf_mp3.writeMaxCount());
    }

    if (++i % 1024 == 0) {
        buf_mp3.printInfo();
    }

    if (buf_mp3.chunkAvailable()) {
        player.playChunk((uint8_t*) buf_mp3.advanceChunk(), buf_mp3.chunkSize());
    }
}