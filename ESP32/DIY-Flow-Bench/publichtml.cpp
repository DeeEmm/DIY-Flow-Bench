/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file publichtml.cpp
 * 
 * @brief html handler class
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/

#include "Arduino.h"
#include "publichtml.h"
#define MINIZ_NO_STDIO
#define MINIZ_NO_TIME
#include "miniz.h"

#include "htmldata.h"

uint8_t decompBuffer[21000]; 
// uint8_t decompBuffer[26000]; 

// Combine arrays

/***********************************************************
 * @brief Combine arrays
 * @details Combines two arrays into a single array
 ***/
uint8_t* combineArrays(const uint8_t* arr1, size_t len1, const uint8_t* arr2, size_t len2) {
    uint8_t* result = new uint8_t[len1 + len2];
    memcpy(result, arr1, len1);
    memcpy(result + len1, arr2, len2);
    return result;
}



/***********************************************************
 * @brief Decompress Byte data
 * @details decompresses byte array data
 ***/
String PublicHTML::decompress(const uint8_t* data, size_t len) {

    // int level = 9; //miniz 

    memset(decompBuffer, 0, sizeof(decompBuffer));
    // uint8_t buffer[1024];

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    
    inflateInit(&stream);
    stream.next_in = (uint8_t*)data;
    stream.avail_in = len;
    stream.next_out = decompBuffer;
    stream.avail_out = sizeof(decompBuffer);
    
    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    
    return String((char*)decompBuffer);
}




/***********************************************************
 * @brief decompressToStream
 * @details decompresses byte array data to stream
 ***/
String PublicHTML::decompressToStream(const uint8_t* data, size_t len) {
    z_stream stream;
    memset(decompBuffer, 0, sizeof(decompBuffer));
    String result;
    
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    
    if (inflateInit(&stream) != Z_OK) {
        return  "Decompression error [Init]";
    }
    
    stream.next_in = (uint8_t*)data;
    stream.avail_in = len;
    
    do {
        stream.next_out = decompBuffer;
        stream.avail_out = sizeof(decompBuffer);
        
        int ret = inflate(&stream, Z_NO_FLUSH);
        
        if (ret != Z_OK && ret != Z_STREAM_END) {
            inflateEnd(&stream);
            return "Decompression error [NOK]";
        }
        
        size_t bytesToWrite = sizeof(decompBuffer) - stream.avail_out;

        // if (bytesToWrite > 0) {
        //     output->write(buffer, bytesToWrite);
        // }

        result += String((char*)decompBuffer, bytesToWrite);
        
    } while (stream.avail_out == 0);
    
    inflateEnd(&stream);
    return result;
}




/***********************************************************
 * @brief Decompress Multiple Byte arrays 
 * @details decompresses byte array data
 ***/
String PublicHTML::decompressMultiple(const uint8_t** arrays, const size_t* lengths, int count) {
  size_t totalLen = 0;
  for(int i = 0; i < count; i++) {
    totalLen += lengths[i];
  }
  
  uint8_t* combined = new uint8_t[totalLen];
  size_t offset = 0;
  
  for(int i = 0; i < count; i++) {
    memcpy(combined + offset, arrays[i], lengths[i]);
    offset += lengths[i];
  }
  
  String result = decompress(combined, totalLen);
  delete[] combined;
  return result;
}



/***********************************************************
 * @brief Decompress Multiple Byte arrays to stream
 * @details decompresses byte array data to stream
 ***/
String PublicHTML::decompressMultipleToStream(const uint8_t** arrays, const size_t* lengths, int count) {
  
//   int level = 9;

  memset(decompBuffer, 0, sizeof(decompBuffer));

  z_stream stream;
  String result;

  for(int i = 0; i < count; i++) {

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (inflateInit(&stream) != Z_OK) {
      return "Decompression error [Init]";
    }
    
    stream.next_in = (uint8_t*)arrays[i];
    stream.avail_in = lengths[i];
    
    int ret;
    do {
      stream.next_out = decompBuffer;
      stream.avail_out = sizeof(decompBuffer);
      
      ret = inflate(&stream, Z_NO_FLUSH);
      if (ret != Z_OK && ret != Z_STREAM_END) {
        inflateEnd(&stream);
        return "Decompression error [NOK]";
      }
      
      size_t bytesToWrite = sizeof(decompBuffer) - stream.avail_out;
      result += String((char*)decompBuffer, bytesToWrite);
      
    } while (stream.avail_out == 0);
    
    inflateEnd(&stream);
  }
  
  return result;
}


/***********************************************************
 * @brief Construct Index page and decompress to stream
 ***/
String PublicHTML::indexPage() {
    const uint8_t* arrays[] = {header_html, index_html, footer_html};
    const size_t lengths[] = {header_html_len, index_html_len, footer_html_len};
    return decompressMultipleToStream(arrays, lengths, 3);
}


/***********************************************************
 * @brief Construct Settings page and decompress to stream
 ***/
String PublicHTML::settingsPage() {
    // NOTE settings page split into two parts to address memory issues
    const uint8_t* arrays[] = {header_html, settings_modals_html, settings_html, footer_html};
    const size_t lengths[] = {header_html_len, settings_modals_html_len, settings_html_len, footer_html_len};
    return decompressMultipleToStream(arrays, lengths, 4);
}


/***********************************************************
 * @brief Construct Data page and decompress to stream
 ***/
String PublicHTML::dataPage() {
    const uint8_t* arrays[] = {header_html, data_html, footer_html};
    const size_t lengths[] = {header_html_len, data_html_len, footer_html_len};
    return decompressMultipleToStream(arrays, lengths, 3);
}


/***********************************************************
 * @brief Construct Calibration page and decompress to stream
 ***/
String PublicHTML::calibrationPage() {
    const uint8_t* arrays[] = {header_html, calibration_html, footer_html};
    const size_t lengths[] = {header_html_len, calibration_html_len, footer_html_len};
    return decompressMultipleToStream(arrays, lengths, 3);
}


/***********************************************************
 * @brief Construct Pins page and decompress to stream
 ***/
String PublicHTML::pinsPage() {
    const uint8_t* arrays[] = {header_html, pins_html, footer_html};
    const size_t lengths[] = {header_html_len, pins_html_len, footer_html_len};
    return decompressMultipleToStream(arrays, lengths, 3);
}


/***********************************************************
 * @brief Construct Config page and decompress to stream
 ***/
String PublicHTML::configPage() {
    const uint8_t* arrays[] = {header_html, config_html, footer_html};
    const size_t lengths[] = {header_html_len, config_html_len, footer_html_len};
    return decompressMultipleToStream(arrays, lengths, 3);
}


/***********************************************************
 * @brief Construct mimic page and decompress to stream
 ***/
String PublicHTML::mimicPage() {
    const uint8_t* arrays[] = {header_html, mimic_html, footer_html};
    const size_t lengths[] = {header_html_len, mimic_html_len, footer_html_len};
    return decompressMultipleToStream(arrays, lengths, 3);
}


/***********************************************************
 * @brief Decompress header and decompress to stream
 ***/
String PublicHTML::header() {
    return decompress(header_html, header_html_len);
}


/***********************************************************
 * @brief Decompress Footer and decompress to stream
 ***/
String PublicHTML::footer() {
    return decompress(footer_html, footer_html_len);
}


/***********************************************************
 * @brief Decompress index and decompress to stream
 ***/
String PublicHTML::index() {
    return decompress(index_html, index_html_len);
}


/***********************************************************
 * @brief Decompress data and decompress to stream
 ***/
String PublicHTML::data() {
    return decompress(data_html, data_html_len);
}


/***********************************************************
 * @brief Decompress settings and decompress to stream
 ***/
String PublicHTML::settings() {
    return decompress(settings_html, settings_html_len);
}


/***********************************************************
 * @brief Decompress pins and decompress to stream
 ***/
String PublicHTML::pins() {
    return decompress(pins_html, pins_html_len);
}


/***********************************************************
 * @brief Decompress css and decompress to stream
 ***/
String PublicHTML::css() {
    return decompress(style_css, style_css_len);
}


/***********************************************************
 * @brief Decompress index.js
 ***/
String PublicHTML::indexJs() {
    return decompress(index_js, index_js_len);
}


/***********************************************************
 * @brief Decompress settings.js
 ***/
String PublicHTML::settingsJs() {
    return decompress(settings_js, settings_js_len);
}


/***********************************************************
 * @brief Decompress config.js
 ***/
String PublicHTML::configJs() {
    return decompress(config_js, config_js_len);
}


/***********************************************************
 * @brief Decompress mimic.js
 ***/
String PublicHTML::mimicJs() {
    return decompress(mimic_js, mimic_js_len);
}


/***********************************************************
 * @brief Decompress data.js
 ***/
String PublicHTML::dataJs() {
    return decompress(data_js, data_js_len);
}


/***********************************************************
 * @brief Decompress calibration.js
 ***/
String PublicHTML::calibrationJs() {
    return decompress(calibration_js, calibration_js_len);
}


/***********************************************************
 * @brief Decompress pins.js
 ***/
// String PublicHTML::pinsJs() {
//     return decompress(pins_js, pins_js_len);
// }

