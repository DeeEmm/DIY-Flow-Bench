/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display when the display is fitted
 with an I2C backpack. The LiquidCrystal library works with all LCD
 displays that are compatible with the Hitachi HD44780 driver. 

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <LiquidCrystalIO.h>
#include <IoAbstractionWire.h>
#include <Wire.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to, there are two
// common possibilities
const int rs = 0, rw = 1, en = 2, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
//const int rs = 2, en = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, rw, en, d4, d5, d6, d7, ioFrom8574(0x20));

void setup() {
  pinMode(9, OUTPUT);
  analogWrite(9, 10);

  lcd.configureBacklightPin(3);
  lcd.backlight();
  
  Wire.begin();
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}


