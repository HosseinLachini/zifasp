/*
 * USBasp - USB in-circuit programmer for Atmel AVR controllers
 *
 * Thomas Fischl <tfischl@gmx.de>
 *
 * License........: GNU GPL v2 (see Readme.txt)
 * Target.........: ATMega8 at 12 MHz
 * Creation Date..: 2005-02-20
 * Last change....: 2009-02-28
 *
 * PC2 SCK speed option.
 * GND  -> slow (8khz SCK),
 * open -> software set speed (default is 375kHz SCK)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "usbasp.h"
#include "usbdrv/usbdrv.h"
#include "isp.h"
#include "clock.h"
#include "tpi.h"
#include "tpi_defs.h"
#include "Zif_Socket/zif.h"
#include "I2C/I2C.h"

/* compatibilty macros for old style */
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

static uchar replyBuffer[8];

static uchar prog_state = PROG_STATE_IDLE;
uchar prog_sck = USBASP_ISP_SCK_AUTO;

static uchar prog_address_newmode = 0;
static unsigned long prog_address;
static unsigned int prog_nbytes = 0;
static unsigned int prog_pagesize;
static uchar prog_blockflags;
static uchar prog_pagecounter;

static uchar i2cBackup[3];


uchar usbFunctionSetup(uchar data[8]) {

	uchar len = 0;

	//if (data[1] == USBASP_FUNC_BOOTLOADER)
    //    jump_to_boot();
	//if (data[1] == USBASP_FUNC_EXTENDED)
    //    return Extended(data);
	if (data[1] == USBASP_FUNC_CONNECT) {
        chip = data[3];
        switch(chip)
        {
        case PROGISP_24CXX:
            //i2cPins();
            i2cConnect();
            data[0] = 1;
            i2cSmart = 8;
            i2cReceive(0xa0, 0, 2, i2cBackup);
            i2cSmart = 16;
            i2cReceive(0xa0, 0, 1, i2cBackup + 2);
            i2cSend(0xa0, 0, 0, data);
            //chip = AT24CXX;
            /* set compatibility mode of address delivering */
            prog_address_newmode = 0;
            prog_address = 0;
            break;
        case PROGISP_93CXX:
            //chip = AT93CXX;
            //at93cxxConnect();
            break;
        case PROGISP_AVR:
        case PROGISP_MCS51:
            en_xtal = data[4] ? 0 : USBASP_EXT_RC_OSCILLATOR;
            //en_xtal = USBASP_EXT_OSCILLATOR;
            ispSetSCKOption(prog_sck);
            ispSearch();
            /* set SCK speed */
            //if ((PINE & (1 << PE0)) == 0) {
            //	ispSetSCKOption(USBASP_ISP_SCK_8);
            //} else {
            //}

            /* set compatibility mode of address delivering */
            prog_address_newmode = 0;

            ispConnect();
            break;
        }
        ledRedOn();
	} else if (data[1] == USBASP_FUNC_DISCONNECT) {
		//ispDisconnect();
		zif_disconnect();
		ledRedOff();

	} else if (data[1] == USBASP_FUNC_TRANSMIT) {
	    switch(chip) {
        case PROGISP_AVR :
            replyBuffer[0] = ispTransmit(data[2]);
            replyBuffer[1] = ispTransmit(data[3]);
            replyBuffer[2] = ispTransmit(data[4]);
            replyBuffer[3] = ispTransmit(data[5]);
            break;
	    case PROGISP_93CXX :
	        if(data[2] == 0xAC && data[3] == 0x80/* && data[4] == 0 && data[5] == 0*/) {
	        //if((unsigned long))
                //at93cxxErasesAll();
                replyBuffer[0] = '9';
                replyBuffer[1] = '3';
                replyBuffer[2] = 'E';
                replyBuffer[3] = 'A';
	        }
	        break;
        default :
            if(data[2]==0x24){
                replyBuffer[0] = ispTransmit(data[2]);
                replyBuffer[1] = ispTransmit(data[3]);
                replyBuffer[2] = ispTransmit(data[4]);
                switch(ispTransmit(data[5])&0x1C){
                    case(0x00):replyBuffer[3]=0xE0;break;
                    case(0x04):replyBuffer[3]=0xE5;break;
                    case(0x0C):replyBuffer[3]=0xEE;break;
                    case(0x1C):replyBuffer[3]=0xFF;break;
                    }
            }
            else if(data[2]==0x30){
                replyBuffer[0] = ispTransmit(0x28);
                replyBuffer[1] = ispTransmit(data[3]);
                replyBuffer[2] = ispTransmit(data[4]);
                replyBuffer[3] = ispTransmit(data[5]);
            }
            else{
                replyBuffer[0] = ispTransmit(data[2]);
                replyBuffer[1] = ispTransmit(data[3]);
                replyBuffer[2] = ispTransmit(data[4]);
                replyBuffer[3] = ispTransmit(data[5]);
            }
        }
		len = 4;

	} else if (data[1] == USBASP_FUNC_READFLASH) {

		if (!prog_address_newmode)
        {
            if(chip == PROGISP_24CXX)
            {
                prog_address = (data[3] << 8) | data[2];
            }
            else
            {
                prog_address = (data[3] << 8) | data[2];
            }
        }

		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_READFLASH;
		len = 0xff; /* multiple in */

	} else if (data[1] == USBASP_FUNC_READEEPROM) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_READEEPROM;
		len = 0xff; /* multiple in */

	} else if (data[1] == USBASP_FUNC_ENABLEPROG) {
	    switch(chip)
        {
        case PROGISP_24CXX :
            replyBuffer[0] = 0;
            i2cEnterProgrammingMode();
            switch(i2cSmart)
            {
            case 0:
                replyBuffer[0] = 1;
                break;
            case 8:
                i2cSend(0xa0, 0, 2, i2cBackup);
                break;
            case 16:
                i2cSend(0xa0, 0, 1, i2cBackup + 2);
                break;
            }
            break;
        case PROGISP_93CXX :
            //replyBuffer[0] = at93cxxEnterProgrammingMode();
            break;
        default :
            replyBuffer[0] = ispEnterProgrammingMode();
        }
		len = 1;

	} else if (data[1] == USBASP_FUNC_WRITEFLASH) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_pagesize = data[4];
		prog_blockflags = data[5] & 0x0F;
		prog_pagesize += (((unsigned int) data[5] & 0xF0) << 4);
		if (prog_blockflags & PROG_BLOCKFLAG_FIRST) {
			prog_pagecounter = prog_pagesize;
		}
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_WRITEFLASH;
		len = 0xff; /* multiple out */

	} else if (data[1] == USBASP_FUNC_WRITEEEPROM) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_pagesize = 0;
		prog_blockflags = 0;
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_WRITEEEPROM;
		len = 0xff; /* multiple out */

	} else if (data[1] == USBASP_FUNC_SETLONGADDRESS) {

		/* set new mode of address delivering (ignore address delivered in commands) */
		prog_address_newmode = 1;
		/* set new address */
		prog_address = *((unsigned long*) &data[2]);

	} else if (data[1] == USBASP_FUNC_SETISPSCK) {

		/* set sck option */
		prog_sck = data[2];
		replyBuffer[0] = 0;
		len = 1;

	} else if (data[1] == USBASP_FUNC_TPI_CONNECT) {
		tpi_dly_cnt = data[2] | (data[3] << 8);

		/* RST high */
		//ISP_OUT |= (1 << ISP_RST);
		//ISP_DDR |= (1 << ISP_RST);
		sbi(ISP_RST_PORT, ISP_RST);
		sbi(ISP_RST_DDR, ISP_RST);

		clockWait(3);

		/* RST low */
		//ISP_OUT &= ~(1 << ISP_RST);
		cbi(ISP_RST_PORT, ISP_RST);
		ledRedOn();

		clockWait(16);
		tpi_init();

	} else if (data[1] == USBASP_FUNC_TPI_DISCONNECT) {

		tpi_send_byte(TPI_OP_SSTCS(TPISR));
		tpi_send_byte(0);

		clockWait(10);

		/* pulse RST */
		//ISP_OUT |= (1 << ISP_RST);
		sbi(ISP_RST_PORT, ISP_RST);
		clockWait(5);
		//ISP_OUT &= ~(1 << ISP_RST);
		cbi(ISP_RST_PORT, ISP_RST);
		clockWait(5);

		/* set all ISP pins inputs */
		//ISP_DDR &= ~((1 << ISP_RST) | (1 << ISP_SCK) | (1 << ISP_MOSI));
		cbi(ISP_RST_DDR, ISP_RST);
        cbi(ISP_SCK_DDR, ISP_SCK);
        cbi(ISP_MOSI_DDR, ISP_MOSI);
		/* switch pullups off */
		//ISP_OUT &= ~((1 << ISP_RST) | (1 << ISP_SCK) | (1 << ISP_MOSI));

        cbi(ISP_RST_PORT, ISP_RST);
        cbi(ISP_SCK_PORT, ISP_SCK);
        cbi(ISP_MOSI_PORT, ISP_MOSI);

		ledRedOff();

	} else if (data[1] == USBASP_FUNC_TPI_RAWREAD) {
		replyBuffer[0] = tpi_recv_byte();
		len = 1;

	} else if (data[1] == USBASP_FUNC_TPI_RAWWRITE) {
		tpi_send_byte(data[2]);

	} else if (data[1] == USBASP_FUNC_TPI_READBLOCK) {
		prog_address = (data[3] << 8) | data[2];
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_TPI_READ;
		len = 0xff; /* multiple in */

	} else if (data[1] == USBASP_FUNC_TPI_WRITEBLOCK) {
		prog_address = (data[3] << 8) | data[2];
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_TPI_WRITE;
		len = 0xff; /* multiple out */

	} else if (data[1] == USBASP_FUNC_GETCAPABILITIES) {
		replyBuffer[0] = USBASP_CAP_0_TPI;
		replyBuffer[1] = 0;
		replyBuffer[2] = 0;
		replyBuffer[3] = 0;
		len = 4;
	}

	usbMsgPtr = replyBuffer;

	return len;
}

uchar usbFunctionRead(uchar *data, uchar len) {

	uchar i;

	/* check if programmer is in correct read state */
	if ((prog_state != PROG_STATE_READFLASH) && (prog_state
			!= PROG_STATE_READEEPROM) && (prog_state != PROG_STATE_TPI_READ)) {
		return 0xff;
	}

	/* fill packet TPI mode */
	if(prog_state == PROG_STATE_TPI_READ)
	{
		tpi_read_block(prog_address, data, len);
		prog_address += len;
		return len;
	}

	if(chip == PROGISP_24CXX)
    {
        if (prog_state == PROG_STATE_READFLASH)
        {
            i2cReceive(0xa0, prog_address, len, data);
            prog_address += len;
        }

        /* last packet? */
        if (len < 8) {
            prog_state = PROG_STATE_IDLE;
        }
        return len;
    }

	/* fill packet ISP mode */
	for (i = 0; i < len; i++) {
		if (prog_state == PROG_STATE_READFLASH) {
			data[i] = ispReadFlash(prog_address);
		} else {
			data[i] = ispReadEEPROM(prog_address);
		}
		prog_address++;
	}

	/* last packet? */
	if (len < 8) {
		prog_state = PROG_STATE_IDLE;
	}

	return len;
}

uchar usbFunctionWrite(uchar *data, uchar len) {

	uchar retVal = 0;
	uchar i;

	/* check if programmer is in correct write state */
	if ((prog_state != PROG_STATE_WRITEFLASH) && (prog_state
			!= PROG_STATE_WRITEEEPROM) && (prog_state != PROG_STATE_TPI_WRITE)) {
		return 0xff;
	}

	if(chip == PROGISP_24CXX)
    {
        if(prog_state == PROG_STATE_WRITEFLASH)
        {
            //while(prog_nbytes)
            {
                //if(i2cSmart)
                {
                    //for(i = 0; i < (len/i2cSmart); i+=i2cSmart)
                    //    i2cSend(0xa0, prog_address + i, i2cSmart, data + i);
                    //for(; i < len; i++)
                    //    i2cSend(0xa0, prog_address + i, 1, data + i);
                    //for(i = 0; i < len; i++)
                    //    i2cSend(0xa0, prog_address + i, 1, data + i);
                    i2cWriteBlock( data, len, prog_address);
                }
                //else
                //{
                //    for(i = 0; i < len; i+=8)
                //        i2cSendStandard(0xa0, prog_address + i, 8, data + i);
                //}
                prog_address += len;
                prog_nbytes -= len;
                if(prog_nbytes <= 0)
                {
                    prog_state = PROG_STATE_IDLE;
                    return 1;
                }
            }
            return 0;
        }
    }

	if (prog_state == PROG_STATE_TPI_WRITE)
	{
		tpi_write_block(prog_address, data, len);
		prog_address += len;
		prog_nbytes -= len;
		if(prog_nbytes <= 0)
		{
			prog_state = PROG_STATE_IDLE;
			return 1;
		}
		return 0;
	}

	for (i = 0; i < len; i++) {

		if (prog_state == PROG_STATE_WRITEFLASH) {
			/* Flash */

			if (prog_pagesize == 0) {
				/* not paged */
				ispWriteFlash(prog_address, data[i], 1);
			} else {
				/* paged */
				ispWriteFlash(prog_address, data[i], 0);
				prog_pagecounter--;
				if (prog_pagecounter == 0) {
					ispFlushPage(prog_address, data[i]);
					prog_pagecounter = prog_pagesize;
				}
			}

		} else {
			/* EEPROM */
			ispWriteEEPROM(prog_address, data[i]);
		}

		prog_nbytes--;

		if (prog_nbytes == 0) {
			prog_state = PROG_STATE_IDLE;
			if ((prog_blockflags & PROG_BLOCKFLAG_LAST) && (prog_pagecounter
					!= prog_pagesize)) {

				/* last block and page flush pending, so flush it now */
				ispFlushPage(prog_address, data[i]);
			}

			retVal = 1; // Need to return 1 when no more data is to be received
		}

		prog_address++;
	}

	return retVal;
}

static void initForUsbConnectivity(void)
{
    uchar   i = 0;
    usbInit();
    /* enforce USB re-enumerate: */
    usbDeviceDisconnect();  /* do this while interrupts are disabled */
    do{             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }while(--i);
    usbDeviceConnect();
    sei();
}

int main(void) {
/*
    unsigned char d = 0x01;
    //i2cPins();
    _delay_ms(2000);
    i2cConnect();
    i2cSmart = 16;
    i2cSend(0xa0, 0, 1, &d);
	ledRedInit();
	ledGreenOn();
    i2cEnterProgrammingMode();
    while(1);
*/
	ledRedInit();
	//ledGreenInit();
	ledRedOn();
	//ledGreenOn();
	initForUsbConnectivity();

    chip = PROGISP_AVR;

	/* init timer */
	clockInit();

	/* main event loop */
	for (;;) {
		usbPoll();
	}
	return 0;
}

