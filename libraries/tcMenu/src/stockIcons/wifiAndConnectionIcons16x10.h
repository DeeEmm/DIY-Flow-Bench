/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _WIFI_AND_CONNECTION_ICONS_H_
#define _WIFI_AND_CONNECTION_ICONS_H_

//
// In this file we have the definition of icons for two widgets. You should stick to multiples of 8 pixels (one byte)
// Each icon must be the same size and finally declared in a array at the bottom see bottom of this file.
// These are then used in title widgets that appear in the title bar on graphical displays.
//

const uint8_t iconWifiNotConnected[] PROGMEM = {
	0b00000001, 0b10000000,
	0b00000110, 0b01100000,
	0b00111000, 0b00011100,
	0b11000000, 0b00000011,
	0b01000000, 0b00000010,
	0b00100000, 0b00000100,
	0b00010000, 0b00001000,
	0b00001000, 0b00010000,
	0b00000100, 0b00100000,
	0b00000011, 0b11000000,
};

const uint8_t iconWifiOneBar[] PROGMEM = {
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconWifiTwoBar[] PROGMEM = {
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000111, 0b11100000,
	0b00001100, 0b00110000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconWifiThreeBar[] PROGMEM = {
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000111, 0b11100000,
	0b00011100, 0b00111000,
	0b00110000, 0b00001100,
	0b00000111, 0b11100000,
	0b00001100, 0b00110000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconWifiFourBar[] PROGMEM = {
	0b00000011, 0b11000000,
	0b00001111, 0b11110000,
	0b01110000, 0b00001110,
	0b11000111, 0b11100011,
	0b00011100, 0b00111000,
	0b00110000, 0b00001100,
	0b00000111, 0b11100000,
	0b00001100, 0b00110000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconConnectionNone[] PROGMEM = {
	0b01111111, 0b11111110,
	0b10110000, 0b00000001,
	0b10001100, 0b00000001,
	0b10000110, 0b00000001,
	0b10000011, 0b00000001,
	0b10000000, 0b11000001,
	0b10000001, 0b00110001,
	0b10000000, 0b00001101,
	0b10000001, 0b10000011,
	0b01111111, 0b11111110,
};

const uint8_t iconConnected[] PROGMEM = {
	0b01111111, 0b11111110,
	0b11000000, 0b00000011,
	0b11011101, 0b11101111,
	0b11000000, 0b00000011,
	0b11011011, 0b00110011,
	0b11000000, 0b00000011,
	0b11000000, 0b00000011,
	0b11000000, 0b00000011,
	0b11000001, 0b10000011,
	0b01111111, 0b11111110,
};

const uint8_t* const iconsWifi[]  PROGMEM = { iconWifiNotConnected, iconWifiOneBar, iconWifiTwoBar, iconWifiThreeBar, iconWifiFourBar };
const uint8_t* const iconsConnection[] PROGMEM = { iconConnectionNone, iconConnected };

#endif
