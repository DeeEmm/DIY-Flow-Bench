#ifndef atmel_eFunction_h
#define atmel_eFunction_h

#if ARDUINO >= 100
#include <Arduino.h> // Arduino 1.0
#else
#include <WProgram.h> // Arduino 0022
#endif
#include <stdint.h>
#include <avr/pgmspace.h>


////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// PowerOf-and-Scale-Funktion //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// INPUT:
//

class eFunction
{
private:
    int32_t     kf[8], ks[8]; // 8*4*2 = 64b
    int32_t     xnode[9]; // = {0,INTERVAL/96,INTERVAL/48,INTERVAL/24,INTERVAL/12,INTERVAL/6,INTERVAL/3,INTERVAL/1.5,INTERVAL};
    int32_t     zero_input = 0, zero_output = 0;
    uint8_t     interpol = 0;
    uint16_t    mean_input, mean_output, max_input, max_output;
public:
    void init(uint16_t zero_inp, uint16_t max_inp, uint16_t max_out, float pwr)
    {
        float       max_pow, scale_pow, yy, xx; // 4*4=16 (nur temporär)
        uint16_t    mean_tmp;

        zero_input   = zero_inp;
        mean_tmp        = max_inp - zero_input;

        if (zero_input >= max_inp)       mean_input = max_input;
        else if (mean_tmp > zero_input)  mean_input = mean_tmp;
        else                             mean_input = zero_input;

        //mean_input  = max_inp >> 1;
        zero_output     = (uint16_t)(float(zero_input) * float(max_out) / float(max_inp));
        mean_output     = zero_output;

        if (max_inp > max_out)  mean_tmp = mean_input;
        else                    mean_tmp = mean_output;

        while (mean_tmp <= 8192)
        {
            interpol++;
            mean_tmp *= 2;
        }

        if (max_inp >= (mean_input*2))
        {
            max_input   = max_inp << interpol;
            max_output  = max_out << interpol;
        }
        else
        {
            max_input   = mean_input <<  (1 + interpol);
            max_output  = mean_output << (1 + interpol);
        }

        max_pow     = pow(max_input, 1/pwr); //interval^(1/1.7)
        scale_pow   = max_pow / float(max_output);

        uint16_t interval = max_input - 1;

        max_input   = max_inp;
        max_output  = max_out;

        // nicht sehr schön, aber keine andere Lösung um auf aktuelle Stützstellen zu kommen
        xnode[0] = long(float(0.0));
        xnode[1] = long(float(interval)/96.0);
        xnode[2] = long(float(interval)/48.0);
        xnode[3] = long(float(interval)/24.0);
        xnode[4] = long(float(interval)/12.0);
        xnode[5] = long(float(interval)/6.00);
        xnode[6] = long(float(interval)/3.00);
        xnode[7] = long(float(interval)/1.50);
        xnode[8] = long(float(interval)/1.00);

        for (uint8_t p = 0; p<8; p++)
        {
            yy = (float)((pow(scale_pow*float(xnode[p+1]),pwr))-(pow(scale_pow*float(xnode[p]),pwr)));
            xx = (float)(xnode[p+1]-xnode[p]);
            // x^pwr = x * kf + ks über Interpolation mit 9 Stützstellen
            kf[p] = (long)((yy*128)/xx);
            ks[p] = (long)((pow(scale_pow*float(xnode[p]),pwr)*128)-float(xnode[p]*kf[p]));
        }
    };

    uint16_t get(long value)
    {
        if (value == zero_input)     return (uint16_t)(zero_output);
        else if (value >= max_input)    return (uint16_t)(max_output);
        else if (value <= 0)            return (uint16_t)(0);

        uint8_t sign, n;

        if (value > zero_input) // TODO: not clean, because of not zero scale_powd
        {
            value   = value - zero_input;
            sign    = 0;
        }
        else
        {
            value   = zero_input - value;
            sign    = 1;
        }
        value = value<<interpol;

        if (value<xnode[4])
        {
            if (value<xnode[2])
            {
                if (value<xnode[1])
                {
                    if (value<=xnode[0])    return  (uint16_t) xnode[0];
                    else                    n = 0;
                }
                else                        n = 1;
            }
            else
            {
                if (value<xnode[3])         n = 2;
                else                        n = 3;
            };
        }
        else
        {
            if (value < xnode[6])
            {
                if (value < xnode[5])       n = 4;
                else                        n = 5;
            }
            else
            {
                if (value < xnode[7])       n = 6;
                else
                {
                    if (value < xnode[8])   n = 7;
                    else                    return (uint16_t) xnode[8];
                };
            };
        };

        value = ((value*kf[n]+ks[n])>>7);// Todo: 7x Rollen umgehen
        value = (value>>interpol);
        //if (value >= INTERVAL) value = 0;

        if (sign)       return (uint16_t)(zero_output - value);
        else            return (uint16_t)(zero_output + value);

    };
};
//eFunction efkt;

#endif
