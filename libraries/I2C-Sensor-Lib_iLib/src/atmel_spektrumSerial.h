#ifndef atmel_spektrumSerial_h
#define atmel_spectrumSerial_h

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Spektrum Serial /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// forked from: https://github.com/esden/ppm_to_spektrum_encoder


#include <avr/io.h>
#include <avr/interrupt.h>

/// Choose only one of the two systems
//#define USE_SPEKTRUM2048 // otherwise SPEKTRUM1024 is used
/// IMPORTANT: dont forget to compensate (<<1) if you just want more channels, but no higher resolution


#ifdef USE_SPEKTRUM2048 // 11 bit frames
static const uint16_t  SPEKTRUM_VAL_MASK       = (0x7FF); // 2047
static const uint8_t   SPEKTRUM_CHAN_COUNT     = (12);
static const uint8_t   SPEKTRUM_CHAN_SHIFT     = (3);
static const uint8_t   SPEKTRUM_CHAN_MASK      = (0x0F);
static const uint8_t   SPEKTRUM_CHAN_STATIC    = (4); // first # Channels get transmitted every time, the others on change
#else                   // 10 bit frames
static const uint16_t  SPEKTRUM_VAL_MASK       = (0x3FF); // 1023
static const uint8_t   SPEKTRUM_CHAN_COUNT     = (7);
static const uint8_t   SPEKTRUM_CHAN_SHIFT     = (2);
static const uint8_t   SPEKTRUM_CHAN_MASK      = (0x07);
#endif // USE_SPEKTRUM2048

static const uint8_t    SPEKTRUM_FRAME_SIZE    = (16);
volatile uint8_t        spectrum_frame[SPEKTRUM_FRAME_SIZE];
volatile uint8_t        frame_counter          = (0);
volatile uint8_t        frame_send             = (0);


void spektrum_init(void)
{
    /* frame loss count */
    spectrum_frame[0] = 0x00; // Let's pretend we are not loosing any frames
    /* Receiver identifier */
    spectrum_frame[1] = 0x01; // Let's pretend we are DX6i or similar

    // Set all initial channel data to 0
    for (uint8_t i=0; i<7; i++)
    {
        spectrum_frame[2+(i*2)]   = ((i&SPEKTRUM_CHAN_MASK) << SPEKTRUM_CHAN_SHIFT);
        spectrum_frame[2+(i*2)+1] = 0x00;
    }

    /* Enable USART subsystem */
    UCSR0B=(0<<RXCIE0)|(0<<TXCIE0)|(0<<RXEN0)|(1<<TXEN0);

    /* Setup baud rate */
    const uint32_t UART_BAUD_RATE   = 115200;   // if you use bad Crystals, adapt this value
    const uint32_t UART_BAUD_SELECT = ((F_CPU/UART_BAUD_RATE/16)-1);

    UBRR0L=((unsigned char)UART_BAUD_SELECT);

    UDR0 = 0xAA;

    /* configure second pin to be output for serial out */
    DDRD |= (1 << 1); // TX

    asm("sei");
};



void spektrum_send(uint16_t *channel_data)
{

#ifdef USE_SPEKTRUM2048
    static uint8_t  position_newVal  = {SPEKTRUM_CHAN_STATIC};
    static uint8_t  position_sameVal = {SPEKTRUM_CHAN_STATIC};
    uint8_t         position_start;
    static uint16_t old_val[SPEKTRUM_CHAN_COUNT-SPEKTRUM_CHAN_STATIC];
    uint16_t        bitmap_selected  = {0};

    // make the position_newVal run through the Channels and until position_start is
    if (position_newVal>SPEKTRUM_CHAN_STATIC) position_start = position_newVal - 1;
    else                                      position_start = SPEKTRUM_CHAN_COUNT - 1;
#endif // USE_SPEKTRUM2048


    for (uint8_t i=0; i<7; i++)
    {
        uint8_t     temp_i;
#ifdef USE_SPEKTRUM2048
        if (i < SPEKTRUM_CHAN_STATIC)      // handle the channels that get transmitted every time
        {
            temp_i = i;
        }
        else
        {
            temp_i = 0xFF;
            while(temp_i == 0xFF)
            {
                if (position_start != position_newVal) // run through the remaining channels and find changed ones
                {
                    if (old_val[position_newVal-SPEKTRUM_CHAN_STATIC] != (channel_data[position_newVal] & SPEKTRUM_VAL_MASK))
                    {
                        old_val[position_newVal-SPEKTRUM_CHAN_STATIC] = (channel_data[position_newVal] & SPEKTRUM_VAL_MASK);
                        temp_i = position_newVal;
                        bitSet(bitmap_selected, position_newVal);
                    }
                    if (++position_newVal >= SPEKTRUM_CHAN_COUNT)           position_newVal = SPEKTRUM_CHAN_STATIC;
                }
                else    // fill the remaining places with unchanged channels
                {
                    if (bitRead(bitmap_selected,position_sameVal) == 0)     temp_i = position_sameVal;
                    if (++position_sameVal >= SPEKTRUM_CHAN_COUNT)          position_sameVal = SPEKTRUM_CHAN_STATIC;
                }
            }
        }
#else
        temp_i = i;
#endif // USE_SPEKTRUM2048
        uint16_t    temp_val;
        if 		(channel_data[temp_i] <= 0) 				temp_val = 0;
        else if (channel_data[temp_i] >= SPEKTRUM_VAL_MASK)	temp_val = SPEKTRUM_VAL_MASK;
        else										        temp_val = channel_data[temp_i];

        spectrum_frame[2+(i*2)] = ((temp_i&SPEKTRUM_CHAN_MASK) << SPEKTRUM_CHAN_SHIFT) | (temp_val>>8);
        spectrum_frame[2+(i*2)+1] = temp_val & 0xFF;

    }

    /* Just enable the serial interrupt the rest will be taken care of */
    frame_send = 0;
    UCSR0B|=(1<<TXCIE0);
};


ISR(USART_TX_vect)
{
    asm("sei");
    UDR0 = spectrum_frame[frame_counter];
    frame_counter++;

    if (frame_counter >= SPEKTRUM_FRAME_SIZE)   /* Frame is over, let's disable ourselves */
    {
        frame_counter = 0; /* reset */
        UCSR0B=(0<<RXCIE0)|(0<<TXCIE0)|(0<<RXEN0)|(1<<TXEN0); /* disable */
        frame_send = 1;
    }
};

#endif
