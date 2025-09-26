#ifndef ZIF_H_INCLUDED
#define ZIF_H_INCLUDED

#define SFR_MACRO

#define ZIF_PORTB   0x3B
#define ZIF_PORTC   0x3F
#define ZIF_PORTD   0xF3

#define ZIF_01          0x10
#define ZIF_01_DDR      DDRB
#define ZIF_01_PIN      PINB
#define ZIF_01_PORT     PORTB

#define ZIF_05          0x02
#define ZIF_05_DDR      DDRB
#define ZIF_05_PIN      PINB
#define ZIF_05_PORT     PORTB

#define ZIF_13          0x10
#define ZIF_13_DDR      DDRD
#define ZIF_13_PIN      PIND
#define ZIF_13_PORT     PORTD

#define ZIF_15          0x10
#define ZIF_15_DDR      DDRB
#define ZIF_15_PIN      PINB
#define ZIF_15_PORT     PORTB

typedef struct SFR_IO_
{
    unsigned char mask;
    volatile unsigned char* port;
    volatile unsigned char* pin;
    volatile unsigned char* ddr;
}SFR_IO, *PSFR_IO;

typedef struct ISP_IO_
{
    SFR_IO gnd;
    SFR_IO vcc;
    SFR_IO rst;
    SFR_IO xtl;
    SFR_IO sck;
    SFR_IO miso;
    SFR_IO mosi;
}ISP_IO, *PISP_IO;

typedef struct I2C_IO_
{
    SFR_IO gnd;
    SFR_IO vcc;
    SFR_IO sda;
    SFR_IO scl;
    SFR_IO wp;
    SFR_IO a0;
    SFR_IO a1;
    SFR_IO a2;
}I2C_IO, *PI2C_IO;


extern void pin_define(unsigned char pin, PSFR_IO sfr);
#ifdef SFR_MACRO
    #define set_bits(sfr, mask)     (*(sfr) |= mask)
    #define clr_bits(sfr, mask)     (*(sfr) &= ~mask)
    #define not_bits(sfr, mask)     (*(sfr) ^= mask)
    #define read_bits(sfr, mask)    (*(sfr) & mask)
#else
    extern void set_bits(volatile unsigned char *sfr, unsigned char mask);
    extern void clr_bits(volatile unsigned char *sfr, unsigned char mask);
    extern void not_bits(volatile unsigned char *sfr, unsigned char mask);
    extern unsigned char read_bits(volatile unsigned char *sfr, unsigned char mask);
#endif // SFR_MACRO

    extern void zif_disconnect(void);

#endif // ZIF_H_INCLUDED
