/**<

this is untidy and not very intuitive BUT it works so far
the code was not refactored once... maybe i have time for this in the future

 */


#include "WS2812B.h"
WS2812B ws2812b;

#define LEDS_SIZE     90 //120
#define LEDS_CHANNELS  3
#define LEDS_DELAY    55
#define LEDS_DIST     30 //(LEDS_SIZE/LEDS_CHANNELS)
#define LEDS_INCR     1 //12 //(255/(LEDS_SIZE/LEDS_CHANNELS))
#define LEDS_MAX      LEDS_INCR*LEDS_DIST
struct cRGB led[LEDS_SIZE]; // cGRB

volatile uint8_t selected_led, actual_led, actual_channel, actual_status;

struct time {
   uint8_t tv_hour;
   uint8_t tv_min;
   uint8_t tv_sec;
   uint16_t tv_msec;
} clock;

struct timeA {
   uint16_t tv_hour;
   uint16_t tv_min;
   uint16_t tv_sec;
} analog;

#define SCALEMAX   12000

#define SCALEHA    (SCALEMAX / 24) // 500
#define SCALEHB    (SCALEHA / 60) // 8.33
#define SCALEMA    (SCALEMAX / 60)
#define SCALEMB    (SCALEMA / 60)
#define SCALESA    (SCALEMAX / 60) // 200
#define SCALESB    (SCALESA / 1000) // 0.2
#define SCALESBi   (1000 / SCALESA) // 5
#define SCALELED   (SCALEMAX / LEDS_SIZE)

uint32_t milli_old;

void get_analog() {
  uint32_t milli_new = millis();
  uint32_t tv_msec = clock.tv_msec + (milli_new - milli_old);
  milli_old     = milli_new;
  uint32_t tv_sec  = clock.tv_sec + (tv_msec/1000);
  uint32_t tv_min  = clock.tv_min + (tv_sec / 60);
  uint32_t tv_hour = clock.tv_hour + (tv_min / 60);
  /*
  uint8_t  tv_day  = (tv_hour / 24);
  clock.tv_hour = tv_hour - tv_day * 24;
  clock.tv_min  = tv_min  - tv_hour* 60;
  clock.tv_sec  = tv_sec  - tv_min * 60;
  clock.tv_msec = tv_msec - tv_sec * 1000;
  */
  while (tv_hour > 23)  tv_hour -= 24;
  clock.tv_hour = tv_hour;
  while (tv_min > 59)   tv_min -= 60;
  clock.tv_min = tv_min;
  while (tv_sec > 59)   tv_sec -= 60;
  clock.tv_sec = tv_sec;
  while (tv_msec > 999) tv_msec -= 1000;
  clock.tv_msec = tv_msec;


  // zahlen normiert
  analog.tv_hour = (clock.tv_hour*SCALEHA) + (clock.tv_min*SCALEHB);
  if (analog.tv_hour >= SCALEMAX)   analog.tv_hour -= SCALEMAX;
  analog.tv_min  = (clock.tv_min*SCALEMA) + (clock.tv_sec*SCALEMB);
  if (analog.tv_min >= SCALEMAX)    analog.tv_min -= SCALEMAX;
  analog.tv_sec  = (clock.tv_sec*SCALESA) + (clock.tv_msec/SCALESBi);
  if (analog.tv_sec >= SCALEMAX)    analog.tv_sec -= SCALEMAX;
}


void setup(void) {
  clock.tv_hour = 3;
  clock.tv_min  = 45;
  clock.tv_sec  = 50;
  clock.tv_msec = 0;
}


void loop(void)
{

  uint8_t red, green, blue;
  uint8_t ledi;

  red   = LEDS_MAX;
  green = 0;
  blue  = 0;

  while(1)
  {

    // Farbkreis - Algo
    ledi = actual_led;
    for (uint8_t x=0; x<LEDS_SIZE; x++) {

      if (x < LEDS_DIST)         { blue  = 0; red   = red   - LEDS_INCR; green = green + LEDS_INCR; }
      else if (x < 2*LEDS_DIST)  { red   = 0; green = green - LEDS_INCR; blue  = blue  + LEDS_INCR; }
      else                       { green = 0; blue  = blue  - LEDS_INCR; red   = red   + LEDS_INCR; }

      #define GROUNDLIGHT 0
      #define SHIFTFACTOR   0
      led[ledi].r = GROUNDLIGHT + (red>>SHIFTFACTOR);
      led[ledi].g = GROUNDLIGHT + (green>>SHIFTFACTOR);
      led[ledi].b = GROUNDLIGHT + (blue>>SHIFTFACTOR);

      if (++ledi >= LEDS_SIZE) ledi = 0;
    }
    //if (++actual_led>=LEDS_SIZE) { actual_led = 0; }  // Clockwise Color-Ring
    if (actual_led--==0)  actual_led = LEDS_SIZE - 1;  // Reverse Color-Ring

    // Uhr einfügen
    #define CLOCK_INTENSITY 255
    #define CLOCK_WIDTH     (1)

    uint8_t  led_mid, led_sel, led_overflow;
    int16_t  value;
    uint16_t analog_LED;

    get_analog();

    //if (led[analog.tv_hour/SCALELED].r < CLOCK_INTENSITY) { led[analog.tv_hour/SCALELED].r = CLOCK_INTENSITY; }
    //if (led[analog.tv_min/SCALELED].g < CLOCK_INTENSITY) { led[analog.tv_min/SCALELED].g = CLOCK_INTENSITY; }

     // HOURS
    led_mid = (analog.tv_hour/SCALELED);
    led_overflow = 0;
    if (led_mid >= CLOCK_WIDTH) { led_sel = led_mid - CLOCK_WIDTH; }

    else                        { led_sel = led_mid + (LEDS_SIZE - CLOCK_WIDTH); analog.tv_hour += SCALEMAX; }
    for (uint8_t x=0; x<=2*CLOCK_WIDTH; x++) {
      if (led_overflow)               { analog_LED = (led_sel + LEDS_SIZE) * SCALELED; }
        else                          { analog_LED = led_sel * SCALELED; }
      if (analog_LED > analog.tv_hour) { value = (analog_LED - analog.tv_hour); }
        else                          { value = (analog.tv_hour - analog_LED); }
      if (value > CLOCK_INTENSITY)    { value = 0; }
        else                          { value = CLOCK_INTENSITY - value; }
      if (led[led_sel].r < value)     { led[led_sel].r = value; }  // intensity normal
      if (++led_sel >= LEDS_SIZE)     { led_sel -= LEDS_SIZE; led_overflow = 1;}
    }

    // MINUTES
    led_mid = (analog.tv_min/SCALELED);
    led_overflow = 0;
    if (led_mid >= CLOCK_WIDTH) { led_sel = led_mid - CLOCK_WIDTH; }
    else                        { led_sel = led_mid + (LEDS_SIZE - CLOCK_WIDTH); analog.tv_min += SCALEMAX; }

    for (uint8_t x=0; x<=2*CLOCK_WIDTH; x++) {
      if (led_overflow)               { analog_LED = (led_sel + LEDS_SIZE) * SCALELED; }
        else                          { analog_LED = led_sel * SCALELED; }
      if (analog_LED > analog.tv_min) { value = (analog_LED - analog.tv_min); }
        else                          { value = (analog.tv_min - analog_LED); }
      if (value > CLOCK_INTENSITY)    { value = 0; }
        else                          { value = CLOCK_INTENSITY - value; }
      if (led[led_sel].g < value)     { led[led_sel].g = value; }  // intensity normal
      if (++led_sel >= LEDS_SIZE)     { led_sel -= LEDS_SIZE; led_overflow = 1;}
    }

    // SECONDS
    led_mid = (analog.tv_sec/SCALELED);
    led_overflow = 0;
    if (led_mid >= CLOCK_WIDTH) { led_sel = led_mid - CLOCK_WIDTH; }
    else                        { led_sel = led_mid + (LEDS_SIZE - CLOCK_WIDTH); analog.tv_sec += SCALEMAX; }

    for (uint8_t x=0; x<=2*CLOCK_WIDTH; x++) {
      if (led_overflow)               { analog_LED = (led_sel + LEDS_SIZE) * SCALELED; }
        else                          { analog_LED = led_sel * SCALELED; }
      if (analog_LED > analog.tv_sec) { value = (analog_LED - analog.tv_sec); }
        else                          { value = (analog.tv_sec - analog_LED); }
      if (value > CLOCK_INTENSITY)    { value = 0; }
        else                          { value = CLOCK_INTENSITY - value; }
      if (led[led_sel].b < value)     { led[led_sel].b = value; }  // intensity normal
      if (++led_sel >= LEDS_SIZE)     { led_sel -= LEDS_SIZE; led_overflow = 1;}
    }


    //led[0].r=255;led[0].g=0;led[0].b=0;    // Write red to array
    ws2812b.setleds(led,LEDS_SIZE);
    _delay_ms(LEDS_DELAY);                         // wait for 500ms.

  }
}
