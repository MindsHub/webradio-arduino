#include "wifi.hpp"

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#else
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#error Unsupported board
#endif
#endif

WiFiClient client;

bool connectWifi(const char* ssid, const char* password) {
    Serial.print("Connecting to SSID: ");
    Serial.println(ssid);

    wl_status_t status = WiFi.begin(ssid, password);

    while (true) {
        if (status == WL_CONNECTED) {
            break;
        } else if (status != WL_IDLE_STATUS) {
            Serial.print("Connection failed, status: ");
            Serial.println(status);
            return false;
        }

        delay(200);
        Serial.print(".");
        status = WiFi.status();
    }

    Serial.print("\nWiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void connectWifiClient(const char* host, int httpPort, const char* path) {
    Serial.print("Connecting to host: ");
    Serial.println(host);

    if (client.connect(host, httpPort)) {
        Serial.print("Requesting stream: ");
        Serial.println(path);

        client.print("GET ");
        client.print(path);
        client.print(" HTTP/1.0\r\nHost: ");
        client.print(host);
        client.print("\r\nConnection: close\r\n\r\n");

    } else {
        Serial.println("Connection failed");
    }
}

void reconnectWifiClientIfNeeded(const char* host, int httpPort, const char* path) {
    if (!client.connected()) {
        Serial.println("Reconnecting...");
        connectWifiClient(host, httpPort, path);
    }
}

int availableInWifiClient() {
    return client.available();
}

int readFromWifiClient(char* buffer, size_t size) {
    return client.read(buffer, size);
}
