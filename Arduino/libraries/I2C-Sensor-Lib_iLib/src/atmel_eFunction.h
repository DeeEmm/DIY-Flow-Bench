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




#define INTERPOL    4
#define MEANVAL     512    // conflicts with zero_setpoint.... not clean implemented
#define INTERVAL    ((MEANVAL<<INTERPOL)-1)
#define OUT_LOW_TH  0
#define OUT_HIGH_TH 1024
// ehemals 255 nun 2^16-1
class eFunction
{
private:
    long kf[8], ks[8]; // 8*4*2 = 64b
    long xnode[9]; // = {0,INTERVAL/96,INTERVAL/48,INTERVAL/24,INTERVAL/12,INTERVAL/6,INTERVAL/3,INTERVAL/1.5,INTERVAL};
    long zero_setpoint;
public:
    eFunction(void): zero_setpoint(0)
    {
        for (uint8_t ivar = 0; ivar < 8; ++ivar)
        {
            kf[ivar] = 1;
            ks[ivar] = 0;
            xnode[ivar] = 1;
        };
        xnode[8] = 1;
    };

    void init(uint16_t interval, float pwr)
    {
        float maxi, base;  // 2*4=8 (nur temporär)
        maxi = pow(interval, 1/pwr); //interval^(1/1.7)
        base = maxi / float(interval);
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

        for (uint8_t p = 0; p<8; ++p)
        {
            float yy, xx; // 2*4=8 (nur temporär)
            yy = (float)((pow(base*float(xnode[p+1]),pwr))-(pow(base*float(xnode[p]),pwr)));
            xx = (float)(xnode[p+1]-xnode[p]);
            // x^pwr = x * kf + ks über Interpolation mit 9 Stützstellen
            kf[p] = (long)((yy*128)/xx);
            ks[p] = (long)((pow(base*float(xnode[p]),pwr)*128)-float(xnode[p]*kf[p]));
        };

        zero_setpoint = 0;
    };

    void set_zeropoint(long new_setpoint)
    {
        zero_setpoint = new_setpoint;
    };

    uint16_t get(long value)
    {
        if (value == zero_setpoint)    return (unsigned int)(zero_setpoint);
        else if (value >= OUT_HIGH_TH) return (unsigned int)(OUT_HIGH_TH);
        else if (value <= OUT_LOW_TH)  return (unsigned int)(OUT_LOW_TH);

        uint8_t sign, n;

        if (value > zero_setpoint) // TODO: not clean, because of not zero based
        {
            value   = value - zero_setpoint;
            sign    = 0;
        }
        else
        {
            value   = zero_setpoint - value;
            sign    = 1;
        }
        value = value<<INTERPOL;

        if (value<xnode[4])
        {
            if (value<xnode[2])
            {
                if (value<xnode[1])
                {
                    if (value<=xnode[0])    return  (unsigned int) xnode[0];
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
                    else                    return (unsigned int) xnode[8];
                };
            };
        };

        value = ((value*kf[n]+ks[n])>>7);// Todo: 7x Rollen umgehen
        value = (value>>INTERPOL);
        if (value >= INTERVAL) value = 0;

        if (sign)       return (unsigned int)(zero_setpoint - value);
        else            return (unsigned int)(zero_setpoint + value);

    };
};
//eFunction efkt;

#endif
