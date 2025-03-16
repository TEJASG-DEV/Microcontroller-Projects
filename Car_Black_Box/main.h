/* 
 * File:   main.h
 * Author: tejas
 *
 * Created on March 6, 2025, 6:25 PM
 */

#ifndef __MAIN_H__
#define	__MAIN_H__

#include <xc.h>
#include "clcd.h"
#include "adc.h"
#include "matrix_keypad.h"
#include "external_eeprom.h"
#include "uart.h"
#include "i2c.h"
#include "ds1307.h"

// Variables

typedef enum {
    ON,
    GR,
    GN,
    G1,
    G2,
    G3,
    G4,
    G5,
    C
} bb_event;

typedef enum {
    DASHBOARD,
    MENU,
    VIEW_LOG,
    DOWNLOAD_LOG,
    CLEAR_LOG,
    SET_TIME
} bb_state;

char event_str[][3] = {"ON", "GR", "GN", "G1", "G2", "G3", "G4", "G5", " C"};

bb_event event = ON;
bb_state state = DASHBOARD;

// EEPROM data structure

typedef struct {
    unsigned char Hour;
    unsigned char Minute;
    unsigned char Second;
    unsigned char Event;
    unsigned char Speed;
} Event_Data;

Event_Data e;

// EEPROM last Write Index:
unsigned char index = 0; // 0, 5, 10, 15, ... , 45

// RTC
unsigned char clock_reg[3];
unsigned char time[9] = "00:00:00";

// ADC
unsigned short speed = 0;

// Matrix Keypad
unsigned char key = 0xFF;

// Delay
unsigned long int delay = 0;

// Functions

void init_config(void) {
    init_clcd();
    init_adc();
    init_matrix_keypad();
    init_uart();
    init_i2c();
    init_ds1307();
}

static void store_event(void) {

    unsigned char temp = 0;
    if (index < (9 * sizeof (e))) {
        write_external_eeprom(index, e.Hour);
        write_external_eeprom(index + 1, e.Minute);
        write_external_eeprom(index + 2, e.Second);
        write_external_eeprom(index + 3, e.Event);
        write_external_eeprom(index + 4, e.Speed);
        index += sizeof (e);
    } else {
        // shift higher to lower... then store at last.
        for (int i = 5; i < 50; i++) {
            // read the index, then shift to previous
            temp = read_external_eeprom(i);
            write_external_eeprom(temp, (i - 5));
        }
        write_external_eeprom(index, e.Hour);
        write_external_eeprom(index + 1, e.Minute);
        write_external_eeprom(index + 2, e.Second);
        write_external_eeprom(index + 3, e.Event);
        write_external_eeprom(index + 4, e.Speed);
    }
}

static void read_event(unsigned char address) {
    // Assumed address < EEPROM Range.
    e.Hour = read_external_eeprom(address);
    e.Minute = read_external_eeprom(address + 1);
    e.Second = read_external_eeprom(address + 2);
    e.Event = read_external_eeprom(address + 3);
    e.Speed = read_external_eeprom(address + 4);
}

void get_time(void) {
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    if (clock_reg[0] & 0x40) {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    } else {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);
    time[8] = '\0';
}

void dashboard(void) {

    // capture RTC time.
    e.Hour = (time[0] - '0') * 10 + (time[1] - '0');
    e.Minute = (time[3] - '0') * 10 + (time[4] - '0');
    e.Second = (time[6] - '0') * 10 + (time[7] - '0');

    // Key read Logic
    if (key == MK_SW2) { // Increment
        if (event == G5) {
            // pass
        } else if (event == C) {
            event = GN;
        } else {
            ++event;
        }

        e.Event = event;
        e.Speed = speed;
        store_event();

    } else if (key == MK_SW3) { // Decrement
        if (event == ON) {
            event = GR;
        } else if (event == GR) {

        } else if (event == C) {
            event = GN;
        } else {
            --event;
        }

        e.Event = event;
        e.Speed = speed;
        store_event();

    } else if (key == MK_SW1) { // Crash
        event = C;

        e.Event = event;
        e.Speed = speed;
        store_event();

    } else {
        // pass
    }


    clcd_print("Time      EV  SP", LINE1(0));
    clcd_print(time, LINE2(0));

    speed = read_adc() / 10.33; // 0 to 99

    // 8, 9
    clcd_putch(' ', LINE2(8));
    clcd_putch(' ', LINE2(9));

    clcd_putch(event_str[event][0], LINE2(10));
    clcd_putch(event_str[event][1], LINE2(11));

    // 12, 13
    clcd_putch(' ', LINE2(12));
    clcd_putch(' ', LINE2(13));

    clcd_putch(speed / 10 + '0', LINE2(14));
    clcd_putch(speed % 10 + '0', LINE2(15));

}

void menu(void) {
    static unsigned char menu_index = 0, star_flag = 0; // set based on up or down pressed.
    static char menu_items[][16] = {
        "   VIEW LOG    ",
        " DOWNLOAD LOG  ",
        "   CLEAR LOG   ",
        "   SET TIME    "
    };

    if (key == MK_SW2) {
        // Up .. Decrement Index

        if (star_flag == 1) {
            star_flag = 0;
        } else if (menu_index > 0) {
            --menu_index;
        } else {
            // pass
        }

        /*
        if (menu_index == 0) {
            // pass
        } else {
            --menu_index;
        }
         */

    } else if (key == MK_SW3) {
        // Down .. Increment Index

        if (star_flag == 0) {
            star_flag = 1;
        } else if (menu_index < 2) { // this is hard to understand!
            ++menu_index;
        } else {
            // pass
        }
        /*
        if (menu_index == 3) {
             // pass
        } else {
            ++menu_index;
        }
         */
    } else if (key == MK_SW4) {
        // Change state to the functions.

        // Hard to Understand!
        state = menu_index + 2 + star_flag; // refer the states enum...
        key = ALL_RELEASED;


    } else if (key == MK_SW5) { // to be implemented within the menu item's function calls.
        menu_index = 0;
        star_flag = 0;
        state = DASHBOARD;
    } else {
        // pass
    }

    // The logic of display

    // '*' char
    if (star_flag) {
        clcd_putch(' ', LINE1(0));
        clcd_putch(0x7E, LINE2(0)); // '*'
    } else {
        clcd_putch(0x7E, LINE1(0)); // '*'
        clcd_putch(' ', LINE2(0));
    }

    // Hard to understand part!
    clcd_print(menu_items[menu_index], LINE1(1));
    clcd_print(menu_items[menu_index + 1], LINE2(1));



}

// Structure was a stupid Idea.

void view_log(void) {

    static unsigned char display[16] = "                ";

    static char view_index = 0;

    if (index == 0) {
        if (delay < 2200) {
            ++delay;
            clcd_print("   LOGS EMPTY!  ", LINE1(0));
            clcd_print("   VIEW FAILED  ", LINE2(0));
        } else {
            delay = 0;
            state = MENU;
        }
    } else {

        clcd_print("VIEW LOGS :-    ", LINE1(0));

        if (key == MK_SW2) {
            // decrement:
            if (view_index == 0) {
                // pass
            } else {
                view_index -= sizeof (e); // or sizeof Event_Data.
            }

            read_event(view_index);

        } else if (key == MK_SW3) {
            // increment:
            if (view_index == (index - 5)) {
                // pass
            } else {
                view_index += sizeof (e);
            }

            read_event(view_index);

        } else if (key == MK_SW5) {
            // go back.
            state = MENU;
            view_index = 0; // This is important.
        } else {
            // pass
        }

        // get the corresponding event data then display it.
        // number
        display[0] = view_index / 5 + '0';

        // Hour:Minute:Second
        display[2] = e.Hour / 10 + '0';
        display[3] = e.Hour % 10 + '0';
        display[4] = ':';
        display[5] = e.Minute / 10 + '0';
        display[6] = e.Minute % 10 + '0';
        display[7] = ':';
        display[8] = e.Second / 10 + '0';
        display[9] = e.Second % 10 + '0';

        // Event
        display[11] = event_str[e.Event][0];
        display[12] = event_str[e.Event][1];

        // speed
        display[14] = e.Speed / 10 + '0';
        display[15] = e.Speed % 10 + '0';

        clcd_print(display, LINE2(0));

    }
}

void download_log(void) {
    // download log, automatically go back... display for 5 sec. Downloaded Log Successful
    // UART code goes here...


    unsigned char display_string[] = "                \n\r", idx = 0;

    if (index == 0) {
        if (delay < 2200) {
            ++delay;
            clcd_print("   LOGS EMPTY!  ", LINE1(0));
            clcd_print("DOWNLOAD FAILED!", LINE2(0));
        } else {
            delay = 0;
            state = MENU;
        }
    } else {
        // send the eeprom data to PC.
        if (delay == 0) {
            puts("Download Success!\n\r");
            puts("# Time     EV SP\n\r");

            // display each event.
            for (idx = 0; idx <= (index - 5); idx += sizeof (e)) {
                read_event(idx);
                // number
                display_string[0] = idx / 5 + '0';
                // time
                display_string[2] = e.Hour / 10 + '0';
                display_string[3] = e.Hour % 10 + '0';
                display_string[4] = ':';
                display_string[5] = e.Minute / 10 + '0';
                display_string[6] = e.Minute % 10 + '0';
                display_string[7] = ':';
                display_string[8] = e.Second / 10 + '0';
                display_string[9] = e.Second % 10 + '0';
                // event
                display_string[11] = event_str[e.Event][0];
                display_string[12] = event_str[e.Event][1];
                // speed
                display_string[14] = e.Speed / 10 + '0';
                display_string[15] = e.Speed % 10 + '0';

                // uart call
                puts(display_string);
            }

            ++delay;
        } else if (delay < 2000) { // 45000 -> 1 min 6 s, 2000 -> ~3s
            ++delay;
            clcd_print(" DOWNLOADED LOG ", LINE1(0));
            clcd_print("  SUCCESSFULLY  ", LINE2(0));
        } else {
            delay = 0;
            state = MENU;
        }
    }

}

void clear_log(void) { // easiest.. just from 0 to 50, overwrite memory with 0s.
    // check if index == 0
    unsigned char idx = 0, j = 0;

    if (index == 0) {
        if (delay < 2200) {
            ++delay;
            clcd_print("     NO LOGS    ", LINE1(0));
            clcd_print("  CLEAR FAILED! ", LINE2(0));
        } else {
            delay = 0;
            state = MENU;
        }
    } else {

        /* No need of the Below  as it writes EEPROM. Very Bad for it's lifetime.
        for (idx = 0; idx < (index / 5) * sizeof (e); idx += sizeof (e)) {
            // write with 0s.
            for (j = 0; j < 5; j++) {
                internal_eeprom_write(0, idx + j);
            }
        }
         */

        if (delay < 2200) {
            ++delay;
            clcd_print("  LOGS CLEARED  ", LINE1(0));
            clcd_print("  SUCCESSFULLY! ", LINE2(0));
        } else {
            delay = 0;
            index = 0;
            state = MENU;
        }
    }
}

void set_time(void) { // accept user input from mkp and then set accordingly.

    static unsigned char once = 1, feild = 2, blink = 1;// feild will be from 0 to 2. ss, mm, hh
    unsigned char dummy = 0;
    static unsigned int delay = 0;

    if (once) {
        e.Hour = (time[0] - '0') * 10 + (time[1] - '0');
        e.Minute = (time[3] - '0') * 10 + (time[4] - '0');
        e.Second = (time[6] - '0') * 10 + (time[7] - '0');
        once = 0;
    }

    clcd_putch(' ', LINE2(0));
    clcd_putch(' ', LINE1(0));

    clcd_print("    HH:MM:SS    ", LINE1(0));

    
    clcd_print(time, LINE2(4));
    
    // Then the set time functionality,
    if (key == MK_SW5) {
        once = 1;
        feild = 2;
        state = MENU;
    } else if (key == MK_SW4) { // save and exit
        // write current time to RTC.
        dummy = e.Hour;
        e.Hour = read_ds1307(0x02);
        e.Hour = (e.Hour & 0xF0 ) | (dummy % 10);
        e.Hour = ( (dummy / 10) << 4 ) | (e.Hour & 0xCF);
        write_ds1307(0x02, e.Hour);
        
        dummy = e.Minute;
        e.Minute = read_ds1307(0x01);
        e.Minute = (e.Minute & 0xF0 ) | (dummy % 10);
        e.Minute = ( dummy / 10 << 4 ) | (e.Minute & 0x8F);
        write_ds1307(0x01, e.Minute);
        
        dummy = e.Second;
        e.Second = read_ds1307(0x00);
        e.Second = (e.Second & 0xF0 ) | (dummy % 10);
        e.Second = (dummy / 10 << 4) | (e.Second & 0x8F);
        write_ds1307(0x00, e.Second);
        
        //finally switch state to menu.
        once = 1;
        feild = 2;
        state = MENU;
        // pass
    } else if (key == MK_SW3) { // change field
        if (feild == 0) {
            feild = 2;
        } else {
            --feild;
        }
    } else if (key == MK_SW2) { // increment
        if (feild == 0) {
            // get register RTC data
            if (e.Second == 59) {
                e.Second = 0;
            } else {
                ++e.Second;
            }
            time[6] = e.Second / 10 + '0';
            time[7] = e.Second % 10 + '0';
        } else if (feild == 1) {
            // get minute data
            
            if (e.Minute == 59) {
                e.Minute = 0;
            } else {
                ++e.Minute;
            }
            time[3] = e.Minute / 10 + '0';
            time[4] = e.Minute % 10 + '0';
        } else if (feild == 2) {
            if (e.Hour == 23) {
                e.Hour = 0;
            } else {
                ++e.Hour;
            }
            time[0] = e.Hour / 10 + '0';
            time[1] = e.Hour % 10 + '0';
        }
    } else {
        // pass
    }

    // blink the clcd data.
    
    if (feild == 0) {
        if (delay == 500) {// toggle the seconds feild
            delay = 0;
            time[3] = e.Minute / 10 + '0';
            time[4] = e.Minute % 10 + '0';
            time[0] = e.Hour / 10 + '0';
            time[1] = e.Hour % 10 + '0';
            if (blink) {
                blink = 0;
                time[6] = e.Second / 10 + '0';
                time[7] = e.Second % 10 + '0';
            } else {
                blink = 1;
                time[6] = '_';
                time[7] = '_';
            }
            
        } else {
            delay++;
        }
    } else if (feild == 1) {
        if (delay == 500) {// toggle the minutes feild
            delay = 0;
            time[0] = e.Hour / 10 + '0';
            time[1] = e.Hour % 10 + '0';
            time[6] = e.Second / 10 + '0';
            time[7] = e.Second % 10 + '0';
            if (blink) {
                blink = 0;
                time[3] = e.Minute / 10 + '0';
                time[4] = e.Minute % 10 + '0';
            } else {
                blink = 1;
                time[3] = '_';
                time[4] = '_';
            }
        } else {
            delay++;
        }
    } else if (feild == 2) { // toggle the hours feild
        if (delay == 500) {
            delay = 0;
            time[3] = e.Minute / 10 + '0';
            time[4] = e.Minute % 10 + '0';
            time[6] = e.Second / 10 + '0';
            time[7] = e.Second % 10 + '0';
            if (blink) {
                blink = 0;
                time[0] = e.Hour / 10 + '0';
                time[1] = e.Hour % 10 + '0';
            } else {
                blink = 1;
                time[0] = '_';
                time[1] = '_';
            }
        } else {
            delay++;
        }
    }
}

#endif	/* __MAIN_H__ */

