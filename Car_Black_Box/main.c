#include "main.h"

// For God Sake >:-(  
extern unsigned char clock_reg[3]; // defined in main.h
extern unsigned char time[9]; // defined in main.h

void main(void) {
    init_config();


    while (1) {

        key = read_switches(STATE_CHANGE);
       

        // if statement to switch states;
        if ( (key == MK_SW4) && (state == DASHBOARD) ) {
            state = MENU;
            key = ALL_RELEASED;
        } else {
            // pass
        }



        switch (state) {
            case DASHBOARD:
                get_time();
                dashboard();
                break;
            case MENU:
                // menu code
                menu();
                break;
            case VIEW_LOG:
                // view Log
                view_log();
                break;
            case DOWNLOAD_LOG:
                // download log
                download_log();
                break;
            case CLEAR_LOG:
                // clear log
                clear_log();
                break;
            case SET_TIME:
                // set time
                set_time();
                break;
            default:
                // pass
                ;
        }
    }

    return;
}