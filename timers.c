#include <xc.h>
#include "main.h"
#include "timers.h"
#include "clcd.h"


//Initialise timer2
void init_timer2(void)
{
    //Switch timer on
    TMR2ON = 0;
    TMR2IE = 1;
    
    //Prescaling 1:16
    T2CKPS1 = 1;
    T2CKPS0 = 1;
    
    //Load 250 soo ticks from 0 - 250
    PR2 = 250;
    
    PEIE = 1;
    
}