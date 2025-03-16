#include <xc.h>
#include "adc.h"

void init_adc(void)
{
    ADCON0 = 0x11; // 0x11 for Potentiometer or 0x19 for Temp sensor
    ADCON1 = 0x0A; // 0x08 for both temp and potentiometer
    ADCON2 = 0xA2;
}

unsigned short read_adc(void)
{
    unsigned short r_val = 0;
    // start conversion
    GO = 1;
    
    // wait until complete.
    while(GO); // blocking delay... will cause Unexpected blinks. when using in led dimmer... So you should use non blocking timer based ISR.
    
    // get the result and return.
    return ADRESH<<8|ADRESL;
}
