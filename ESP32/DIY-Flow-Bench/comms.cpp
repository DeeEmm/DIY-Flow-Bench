/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file wifi.cpp
 * 
 * @brief wifi class
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
#include "comms.h"

#include "structs.h"
#include "messages.h"
#include "datahandler.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>

/***********************************************************
 * @brief INITIALISE WIFI
 * @note WiFi Error Codes
 * Value	Constant	Meaning
 * 0	WL_IDLE_STATUS	temporary status assigned when WiFi.begin() is called
 * 1	WL_NO_SSID_AVAIL	 when no SSID are available
 * 2	WL_SCAN_COMPLETED	scan networks is completed
 * 3	WL_CONNECTED	when connected to a WiFi network
 * 4	WL_CONNECT_FAILED	when the connection fails for all the attempts
 * 5	WL_CONNECTION_LOST	when the connection is lost
 * 6	WL_DISCONNECTED	when disconnected from a network
 * 
 ***/
void Comms::initaliseWifi() {

  extern struct ConfigSettings config;
  // extern struct Language language;
  extern struct DeviceStatus status;

  int wifiStatusCode;

  Messages _message;
  DataHandler _data;

  // if WiFi password is unedited or blank force AP mode
  if ((strstr(String(config.wifi_pswd).c_str(), String("PASSWORD").c_str())) || (String(config.wifi_pswd).c_str() == "")) {
    config.ap_mode = true;
  } 
  
  // Connect to WiFi
  if (config.ap_mode == false)  {

    // WiFi.useStaticBuffers(true);   
    this->resetWifi();
    WiFi.mode(WIFI_STA);

    // Set MAC address
    #ifdef MAC_ADDRESS
      uint8_t newMACAddress[] = MAC_ADDRESS;
      esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
    #endif

    // Display MAC Address
    _message.serialPrintf("WiFi MAC Address: %s \n", String(WiFi.macAddress()).c_str());  
    _message.serialPrintf("Connecting to WiFi \n");
    #ifdef STATIC_IP
      // Configures static IP address
      if (!WiFi.config(STATIC_IP, GATEWAY, SUBNET)) {
        Serial.println("STA Failed to configure");
      }
    #endif
    wifiStatusCode = this->getWifiConnection();
  }

  // Test for connection success else create an accesspoint
  if (wifiStatusCode == 3 && config.ap_mode == false) { 
    // STA Connection success
    _message.serialPrintf("Connected to %s \n", config.wifi_ssid);
    status.local_ip_address = WiFi.localIP().toString().c_str();
    _message.serialPrintf("IP address: %s \n", WiFi.localIP().toString().c_str());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    
  }  else  { // Go into AP Mode
    if (config.ap_mode == true) { // AP mode is Default
      _message.serialPrintf("Defaulting to AP Mode \n");
    } else { // AP mode is Fallback
      _message.serialPrintf("Failed to connect to Wifi \n");
      _message.serialPrintf("Wifi Status message: ");
      _message.serialPrintf(String(wifiStatusCode).c_str());
      _message.serialPrintf("\n");
    }

    _message.serialPrintf("Creating WiFi Access Point:  %s  \n", config.wifi_ap_ssid); // NOTE: Default AP SSID / PW = DIYFB / 123456789
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.wifi_ap_ssid, config.wifi_ap_pswd);
    status.local_ip_address = WiFi.softAPIP().toString().c_str();
    _message.serialPrintf("Access Point IP address: %s \n", WiFi.softAPIP().toString().c_str());
    status.apMode = true;
  }

  // Set up Multicast DNS
  if (!MDNS.begin(config.hostname))  {
    _message.serialPrintf("Error starting mDNS \n");
  }  else  {
    status.hostname = config.hostname;
    _message.serialPrintf("Multicast: http://%s.local \n", status.hostname);
  }

}








/***********************************************************
 * @brief reset wifi connection
 * @note helps to overcome ESP32/Arduino Wifi bug [#111]
 * @note https://github.com/espressif/arduino-esp32/issues/2501
 ***/
void Comms::resetWifi ( void ) {
	WiFi.persistent(false);
  // WiFi.disconnect(true);
    WiFi.disconnect(true, true); // clears NVS
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_MODE_NULL);
  // delay(500);
}







/***********************************************************
 * @brief reset wifi connection
 * @note helps to overcome ESP32/Arduino Wifi bug [#111]
 * @note https://github.com/espressif/arduino-esp32/issues/2501
 * @note instantiated via API call
 * @todo instatiate via I/O (pushbutton)
 ***/
void Comms::wifiReconnect ( void ) {

  WiFi.reconnect();
    
}






/***********************************************************
 * @brief get wifi connection
 * @return status value
 * @var wifi_retries : number of retry attempts
 * @var wifi_timeout : time in milliseconds
 ***/
int Comms::getWifiConnection(){

  Messages _message;
  extern struct ConfigSettings config;
  
  uint8_t wifiConnectionAttempt = 1;
  uint8_t wifiConnectionStatus;

  for(;;) {
          
    WiFi.begin(config.wifi_ssid, config.wifi_pswd); 
    wifiConnectionStatus = WiFi.waitForConnectResult(config.wifi_timeout);
    if (wifiConnectionStatus == WL_CONNECTED || wifiConnectionAttempt > config.wifi_retries){
      break;
    } else if (wifiConnectionStatus != WL_DISCONNECTED) {
      resetWifi();
      delay (2000);
    } else {
      WiFi.disconnect();
    }
    _message.serialPrintf(".");
    wifiConnectionAttempt++;
    // WiFi.reconnect();
  }
 
  return wifiConnectionStatus;

}