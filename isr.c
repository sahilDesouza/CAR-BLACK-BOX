
#include <xc.h>
#include "main.h"
#include "timers.h"

int return_time;
void __interrupt() isr(void)
{
    static unsigned int count0 = 0;
   
    if (TMR2IF == 1)
    {
        //TMR2 Register value + 6 (offset count to get 250 ticks) + 2 Inst Cycle 
       
         //Time calculated for 1 second
        if (count0++ == 1250)
        {
            count0 = 0;
            if(return_time > 0)
                return_time--;
            //LED3 = !LED3;
            
        }
        //Reset flag
        TMR2IF = 0;
    }
}