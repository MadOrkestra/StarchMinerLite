// miner.h
#pragma once

#include <Arduino.h>

namespace Miner {
    // Initialize miner module
    void begin();
    
    // Fetch the latest blockchain hash from the API
    // Returns true if successful, false on error
    bool fetchLastHash();
    
    // Calculate SHA-256 hash of a string
    // Returns hex string representation of the hash
    String sha256(String ascii);
    
    // Submit a block to the blockchain API
    // Returns true if successful, false on error
    bool submitBlock(String payload);
    
    // Get the number of successfully submitted blocks (in-memory only)
    int getSubmittedBlockCount();
}
