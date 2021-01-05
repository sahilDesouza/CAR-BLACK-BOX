

#include "i2c.h"
#include "ds1307.h"

void init_ds1307(void)
{
    unsigned char dummy;
    
    dummy = read_ds1307(SEC_ADDR);
    dummy = dummy & 0x7F;
    write_ds1307(SEC_ADDR, dummy);
}

unsigned char read_ds1307(unsigned char addr)
{
    unsigned char data;
    
    i2c_start();
    //write to desired slave with intentions(read/write))
    //7 bit address + 1 bit W
    //Master informing slave that it wants to write to it
    i2c_write(SLAVE_WRITE);
    
    //need to send the address from where you want to read in the RTC
    //Write address of HOURS/MINS/SECS address into the RTC
    i2c_write(addr);
    i2c_rep_start();
    
    //7 bit address + 1 bit R
    //Master informing slave that it wants to read from it
    i2c_write(SLAVE_READ);
   
    //reading from slave
    //already knows the address from where your reading
    data = i2c_read(0);
    i2c_stop();
    
    return data;
}

void write_ds1307(unsigned char addr, unsigned char data)
{
    i2c_start();
    i2c_write(SLAVE_WRITE);
    //need to send the address from where you want to write the data in the RTC
    i2c_write(addr);
    //write the data
    i2c_write(data);
    i2c_stop();
}
