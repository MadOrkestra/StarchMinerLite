// version.cpp
#include "version.h"
#include <Arduino.h>

namespace Version {

const char* getFirmwareVersion() {
    return FIRMWARE_VERSION;
}

const char* getBuildDate() {
    return FIRMWARE_BUILD_DATE;
}

const char* getBuildTime() {
    return FIRMWARE_BUILD_TIME;
}

// Simple semantic version comparison (major.minor.patch)
int compareVersions(const char* v1, const char* v2) {
    int major1 = 0, minor1 = 0, patch1 = 0;
    int major2 = 0, minor2 = 0, patch2 = 0;
    
    // Parse v1
    sscanf(v1, "%d.%d.%d", &major1, &minor1, &patch1);
    
    // Parse v2
    sscanf(v2, "%d.%d.%d", &major2, &minor2, &patch2);
    
    // Compare major
    if (major1 < major2) return -1;
    if (major1 > major2) return 1;
    
    // Compare minor
    if (minor1 < minor2) return -1;
    if (minor1 > minor2) return 1;
    
    // Compare patch
    if (patch1 < patch2) return -1;
    if (patch1 > patch2) return 1;
    
    return 0; // Equal
}

} // namespace Version
