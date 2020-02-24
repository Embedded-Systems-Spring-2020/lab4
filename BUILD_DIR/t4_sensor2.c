#include "pic24_all.h"
#include <p33EP512GP806.h>
#include "pic24_ports_config.h"

#include <esos.h>
#include <esos_task.h>
#include "esos_comm.h"
#include "esos_f14ui.h"
#include "esos_pic24.h"
#include "esos_sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOOP_DELAY 1000
#define ONESHOT '1'
#define AVG '2'
#define MIN '3'
#define MAX '4'
#define MEDIAN '5'
#define SAMPLES2 '1'
#define SAMPLES4 '2'
#define SAMPLES8 '3'
#define SAMPLES16 '4'
#define SAMPLES32 '5'
#define SAMPLES64 '6'
#define SAMPLES_ONESHOT '7'

// defined in "esos_sensor.h"
static esos_sensor_process_t sensor_processing_mode;

char buffer[30];
BOOL b_keepLooping = FALSE;

ESOS_CHILD_TASK(barGraph_child, uint16_t u16_num2graph){  //visual display of data
	static uint8_t u8_barGraph_value = 0;
	static uint8_t i;
	static uint8_t j;
	ESOS_TASK_BEGIN();
	ESOS_TASK_WAIT_ON_SEND_STRING("   |");     //draws a 20 '_' long line with a moving '|' 
	u8_barGraph_value = u16_num2graph / 50;    //max output 2^10 ~= 1000; /50 gives increments of 20
	for (i=0; i<u8_barGraph_value; i++){
			ESOS_TASK_WAIT_ON_SEND_STRING("_");
	}
	ESOS_TASK_WAIT_ON_SEND_STRING("|");        //after appropriate '_'s this is the values line
	for (j=0; j<(20-u8_barGraph_value); j++){  //finish the 20 '_'s
			ESOS_TASK_WAIT_ON_SEND_STRING("_");
	}
	ESOS_TASK_WAIT_ON_SEND_STRING("|\n");
	ESOS_TASK_END();
}

ESOS_CHILD_TASK(menu) {
    // neither of these will be used across yield or waits, so no need for static(-ness)
    static char proc_mode;
    static char num_samples;

    ESOS_TASK_BEGIN();
    ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();     // wait until we can grab the output stream
    // print a very pretty menu
    ESOS_TASK_WAIT_ON_SEND_STRING("\n+===========================+");
    ESOS_TASK_WAIT_ON_SEND_STRING("|  Select Processing mode   |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   1. one-shot             |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   2. average              |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   3. minimum              |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   4. maximum              |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   5. median               |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("+===========================+\n\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("> ");
    ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();      // let everyone else know we are done with the out stream

    ESOS_TASK_WAIT_ON_AVAILABLE_IN_COMM();      // wait until we can grab the input stream
    ESOS_TASK_WAIT_ON_GET_UINT8(proc_mode);     // write the user's input to the aforementioned buffer
    ESOS_TASK_SIGNAL_AVAILABLE_IN_COMM();       // let everyone else know we are done with the in stream

    // start the second part of the menu (largely same as above)
    ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
    ESOS_TASK_WAIT_ON_SEND_STRING("\n+===========================+\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|  Select Number of Samples |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   1. two                  |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   2. four                 |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   3. eight                |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   4. sixteen              |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   5. thirty-two           |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   6. sixty-four           |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("|   7. one-shot             |\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("+===========================+\n\n");
    ESOS_TASK_WAIT_ON_SEND_STRING("> ");
    ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

    ESOS_TASK_WAIT_ON_AVAILABLE_IN_COMM();
    ESOS_TASK_WAIT_ON_GET_UINT8(num_samples);
    ESOS_TASK_SIGNAL_AVAILABLE_IN_COMM();

    // assign the appropriate sensor_processing_mode based on user input
    // in the order defined in esos_sensor.h
    sensor_processing_mode = 
            proc_mode == ONESHOT && num_samples == SAMPLES_ONESHOT ? ESOS_SENSOR_ONE_SHOT

        :   proc_mode == AVG && num_samples == SAMPLES2 ? ESOS_SENSOR_AVG2
        :   proc_mode == AVG && num_samples == SAMPLES4 ? ESOS_SENSOR_AVG4
        :   proc_mode == AVG && num_samples == SAMPLES8 ? ESOS_SENSOR_AVG8
        :   proc_mode == AVG && num_samples == SAMPLES16 ? ESOS_SENSOR_AVG16
        :   proc_mode == AVG && num_samples == SAMPLES32 ? ESOS_SENSOR_AVG32
        :   proc_mode == AVG && num_samples == SAMPLES64 ? ESOS_SENSOR_AVG64
        
        :   proc_mode == MIN && num_samples == SAMPLES2 ? ESOS_SENSOR_MIN2
        :   proc_mode == MIN && num_samples == SAMPLES4 ? ESOS_SENSOR_MIN4
        :   proc_mode == MIN && num_samples == SAMPLES8 ? ESOS_SENSOR_MIN8
        :   proc_mode == MIN && num_samples == SAMPLES16 ? ESOS_SENSOR_MIN16
        :   proc_mode == MIN && num_samples == SAMPLES32 ? ESOS_SENSOR_MIN32
        :   proc_mode == MIN && num_samples == SAMPLES64 ? ESOS_SENSOR_MIN16

        :   proc_mode == MAX && num_samples == SAMPLES2 ? ESOS_SENSOR_MAX2
        :   proc_mode == MAX && num_samples == SAMPLES4 ? ESOS_SENSOR_MAX4
        :   proc_mode == MAX && num_samples == SAMPLES8 ? ESOS_SENSOR_MAX8
        :   proc_mode == MAX && num_samples == SAMPLES16 ? ESOS_SENSOR_MAX16
        :   proc_mode == MAX && num_samples == SAMPLES32 ? ESOS_SENSOR_MAX32
        :   proc_mode == MAX && num_samples == SAMPLES64 ? ESOS_SENSOR_MAX16

        :   proc_mode == MEDIAN && num_samples == SAMPLES2 ? ESOS_SENSOR_MEDIAN2
        :   proc_mode == MEDIAN && num_samples == SAMPLES4 ? ESOS_SENSOR_MEDIAN4
        :   proc_mode == MEDIAN && num_samples == SAMPLES8 ? ESOS_SENSOR_MEDIAN8
        :   proc_mode == MEDIAN && num_samples == SAMPLES16 ? ESOS_SENSOR_MEDIAN16
        :   proc_mode == MEDIAN && num_samples == SAMPLES32 ? ESOS_SENSOR_MEDIAN32
        :   proc_mode == MEDIAN && num_samples == SAMPLES64 ? ESOS_SENSOR_MEDIAN16
        :   0x00;

    ESOS_TASK_YIELD();
    ESOS_TASK_END();
}

ESOS_USER_TASK(loop) {
    static uint16_t u16_data;
	static ESOS_TASK_HANDLE th_child; //declare storage for handle to child task 
    ESOS_TASK_BEGIN();{
      for (;;) {     //same as while(true)

        ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Pressed() || esos_uiF14_isSW2Pressed());  //on either switch, start the DO loop
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
            ESOS_TASK_WAIT_SENSOR_READ(u16_data, sensor_processing_mode, ESOS_SENSOR_FORMAT_BITS);

            //wait for UART availability to send output to Bully Bootloader
            ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
            
            ESOS_TASK_WAIT_ON_SEND_UINT16_AS_HEX_STRING(u16_data); //extra zeros but acceptable
            
            ESOS_ALLOCATE_CHILD_TASK(th_child);
            ESOS_TASK_SPAWN_AND_WAIT(th_child, barGraph_child, u16_data);
            
            ESOS_TASK_WAIT_ON_SEND_STRING("\n");
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

            if (esos_uiF14_isSW3Pressed()){  //sw1 will stop the sw2 initiated continous looping
                ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW3Released());
                b_keepLooping = FALSE;
                ESOS_TASK_SPAWN_AND_WAIT(th_child, menu);
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
    esos_RegisterTask(loop);
}
