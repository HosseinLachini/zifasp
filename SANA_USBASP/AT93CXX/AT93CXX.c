#include "AT93CXX.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "../Zif_Socket/zif.h"
#include "../isp.h"

#define		delay	_delay_us(10)

#define AT93CXX_READ    0b11000
#define AT93CXX_EWEN    0b10011
#define AT93CXX_ERASE   0b11100
#define AT93CXX_ERAL    0b10010
#define AT93CXX_WRITE   0b10100
#define AT93CXX_WRAL    0b10001
#define AT93CXX_EWDS    0b10000


static AT93CXX_IO at93cxx;
static unsigned char at93cxx_chip;

void at93cxxConnect(void) {

 zif_disconnect();

 pin_define(  1, &at93cxx.cs);
 pin_define(  2, &at93cxx.sk);
 pin_define(  3, &at93cxx.mosi);
 pin_define(  4, &at93cxx.miso);
 pin_define( 24, &at93cxx.gnd);
 pin_define( 30, &at93cxx.org);
 pin_define( 27, &at93cxx.vcc);

 set_bits( at93cxx.vcc.ddr, at93cxx.vcc.mask);
 set_bits( at93cxx.vcc.port, at93cxx.vcc.mask);

 set_bits( at93cxx.gnd.ddr, at93cxx.gnd.mask);
 clr_bits( at93cxx.gnd.port, at93cxx.gnd.mask);

 set_bits( at93cxx.cs.ddr, at93cxx.cs.mask);
 set_bits( at93cxx.sk.ddr, at93cxx.sk.mask);
 set_bits( at93cxx.mosi.ddr, at93cxx.mosi.mask);
 clr_bits( at93cxx.miso.ddr, at93cxx.miso.mask);
 set_bits( at93cxx.org.ddr, at93cxx.org.mask);

 clr_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 clr_bits( at93cxx.cs.port, at93cxx.cs.mask);
 clr_bits( at93cxx.sk.port, at93cxx.sk.mask);
 clr_bits( at93cxx.org.port, at93cxx.org.mask);
 set_bits( at93cxx.miso.port, at93cxx.miso.mask);

}

void at93cxxClock() {
 delay;
 set_bits( at93cxx.sk.port, at93cxx.sk.mask);
 delay;
 clr_bits( at93cxx.sk.port, at93cxx.sk.mask);
 delay;
}

unsigned int at93cxxTransmit(unsigned char Instruction, unsigned int Address, unsigned int Data)
{
 unsigned char len = at93cxx_chip + 3;

 if(Instruction == AT93CXX_WRITE || Instruction == AT93CXX_WRAL) {
    if(read_bits( at93cxx.org.pin, at93cxx.org.mask))
        len += 16;
    else
        len += 8;
 }
 set_bits( at93cxx.cs.port, at93cxx.cs.mask);
 clr_bits( at93cxx.cs.port, at93cxx.cs.mask);
}
unsigned char at93cxxEnterProgrammingMode() {
 unsigned char dout;

 at93cxx_chip = NO_CHIP;

 set_bits( at93cxx.cs.port, at93cxx.cs.mask);
 set_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 at93cxxClock();
 at93cxxClock();
 clr_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 at93cxxClock();

 do{
    at93cxx_chip++;
    set_bits( at93cxx.sk.port, at93cxx.sk.mask);
    delay;
    dout = read_bits( at93cxx.miso.pin, at93cxx.miso.mask);
    clr_bits( at93cxx.sk.port, at93cxx.sk.mask);
    delay;
 }while((at93cxx_chip <= AT93C86) && (dout));

 clr_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 clr_bits( at93cxx.cs.port, at93cxx.cs.mask);
 clr_bits( at93cxx.sk.port, at93cxx.sk.mask);

 if((at93cxx_chip < AT93C46) || (at93cxx_chip > AT93C86) || (dout != 0)) {
    at93cxx_chip = NO_CHIP;
    return 1;
 }
 return 0;
}

void at93cxxWriteEnable()
{
 unsigned char i = at93cxx_chip;
 set_bits( at93cxx.cs.port, at93cxx.cs.mask);
 set_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 at93cxxClock();
 clr_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 at93cxxClock();
 at93cxxClock();
 set_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 while(i--)
    at93cxxClock();
 clr_bits( at93cxx.cs.port, at93cxx.cs.mask);
}

void at93cxxErasesAll()
{
 unsigned char i = at93cxx_chip - 1;
 at93cxxWriteEnable();
 set_bits( at93cxx.cs.port, at93cxx.cs.mask);
 set_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 at93cxxClock();
 clr_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 at93cxxClock();
 at93cxxClock();
 set_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 at93cxxClock();
 clr_bits( at93cxx.mosi.port, at93cxx.mosi.mask);
 while(i--)
    at93cxxClock();
 clr_bits( at93cxx.cs.port, at93cxx.cs.mask);
}
