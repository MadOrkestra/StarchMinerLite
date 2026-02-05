// version.h
// Version information for the StarchMiner firmware
#ifndef VERSION_H
#define VERSION_H

// Allow version to be overridden by build flags
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

#define FIRMWARE_BUILD_DATE __DATE__
#define FIRMWARE_BUILD_TIME __TIME__

// Version comparison helper
namespace Version {
    // Get current firmware version
    const char* getFirmwareVersion();
    
    // Get build date and time
    const char* getBuildDate();
    const char* getBuildTime();
    
    // Compare version strings (returns -1 if v1 < v2, 0 if equal, 1 if v1 > v2)
    int compareVersions(const char* v1, const char* v2);
}

#endif // VERSION_H
