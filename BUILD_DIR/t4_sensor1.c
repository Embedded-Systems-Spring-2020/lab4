#include <esos.h>
#include "esos_pic24.h"
#include "esos_pic24_rs232.h"
#include <p33EP512GP806.h>
#include <pic24_all.h>
#include <esos_f14ui.h>

#include <esos_sensor.h>

#include <stdio.h>
#include <stdlib.h>

#define LOOP_DELAY 1000

char buffer[30];
BOOL b_keepLooping = FALSE;
ESOS_USER_TASK(loop) {
    static uint16_t u16_data;

    ESOS_TASK_BEGIN();{
      for (;;) {

        ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Pressed() || esos_uiF14_isSW2Pressed());
		if (esos_uiF14_isSW2Pressed()){
			b_keepLooping = TRUE;
		}
		else {b_keepLooping = FALSE;
		}
        ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Released() && esos_uiF14_isSW2Released());
        do{                    //claim the ADC
        ESOS_TASK_WAIT_ON_AVAILABLE_SENSOR(ESOS_SENSOR_CH02, ESOS_SENSOR_VREF_3V3);

        //Now we have exclusive use, grab data
        ESOS_TASK_WAIT_SENSOR_QUICK_READ(u16_data);

        //Grab the output
        ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
		
        sprintf(buffer, "%d\n", u16_data);
        ESOS_TASK_WAIT_ON_SEND_STRING(buffer);
        ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();
		ESOS_TASK_WAIT_TICKS(LOOP_DELAY /2);
		if (esos_uiF14_isSW1Pressed()){
			ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Released());
			b_keepLooping = FALSE;
		}
		if (esos_uiF14_isSW2Pressed()){
			ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW2Released());
			b_keepLooping = FALSE;
		}
		ESOS_TASK_WAIT_TICKS(LOOP_DELAY /2);
		}while(b_keepLooping);
		
        //Release the potentiometer
        ESOS_SENSOR_CLOSE();        

      }  
    }

    ESOS_TASK_END();
}

void user_init(void){
    config_esos_uiF14();

    // Config heartbeat
    esos_uiF14_flashLED3(500);

    esos_RegisterTask(loop);
}