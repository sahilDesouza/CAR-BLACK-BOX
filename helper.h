/* 
 * File:   helper.h
 * Author: sahil
 *
 * Created on 21 December, 2020, 7:44 PM
 */

#ifndef HELPER_H
#define	HELPER_H

void display_dash_board(char *, unsigned char);
void display_time(void);
void get_time(void);
void log_car_event(char * event, unsigned char speed);
void log_event(void);
char login_menu(unsigned char key, unsigned char reset_flag);
char login(unsigned char key, unsigned char reset_flag);
void view_log(unsigned char key, unsigned char reset_flag);
void clear_log(void);
unsigned char reset_new_password(unsigned char key, unsigned char reset_flag);
unsigned char confirm_password(unsigned char key);
unsigned char download_data(void);
unsigned char set_time(unsigned char key, unsigned char reset_flag);
#endif	/* HELPER_H */

