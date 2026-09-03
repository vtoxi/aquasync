#pragma once
#include <Arduino.h>
#include <WebServer.h>

// Serves a small authenticated web page for flashing a new firmware .bin
// over WiFi -- point a browser at http://<OTA_HOSTNAME>.local/ (or the
// device's IP) once WiFi is up, log in with OTA_USERNAME/OTA_PASSWORD,
// and upload a .bin from a GitHub Release. Update.h writes it into the
// inactive OTA partition and reboots into it on success.
//
// The very first flash still has to happen over USB (pio run --target
// upload) -- a board with no AquaSync firmware yet has no OTA page to
// talk to. This is plain HTTP (no TLS), same trust model as the MQTT
// broker -- fine on a home LAN, not something to expose to the internet.
class OtaHandler {
public:
    OtaHandler(const char* username, const char* password, const char* hostname);

    void begin();
    void loop(); // call every iteration; non-blocking

private:
    WebServer server_;
    String username_;
    String password_;
    String hostname_;
    bool uploadAuthorized_ = false;

    void handleRoot();
    void handleUpdatePost();
    void handleUpdateUpload();
};
