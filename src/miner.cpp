// miner.cpp
#include "miner.h"
#include "config.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <vector>

namespace Miner {

// Track the number of successfully submitted blocks (in-memory only)
static int submittedBlockCount = 0;

// Initialize miner module
void begin() {
    submittedBlockCount = 0;
    Serial.println("Miner: Miner initialized (stats in memory only)");
}

uint32_t rightRotate(uint32_t value, uint32_t amount) {
    return (value >> amount) | (value << (32 - amount));
}

String sha256(String ascii) {
    // Static persistent arrays like sha256.h and sha256.k in JS
    static std::vector<uint32_t> hash;
    static std::vector<uint32_t> k;
    
    double maxWord = 4294967296.0; // 2^32 as double for precision
    int lengthProperty = ascii.length();
    int i, j;
    String result = "";
    std::vector<uint32_t> words;
    uint64_t asciiBitLength = lengthProperty * 8;
    
    // Initialize hash and k arrays if not done yet
    // Using pre-calculated values that match what the JS dynamic calculation produces
    if (k.size() < 64) {
        // These are the exact values the JS function generates from primes
        uint32_t hash_init[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };
        
        uint32_t k_init[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };
        
        for (i = 0; i < 8; i++) {
            hash.push_back(hash_init[i]);
        }
        
        // The JS function actually generates MORE than 8 hash values (64 total from the primes)
        // But only uses the first 8 for initial state. Let's pad to 64 for compatibility
        for (i = 8; i < 64; i++) {
            hash.push_back(0);
        }
        
        for (i = 0; i < 64; i++) {
            k.push_back(k_init[i]);
        }
    }
    
    // Padding: append 0x80 byte
    ascii += '\x80';
    
    // Pad with 0x00 until length % 64 == 56
    while (ascii.length() % 64 != 56) {
        ascii += '\x00';
    }
    
    // Pack bytes into 32-bit words (big-endian) exactly as JS does
    for (i = 0; i < ascii.length(); i++) {
        j = (uint8_t)ascii[i];
        if (j >> 8) return ""; // ASCII check: only 0-255
        
        int wordIndex = i >> 2;
        // Ensure words vector is large enough
        while (words.size() <= wordIndex) {
            words.push_back(0);
        }
        words[wordIndex] |= j << ((3 - (i % 4)) * 8);
    }
    
    // Append length as two 32-bit words (high and low) - exactly as JS does
    int wordsLength = words.size();
    words.push_back((uint32_t)(asciiBitLength / maxWord)); // High 32 bits
    words.push_back((uint32_t)asciiBitLength);             // Low 32 bits
    
    // Initialize working hash with first 8 values from static hash array
    std::vector<uint32_t> workingHash;
    for (i = 0; i < 8; i++) {
        workingHash.push_back(hash[i]);
    }
    
    // Process message in 16-word chunks
    for (j = 0; j < words.size(); ) {
        // Take slice of 16 words - exactly as JS: words.slice(j, j += 16)
        std::vector<uint32_t> w(64, 0);
        for (i = 0; i < 16 && (j + i) < words.size(); i++) {
            w[i] = words[j + i];
        }
        j += 16;
        
        // Save old hash state - exactly as JS: var oldHash = hash;
        std::vector<uint32_t> oldHash = workingHash;
        
        // Main compression loop - 64 rounds
        for (i = 0; i < 64; i++) {
            uint32_t w15 = (i >= 15) ? w[i - 15] : 0;
            uint32_t w2 = (i >= 2) ? w[i - 2] : 0;
            
            uint32_t a = workingHash[0];
            uint32_t e = workingHash[4];
            
            // Calculate temp1 - exactly as JS
            uint32_t S1 = rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25);
            uint32_t ch = (e & workingHash[5]) ^ ((~e) & workingHash[6]);
            
            // Expand w[i] if needed - exactly as JS
            if (i >= 16) {
                uint32_t s0 = rightRotate(w15, 7) ^ rightRotate(w15, 18) ^ (w15 >> 3);
                uint32_t s1 = rightRotate(w2, 17) ^ rightRotate(w2, 19) ^ (w2 >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }
            
            uint32_t temp1 = workingHash[7] + S1 + ch + k[i] + w[i];
            
            // Calculate temp2 - exactly as JS
            uint32_t S0 = rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22);
            uint32_t maj = (a & workingHash[1]) ^ (a & workingHash[2]) ^ (workingHash[1] & workingHash[2]);
            uint32_t temp2 = S0 + maj;
            
            // Shift hash array - exactly as JS: hash = [(temp1 + temp2)|0].concat(hash)
            for (int shift = 7; shift > 0; shift--) {
                workingHash[shift] = workingHash[shift - 1];
            }
            workingHash[0] = temp1 + temp2;
            workingHash[4] = workingHash[4] + temp1;
        }
        
        // Add chunk hash to result - exactly as JS: hash[i] = (hash[i] + oldHash[i])|0
        for (i = 0; i < 8; i++) {
            workingHash[i] = workingHash[i] + oldHash[i];
        }
    }
    
    // Format output as hex string - exactly as JS
    for (i = 0; i < 8; i++) {
        for (j = 3; j >= 0; j--) {
            uint8_t b = (workingHash[i] >> (j * 8)) & 255;
            if (b < 16) result += "0";
            result += String(b, HEX);
        }
    }
    
    result.toLowerCase();
    return result;
}

bool submitBlock(String payload) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Miner: WiFi not connected, cannot submit block");
        return false;
    }

    const char *host = "api.starch.one";
    String path = "/submit_blocks";

    Serial.print("Miner: Submitting block to https://");
    Serial.print(host);
    Serial.println(path);

    WiFiClientSecure client;
    client.setInsecure(); // skip cert verification for now
    if (!client.connect(host, 443)) {
        Serial.println("Miner: Failed to connect to submit API host");
        return false;
    }

    // Send POST request
    client.printf("POST %s HTTP/1.1\r\n", path.c_str());
    client.printf("Host: %s\r\n", host);
    client.printf("Content-Type: application/json\r\n");
    client.printf("Content-Length: %d\r\n", payload.length());
    client.printf("Connection: close\r\n");
    client.printf("User-Agent: ESP32\r\n\r\n");
    client.print(payload);

    unsigned long start = millis();
    while (!client.available() && millis() - start < 5000) {
        delay(10);
    }

    if (!client.available()) {
        Serial.println("Miner: No response from submit API");
        client.stop();
        return false;
    }

    // Read response
    String response = "";
    while (client.available()) {
        response += client.readStringUntil('\n');
    }
    client.stop();

    Serial.print("Miner: Submit API response: ");
    Serial.println(response);

    // Parse JSON response to check for success
    // Look for JSON body in the response (skip HTTP headers)
    int jsonStart = response.indexOf('{');
    if (jsonStart >= 0) {
        String jsonBody = response.substring(jsonStart);
        
        JsonDocument responseDoc;
        DeserializationError err = deserializeJson(responseDoc, jsonBody);
        
        if (!err) {
            // Check if our miner_id exists in response
            String minerId = Config::minerId();
            if (responseDoc[minerId].is<JsonObject>()) {
                JsonObject minerResponse = responseDoc[minerId];
                if (minerResponse["block"].as<String>() == "pending") {
                    Serial.println("Miner: Block accepted - status: pending");
                    return true;
                } else {
                    String blockStatus = minerResponse["block"].as<String>();
                    Serial.print("Miner: Block rejected - status: ");
                    Serial.println(blockStatus);
                    return false;
                }
            }
        }
    }

    // Fallback: check for 200 OK in headers
    if (response.indexOf("200 OK") >= 0) {
        return true;
    }

    return false;
}

bool fetchLastHash() {
    static String previousHash = ""; // Track the last hash we processed
    static String lastSubmittedHash = ""; // Track the last hash we successfully submitted
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Miner: WiFi not connected, cannot fetch last hash");
        return false;
    }

    const char *host = "api.starch.one";
    String path = "/blockchain/last_hash";

    Serial.print("Miner: Fetching https://");
    Serial.print(host);
    Serial.println(path);

    WiFiClientSecure client;
    client.setInsecure(); // skip cert verification for now
    if (!client.connect(host, 443)) {
        Serial.println("Miner: Failed to connect to API host");
        return false;
    }

    // Send request
    client.printf("GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nUser-Agent: ESP32\r\n\r\n", path.c_str(), host);

    unsigned long start = millis();
    while (!client.available() && millis() - start < 5000) {
        delay(10);
    }

    if (!client.available()) {
        Serial.println("Miner: No response from API");
        client.stop();
        return false;
    }

    // Skip headers
    String line;
    while (client.available()) {
        line = client.readStringUntil('\n');
        if (line == "\r" || line.length() == 0) break;
    }

    // Read body
    String body;
    while (client.available()) {
        body += client.readStringUntil('\n');
    }
    client.stop();

    if (body.length() == 0) {
        Serial.println("Miner: Empty response body");
        return false;
    }

    // Parse JSON response
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        Serial.print("Miner: JSON parse error: ");
        Serial.println(err.c_str());
        Serial.print("Miner: Raw response: ");
        Serial.println(body);
        return false;
    }

    // Extract and log the hash
    if (!doc["hash"].isNull()) {
        String lastHash = doc["hash"].as<String>();
        Serial.print("Miner: Last blockchain hash: ");
        Serial.println(lastHash);
        
        // Check if we already successfully submitted a block for this hash
        if (lastHash == lastSubmittedHash) {
            Serial.println("Miner: Block already submitted for this hash");
            return true; // Already done
        }
        
        // Check if this is a new hash we haven't seen before
        if (lastHash != previousHash) {
            Serial.println("Miner: New hash detected, creating block");
            Serial.print("Miner: Previous hash was: ");
            Serial.println(previousHash.length() > 0 ? previousHash : "(none)");
            previousHash = lastHash; // Update the tracked hash
        } else {
            Serial.println("Miner: Retrying block submission for current hash");
        }
        
        Serial.print("Miner: Working with hash: ");
        Serial.println(lastHash);
        Serial.print("Miner: Last submitted hash: ");
        Serial.println(lastSubmittedHash.length() > 0 ? lastSubmittedHash : "(none)");
        
        // Create new block JSON object
        String hashInput = lastHash + " " + Config::minerId() + " " + Config::minerColor();
        Serial.print("Miner: Hash input string: ");
        Serial.println(hashInput);
        String blockHash = sha256(hashInput);
        
        JsonDocument blockDoc;
        blockDoc["hash"] = blockHash;
        blockDoc["miner_id"] = Config::minerId();
        blockDoc["color"] = Config::minerColor();
        
        // Serialize and log the block
        String blockJson;
        serializeJson(blockDoc, blockJson);
        Serial.print("Miner: Created new block: ");
        Serial.println(blockJson);
        
        // Submit the block to the API
        JsonDocument submitDoc;
        JsonArray blocksArray = submitDoc["blocks"].to<JsonArray>();
        blocksArray.add(blockDoc);
        
        String submitPayload;
        serializeJson(submitDoc, submitPayload);
        
        Serial.print("Miner: Submitting payload: ");
        Serial.println(submitPayload);
        
        bool submitted = submitBlock(submitPayload);
        if (submitted) {
            Serial.println("Miner: Block submitted successfully");
            lastSubmittedHash = lastHash; // Mark this hash as successfully submitted
            submittedBlockCount++; // Increment the counter
            Serial.print("Miner: Total blocks submitted: ");
            Serial.println(submittedBlockCount);
            
            return true;
        } else {
            Serial.println("Miner: Failed to submit block - will retry on next attempt");
            return false; // Return false to indicate failure
        }
    } else {
        Serial.println("Miner: No 'hash' field found in response");
        Serial.print("Miner: Response keys: ");
        for (JsonPair kv : doc.as<JsonObject>()) {
            Serial.print(kv.key().c_str());
            Serial.print(" ");
        }
        Serial.println();
        return false;
    }
}

int getSubmittedBlockCount() {
    return submittedBlockCount;
}

} // namespace Miner