#include <avr/io.h>
#include <avr/pgmspace.h>
//#include <avr/interrupt.h>
#include <util/delay.h>

#include "I2C.h"
#include "../clock.h"
//#include "usbasp.h"
#include "../Zif_Socket/zif.h"
#include "../isp.h"

/* compatibilty macros for old style */
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define SDAPORT PORTC
#define SDAPIN  PINC
#define SDADDR  DDRC
#define SDA     5

#define SCLPORT PORTC
#define SCLPIN  PINC
#define SCLDDR  DDRC
#define SCL     1

#define I2CVCCPORT  PORTC
#define I2CVCCPIN   PINC
#define I2CVCCDDR   DDRC
#define I2CVCC      2

#define I2CGNDPORT  PORTB
#define I2CGNDPIN   PINB
#define I2CGNDDDR   DDRB
#define I2CGND      5

#define I2CA0PORT   PORTB
#define I2CA0PIN    PINB
#define I2CA0DDR    DDRB
#define I2CA0       4

#define I2CA1PORT   PORTB
#define I2CA1PIN    PINB
#define I2CA1DDR    DDRB
#define I2CA1       3

#define I2CA2PORT   PORTD
#define I2CA2PIN    PIND
#define I2CA2DDR    DDRD
#define I2CA2       5

#define I2CWPPORT   PORTC
#define I2CWPPIN    PINC
#define I2CWPDDR    DDRC
#define I2CWP       0

//#define QDEL _delay_loop_2(3)
//#define HDEL _delay_loop_2(5)

// i2c quarter-bit delay
#define QDEL	asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");
// i2c half-bit delay
#define HDEL	asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop"); asm volatile("nop");

#define I2C_SDL_LO      cbi( SDAPORT, SDA)
#define I2C_SDL_HI      sbi( SDAPORT, SDA)

#define I2C_SCL_LO      cbi( SCLPORT, SCL);
#define I2C_SCL_HI      sbi( SCLPORT, SCL);

#define I2C_SCL_TOGGLE  HDEL; I2C_SCL_HI; HDEL; I2C_SCL_LO;
#define I2C_START       I2C_SDL_LO; QDEL; I2C_SCL_LO;
#define I2C_STOP        HDEL; I2C_SCL_HI; QDEL; I2C_SDL_HI; HDEL;

//static I2C_IO i2c;
unsigned char i2cSmart;
static unsigned char i2cBuf[16];
static unsigned char i2cIdx = 0;

/*
void i2cPins(void)
{
    zif_disconnect();
    pin_define(1, &i2c.a0);
    pin_define(2, &i2c.a1);
    pin_define(3, &i2c.a2);
    pin_define(4, &i2c.gnd);
    pin_define(24, &i2c.sda);
    pin_define(30, &i2c.scl);
    pin_define(31, &i2c.wp);
    pin_define(27, &i2c.vcc);
}

void i2cVccOn()
{
    set_bits( i2c.vcc.ddr, i2c.vcc.mask);
    set_bits( i2c.vcc.port, i2c.vcc.mask);
    set_bits( i2c.gnd.ddr, i2c.gnd.mask);
    clr_bits( i2c.gnd.port, i2c.gnd.mask);
}

void i2cVccOff()
{
    clr_bits( i2c.vcc.ddr, i2c.vcc.mask);
    clr_bits( i2c.vcc.port, i2c.vcc.mask);
    clr_bits( i2c.gnd.ddr, i2c.gnd.mask);
    clr_bits( i2c.gnd.port, i2c.gnd.mask);
}
*/

void i2cConnect()
{
    /*
    i2cVccOn();
    set_bits( i2c.scl.ddr, i2c.scl.mask);
    set_bits( i2c.scl.port, i2c.scl.mask);
    set_bits( i2c.sda.ddr, i2c.sda.mask);
    set_bits( i2c.sda.port, i2c.sda.mask);

    set_bits( i2c.wp.ddr, i2c.wp.mask);
    clr_bits( i2c.wp.port, i2c.wp.mask);

    set_bits( i2c.a0.ddr, i2c.a0.mask);
    clr_bits( i2c.a0.port, i2c.a0.mask);
    set_bits( i2c.a1.ddr, i2c.a1.mask);
    clr_bits( i2c.a1.port, i2c.a1.mask);
    set_bits( i2c.a2.ddr, i2c.a2.mask);
    clr_bits( i2c.a2.port, i2c.a2.mask);
    */

    I2CGNDDDR   |=  _BV(I2CGND);
    I2CGNDPORT  &=  ~_BV(I2CGND);

    I2CVCCDDR   |=  _BV(I2CVCC);
    I2CVCCPORT  |=  _BV(I2CVCC);

    I2CWPDDR    |=  _BV(I2CWP);
    I2CWPPORT   &=  ~_BV(I2CWP);

    I2CA0DDR    |=  _BV(I2CA0);
    I2CA0PORT   &=  ~_BV(I2CA0);

    I2CA1DDR    |=  _BV(I2CA1);
    I2CA1PORT   &=  ~_BV(I2CA1);

    I2CA2DDR    |=  _BV(I2CA2);
    I2CA2PORT   &=  ~_BV(I2CA2);

    SDADDR   |=  _BV(SDA);
    SDAPORT  |=  _BV(SDA);

    SCLDDR   |=  _BV(SCL);
    SCLPORT  |=  _BV(SCL);

    i2cSmart    =   16;
    i2cIdx = 0;
}
/*

void i2cStart()
{
    clr_bits( i2c.sda.port, i2c.sda.mask);
    Q_DEL;
    clr_bits( i2c.scl.port, i2c.scl.mask);
}

void i2cStop()
{
    H_DEL;
    set_bits( i2c.scl.port, i2c.scl.mask);
    Q_DEL;
    set_bits( i2c.sda.port, i2c.sda.mask);
    H_DEL;
}
*/
unsigned int i2cPutbyte(unsigned char b)
{
	int i;

	for (i=7;i>=0;i--)
	{
		if ( b & (1<<i) )
			I2C_SDL_HI;
			//set_bits( i2c.sda.port, i2c.sda.mask);
		else
			I2C_SDL_LO;			// address bit
        I2C_SCL_TOGGLE;		// clock HI, delay, then LO
			//clr_bits( i2c.sda.port, i2c.sda.mask);
        //H_DEL;
        //set_bits( i2c.scl.port, i2c.scl.mask);
        //H_DEL;
        //clr_bits( i2c.scl.port, i2c.scl.mask);
	}

	I2C_SDL_HI;					// leave SDL HI
	//set_bits( i2c.sda.port, i2c.sda.mask);
	// added
	//cbi(SDADDR, SDA);			// change direction to input on SDA line (may not be needed)
	//clr_bits( i2c.sda.ddr, i2c.sda.mask);
	HDEL;
	I2C_SCL_HI;					// clock back up
	//set_bits( i2c.scl.port, i2c.scl.mask);
  	b = SDAPIN & (1<<SDA);	// get the ACK bit
  	//b = read_bits(i2c.sda.pin, i2c.sda.mask);

	HDEL;
	I2C_SCL_LO;					// not really ??
	//clr_bits( i2c.scl.port, i2c.scl.mask);
	//sbi(SDADDR, SDA);			// change direction back to output
	//set_bits( i2c.sda.ddr, i2c.sda.mask);
	HDEL;
	return (b == 0);			// return ACK value
}

unsigned char i2cGetbyte(unsigned int last)
{
	int i;
	unsigned char c,b = 0;

	I2C_SDL_HI;					// make sure pullups are ativated
	//set_bits( i2c.sda.port, i2c.sda.mask);
	cbi(SDADDR, SDA);			// change direction to input on SDA line (may not be needed)
	//clr_bits( i2c.sda.ddr, i2c.sda.mask);

	for(i=7;i>=0;i--)
	{
		HDEL;
		I2C_SCL_HI;				// clock HI
		//set_bits( i2c.scl.port, i2c.scl.mask);
	  	c = SDAPIN & (1<<SDA);
	  	//b = read_bits(i2c.sda.pin, i2c.sda.mask);
		b <<= 1;
		if(c) b |= 1;
		HDEL;
    	I2C_SCL_LO;				// clock LO
    	//clr_bits( i2c.scl.port, i2c.scl.mask);
	}

	sbi(SDADDR, SDA);			// change direction to output on SDA line
	//set_bits( i2c.sda.ddr, i2c.sda.mask);

	if (last)
		I2C_SDL_HI;				// set NAK
		//set_bits( i2c.sda.port, i2c.sda.mask);
	else
		I2C_SDL_LO;				// set ACK
		//clr_bits( i2c.sda.port, i2c.sda.mask);

	I2C_SCL_TOGGLE;				// clock pulse
	//H_DEL;
    //set_bits( i2c.scl.port, i2c.scl.mask);
    //H_DEL;
    //clr_bits( i2c.scl.port, i2c.scl.mask);
	I2C_SDL_HI;					// leave with SDL HI
	//set_bits( i2c.sda.port, i2c.sda.mask);
	return b;					// return received byte
}

//************************
//* I2C public functions *
//************************

//! Send a byte sequence on the I2C bus
void i2cSend(unsigned char device, unsigned long subAddr, unsigned char length, unsigned char *data)
{
	I2C_START;      			// do start transition
	//i2cStart();
	if(i2cSmart < 16)
    {
        i2cPutbyte(device | ((subAddr & 0x0000ff00) >> 7)); 		// send DEVICE address
        i2cPutbyte(subAddr);		// and the subaddress
    }
    else
    {
        i2cPutbyte(device | ((subAddr & 0x00ff0000) >> 15)); 		// send DEVICE address
        i2cPutbyte((subAddr & 0x0000ff00) >> 8);		// and the subaddress
        i2cPutbyte(subAddr & 0x000000ff);		// and the subaddress
    }

	// send the data
	while (length--)
		i2cPutbyte(*data++);

	I2C_SDL_LO;					// clear data line and
	//clr_bits( i2c.sda.port, i2c.sda.mask);
	I2C_STOP;					// send STOP transition
	//i2cStop();
	clockWait(30); // wait 9,6 ms
}

//! Retrieve a byte sequence on the I2C bus
void i2cReceive(unsigned char device, unsigned long subAddr, unsigned char length, unsigned char *data)
{
	int j = length;
	unsigned char *p = data;

	I2C_START;					// do start transition
	//i2cStart();
	if(i2cSmart < 16)
    {
        i2cPutbyte(device | ((subAddr & 0x0000ff00) >> 7)); 		// send DEVICE address
        i2cPutbyte(subAddr);		// and the subaddress
    }
    else
    {
        i2cPutbyte(device | ((subAddr & 0x00ff0000) >> 15)); 		// send DEVICE address
        i2cPutbyte((subAddr & 0x0000ff00) >> 8);		// and the subaddress
        i2cPutbyte(subAddr & 0x000000ff);		// and the subaddress
    }
	HDEL;
	I2C_SCL_HI;      			// do a repeated START
	//set_bits( i2c.scl.port, i2c.scl.mask);
	I2C_START;					// transition
	//i2cStart();

	if(i2cSmart < 16)
        i2cPutbyte(device | READ | ((subAddr & 0x0000ff00) >> 7));	// resend DEVICE, with READ bit set
    else
        i2cPutbyte(device | READ | ((subAddr & 0x00ff0000) >> 15));	// resend DEVICE, with READ bit set

	// receive data bytes
	while (j--)
		*p++ = i2cGetbyte(j == 0);

	I2C_SDL_LO;					// clear data line and
	//clr_bits( i2c.sda.port, i2c.sda.mask);
	I2C_STOP;					// send STOP transition
	//i2cStop();
}

void i2cEnterProgrammingMode()
{
    unsigned char d,count;
    count = 10;
    while (count--)
    {
        i2cSmart = 8;
        i2cReceive(0xa0, 0, 1, &d);
        if(d == 0)
            return; // i2cSmart = 8;
        i2cSmart = 16;
        i2cReceive(0xa0, 0, 1, &d);
        if(d == 1)
            return; // i2cSmart = 16;
        d = 1;
        i2cSend(0xa0, 0, 1, &d);
    }
    i2cSmart = 0;
}

void i2cWriteBlock(unsigned char *data, unsigned char length, unsigned long subAddr)
{
    while(length--)
    {
        if(i2cIdx<i2cSmart)
            i2cBuf[i2cIdx++] = *data++;
        if(i2cIdx == i2cSmart)
        {
            //i2cSend(0xa0, subAddr - (subAddr%i2cSmart), i2cSmart, i2cBuf);
            i2cSend(0xa0, (subAddr/i2cSmart)*i2cSmart, i2cSmart, i2cBuf);
            i2cIdx = 0;
        }
    }
}
