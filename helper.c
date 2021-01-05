
#include <xc.h>
#include "i2c.h"
#include "ds1307.h"
#include "clcd.h"
#include "main.h"
#include "helper.h"
#include <string.h>
#include "AT24CS04.h"
#include "digital_keypad.h"
#include "timers.h"
#include "uart.h"

//global variables declared
char time[7];
char log[11];
int pos = -1;
unsigned char rpos;
int access = -1;
unsigned char clock_reg[3];
extern int return_time;
char *menu[] = {"View log", "Clear log", "Download log", "Set time", "Change pwd"};
char reset_password[4];

void display_dash_board(char * event, unsigned char speed)
{
    clcd_print("  TIME     E  SP", LINE1(0));
    
    display_time();
    
    //to display event
    clcd_print(event, LINE2(11));
    
    //to display speed
    clcd_putch(speed / 10 + '0', LINE2(14));
    clcd_putch(speed % 10 + '0', LINE2(15));
    
    
}

void display_time(void)
{
    //get the HH MM SSvalue from RTC
    get_time();
    
    //print HH
    clcd_putch(time[0], LINE2(2));
    clcd_putch(time[1], LINE2(3));
    
    //Print ':'
    clcd_putch(':', LINE2(4));
    
    //print MM
    clcd_putch(time[2], LINE2(5));
    clcd_putch(time[3], LINE2(6));
    
    //Print ':'
    clcd_putch(':', LINE2(7));
    
    //print SS
    clcd_putch(time[4], LINE2(8));
    clcd_putch(time[5], LINE2(9));
    
    
}
void get_time(void)
{
    //get the HH:MM:SS and store in array
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);
    
    //Need to convert BCD to char to print on clcd
    //HH
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    //MM
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    //SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
      
}

void log_car_event(char * event, unsigned char speed)
{
    //TIME on speed -> HHMMSSONSP ->10 byte data
    
    //to get current time for event
    //time array updated to latest time
    get_time();
    
    //log:HHMMSS -> 6 bytes
    strncpy(log, time, 6);
            
    //log: event(ON) -> 2 bytes
    strncpy(&log[6], event, 2);
    
    //log: seconds 
    log[8] = speed / 10 + '0';
    log[9] = speed % 10 + '0';
    
    log[10] = '\0';
    
    //call log_event function to write to EEPROM
    log_event();
    
}
//function to write data to EEPROM
void log_event(void)
{
    unsigned char add;
    if(++pos == 10)
        pos = 0;
    
    //5, 15, 25 ... 
    //10 bytes of data always written HHMMSSEESS(10bytes))
    //logging/tracking total 10 events soo position increments by 10 each time uptill 105
    add = (pos * 10) + 5;
    
    //5-14, 15-24, ... 95-104
    //write into ext EEPROM
    eeprom_at24c04_str_write(add, log);
    
    //to keep track of events if access
    if(access < 9)
        access++;
}

//function that allows to enter into menu_bar only if password is correct
char login(unsigned char key, unsigned char reset_flag)
{
    //SW4:4, SW4:5
    //password -> 4554 ->SW4SW5SW5SW4
    static char npassword[4];
    static char spassword[4];
    static char i = 0, attempts_rem = '3';
    
    //re-intialise variables on initial entry
    if(reset_flag == RESET_PASSWORD)
    {
        key = ALL_RELEASED;
        i = 0;
        attempts_rem = '3';
        return_time = 5;
    }
    
    if(!return_time)
        return RETURN_BACK;
    
    //increment i on switch press
    if(key == SW4 && i < 4)
    {
        return_time = 5;
        npassword[i] = '4';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    if(key == SW5 && i < 4)
    {
        return_time = 5;
        npassword[i] = '5';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    //i equals 4 after 4 digit password entered
    if(i == 4)
    {
        //get password from EEPROM 1st 4 locations
        for(char j = 0; j < 4; j++)
        {
            spassword[j] = eeprom_at24c04_random_read(j);
        }
        
        //spassword = "4554"
        //compare password
        if(strncmp(spassword, npassword, 4) == 0)
        {
            //correct password
            //clcd_print("Correct password", LINE2(0));
            return LOGIN_SUCCESS;
        }
        //wrong comparison results in reduced number of attempts
        else
        {
            attempts_rem--;
            //once attempts exhausted then user blocked for 10 seconds
            if(attempts_rem == '0')
            {
                clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                __delay_us(500);
                clcd_print("You Are Blocked", LINE1(0));
                clcd_print("For 10 Seconds", LINE2(0));
                //block for 10 secs
                __delay_ms(10000);
                attempts_rem = '3';
            }
            //if attempts not 0 then print to clcd of number of attempts remaining
            else
            {
                clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                __delay_us(500);
                
                clcd_print("WRONG PASSWORD", LINE1(1));
                clcd_putch(attempts_rem, LINE2(0));
                clcd_print("Attempts remain", LINE2(1));
                __delay_ms(3000);
            }
            //clear the display
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(500);
            clcd_print(" ENTER PASSWORD ", LINE1(0));
            i = 0;
            
        }
    
    }
    
    return 0x10;
    
}

//function to select option in menu_bar and return desired position of '*'
char login_menu(unsigned char key, unsigned char reset_flag)
{
    static char menu_pos;
    static char select_pos;
    //reset variables upon re-entry
    if(reset_flag == RESET_LOGIN_MENU)
    {
        select_pos = 1;
        menu_pos = 0;
        clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
        __delay_us(500);
    }
    //increment/decrement menu position on switch press
    if((key == SW5) && menu_pos < 4)
    {
        clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
        __delay_us(500);
        menu_pos++;
        select_pos = 2;
    }
    else if((key == SW4) && menu_pos > 0)
    {
        clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
        __delay_us(500);
        menu_pos--;
        select_pos = 1;
    }
    // char '*' helps user to identify selected position on clcd display
    if(select_pos == 1)
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1], LINE2(2));
    }
    else if(select_pos == 2)
    {
        clcd_putch('*', LINE2(0));
        clcd_print(menu[menu_pos - 1], LINE1(2));
        clcd_print(menu[menu_pos], LINE2(2));
    }
    return menu_pos;

}

//function to view logs
void view_log(unsigned char key, unsigned char reset_flag)
{
    char rlog[11];
    //unsigned char rpos;
    unsigned char add;
    static unsigned char flag = 1, flag2;
    
    //if logs cleared then access == -1
    if(access == -1)
    {
        clcd_print("No Logs", LINE1(0));
        flag2 = 1;
    }
    else
    {
        //clear screen
        if(flag2)
        {
            clcd_print("       ", LINE1(0));
            flag2 = 0;
        }
        if(flag)
        {
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(100);
            flag = 0;
        }
        //reset variables back to default upon re-entry
        if(reset_flag == RESET_VIEW_LOG_POS)
        {
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(100);
            add = 5;
            rpos = 0;
            key = 0x00;
            
        }
        
        //depending on rpos data will be read from external eeprom
        //rpos determines number of events read
        //rpos increments or decrements depending on switch pressed
        if(key == SW4 && rpos < access)
        {
            //clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            //__delay_us(100);
            flag = 1;
            rpos++;
        }
        if(key == SW5 && rpos > 0)
        {
            //clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            //__delay_us(100);
            flag = 1;
            rpos--;
        }
        //read 10 bytes of data
        //total events depends on rpos
        for(unsigned char i = 0; i < 10; i++)
        {
            add = (rpos * 10) + 5;
            rlog[i] = eeprom_at24c04_random_read(add + i);
        }
        //print the logs on the clcd screen
        //logs contains HH:MM:SS E SP
        clcd_putch(rpos + '0', LINE2(0));
        clcd_putch(rlog[0], LINE2(2));
        clcd_putch(rlog[1], LINE2(3));
        clcd_putch(':', LINE2(4));
        clcd_putch(rlog[2], LINE2(5));
        clcd_putch(rlog[3], LINE2(6));
        clcd_putch(':', LINE2(7));
        clcd_putch(rlog[4], LINE2(8));
        clcd_putch(rlog[5], LINE2(9));
        clcd_putch(rlog[6], LINE2(11));
        clcd_putch(rlog[7], LINE2(12));
        clcd_putch(rlog[8], LINE2(14));
        clcd_putch(rlog[9], LINE2(15));
    }


}

//function to clear the logs
void clear_log(void)
{
    //clearing all the global variables
    clcd_print("Cleared Logs", LINE1(0));
    access = -1;
    pos = -1;
    rpos = 0;
}

//function to enter new password
unsigned char reset_new_password(unsigned char key, unsigned char reset_flag)
{
    //static char npassword[4];
    //static char spassword[4];
    //declare the variables
    static char i = 0;
    
    //condition to reset the arguments and variables
    if(reset_flag == RESET_PASSWORD)
    {
        key = ALL_RELEASED;
        i = 0;
        //attempts_rem = '3';
        //return_time = 5;
    }
    //input character into array depending upon the key_press
    if(key == SW4 && i < 4)
    {
        //return_time = 5;
        reset_password[i] = '4';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    if(key == SW5 && i < 4)
    {
        //return_time = 5;
        reset_password[i] = '5';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    //once first 4 digit password entered then clear display and return a macro
    //this macro will call another function to compare it with the intitial password
    if(i == 4)
    {
        i = 0;
        clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
        __delay_us(100);
        return CONFIRM_CHANGE;
    }
     //need to call same function again soo return CHANGE_PASSWORD_FLAG macro untill CONFIRM_CHANGE
    return CHANGE_PASSWORD_FLAG;
}

//function to confirm newly entered password
unsigned char confirm_password(unsigned char key)
{
    static char confirm_password[4];
    static char i = 0, flag = 1;
    
    //flag set inorder to blink the cursor
    if(flag)
    {
        clcd_putch(' ', LINE2(5));
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
        __delay_us(100);
        //flag set to 0
        flag = 0;
    }
    //if SW4 pressed '4' added into char array
    if(key == SW4 && i < 4)
    {
        flag = 0;
        return_time = 5;
        confirm_password[i] = '4';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    //if SW5 pressed '5' added into char array
    if(key == SW5 && i < 4)
    {
        flag = 0;
        return_time = 5;
        confirm_password[i] = '5';
        clcd_putch('*', LINE2(6 + i));
        i++;
    }
    //i equals 4 after 4 digit password has been entered
    if(i == 4)
    {
        //reset the flag soo that next confirm_pass call cursor will blink
        flag = 1;
        i = 0;
        
        //compare re-entered password with confirm_password string
        //if equal then new password set and go back to dash board
        if(strncmp(confirm_password, reset_password, 4) == 0)
        {
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(100);
            clcd_print("New password set", LINE1(0));
            for(unsigned char j = 0; j < 4; j++)
                eeprom_at24c04_byte_write(j, confirm_password[j]);
            __delay_ms(3000);
            return DASH_BOARD_FLAG;
        }
        //if not equal then invalid password and go back to dashboard
        else
        {
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(100);
            clcd_print("Invalid password set", LINE1(0));
            __delay_ms(3000);
            return DASH_BOARD_FLAG;
        }
    }
    return CONFIRM_CHANGE;
}

//function to dowload data and view on serial terminal
unsigned char download_data(void)
{
    char r_log[11];
    static unsigned char position = 0;
    unsigned char add;
    
    //if access  == -1 then no logs present
    if(access == -1)
    {
        puts("NO LOGS");
        return DASH_BOARD_FLAG;
    }
    //reading 10 bytes of data each time for 1 event
    for(unsigned char i = 0; i < 10; i++)
    {
        add = (position * 10) + 5;
        r_log[i] = eeprom_at24c04_random_read(add + i);
    }
    //adding null pointer at the final index
    r_log[10] = '\0';
    
    //printing the characters from array into the serial terminal
    if(r_log[0] != 0xFF)
    {
        puts("LOGS: \n");
        putchar(r_log[0]);
        putchar(r_log[1]);
        putchar(':');
        putchar(r_log[2]);
        putchar(r_log[3]);
        putchar(':');
        putchar(r_log[4]);
        putchar(r_log[5]);
        putchar(' ');
        putchar(' ');
        putchar(r_log[6]);
        putchar(r_log[7]);
        puts("      ");
        putchar(r_log[8]);
        putchar(r_log[9]);
        
        //puts(r_log);
        putchar('\n');
    }
    //postion limit depends on access
    //if 10 events logged then position increments till 10 
    if(position++ == access)
    {
        position = 0;
        return DASH_BOARD_FLAG;
    }
    
    return DOWNLOAD_LOG_FLAG;
}

//function to set the time 
unsigned char set_time(unsigned char key, unsigned char reset_flag)
{
    //declare static variables
    static int bcd[6];
    static int index = 5;
    static unsigned char count;
    static unsigned char bcd_hours, bcd_mins, bcd_seconds, hours, mins, seconds;
    static char time[7] = "000000";
    
    if(reset_flag == RESET_TIME)
    {
        //reset static variables back to default values
        key = ALL_RELEASED;
        index = 5;
        count = 9;
        for(unsigned char j = 0; j < 6; j++)
            bcd[j] = 0;
        
        //reset static variables back to 0
        bcd_hours = 0;
        bcd_mins = 0; 
        bcd_seconds = 0; 
        hours = 0; 
        mins = 0; 
        seconds = 0;
        
        //reset back to default time
        for(unsigned char j = 0; j < 6; j++)
            time[j] = '0';
    }
    //increment the time of HH/MM/SS using SW4 switch
    if(key == SW4)
    {
        //increment lower seconds 0-9
        if(index == 5 && time[index] < '9')
            time[index]++;
        //increment upper seconds 0-6
        if(index == 4 && time[index] < '5')
            time[index]++;
        //increment lower minute 0-9
        if(index == 3 && time[index] < '9')
            time[index]++;
        //increment upper minute 0-6
        if(index == 2 && time[index] < '5')
            time[index]++;
        //increment lower hour 0-9
        if(index == 1 && time[index] < '9')
            time[index]++;
        //increment upper hour 0-2
        if(index == 0 && time[index] < '2')
        {
            //if lower hour > 3 then upper hour wont be incremented
            if((time[index] == '1') && time[index + 1] > '3')
            {
                ;
            }
            else
            {
                time[index]++;
            }
        }
        bcd[index] =  time[index] - '0';
    }
    if(key == SW5)
    {
        //when index == -1 then we need to start conversion and write to RTC
        index--;
        count--;
        if((count == 7) || (count == 4))
            count--;
        //if(index < 0)
            //index = 5;
    }
    //clcd_print(time, LINE2(2));
    clcd_putch(' ', LINE2(count));
    __delay_us(100);
    clcd_putch(time[0], LINE2(2));
    clcd_putch(time[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(time[2], LINE2(5));
    clcd_putch(time[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(time[4], LINE2(8));
    clcd_putch(time[5], LINE2(9));
    
    //when index == -1 then do the conversion and write new time to RTC
    //then go back to dash board
    if(index == -1)
    {
        //converting from CHAR to INT
        hours = bcd[0]*10 + bcd[1];
        mins = bcd[2]*10 + bcd[3];
        seconds = bcd[4]*10 + bcd[5];
        __delay_us(1000);
        
        //calculating and converting from INT to BCD
        bcd_hours = (((hours / 10) << 4) | ((hours % 10)));
        bcd_mins = (((mins / 10) << 4) | ((mins % 10)));
        bcd_seconds = (((seconds / 10) << 4) | ((seconds % 10)));
        
        //writing to RTC with new time
        __delay_us(1000);
        write_ds1307(HOUR_ADDR, bcd_hours);
        write_ds1307(MIN_ADDR, bcd_mins);
        write_ds1307(SEC_ADDR, bcd_seconds);
        clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
        __delay_us(100);
        
        return DASH_BOARD_FLAG;
    }
    //need to call same function again soo return set_time macro untill password set
    return SET_TIME_FLAG;
    
}