#ifndef EXT_EEPROM_H
#define EXT_EEPROM_H

void write_external_eeprom(unsigned char address1,  unsigned char data);
unsigned char read_external_eeprom(unsigned char address1);

#endif