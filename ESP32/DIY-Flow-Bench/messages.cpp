/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * messages.cpp - messages functions class
 *
 * Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
 * 
 * For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions 
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * 
 ***/
 
 /*
 
 TODO: Set up messaging
 
 - Handler: 
	 Potentially we can push mulitple messages onto a stack and display them in priority or on rotation
	 Messages are active until cancelled?
	 Messages are active until error has cleared?
 
 - Serial:
	 Handles.serial print messages
	 
 - API:
	 We already have a dedicated API file but ostensibly it could also fall under 'messages'
	 
 - Websocket messages:
	 We send message status to browser to let user know whats happening (currently prepared by this->handler)
	 
 - Log:
	 Send messages to log file
 
 */
 
#include <Arduino.h>

#include "configuration.h"
#include "constants.h"
#include "structs.h"
#include "pins.h"

#include <Wire.h>
#include "messages.h"
#include LANGUAGE_FILE


Messages::Messages() {
	
}


/***********************************************************
* Initialise class
*/
void Messages::begin(void) {
		
	this->beginSerial();
	
}


/***********************************************************
* Begin Serial
*
* Default port Serial0 (U0UXD) used. (Same as programming port / usb)
* NOTE: Serial1 reserved for SPI
* NOTE: Serial2 reserved for gauge comms
*
* Serial.begin(baud-rate, protocol, RX pin, TX pin);
*
* TODO:  Should we move into hardware? 
* Serial port is not specifically tied to messages also shared with API
*/
void Messages::beginSerial(void) {
	
	#if defined SERIAL0_ENABLED
		Serial.begin(SERIAL0_BAUD, SERIAL_8N1 , SERIAL0_RX_PIN, SERIAL0_TX_PIN); 
	#endif
	
}



/***********************************************************
* Message Handler
*
* Processes translated status messages for display in the browser
* Example:
*   _message.Handler(translate.LANG_VAL_SAVING_CONFIG);
*
* NOTE: Language strings are defined in the current language file i.e. /language/XX_Language.h
* TODO: Store last message received for later recall / Store several message that are displayed on rotation
*/
void Messages::Handler(const String langPhrase) {

	extern struct DeviceStatus status;

	// Push the string to the Status Message
	status.statusMessage = langPhrase;
	
	//If we have debug enabled send the message to the serial port
	#if defined DEBUG && defined SERIAL0_ENABLED
		this->serialPrintf("%s  \n", langPhrase); 
	#endif
}



/***********************************************************
* serialPrintf
* Always prints to serial port
* 
* Encapsulates vsnprintf method and uses standard c++ xxprintf formatting
*
* Based on...
* https://forum.arduino.cc/t/esp32-where-can-i-find-the-reference-for-serial-printf-the-print-with-the-f-as-a-suffix/1007598/8
*
* For string formatters see...
* https://cpp4arduino.com/2020/02/07/how-to-format-strings-without-the-string-class.html
* https://en.wikipedia.org/wiki/Printf_format_string
*
* Example using d2str to convert float to char (double to string)
* _message.serialPrintf("kg/h = %c", dtostrf((flowRateRAW / 1000), 7, 2, _message.floatBuffer)); 
*/
size_t Messages::serialPrintf(const std::string format, ...) {
	
	#ifdef SERIAL0_ENABLED
	
		char buf[API_RESPONSE_LENGTH];
		va_list ap;
		va_start(ap, format);
		vsnprintf(buf, sizeof(buf), format.c_str(), ap);
		va_end(ap);
		return(Serial.write(buf));
		
	#endif
}




/***********************************************************
* blobPrintf
* Prints blob to serial port
* 
* Follows same formatting as serialPrintf
*/
size_t Messages::blobPrintf(std::string format, ...) {

	#ifdef SERIAL0_ENABLED
	
		char buf[API_BLOB_LENGTH];
		va_list ap;
		va_start(ap, format);
		vsnprintf(buf, sizeof(buf), format.c_str(), ap);
		va_end(ap);
		return(Serial.write(buf));

	#endif
}



/***********************************************************
* statusPrintf
* Prints to serial port if status_print_mode enabled
* 
* Follows same formatting as serialPrintf
*/
size_t Messages::statusPrintf(const std::string format, ...) {

	#ifdef SERIAL0_ENABLED
	
		extern struct ConfigSettings config;
	
		if (config.status_print_mode) {
			char buf[API_STATUS_LENGTH];
			va_list ap;
			va_start(ap, format);
			vsnprintf(buf, sizeof(buf), format.c_str(), ap);
			va_end(ap);
			return(Serial.write(buf));
		} else {
			return 0;
		}
		
	#endif
}



/***********************************************************
* debugPrintf
* Prints to serial port if debug_mode enabled
* 
* Follows same formatting as serialPrintf
*/
size_t Messages::debugPrintf(const std::string format, ...) {

	#ifdef SERIAL0_ENABLED
	
		extern struct ConfigSettings config;
	
		if (config.debug_mode) {
			char buf[API_STATUS_LENGTH];
			va_list ap;
			va_start(ap, format);
			vsnprintf(buf, sizeof(buf), format.c_str(), ap);
			va_end(ap);
			return(Serial.write(buf));
		} else {
			return 0;
		}
		
	#endif
}













/***********************************************************
* serialPrintf
* 
* Threadsafe print
*/
// extern int  ets_putc(int);
// void * ICACHE_FLASH_ATTR Messages::serialPrintf(const char *s, ...) {
//    int i=0;
//    for (i=0; i<(sizeof(user_printfHook)/4); i++)   {
// 	  if (user_printfHook[i] != NULL)  {
// 		 ((void (*)(const char *))user_printfHook[i])(s);
// 	  }
//    }
//    va_list args;
//    va_start(args, s);
//    ets_vprintf(ets_putc, s, args);
//    va_end(args);
// }



