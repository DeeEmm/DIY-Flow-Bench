
#include <math.h>
//#include <JeeLib.h>


#include <atmel_eFunction.h>
eFunction eFkt;

#define PWR         1.6
#define MAX_X       1024
#define STDVALUE    512

void setup()
{
    Serial.begin(115200);

    eFkt.init(STDVALUE, MAX_X, MAX_X,PWR);

    Serial.println("Test Function PowerOf and Scale: ");
    Serial.println(" X, Y_orig, Y_new");
}

void loop()
{

    uint32_t    time_start, duration_original, duration_new, result_original, result_new, result_error;
    float       scale, max_y, max_x;

    max_x       = MAX_X-STDVALUE;
    max_y       = pow(max_x, PWR); //interval^(1/1.7)
    scale       = max_x / max_y;

    result_error = 0;

    for (uint16_t ivar = 0; ivar < 1034; ivar++)
    {
        if (ivar >= STDVALUE)   result_original = pow(ivar - STDVALUE, PWR)*scale + STDVALUE;
        else                    result_original = STDVALUE - pow(STDVALUE - ivar, PWR)*scale;

        result_new      = eFkt.get(ivar);

        if (result_new > result_original)   result_error += result_new - result_original;
        else                                result_error += result_original - result_new;

        Serial.print(" ");
        Serial.print(ivar);
        Serial.print(", ");
        Serial.print(result_original);
        Serial.print(", ");
        Serial.print(result_new);
        Serial.println("");
    }

    result_original = 0;
    result_new      = 0;

    time_start          = micros();
    for (uint16_t ivar = 0; ivar < 1024; ivar++)
    {
        if (ivar >= STDVALUE)   result_original += pow(ivar - STDVALUE, PWR)*scale + STDVALUE;
        else                    result_original += STDVALUE - pow(STDVALUE - ivar, PWR)*scale;
    }
    duration_original   = micros() - time_start;

    time_start         = micros();
    for (uint16_t ivar = 0; ivar < 1024; ivar++)
    {
        result_new += eFkt.get(ivar);
    }
    duration_new        = micros() - time_start;

    Serial.print("Result_sum : ");
    Serial.print(result_original);
    Serial.print(", ");
    Serial.print(result_new);
    Serial.println("");

    Serial.print("Calculation_time_sum us : ");
    Serial.print(duration_original);
    Serial.print(", ");
    Serial.print(duration_new);
    Serial.println("");

    Serial.print("Error/Mean : ");
    Serial.print(result_error / 1024.0);
    Serial.println("");

    while(1)
    {
        ;
    }

}

/**<

Program size:
A1.0.5:
A1.5.7:

 */

