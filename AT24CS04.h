/* 
 * File:   AT24CS04.h
 * Author: sahil
 *
 * Created on 21 December, 2020, 9:12 PM
 */

#ifndef AT24CS04_H
#define	AT24CS04_H

#define SLAVE_WRITE_EEPROM             0b10100000 //A2 and A1 pins grouded 
#define SLAVE_READ_EEPROM              0b10100001


//void init_AT24CS04(void);
unsigned char eeprom_at24c04_random_read(unsigned char addr);
void eeprom_at24c04_byte_write(unsigned char addr, unsigned char data);
void eeprom_at24c04_str_write(unsigned char , char *);

#endif	/* AT24CS04_H */

