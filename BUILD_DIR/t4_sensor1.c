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
      for (;;) {     //same as while(true)

        ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Pressed() || esos_uiF14_isSW2Pressed());  /on either switch, start the DO loop
		if (esos_uiF14_isSW2Pressed()){
			b_keepLooping = TRUE;   //if sw2 then keep looping; checked at the bottom while statement
		}
		else {b_keepLooping = FALSE;   //if sw1 don't keep looping just do once
		}
        ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Released() && esos_uiF14_isSW2Released());  /*wait for the release so
	   sw1 can exit the loop when pressed again. The loop is fast enough to go around and sense sw1 still pressed
	   and begin again*/
        do{                    //claim the ADC, setup for CH2(potentiometer) and 3V3 for upper Vref
        ESOS_TASK_WAIT_ON_AVAILABLE_SENSOR(ESOS_SENSOR_CH02, ESOS_SENSOR_VREF_3V3);

        //do single read of ADC
        ESOS_TASK_WAIT_SENSOR_QUICK_READ(u16_data);

        //wait for UART availability to send output to Bully Bootloader
        ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
		
        sprintf(buffer, "%d\n", u16_data);
        ESOS_TASK_WAIT_ON_SEND_STRING(buffer); //wait for data in buffer to be sent and release UART
        ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();
		ESOS_TASK_WAIT_TICKS(LOOP_DELAY /2);  /*this is half of the 1 second delay between samples
		  the following sw presses could be missed during the full second so we split the delay into 2 parts*/
		if (esos_uiF14_isSW1Pressed()){  //sw1 will stop the sw2 initiated continous looping
			ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Released());
			b_keepLooping = FALSE;
		}
		if (esos_uiF14_isSW2Pressed()){  //sw2 will also stop the looping
			ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW2Released());
			b_keepLooping = FALSE;
		}
		ESOS_TASK_WAIT_TICKS(LOOP_DELAY /2);  //the other half of the loop sw2 sensor reading delay
		}while(b_keepLooping); //keep doing the DO loop if sw2 initiated it
		
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
