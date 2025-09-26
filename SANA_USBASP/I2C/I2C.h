#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

// defines and constants
#define READ		0x01	// I2C READ bit

extern unsigned char i2cSmart;


extern void i2cConnect(void);
//extern void i2cPins(void);
extern void i2cSend(unsigned char, unsigned long, unsigned char, unsigned char*);
extern void i2cReceive(unsigned char, unsigned long, unsigned char, unsigned char*);
extern void i2cEnterProgrammingMode(void);
extern void i2cWriteBlock(unsigned char *data, unsigned char length, unsigned long subAddr);

#endif // I2C_H_INCLUDED
