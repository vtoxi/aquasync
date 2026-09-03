#include "OtaHandler.h"
#include <Update.h>
#include <ESPmDNS.h>
#include "Version.h"

namespace {
const char kUploadPage[] PROGMEM = R"(<!DOCTYPE html>
<html><head><title>AquaSync OTA</title></head>
<body>
<h2>AquaSync firmware update</h2>
<p>Running: %VERSION%</p>
<p>Upload a .bin from the project's GitHub Releases page. The device
reboots automatically once the write succeeds.</p>
<form method="POST" action="/update" enctype="multipart/form-data">
  <input type="file" name="firmware" accept=".bin" required>
  <input type="submit" value="Upload">
</form>
</body></html>
)";
}

OtaHandler::OtaHandler(const char* username, const char* password, const char* hostname)
    : server_(80), username_(username), password_(password), hostname_(hostname) {}

void OtaHandler::begin() {
    if (MDNS.begin(hostname_.c_str())) {
        MDNS.addService("http", "tcp", 80);
    }

    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/update", HTTP_POST,
               [this]() { handleUpdatePost(); },
               [this]() { handleUpdateUpload(); });
    server_.begin();
}

void OtaHandler::loop() {
    server_.handleClient();
}

void OtaHandler::handleRoot() {
    if (!server_.authenticate(username_.c_str(), password_.c_str())) {
        return server_.requestAuthentication();
    }
    String page = kUploadPage;
    page.replace("%VERSION%", FIRMWARE_VERSION);
    server_.send(200, "text/html", page);
}

void OtaHandler::handleUpdateUpload() {
    HTTPUpload& upload = server_.upload();

    if (upload.status == UPLOAD_FILE_START) {
        // Auth has to be checked here, before any flash write -- by the
        // time the matching handleUpdatePost() runs, the whole file has
        // already been streamed in.
        uploadAuthorized_ = server_.authenticate(username_.c_str(), password_.c_str());
        if (!uploadAuthorized_) {
            Serial.println("OTA update rejected: bad credentials");
            return;
        }
        Serial.printf("OTA update starting: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (!uploadAuthorized_) return;
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (!uploadAuthorized_) return;
        if (Update.end(true)) {
            Serial.printf("OTA update complete: %u bytes\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

void OtaHandler::handleUpdatePost() {
    if (!uploadAuthorized_) {
        server_.send(401, "text/plain", "Unauthorized");
        return;
    }

    bool ok = !Update.hasError();
    server_.send(200, "text/plain", ok ? "Update OK, rebooting..." : "Update FAILED, see device log");
    uploadAuthorized_ = false;
    if (ok) {
        delay(500); // let the response flush before the reboot drops the connection
        ESP.restart();
    }
}
