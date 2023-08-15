#pragma once
#ifndef _WIFI_HPP_
#define _WIFI_HPP_

#include <Arduino.h>

bool connectWifi(const char* ssid, const char* password);
void connectWifiClient(const char* host, int httpPort, const char* path);
void reconnectWifiClientIfNeeded(const char* host, int httpPort, const char* path);

int availableInWifiClient();
int readFromWifiClient(char* buffer, size_t size);

bool turnOnWifiAP(const char* ssid, const char* password);

#endif // _WIFI_HPP_