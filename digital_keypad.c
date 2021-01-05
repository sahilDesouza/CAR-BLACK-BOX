

#include <xc.h>
#include "main.h"
#include "digital_keypad.h"

void init_digital_keypad(void)
{
    //Set Keypad Port as input 
    //Setting lower 6 bits of TRISB register as input
    //Done soo inorder to save the upper bits for other operations
    KEYPAD_PORT_DDR = KEYPAD_PORT_DDR | 0x3F; 
    //PORTB upper pins is set to as input port
}

unsigned char read_digital_keypad(unsigned char mode)
{
    static unsigned char once = 1, pre_key;
    static int long_press = 0;
    
    //LEVEL triggered
    if (mode == LEVEL)
    {
        return KEYPAD_PORT & 0x3F;
    }
    else
    {
        //EDGE triggered
        if (((KEYPAD_PORT & INPUT_LINES) != ALL_RELEASED) && once)
        {
            //Once set to 0 to prevent this block from running again if button remains pressed
            once = 0;
            long_press = 0;
            pre_key = KEYPAD_PORT & INPUT_LINES;
        }
        else if(!once && (pre_key == (KEYPAD_PORT & INPUT_LINES)) && long_press < 50 )
        {
            long_press++;
            //return 0x80 | pre_key;
        }
        else if(long_press == 50)
        {
            long_press++;
            return 0x80 | pre_key;
        }
        else if ((KEYPAD_PORT & 0x3F) == ALL_RELEASED && !once)
        {
            //Reset once to 1
            once = 1;
            if(long_press < 50)
            {
                return pre_key;
            }
        }
    }
    //Return all_released macro if no buttons are pressed
    return ALL_RELEASED;
}