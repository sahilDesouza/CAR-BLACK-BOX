
#include <xc.h>
#include "i2c.h"
#include "AT24CS04.h"
#include "clcd.h"

/*
void init_ds1307(void)
{
    unsigned char dummy;
    
    dummy = read_ds1307(SEC_ADDR);
    dummy = dummy & 0x7F;
    write_ds1307(SEC_ADDR, dummy);
}
*/

unsigned char eeprom_at24c04_random_read(unsigned char addr)
{
    unsigned char data;
    
    i2c_start();
    //write to desired slave with intentions(read/write))
    //7 bit address + 1 bit Write
    //Master informing slave that it wants to write to it
    i2c_write(SLAVE_WRITE_EEPROM);
    
    //need to send the address from where you want to read in the RTC
    //Write address of HOURS/MINS/SECS address into the RTC
    i2c_write(addr);
    i2c_rep_start();
    
    //7 bit address + 1 bit Read
    //Master informing slave that it wants to read from it
    i2c_write(SLAVE_READ_EEPROM);
   
    //reading from slave
    //already knows the address from where your reading
    data = i2c_read(0);
    i2c_stop();
    
    return data;
}

void eeprom_at24c04_byte_write(unsigned char addr, unsigned char data)
{
    i2c_start();
    i2c_write(SLAVE_WRITE_EEPROM);
    //need to send the address from where you want to write the data in the RTC
    i2c_write(addr);
    
    //slave is already in the write mode
    //soo directly send/write the data
    i2c_write(data);
    i2c_stop();
    
    __delay_us(100);
}

void eeprom_at24c04_str_write(unsigned char addr, char * data)
{
    //i2c_start();
    //i2c_write(SLAVE_WRITE_EEPROM);
    //need to send the address from where you want to write the data in the RTC
    
    while(*data)
    {
        eeprom_at24c04_byte_write(addr, *data);
        //i2c_write(addr);
        //i2c_write(*data);
        data++;
        addr++;
    }
    //i2c_stop();
    
    __delay_us(100);
}
 
