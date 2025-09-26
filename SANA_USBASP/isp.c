/*
 * isp.c - part of USBasp
 *
 * Autor..........: Thomas Fischl <tfischl@gmx.de>
 * Description....: Provides functions for communication/programming
 *                  over ISP interface
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2005-02-23
 * Last change....: 2010-01-19
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "isp.h"
#include "clock.h"
#include "usbasp.h"
#include "Zif_Socket/zif.h"

/* compatibilty macros for old style */
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define spiHWdisable() SPCR = 0

uchar sck_sw_delay;
uchar isp_hiaddr;

static ISP_IO isp;
//static sck_auto = 0;
extern uchar prog_sck;

const unsigned char ISP_SET_SCK[] PROGMEM =
{
    0   ,   //  USBASP_ISP_SCK_AUTO   0
    255 ,   //  USBASP_ISP_SCK_0_5    1   /* 500 Hz */
    128 ,   //  USBASP_ISP_SCK_1      2   /*   1 kHz */
    64  ,   //  USBASP_ISP_SCK_2      3   /*   2 kHz */
    32  ,   //  USBASP_ISP_SCK_4      4   /*   4 kHz */
    16  ,   //  USBASP_ISP_SCK_8      5   /*   8 kHz */
    8   ,   //  USBASP_ISP_SCK_16     6   /*  16 kHz */
    4   ,   //  USBASP_ISP_SCK_32     7   /*  32 kHz */
    2   ,   //  USBASP_ISP_SCK_93_75  8   /*  93.75 kHz */
    1   ,   //  USBASP_ISP_SCK_187_5  9   /* 187.5  kHz */
    0   ,   //  USBASP_ISP_SCK_375    10  /* 375 kHz   */
    0   ,   //  USBASP_ISP_SCK_750    11  /* 750 kHz   */
    0   ,   //  USBASP_ISP_SCK_1500   12  /* 1.5 MHz   */
};

const unsigned char AVR_PIN_CONFIG[][8] PROGMEM =
{
//    //  MOSI - MISO - SCK - RST - VCC - GND - XTL - NEXT CONFIG
//       {10    ,5     ,3    ,7    ,4    ,0    ,8    ,1 },//ISP OUT
//       {6     ,7     ,8    ,9    ,10   ,11   ,13   ,2 },//ATMEGA16/ATMEGA32/AT90S4434/AT90S8535/ATMEGA1284P/ATMEGA163/ATMEGA164P/ATMEGA323/ATMEGA644P/ATMEGA8535
//       {29    ,30    ,31   ,1    ,7    ,8    ,9    ,3 },//ATMEGA8/AT90S4433/ATMEGA168/ATMEGA328/ATMEGA48/ATMEGA88/ATtiny88
//       {7     ,8     ,9    ,5    ,37   ,35   ,10   ,4 },//AT86RF401
//       {5     ,4     ,12   ,2    ,6    ,7    ,10   ,5 },//AT90PWM216/AT90PWM2
//       {6     ,5     ,16   ,3    ,8    ,9    ,14   ,6 },//AT90PWM3
//       {37    ,38    ,39   ,1    ,40   ,10   ,5    ,7 },//AT90S1200/AT90S2313/ATtiny2313/ATtiny4313
//       {37    ,38    ,39   ,1    ,40   ,4    ,2    ,8 },//AT90S2323/AT90S2343/ATtiny13/ATtiny22/ATtiny25/ATtiny45/ATtiny85
//       {6     ,7     ,8    ,9    ,40   ,20   ,19   ,9 },//AT90S4414/AT90S8515/ATMEGA161/ATMEGA162/ATMEGA8515
//       {37    ,38    ,39   ,1    ,40   ,4    ,3    ,10},//ATtiny15
//       {7     ,3     ,8    ,31   ,35   ,36   ,34   ,11},//ATtiny167
//       {7     ,34    ,35   ,4    ,1    ,40   ,2    ,12},//ATtiny24/ATtiny44/ATtiny84
//       {1     ,2     ,3    ,10   ,5    ,6    ,7    ,0}  //ATtiny26/ATtiny261/ATtiny461/ATtiny861
    //  MOSI - MISO - SCK - RST - VCC - GND - XTL - NEXT CONFIG
//       {10    ,5     ,3    ,7    ,4    ,0    ,8    ,1 },//ISP OUT
//       {4     ,7     ,8    ,3    ,10   ,11   ,13   ,2 },//ATMEGA16/ATMEGA32/AT90S4434/AT90S8535/ATMEGA1284P/ATMEGA163/ATMEGA164P/ATMEGA323/ATMEGA644P/ATMEGA8535
//       {4     ,7     ,8    ,3    ,27   ,20   ,19   ,3 },//AT90S4414/AT90S8515/ATMEGA161/ATMEGA162/ATMEGA8515
//       {24    ,30    ,31   ,1    ,27   ,10   ,5    ,4 },//AT90S1200/AT90S2313/ATtiny2313/ATtiny4313
//       {24    ,30    ,31   ,1    ,27   ,4    ,2    ,5 },//AT90S2323/AT90S2343/ATtiny13/ATtiny22/ATtiny25/ATtiny45/ATtiny85
//       {24    ,30    ,31   ,1    ,27   ,4    ,3    ,6 },//ATtiny15
//       {27    ,30    ,31   ,1    ,7    ,8    ,3    ,7 },//ATMEGA8/AT90S4433/ATMEGA168/ATMEGA328/ATMEGA48/ATMEGA88/ATtiny88
//       {1     ,2     ,3    ,10   ,5    ,4    ,7    ,8 },//ATtiny26/ATtiny261/ATtiny461/ATtiny861
//       {7     ,26    ,25   ,4    ,1    ,27   ,2    ,0 } //ATtiny24/ATtiny44/ATtiny84
    //  MOSI - MISO - SCK - RST - VCC - GND - XTL - NEXT CONFIG
       {10    ,5     ,3    ,7    ,4    ,0    ,8    ,1 },//ISP OUT
       {4     ,7     ,8    ,3    ,10   ,11   ,13   ,2 },//ATMEGA16/ATMEGA32/AT90S4434/AT90S8535/ATMEGA1284P/ATMEGA163/ATMEGA164P/ATMEGA323/ATMEGA644P/ATMEGA8535
       {99    ,99    ,99   ,99   ,27   ,20   ,19   ,3 },//AT90S4414/AT90S8515/ATMEGA161/ATMEGA162/ATMEGA8515
       {24    ,30    ,31   ,1    ,99   ,10   ,5    ,4 },//AT90S1200/AT90S2313/ATtiny2313/ATtiny4313
       {99    ,99    ,99   ,99   ,99   ,4    ,2    ,5 },//AT90S2323/AT90S2343/ATtiny13/ATtiny22/ATtiny25/ATtiny45/ATtiny85
       {99    ,99    ,99   ,99   ,99   ,99   ,3    ,6 },//ATtiny15
       {27    ,99    ,99   ,99   ,7    ,8    ,99   ,7 },//ATMEGA8/AT90S4433/ATMEGA168/ATMEGA328/ATMEGA48/ATMEGA88/ATtiny88
       {1     ,2     ,3    ,10   ,5    ,4    ,7    ,8 },//ATtiny26/ATtiny261/ATtiny461/ATtiny861
       {7     ,26    ,25   ,4    ,1    ,27   ,2    ,0 } //ATtiny24/ATtiny44/ATtiny84
};

const unsigned char S5X_PIN_CONFIG[][8] PROGMEM =
{
    //  MOSI - MISO - SCK - RST - VCC - GND - XTL - NEXT CONFIG
       {10    ,5     ,3    ,7    ,4    ,0    ,8    ,1 },//ISP OUT
       {4     ,7     ,8    ,3    ,27   ,20   ,19   ,0 },//AT89s51/AT89s52/AT89s53
};

unsigned char zif_scan()
{
    unsigned char res;

    DDRB    |= ZIF_PORTB;
    PORTB   &= ~ZIF_PORTB;

    DDRC    |= ZIF_PORTC;
    PORTC   = ~ZIF_PORTC;

    DDRD    |= ZIF_PORTD;
    PORTD   &= ~ZIF_PORTD;

    ZIF_01_DDR &= ~ZIF_01;
    ZIF_01_PORT |= ZIF_01;
    res = ZIF_01_PIN & ZIF_01;

    DDRB    &= ~ZIF_PORTB;//0xC4;
    PORTB   &= ~ZIF_PORTB;//0xC4;

    DDRC    = ~ZIF_PORTC;//0x00;
    PORTC   = ~ZIF_PORTC;//0x00;

    DDRD    &= ~ZIF_PORTD;//0x0C;
    PORTD   &= ~ZIF_PORTD;//0x0C;

    return res;
}

unsigned char ispSearch()
{
    static unsigned char Last_AVR_Pins = 0;
    unsigned char next, start;
	uchar led = 0;
	uchar res = 0;
	uchar idx = 0;
	//if(zif_scan())
    //{
    //    ispPins(AVR_PIN_CONFIG[0]);
    //    return 0;
    //}
    zif_disconnect();
//    if(chip != PROGISP_AVR)
//    {
//        chip = PROGISP_AVR;
//        Last_AVR_Pins = 1;
//    }
    if(Last_AVR_Pins == 0)
        Last_AVR_Pins = 1;
    start = next = Last_AVR_Pins;
    //ispSetSCKOption(USBASP_ISP_SCK_AUTO);
    ledRedOn();
    ledGreenOff();
    if(chip == PROGISP_AVR) {
        do
        {
            if(led)
            {
                led = 0;
                ledRedOff();
            }
            else
            {
                led = 1;
                ledRedOn();
            }
            if(res && (next == start) && (en_xtal == USBASP_EXT_RC_OSCILLATOR))
            {
                en_xtal = USBASP_EXT_OSCILLATOR;
            }
            ispPins(AVR_PIN_CONFIG[next]);
            Last_AVR_Pins = next;
            next = pgm_read_byte(AVR_PIN_CONFIG[next] + 7);
            res = ispEnterProgrammingMode();
            if((res != 0) && (prog_sck == USBASP_ISP_SCK_AUTO))
            {
                if(en_xtal == USBASP_EXT_RC_OSCILLATOR){
                    sck_sw_delay = 8;
                    res = ispEnterProgrammingMode();
                }
                else{

                    for(idx = USBASP_ISP_SCK_1500; res && (idx > USBASP_ISP_SCK_0_5); idx--)
                    {
                        sck_sw_delay = pgm_read_byte(ISP_SET_SCK + idx);
                        res = ispEnterProgrammingMode();
                    }
                    // double speed decrement to stable
                    if(idx > USBASP_ISP_SCK_0_5)
                    {
                        idx--;
                        sck_sw_delay = pgm_read_byte(ISP_SET_SCK + idx);
                    }
                }
            }
        }while(res && ((next != start) || (en_xtal == USBASP_EXT_RC_OSCILLATOR)));
    }
    ledGreenOn();
    if(res || chip == PROGISP_MCS51)
    {
        chip = PROGISP_MCS51;
        en_xtal = USBASP_EXT_OSCILLATOR;
        ispPins(S5X_PIN_CONFIG[0]);
        if(ispEnterProgrammingMode())
        {
            ispPins(S5X_PIN_CONFIG[1]);
            if(ispEnterProgrammingMode())
            {
                ledRedOff();
                return 1;
            }
        }
    }
    ledRedOn();
    return 0;
}

void ispXtal(unsigned char pulse)
{
    while(pulse--)
    {
        not_bits(isp.xtl.port, isp.xtl.mask);
        not_bits(isp.xtl.port, isp.xtl.mask);
    }
}


void ispPins(const unsigned char *config_pin)
{
    zif_disconnect();
    /*
    if(pgm_read_byte(config_pin) == 0)
    {
        isp.mosi.ddr = &ISP_MOSI_DDR;
        isp.mosi.port = &ISP_MOSI_PORT;
        isp.mosi.pin = &ISP_MOSI_PIN;
        isp.mosi.mask = (1 << ISP_MOSI);

        isp.miso.ddr = &ISP_MISO_DDR;
        isp.miso.port = &ISP_MISO_PORT;
        isp.miso.pin = &ISP_MISO_PIN;
        isp.miso.mask = (1 << ISP_MISO);

        isp.sck.ddr = &ISP_SCK_DDR;
        isp.sck.port = &ISP_SCK_PORT;
        isp.sck.pin = &ISP_SCK_PIN;
        isp.sck.mask = (1 << ISP_SCK);

        isp.rst.ddr = &ISP_RST_DDR;
        isp.rst.port = &ISP_RST_PORT;
        isp.rst.pin = &ISP_RST_PIN;
        isp.rst.mask = (1 << ISP_RST);

        isp.vcc.ddr = &ISP_VCC_DDR;
        isp.vcc.port = &ISP_VCC_PORT;
        isp.vcc.pin = &ISP_VCC_PIN;
        isp.vcc.mask = (1 << ISP_VCC);

        pin_define(1, &isp.gnd);

        isp.xtl.ddr = &ISP_XTL1_DDR;
        isp.xtl.port = &ISP_XTL1_PORT;
        isp.xtl.pin = &ISP_XTL1_PIN;
        isp.xtl.mask = (1 << ISP_XTL1);
    }
    else
        */
    {
        pin_define(pgm_read_byte(config_pin), &isp.mosi);
        pin_define(pgm_read_byte(config_pin + 1), &isp.miso);
        pin_define(pgm_read_byte(config_pin + 2), &isp.sck);
        pin_define(pgm_read_byte(config_pin + 3), &isp.rst);
        pin_define(pgm_read_byte(config_pin + 4), &isp.vcc);
        pin_define(pgm_read_byte(config_pin + 5), &isp.gnd);
        pin_define(pgm_read_byte(config_pin + 6), &isp.xtl);
    }
    //ispConnect();
}

void ispVccOn()
{
    set_bits( isp.vcc.ddr, isp.vcc.mask);
    set_bits( isp.vcc.port, isp.vcc.mask);
    set_bits( isp.gnd.ddr, isp.gnd.mask);
    clr_bits( isp.gnd.port, isp.gnd.mask);
}

//void ispVccOff()
//{
//    clr_bits( isp.vcc.ddr, isp.vcc.mask);
//    clr_bits( isp.vcc.port, isp.vcc.mask);
//    clr_bits( isp.gnd.ddr, isp.gnd.mask);
//    clr_bits( isp.gnd.port, isp.gnd.mask);
//}

//void ispSetSCKOption(uchar option) {
//
////	if (option == USBASP_ISP_SCK_AUTO)
////    {
////        if(isp.sck.ddr == &ISP_SCK_DDR)
////            option = USBASP_ISP_SCK_375;
////    }
////
////	if (option >= USBASP_ISP_SCK_93_75 && isp.gnd.mask == 0) {
////		ispTransmit = ispTransmit_hw;
////		sck_spsr = 0;
////		sck_sw_delay = 1;	/* force RST#/SCK pulse for 320us */
////
////		switch (option) {
////
////		case USBASP_ISP_SCK_1500:
////			/* enable SPI, master, 1.5MHz, XTAL/8 */
////			sck_spcr = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
////			sck_spsr = (1 << SPI2X);
////		case USBASP_ISP_SCK_750:
////			/* enable SPI, master, 750kHz, XTAL/16 */
////			sck_spcr = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
////			break;
////		case USBASP_ISP_SCK_375:
////		default:
////			/* enable SPI, master, 375kHz, XTAL/32 (default) */
////			sck_spcr = (1 << SPE) | (1 << MSTR) | (1 << SPR1);
////			sck_spsr = (1 << SPI2X);
////			break;
////		case USBASP_ISP_SCK_187_5:
////			/* enable SPI, master, 187.5kHz XTAL/64 */
////			sck_spcr = (1 << SPE) | (1 << MSTR) | (1 << SPR1);
////			break;
////		case USBASP_ISP_SCK_93_75:
////			/* enable SPI, master, 93.75kHz XTAL/128 */
////			sck_spcr = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
////			break;
////		}
////
////	} else {
//        //sck_auto = 0;
//		ispTransmit = ispTransmit_sw;
//		switch (option) {
//
//		case USBASP_ISP_SCK_AUTO:
//			sck_sw_delay = 0;
//			//sck_auto = 1;
//			break;
//
//		case USBASP_ISP_SCK_187_5:
//			sck_sw_delay = 1;
//			break;
//
//		case USBASP_ISP_SCK_93_75:
//			sck_sw_delay = 2;
//			break;
//
//		case USBASP_ISP_SCK_32:
//			sck_sw_delay = 4;
//			break;
//
//		case USBASP_ISP_SCK_16:
//			sck_sw_delay = 8;
//			break;
//
//		case USBASP_ISP_SCK_8:
//			sck_sw_delay = 16;
//			break;
//
//		case USBASP_ISP_SCK_4:
//			sck_sw_delay = 32;
//			break;
//
//		case USBASP_ISP_SCK_2:
//			sck_sw_delay = 64;
//
//			break;
//		case USBASP_ISP_SCK_1:
//			sck_sw_delay = 128;
//			break;
//
//		case USBASP_ISP_SCK_0_5:
//			sck_sw_delay = 255;
//			break;
//
//		default:
//			sck_sw_delay = 0;
//			break;
//
//		}
////	}
//}

void ispDelay(void) {
	uint8_t starttime = TIMERVALUE;
	if(en_xtal == USBASP_EXT_OSCILLATOR)
    {
        do {
            ispXtal(2);
        }while ((uint8_t) (TIMERVALUE - starttime) < sck_sw_delay);
	}
	else
    {
        while ((uint8_t) (TIMERVALUE - starttime) < sck_sw_delay);
    }
}

void ispConnect() {

    ispVccOn();
    if(en_xtal == USBASP_EXT_RC_OSCILLATOR)
    {
        // for External RC Oscillator : set internal R (20k~50k) pull up
        // and CKOPT 36 pf.
        clr_bits( isp.xtl.ddr, isp.xtl.mask);
        set_bits( isp.xtl.port, isp.xtl.mask);

    }
    else
    {
        set_bits( isp.xtl.ddr, isp.xtl.mask);
        clr_bits( isp.xtl.port, isp.xtl.mask);
        //if(en_xtal == USBASP_EXT_OSCILLATOR)
            //ispXtal(100);
    }
    clockWait(1);
    //set_bits( isp.xtl.port, isp.xtl.mask);

	/* all ISP pins are inputs before */
	/* now set output pins */
    //sbi(ISP_RST_DDR, ISP_RST);
    //sbi(ISP_SCK_DDR, ISP_SCK);
    //sbi(ISP_MOSI_DDR, ISP_MOSI);
    set_bits( isp.rst.ddr, isp.rst.mask);
    set_bits( isp.sck.ddr, isp.sck.mask);
    set_bits( isp.mosi.ddr, isp.mosi.mask);

    if(chip == PROGISP_AVR)
    {
        /* reset device */
        //cbi(ISP_RST_PORT, ISP_RST);
        //cbi(ISP_SCK_PORT, ISP_SCK);
        clr_bits( isp.rst.port, isp.rst.mask);
        clr_bits( isp.sck.port, isp.sck.mask);

        /* positive reset pulse > 2 SCK (target) */
        ispDelay();
        //sbi(ISP_RST_PORT, ISP_RST);
        set_bits( isp.rst.port, isp.rst.mask);
        ispDelay();
        //cbi(ISP_RST_PORT, ISP_RST);
        clr_bits( isp.rst.port, isp.rst.mask);
    }
    else
    {
        /* reset device */
        //ISP_OUT |= (1 << ISP_RST);   /* RST high */
        //ISP_OUT &= ~(1 << ISP_SCK);   /* SCK low */
        set_bits( isp.rst.port, isp.rst.mask);
        clr_bits( isp.sck.port, isp.sck.mask);

        /* positive reset pulse > 2 SCK (target) */
        ispDelay();
        //ISP_OUT &= ~(1 << ISP_RST);    /* RST low */
        clr_bits( isp.rst.port, isp.rst.mask);
        ispDelay();
        //ISP_OUT |= (1 << ISP_RST);   /* RST high */
        set_bits( isp.rst.port, isp.rst.mask);
        ispDelay();
        ispXtal(50);
    }


	/* Initial extended address value */
	isp_hiaddr = 0;
}

//void ispDisconnect() {
//
//    ispVccOff();
//    clr_bits( isp.xtl.ddr, isp.xtl.mask);
//    clr_bits( isp.xtl.port, isp.xtl.mask);
//	/* set all ISP pins inputs */
//    //cbi(ISP_RST_DDR, ISP_RST);
//    //cbi(ISP_SCK_DDR, ISP_SCK);
//    //cbi(ISP_MOSI_DDR, ISP_MOSI);
//    clr_bits( isp.rst.ddr, isp.rst.mask);
//    clr_bits( isp.sck.ddr, isp.sck.mask);
//    clr_bits( isp.mosi.ddr, isp.mosi.mask);
//	/* switch pullups off */
//    //cbi(ISP_RST_PORT, ISP_RST);
//    //cbi(ISP_SCK_PORT, ISP_SCK);
//    //cbi(ISP_MOSI_PORT, ISP_MOSI);
//    clr_bits( isp.rst.port, isp.rst.mask);
//    clr_bits( isp.sck.port, isp.sck.mask);
//    clr_bits( isp.mosi.port, isp.mosi.mask);
//}

uchar ispTransmit(uchar send_byte) {

	uchar rec_byte = 0;
	uchar i = 8;
	while(i) {
        i--;
		/* set MSB to MOSI-pin */
		if ((send_byte & 0x80) != 0) {
			//sbi(ISP_MOSI_PORT, ISP_MOSI);
			set_bits( isp.mosi.port, isp.mosi.mask);
		} else {
			//cbi(ISP_MOSI_PORT, ISP_MOSI);
			clr_bits( isp.mosi.port, isp.mosi.mask);
		}
		/* shift to next bit */
		send_byte = send_byte << 1;

		/* receive data */
		rec_byte = rec_byte << 1;
		if (read_bits(isp.miso.pin, isp.miso.mask) != 0) {
			rec_byte++;
		}

		/* pulse SCK */
		//sbi(ISP_SCK_PORT, ISP_SCK);
		set_bits( isp.sck.port, isp.sck.mask);
		ispDelay();
		//cbi(ISP_SCK_PORT, ISP_SCK);
		clr_bits( isp.sck.port, isp.sck.mask);
		ispDelay();
	}

	return rec_byte;
}

uchar ispEnterProgrammingMode() {

	uchar count = 1;

    ispConnect();
	while (count--) {
		ispTransmit(0xAC);
		ispTransmit(0x53);
		if(chip == PROGISP_AVR)
        {
            if(ispTransmit(0) == 0x53){
                if(ispTransmit(0) == 0){
                    if(ispTransmit(0x30) == 0){
                        if(ispTransmit(0x00) == 0x30){
                            if(ispTransmit(0x00) == 0){
                                if(ispTransmit(0x00) == 0x1E){
                                    if(ispTransmit(0x30) == 0){
                                        if(ispTransmit(0x00) == 0x30){
                                            if(ispTransmit(0x01) == 0){
                                                if((ispTransmit(0x00) & 0xF0) == 0x90)
                                                    return 0;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

//            if(en_xtal == USBASP_EXT_RC_OSCILLATOR)
//            {
//                en_xtal = USBASP_EXT_OSCILLATOR;
//                set_bits( isp.xtl.ddr, isp.xtl.mask);
//                clr_bits( isp.xtl.port, isp.xtl.mask);
//            }
//            else
//                return 1;
        }
        else
        {
            ispTransmit(0);
            if (ispTransmit(0) == 0x69) {
                return 0;
            }
        }

		/* pulse RST */
		ispDelay();
		//sbi(ISP_RST_PORT, ISP_RST);
		set_bits( isp.rst.port, isp.rst.mask);
		ispDelay();
		//cbi(ISP_RST_PORT, ISP_RST);
		clr_bits( isp.rst.port, isp.rst.mask);
		ispDelay();
	}
	return 1; /* error: device dosn't answer */
}

static void ispUpdateExtended(unsigned long address)
{
	uchar curr_hiaddr;

	curr_hiaddr = (address >> 17);

	/* check if extended address byte is changed */
	if(isp_hiaddr != curr_hiaddr)
	{
		isp_hiaddr = curr_hiaddr;
		/* Load Extended Address byte */
		ispTransmit(0x4D);
		ispTransmit(0x00);
		ispTransmit(isp_hiaddr);
		ispTransmit(0x00);
	}
}

uchar ispReadFlash(unsigned long address) {

    uchar   sck_sw_delay_temp;
	ispUpdateExtended(address);
    if(chip == PROGISP_AVR)
    {
        sck_sw_delay_temp = sck_sw_delay;
        if(sck_sw_delay < 8)
            sck_sw_delay = 0;
        ispTransmit(0x20 | ((address & 1) << 3));
        ispTransmit(address >> 9);
        ispTransmit(address >> 1);
        sck_sw_delay = sck_sw_delay_temp;
    }
    else
    {
        ispTransmit(0x20);
        ispTransmit(address>>8);
        ispTransmit(address);
    }
    return ispTransmit(0);
}

uchar ispWriteFlash(unsigned long address, uchar data, uchar pollmode) {

	/* 0xFF is value after chip erase, so skip programming
	 if (data == 0xFF) {
	 return 0;
	 }
	 */

	ispUpdateExtended(address);
    if(chip==PROGISP_AVR)
    {
        ispTransmit(0x40 | ((address & 1) << 3));
        ispTransmit(address >> 9);
        ispTransmit(address >> 1);
        ispTransmit(data);

        if (pollmode == 0)
            return 0;

        if (data == 0x7F) {
            clockWait(15); /* wait 4,8 ms */
            return 0;
        } else {

            /* polling flash */
            uchar retries = 30;
            uint8_t starttime = TIMERVALUE;
            while (retries != 0) {
                if (ispReadFlash(address) != 0x7F) {
                    return 0;
                };

                if ((uint8_t) (TIMERVALUE - starttime) > CLOCK_T_320us) {
                    starttime = TIMERVALUE;
                    retries--;
                }
            }
        }
    }
    else
    {
        ispTransmit(0x40);
        ispTransmit(address >> 8);
        ispTransmit(address);
        ispTransmit(data);
        return 0;
    }
    return 1; /* error */
}

uchar ispFlushPage(unsigned long address, uchar pollvalue) {

	ispUpdateExtended(address);

	ispTransmit(0x4C);
	ispTransmit(address >> 9);
	ispTransmit(address >> 1);
	ispTransmit(0);

	if (pollvalue == 0xFF) {
		clockWait(15);
		return 0;
	} else {

		/* polling flash */
		uchar retries = 30;
		uint8_t starttime = TIMERVALUE;

		while (retries != 0) {
			if (ispReadFlash(address) != 0xFF) {
				return 0;
			};

			if ((uint8_t) (TIMERVALUE - starttime) > CLOCK_T_320us) {
				starttime = TIMERVALUE;
				retries--;
			}

		}

		return 1; /* error */
	}

}

uchar ispReadEEPROM(unsigned int address) {
	ispTransmit(0xA0);
	ispTransmit(address >> 8);
	ispTransmit(address);
	return ispTransmit(0);
}

uchar ispWriteEEPROM(unsigned int address, uchar data) {

	ispTransmit(0xC0);
	ispTransmit(address >> 8);
	ispTransmit(address);
	ispTransmit(data);

	clockWait(30); // wait 9,6 ms

	return 0;
}
