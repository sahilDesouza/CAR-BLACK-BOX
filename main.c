/*
 * File:   main.c
 * Author: Sahil Desouza_20001
 * Description:  Car black box which display current time, events, car speed.
 *               Menu_login can be accessed only by inputting correct password.
 *               On failure to input correct password will result in user blocked.
 *               Events, time, speed, password are all logged and stored on an external EEPROM using I2C protocol.
 *               Logs can be viewed using view_log present in menu bar and also downloaded using UART protocol.
 *               Time can be written/set to DS1307 using I2C protocol option present in menu bar.
 *               password can be changed and is written onto the EEPROM upn confirmation.
 * Created on 28 December, 2020, 9:00 PM
 */

#include <xc.h>
#include "i2c.h"
#include "ds1307.h"
#include "clcd.h"
#include "main.h"
#include "helper.h"
#include "adc.h"
#include "digital_keypad.h"
#include <string.h>
#include "AT24CS04.h"
#include "timers.h"
#include "uart.h"

#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT disabled)


static void init_config(void) {
    init_i2c(9600);
    init_ds1307();  
    init_clcd();
    init_adc();
    init_digital_keypad();
    init_timer2();
    init_uart(9600);
    puts("Ready To Download Logs\n\r");
    PEIE = 1;
    GIE = 1;
}
char * gear[] = {"GN", "GR", "G1", "G2", "G3", "G4"};
void main(void) 
{
    //unsigned char clock_reg[3];
    unsigned char key, reset_flag;
    char event[3] = "ON";
    int j;
    //Max speed is 99
    unsigned char speed = 0;
    int gr = 0;
    char menu_pos;
    init_config();
    log_car_event(event, speed);
    unsigned char control_flag = DASH_BOARD_FLAG;
    
    eeprom_at24c04_byte_write(0x00, '4');
    eeprom_at24c04_byte_write(0x01, '5');
    eeprom_at24c04_byte_write(0x02, '5');
    eeprom_at24c04_byte_write(0x03, '4');

    while (1) 
    {
        //scaling the input range to 1:10
        speed = read_adc() / 10;
        
        //when speed becomes > 99 then fix it to 99
        //max speed is retained to 99
        if(speed > 99)
            speed = 99;
        key = read_digital_keypad(STATE);
        for(j = 3000; j-- ;);
        
        if(key == SW1)
        {
            //event for collision C
            strcpy(event, "C ");
            log_car_event(event, speed);
        }
        else if(key == SW2)
        {
            //gr = 0
            strcpy(event, gear[gr]);
            log_car_event(event, speed);
            if(gr < 5)
                gr++;
        }
        else if (key == SW3)
        {
            if(gr > 0)
                gr--;
            
            strcpy(event, gear[gr]);
            log_car_event(event, speed);
            
        }
        else if((key == SW4 || key == SW5) && control_flag == DASH_BOARD_FLAG)
        {
           //prompt for password
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(500);
            clcd_print(" ENTER PASSWORD ", LINE1(0));
            clcd_putch(' ', LINE2(5));
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            control_flag = LOGIN_FLAG;
            reset_flag = RESET_PASSWORD;
            TMR2ON = 1;
            
        }
        //hold key to enter into menu option
        if(key == LSW4 && control_flag == LOGIN_MENU_FLAG)
        {
            switch(menu_pos)
            {
                case 0:
                    //to view log
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(100);
                    control_flag = VIEW_LOG_FLAG;
                    //view_log(key, reset_flag);
                    break;
                case 1:
                    //to clear log
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(100);
                    control_flag = CLEAR_LOG_FLAG;
                    break;
                case 2:
                    //to download log
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(100);
                    puts("LOGS:     TIME   EVENT SPEED\n\r");
                    control_flag = DOWNLOAD_LOG_FLAG;
                    break;
                case 3:
                    //to set time
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(100);
                    clcd_print("HH:MM:SS", LINE1(2));
                    control_flag = SET_TIME_FLAG;
                    reset_flag = RESET_TIME;
                    break;
                case 4:
                    //to password
                    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
                    __delay_us(100);
                    clcd_print(" ENTER PASSWORD ", LINE1(0));
                    //control_flag = reset_new_password(key);
                    control_flag = CHANGE_PASSWORD_FLAG;
                    reset_flag = RESET_PASSWORD;
                    clcd_putch(' ', LINE2(5));
                    clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
                    break;
                            
            }
        
        }
        //hold key to go back
        if(key == LSW5 && ((control_flag == VIEW_LOG_FLAG) || (control_flag == CLEAR_LOG_FLAG) || (control_flag == DOWNLOAD_LOG_FLAG) || (control_flag == CHANGE_PASSWORD_FLAG)))
        {
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            __delay_us(100);
            control_flag = LOGIN_MENU_FLAG;
        }
         
        switch(control_flag)
        {
            //case to display dashboard
            case DASH_BOARD_FLAG:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                display_dash_board(event, speed);
                break;
            
            //login flag used to enter login menu if password is correct
            case LOGIN_FLAG:
                //login(key, reset_flag);
                //break;
                switch(login(key, reset_flag))
                {
                    case RETURN_BACK:
                        control_flag = DASH_BOARD_FLAG;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON = 0;
                        
                        break;
                    case LOGIN_SUCCESS:
                        control_flag = LOGIN_MENU_FLAG;
                        reset_flag = RESET_LOGIN_MENU;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        //clcd_print("PASSWORD CORRECT", LINE2(0));
                        TMR2ON = 0;
                        continue;
                       
                }
                break;
            //case to display login menu    
            case LOGIN_MENU_FLAG:
                menu_pos = login_menu(key, reset_flag);
                break;
            //display view_log option
            case VIEW_LOG_FLAG:
                view_log(key, reset_flag);
                break;
            //display clear_log option
            case CLEAR_LOG_FLAG:
                clear_log();
                break;
            //display change password option
            case CHANGE_PASSWORD_FLAG:
                control_flag = reset_new_password(key, reset_flag);
                if(control_flag == CONFIRM_CHANGE)
                {
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    clcd_print("CONFIRM PASSWORD ", LINE1(0));
                }
                break;
            //once password inputted once need to confirm
            //case to compare with previous inputted password
            case CONFIRM_CHANGE:
                //clcd_print("CONFIRM PASSWORD ", LINE1(0));
                control_flag = confirm_password(key);
                break;
            //case to download flag
            case DOWNLOAD_LOG_FLAG: 
                clcd_print("DOWNLOADING...", LINE1(0));
                __delay_ms(100);
                control_flag = download_data();
                break;
            //case to enter time_menu
            case SET_TIME_FLAG: 
                control_flag = set_time(key, reset_flag);
                break;                
        }
        reset_flag = RESET_NOTHING;
        //display the start dash_board
        //display_dash_board(event, speed);
        
    }
    
    return;
}
