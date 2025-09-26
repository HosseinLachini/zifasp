/*
 * isp.h - part of USBasp
 *
 * Autor..........: Thomas Fischl <tfischl@gmx.de>
 * Description....: Provides functions for communication/programming
 *                  over ISP interface
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2005-02-23
 * Last change....: 2009-02-28
 */

#ifndef __isp_h_included__
#define	__isp_h_included__

#ifndef uchar
#define	uchar	unsigned char
#endif

#define ISP_XTL1        PB1
#define ISP_XTL1_PORT   PORTB
#define ISP_XTL1_PIN    PINB
#define ISP_XTL1_DDR    DDRB

#define ISP_VCC         PB0
#define ISP_VCC_PORT    PORTB
#define ISP_VCC_PIN     PINB
#define ISP_VCC_DDR     DDRB

#define ISP_RST         PB1
#define ISP_RST_PORT    PORTB
#define ISP_RST_PIN     PINB
#define ISP_RST_DDR     DDRB

#define ISP_MOSI        PB2
#define ISP_MOSI_PORT   PORTB
#define ISP_MOSI_PIN    PINB
#define ISP_MOSI_DDR    DDRB

#define ISP_MISO        PB3
#define ISP_MISO_PORT   PORTB
#define ISP_MISO_PIN    PINB
#define ISP_MISO_DDR    DDRB

#define ISP_SCK         PB1
#define ISP_SCK_PORT    PORTB
#define ISP_SCK_PIN     PINB
#define ISP_SCK_DDR     DDRB

//#define ATM 0x00
//#define S5x 0xFF
//#define AT24CXX 0x01
//#define AT93CXX 0x02

extern const unsigned char ISP_SET_SCK[] PROGMEM;
extern uchar sck_sw_delay;

unsigned char chip;
unsigned char en_xtal;

/* scan ZIF socket for pull up diode AVR pin 1 */
extern unsigned char zif_scan(void);
/* Search ZIF socket & ISP for AVR */
unsigned char ispSearch(void);
/* define isp pin's */
void ispPins(const unsigned char *config_pin);

/* Xtal1 apply to target device */
void ispXtal1On(void);

/* Xtal1 misapply to target device */
void ispXtal1Off(void);

/* Apply VCC to target device */
void ispVccOn(void);

/* Misapply VCC to target device */
//void ispVccOff();

/* Prepare connection to target device */
void ispConnect(void);

/* Close connection to target device */
//void ispDisconnect();

/* read an write a byte from isp using software (slow) */
uchar ispTransmit(uchar send_byte);

/* enter programming mode */
uchar ispEnterProgrammingMode(void);

/* read byte from eeprom at given address */
uchar ispReadEEPROM(unsigned int address);

/* write byte to flash at given address */
uchar ispWriteFlash(unsigned long address, uchar data, uchar pollmode);

uchar ispFlushPage(unsigned long address, uchar pollvalue);

/* read byte from flash at given address */
uchar ispReadFlash(unsigned long address);

/* write byte to eeprom at given address */
uchar ispWriteEEPROM(unsigned int address, uchar data);

///* pointer to sw or hw transmit function */
//uchar (*ispTransmit)(uchar);

/* set SCK speed. call before ispConnect! */
#define ispSetSCKOption(sckoption)  (sck_sw_delay = pgm_read_byte(ISP_SET_SCK + sckoption))

/* load extended address byte */
void ispLoadExtendedAddressByte(unsigned long address);

#endif /* __isp_h_included__ */
