/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file messages.cpp
 * 
 * @brief Message class 
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
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
#include "system.h"
#include "constants.h"
#include "structs.h"
// #include "pins.h"

#include <Wire.h>
#include "messages.h"
// #include LANGUAGE_FILE


Messages::Messages() {
	
}





/***********************************************************
 * @brief Message Handler
 *
 * @details Translates status messages and stores in global struct
 * @example _message.Handler(language.LANG_SAVING_CONFIG);
 * @note - Language strings are defined in the current language file i.e. /language/XX_Language.h
 * @note - statusMessage is pushed to client as part of JSON data created in webserver::getJsonData()
 * 
 * TODO: Store last message received for later recall / Store several message that are displayed on rotation
 ***/
void Messages::Handler(const String langPhrase) {

	extern struct DeviceStatus status;

	// store the string to the Status Message global struct
	status.statusMessage = langPhrase;
	
	// If we have debug enabled send the message to the serial port
	#if defined DEBUG && defined SERIAL0_ENABLED
		this->serialPrintf("%s  \n", langPhrase); 
	#endif
}



/***********************************************************
* @brief serialPrintf
* @details Always prints to serial port
* 
* @note Encapsulates vsnprintf method and uses standard c++ xxprintf formatting
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
	#else	
		return 0;
	#endif
}




/***********************************************************
* @brief blobPrintf
* @details Prints blob to serial port
* 
* @note Follows same formatting as serialPrintf
*/
size_t Messages::blobPrintf(std::string format, ...) {

	#ifdef SERIAL0_ENABLED
	
		char buf[API_BLOB_LENGTH];
		va_list ap;
		va_start(ap, format);
		vsnprintf(buf, sizeof(buf), format.c_str(), ap);
		va_end(ap);
		return(Serial.write(buf));
	#else	
		return 0;
	#endif
}



/***********************************************************
* @brief statusPrintf
* @details Prints to serial port if status_print_mode enabled
* 
* @note Follows same formatting as serialPrintf
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
	#else	
		return 0;
	#endif
}



/***********************************************************
* @brief debugPrintf
* @details Prints to serial port if debug_mode enabled
* 
* @note Follows same formatting as serialPrintf
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
	#else	
		return 0;	
	#endif
}

