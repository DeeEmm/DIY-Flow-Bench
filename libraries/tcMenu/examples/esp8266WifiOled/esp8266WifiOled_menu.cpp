/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#include <tcMenu.h>
#include "esp8266WifiOled_menu.h"

// Global variable declarations

U8g2GfxMenuConfig gfxConfig;
U8g2MenuRenderer renderer;
WiFiServer server(3333);

// Global Menu Item declarations

RENDERING_CALLBACK_NAME_INVOKE(fnIpAddressRtCall, ipAddressRenderFn, "IpAddress", -1, NULL)
IpAddressMenuItem menuIpAddress(fnIpAddressRtCall, 10, NULL);
RENDERING_CALLBACK_NAME_INVOKE(fnPwdRtCall, textItemRenderFn, "Pwd", 23, NULL)
TextMenuItem menuPwd(fnPwdRtCall, 12, 15, &menuIpAddress);
RENDERING_CALLBACK_NAME_INVOKE(fnSSIDRtCall, textItemRenderFn, "SSID", 8, NULL)
TextMenuItem menuSSID(fnSSIDRtCall, 11, 15, &menuPwd);
RENDERING_CALLBACK_NAME_INVOKE(fnConnectivityRtCall, backSubItemRenderFn, "Connectivity", -1, NULL)
const PROGMEM SubMenuInfo minfoConnectivity = { "Connectivity", 9, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackConnectivity(fnConnectivityRtCall, &menuSSID);
SubMenuItem menuConnectivity(&minfoConnectivity, &menuBackConnectivity, NULL);
const PROGMEM AnyMenuInfo minfoSaveAll = { "Save All", 8, 0xffff, 0, onSaveAll };
ActionMenuItem menuSaveAll(&minfoSaveAll, NULL);
const char enumStrWinOpening_0[] PROGMEM = "NARROW";
const char enumStrWinOpening_1[] PROGMEM = "WIDE";
const char* const enumStrWinOpening[] PROGMEM  = { enumStrWinOpening_0, enumStrWinOpening_1 };
const PROGMEM EnumMenuInfo minfoWinOpening = { "Win Opening", 7, 6, 1, onWindowOpening, enumStrWinOpening };
EnumMenuItem menuWinOpening(&minfoWinOpening, 0, &menuSaveAll);
const char enumStrHeaterPower_0[] PROGMEM = "LOW";
const char enumStrHeaterPower_1[] PROGMEM = "MEDIUM";
const char enumStrHeaterPower_2[] PROGMEM = "HIGH";
const char* const enumStrHeaterPower[] PROGMEM  = { enumStrHeaterPower_0, enumStrHeaterPower_1, enumStrHeaterPower_2 };
const PROGMEM EnumMenuInfo minfoHeaterPower = { "Heater Power", 6, 4, 2, onHeaterPower, enumStrHeaterPower };
EnumMenuItem menuHeaterPower(&minfoHeaterPower, 0, &menuWinOpening);
RENDERING_CALLBACK_NAME_INVOKE(fnSetupRtCall, backSubItemRenderFn, "Setup", -1, NULL)
const PROGMEM SubMenuInfo minfoSetup = { "Setup", 5, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackSetup(fnSetupRtCall, &menuHeaterPower);
SubMenuItem menuSetup(&minfoSetup, &menuBackSetup, &menuConnectivity);
const PROGMEM BooleanMenuInfo minfoElectricHeater = { "Electric Heater", 4, 3, 1, onElectricHeater, NAMING_ON_OFF };
BooleanMenuItem menuElectricHeater(&minfoElectricHeater, false, &menuSetup);
const PROGMEM BooleanMenuInfo minfoWindowOpen = { "Window Open", 3, 2, 1, onWindowOpen, NAMING_YES_NO };
BooleanMenuItem menuWindowOpen(&minfoWindowOpen, false, &menuElectricHeater);
const PROGMEM AnalogMenuInfo minfoCucumberTemp = { "Cucumber Temp", 2, 0xffff, 255, NO_CALLBACK, -20, 4, "C" };
AnalogMenuItem menuCucumberTemp(&minfoCucumberTemp, 0, &menuWindowOpen);
const PROGMEM AnalogMenuInfo minfoTomatoTemp = { "Tomato Temp", 1, 0xffff, 255, NO_CALLBACK, -20, 4, "C" };
AnalogMenuItem menuTomatoTemp(&minfoTomatoTemp, 0, &menuCucumberTemp);
const PROGMEM ConnectorLocalInfo applicationInfo = { "Greenhouse", "01b9cb76-c108-4be3-a133-6159f8f1c9c1" };

// Set up code

void setupMenu() {
    prepareBasicU8x8Config(gfxConfig);
    renderer.setGraphicsDevice(&gfx, &gfxConfig);
    switches.initialise(io8574, true);
    menuMgr.initForEncoder(&renderer, &menuTomatoTemp, ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PIN_OK);
    remoteServer.begin(&server, &applicationInfo);

    // Read only and local only function calls
    menuCucumberTemp.setReadOnly(true);
    menuIpAddress.setReadOnly(true);
    menuTomatoTemp.setReadOnly(true);
    menuPwd.setLocalOnly(true);
    menuSaveAll.setLocalOnly(true);
    menuSSID.setLocalOnly(true);
}
