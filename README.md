# ESP8266/ESP32 + VS1053 webradio

This project allows an ESP8266 or an ESP32 board to play MP3 web radios through a VS1053 board. It features a web interface for configuration that allows specifying WiFi login information and the radio URL to connect to.

## Example radios

- [RadioALA](http://cast.radioala.it:8000/stream)
```cpp
const char *host = "cast.radioala.it";
const char *path = "/stream";
int httpPort = 8000;
```

- [RadioALA Velluti](http://velluto.radioala.it/listen/radio_ala/radio.mp3")
```cpp
const char *host = "velluto.radioala.it";
const char *path = "/listen/radio_ala/radio.mp3";
int httpPort = 80;
```

- [RAI Radio 1](http://icestreaming.rai.it/1.mp3") (requires using HTTP 1.1 instead of 1.0)
```cpp
const char *host = "icestreaming.rai.it";
const char *path = "/1.mp3";
int httpPort = 80;
```