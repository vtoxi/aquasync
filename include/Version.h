#pragma once

// Bumped by the release workflow to match the git tag when it builds a
// release binary (see .github/workflows/release.yml) -- local/dev builds
// keep this default so it's obvious on the OTA page and over MQTT that
// you're not running a tagged release.
#define FIRMWARE_VERSION "0.0.0-dev"
