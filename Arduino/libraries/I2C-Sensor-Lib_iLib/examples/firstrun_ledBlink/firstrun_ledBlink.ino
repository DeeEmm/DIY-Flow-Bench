/**<

Program Fuses:

EXT     0xFD
HIGH    0xDF or xDE (bootrst EN)
LOW     0xFF

 */

#define TIMEON   20
#define TIMEOFF  (1000-TIMEON)

#define LEDPINA   8
#define LEDPINB   11

void setup() {
  pinMode(LEDPINA, OUTPUT);
  pinMode(LEDPINB, OUTPUT);
  digitalWrite(LEDPINA,LOW);
  digitalWrite(LEDPINB,LOW);

}

void loop() {

  delay(TIMEOFF);
  digitalWrite(LEDPINA,HIGH);
  digitalWrite(LEDPINB,HIGH);
  delay(TIMEON); // busywaiting
  digitalWrite(LEDPINA,LOW);
  digitalWrite(LEDPINB,LOW);

}


