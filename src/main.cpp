#include <VS1053.h>
#include "wifi.hpp"
#include "eeprom.hpp"
#include "circular_buffer.hpp"
#include "static_resources.hpp"

#ifdef ARDUINO_ARCH_ESP8266
#include <ESPAsyncTCP.h>
#define VS1053_CS     D1
#define VS1053_DCS    D0
#define VS1053_DREQ   D3
#else
#ifdef ARDUINO_ARCH_ESP32
#include <AsyncTCP.h>
#define VS1053_CS     5
#define VS1053_DCS    16
#define VS1053_DREQ   4
#else
#error Unsupported board
#endif
#endif
#include <ESPAsyncWebServer.h>


constexpr uint8_t volume = 100; // default to max volume
VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

char buf_mp3_data[16384];
// The chunk size of 64 seems to be optimal. At 32 and 128 the sound might be brassy.
CircularBuffer buf_mp3{buf_mp3_data, sizeof(buf_mp3_data), 64};

constexpr int eeprom_addr_ssid = 0;
constexpr int eeprom_addr_password = 32;
constexpr int eeprom_addr_port = 96;
constexpr int eeprom_addr_host = 128;
constexpr int eeprom_addr_path = 256;
char buf_ssid[33]; // SSID max length is 32
char buf_password[65]; // password max length is 64
int buf_port; // hold the port (an int is 4 bytes usually)
char buf_host[129]; // hold a host of 128 bytes
char buf_path[257]; // hold a path of 256 bytes

constexpr const char* AP_ssid = "Riproduttore RadioALA";


void saveString(String s, char* buffer, size_t size, int eepromAddress) {
    memcpy(buffer, s.c_str(), s.length());
    memset(buffer + s.length(), 0, size - s.length());
    saveToEEPROM(eepromAddress, buffer, size);
}

void connectWifiOrStartHotspot() {
    readFromEEPROM(eeprom_addr_ssid, buf_ssid, sizeof(buf_ssid));
    readFromEEPROM(eeprom_addr_password, buf_password, sizeof(buf_password));

    if (connectWifi(buf_ssid, buf_password)) {
        readFromEEPROM(eeprom_addr_port, &buf_port);
        readFromEEPROM(eeprom_addr_host, buf_host, sizeof(buf_host));
        readFromEEPROM(eeprom_addr_path, buf_path, sizeof(buf_path));
        connectWifiClient(buf_host, buf_port, buf_path);
        return;
    }

    // connecting to the Wifi network failed, open the hotspot
    if (turnOnWifiAP(AP_ssid, nullptr)) {
        AsyncWebServer server(80);

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", homePageHtml);
        });

        server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){
            constexpr const char* PARAM_SSID = "ssid", * PARAM_PASSWORD = "password";
            if (!request->hasParam(PARAM_SSID, true) || !request->hasParam(PARAM_PASSWORD, true)) {
                request->send(400);
                return;
            }

            String ssid = request->getParam(PARAM_SSID, true)->value();
            String password = request->getParam(PARAM_PASSWORD, true)->value();
            if (ssid.length() >= sizeof(buf_ssid) || password.length() >= sizeof(buf_password)) {
                request->send(422);
                return;
            }

            saveString(ssid, buf_ssid, sizeof(buf_ssid), eeprom_addr_ssid);
            saveString(password, buf_password, sizeof(buf_password), eeprom_addr_password);

            request->send(200);
        });

        server.on("/radio", HTTP_POST, [](AsyncWebServerRequest *request){
            constexpr const char* PARAM_HOST = "host", * PARAM_PORT = "port", * PARAM_PATH = "path";
            if (!request->hasParam(PARAM_HOST, true) || !request->hasParam(PARAM_PORT, true) ||
                    !request->hasParam(PARAM_PATH, true)) {
                request->send(400);
                return;
            }

            String host = request->getParam(PARAM_HOST, true)->value();
            String port = request->getParam(PARAM_PORT, true)->value();
            String path = request->getParam(PARAM_PATH, true)->value();
            if (host.length() >= sizeof(buf_host) || port.length() > 5 || path.length() >= sizeof(buf_path)) {
                request->send(422);
                return;
            }

            long portInt = port.toInt();
            if (portInt <= 0 || portInt >= 65536) {
                request->send(422);
                return;
            }

            saveString(host, buf_host, sizeof(buf_host), eeprom_addr_host);
            saveString(path, buf_path, sizeof(buf_path), eeprom_addr_path);
            buf_port = portInt;
            saveToEEPROM(eeprom_addr_port, buf_port);

            request->send(200);
        });

        server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request){
            request->send(200);
            ESP.restart();
        });

        server.begin();

        while (true) delay(100000);
    } else {
        while (true) delay(100000);
    }
}

void setup() {
    int timeBeginning = millis();
    Serial.begin(115200);
    beginEEPROM();

    connectWifiOrStartHotspot();

    if (int remainingTime = 3000 - (millis() - timeBeginning); remainingTime > 0) {
        delay(remainingTime); // wait at least 3 seconds before initializing VS1053
    }

    SPI.begin();
    player.begin();
    if (!player.isChipConnected()) {
        Serial.println("VS1053 chip is not connected correctly!");
    }

    Serial.print("Loading VS1053 patches... ");
    player.loadDefaultVs1053Patches();
    Serial.println("Done!");
    player.switchToMp3Mode();
    player.setVolume(volume);
}

int i = 0;
void loop() {
    reconnectWifiClientIfNeeded(buf_host, buf_port, buf_path);

    if (buf_mp3.writeMaxCount() > 0 && availableInWifiClient() > 0) {
        int written = readFromWifiClient(buf_mp3.writeAddress(), buf_mp3.writeMaxCount());
        buf_mp3.advanceWritten(written);
    }

    if (++i % 1024 == 0) {
        buf_mp3.printInfo();
    }

    if (buf_mp3.chunkAvailable()) {
        player.playChunk((uint8_t*) buf_mp3.advanceChunk(), buf_mp3.chunkSize());
    }
}