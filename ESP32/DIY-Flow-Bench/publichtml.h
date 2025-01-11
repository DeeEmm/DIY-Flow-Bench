

#pragma once
#include <Arduino.h>

class PublicHTML {
private:
    String decompress(const uint8_t* data, size_t len);
    String decompressMultiple(const uint8_t** arrays, const size_t* lengths, int count);
    String decompressMultipleToStream(const uint8_t** arrays, const size_t* lengths, int count);

public:
    String indexPage();
    String settingsPage();
    String configPage();
    String dataPage();
    String pinsPage();
    String mimicPage();

    String header();
    String footer(); 
    String index();
    String data();
    String settings();
    String pins();
    String css();
    
    String mainJs();
    String indexJs();
    String settingsJs();
    String configJs();
    String pinsJs();
    String mimicJs();
    String dataJs();
};