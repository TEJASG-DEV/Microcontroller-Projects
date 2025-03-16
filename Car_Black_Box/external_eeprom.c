#include "external_eeprom.h"
#include "i2c.h"
#include <xc.h>

void write_external_eeprom(unsigned char address, unsigned char data)
{
	i2c_start();
	i2c_write(0xA0);// data sheet page no 8. 24c02
	i2c_write(address);
	i2c_write(data);
	i2c_stop();
    // EEPROM delay to properly store
    for (unsigned long wait = 3000; wait--;);
}

unsigned char read_external_eeprom(unsigned char address)
{
	unsigned char data;

	i2c_start();
	i2c_write(0xA0); // data sheet page no 8. 24c02
	i2c_write(address);
	i2c_rep_start();
	i2c_write(0xA1);
	data = i2c_read();
	i2c_stop();

	return data;
}