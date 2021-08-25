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
 
 - Status: 
	 Receives int from calling statement, this updates global message variable with current status
	 Potentially we can push mulitple messages onto a stack and display them in priority or on rotation
	 Messages are active until cancelled?
	 Messages are active until error has cleared?
 
 - Serial:
	 Handles.serial print messages
	 messages can be allocated to any serial port (set via config)
	 
 - API:
	 We already have a dedicated API file but ostensibly it could also fall under 'messages'
	 
 - Websocket messages:
	 We send message status to browser to let user know whats happening
	 
 - Log:
	 Send messages to log file
 
 */

#include "messages.h"
#include "structs.h"

Messages::Messages () {


}



/***********************************************************
* Handler
*/
void Messages::Handler(String languageString) {

	extern struct DeviceStatus status;

	// NOTE: Language string is #defined in the current language file
	
	// Push the string to the Status Message
	status.statusMessage = languageString;
	
	//If we have debug enabled send the message to the serial port
	#ifdef DEBUG 
		this->serialPrintLn(status.statusMessage); 
	#endif
	
	
}



/***********************************************************
* DebugPrint
*/
void Messages::DebugPrint(String message) {

	extern struct ConfigSettings config;

	if (config.debug_mode) {
		this->SerialPrint(message);
	}
		
}




/***********************************************************
* DebugPrintLn
*/
void Messages::DebugPrintLn(String message) {

	extern struct ConfigSettings config;

	if (config.debug_mode) {
		this->SerialPrintLn(message);
	}
		
}


/***********************************************************
* SerialPrint
*/
void Messages::SerialPrint (String message) {
	Serial.print(message);
}



/***********************************************************
* SerialPrintLn
*/
void Messages::SerialPrintLn (String message) {
	Serial.println(message);
}


