

#include <xc.h>
#include "i2c.h"

void init_i2c(unsigned long baud)
{
    /* Set I2C Master Mode */
    SSPM3 = 1;
    
    /* Set the Required Baudrate */
    SSPADD  = (FOSC / (4 * baud)) - 1;
    
    /* Enable SSP serial transmission*/
    SSPEN = 1;
}

static void i2c_wait_for_idle(void)
{
    /* Wait till no activity on the bus */
    //R/W is 0
    //if SEN, RSEN, PEN, RCEN, ACKEN is 0 then bus is free
    while (R_nW || (SSPCON2 & 0x1F));
}

void i2c_start(void)
{
    i2c_wait_for_idle();
    //Initiate Start condition on SDA and SCL pins
    //start comm between master and slave
    SEN = 1;
}

void i2c_rep_start(void)
{
    i2c_wait_for_idle();
    RSEN = 1;
}

void i2c_stop(void)
{
    i2c_wait_for_idle();
    PEN = 1;
}

unsigned char i2c_read(unsigned char ack)
{
    unsigned char data;
    
    i2c_wait_for_idle();
    //Receive Enable bit
    RCEN = 1;
    
    i2c_wait_for_idle();
    //data received is stored in SSPBUF and then transfered to a local variable
    data = SSPBUF;
    
    //ack = 0, soo ACKDT == 0
    //1 = Not Acknowledge, 0 = Acknowledge
    if (ack == 1)
    {
        ACKDT = 1;
    }
    else
    {
        ACKDT = 0;
    }
    
    ACKEN = 1;
    
    return data;
}

int i2c_write(unsigned char data)
{
    i2c_wait_for_idle();
    
    //SSPBUF loaded with address/data and BF bit is set when SSPBUF reg is full
    SSPBUF = data;
    
    //SSPBUF bits are shifted out 1 bit after another and when buf reg is empty
    //BF = 0
    
    return !ACKSTAT;
}