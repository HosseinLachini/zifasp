#include "zif.h"
#include <avr/io.h>



void zif_disconnect()
{
    DDRB    &= ~ZIF_PORTB;//0xC4;
    PORTB   &= ~ZIF_PORTB;//0xC4;

    DDRC    = ~ZIF_PORTC;//0x00;
    PORTC   = ~ZIF_PORTC;//0x00;

    DDRD    &= ~ZIF_PORTD;//0x0C;
    PORTD   &= ~ZIF_PORTD;//0x0C;
}

static uint8_t pin_to_mask(uint8_t pin)
{
    switch(pin)
    {
    case 7:
    case 20:
    case 31:
    //case 39:
        return 0x01;
        break;
    case 5:
    case 19:
    case 30:
    //case 38:
        return 0x02;
        break;
    case 27:
    //case 29:
    //case 40:
        return 0x04;
        break;
    case  2:
    case 26:
    case 34:
    case 36:
        return 0x08;
        break;
    case  1:
    case 13:
    case 25:
    //case 32:
    //case 35:
        return 0x10;
        break;
    case  4:
    //case  6:
    case  3:
    //case  9:
    case 24:
    //case 37:
        return 0x20;
        break;
    case 10:
        return 0x40;
        break;
    case  8:
        return 0x80;
        break;
    default :
        return 0x00;
    }
}

void pin_define(unsigned char pin, PSFR_IO sfr)
{
    if(pin == 99)
        return;
    sfr->mask = pin_to_mask(pin);
    if(sfr->mask == 0)
        return;
    switch(pin)
    {
    case  7:
    case  5:
    case  2:
    case  1:
    case  4:
    //case  6:
        sfr->port = &PORTB;
        sfr->pin = &PINB;
        sfr->ddr = &DDRB;
        break;
    case 31:
    //case 39:
    case 30:
    //case 38:
    case 27:
    //case 29:
    //case 40:
    case 26:
    //case 34:
    //case 36:
    case 25:
    //case 32:
    //case 35:
    case 24:
    //case 37:
        sfr->port = &PORTC;
        sfr->pin = &PINC;
        sfr->ddr = &DDRC;
        break;
    case 20:
    case 19:
    case 13:
    case  3:
    //case  9:
    case 10:
    case  8:
        sfr->port = &PORTD;
        sfr->pin = &PIND;
        sfr->ddr = &DDRD;
        break;
    }
}

#ifndef SFR_MACRO
void set_bits(volatile uint8_t *sfr, uint8_t mask)
{
    *sfr |= mask;
}

void clr_bits(volatile uint8_t *sfr, uint8_t mask)
{
    *sfr &= ~mask;
}

inline void not_bits(volatile uint8_t *sfr, uint8_t mask)
{
    *sfr ^= mask;
}

uint8_t read_bits(volatile uint8_t *sfr, uint8_t mask)
{
    return (*sfr & mask);
}
#endif // SFR_MACRO
