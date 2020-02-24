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

ESOS_CHILD_TASK(barGraph_child, uint8_t u8_num2graph){  //visual display of data
	
	ESOS_TASK_BEGIN();
	 //wait for UART availability to send output to Bully Bootloader
    ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
    ESOS_TASK_WAIT_ON_SEND_UINT8_AS_HEX_STRING(u8_num2graph); 
	ESOS_TASK_WAIT_ON_SEND_STRING(" C\n");
	ESOS_TASK_WAIT_ON_SEND_STRING(buffer); //wait for data in buffer to be sent and release UART
    ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();
	ESOS_TASK_END();
}

ESOS_CHILD_TASK(menu) {
    static uint8_t proc_mode = 0x00;
    static uint8_t num_samples = 0x00;

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
  	ESOS_TASK_END();
}

ESOS_USER_TASK(loop) {
	static uint8_t u8_temp_data;
    static uint16_t u16_data;
	static uint64_t u64_temp_data;
	static ESOS_TASK_HANDLE th_child_bar_graph; //declare storage for handle to child task
    static ESOS_TASK_HANDLE th_child_menu;
    ESOS_TASK_BEGIN();
    for (;;) {     //same as while(true)

        ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Pressed() || esos_uiF14_isSW2Pressed() || \
							esos_uiF14_isSW3Pressed());  //on any switch, start the sample DO loop
        if (esos_uiF14_isSW2Pressed()){
            b_keepLooping = TRUE;   //if sw2 then keep looping; checked at the bottom while statement
        }
        else {b_keepLooping = FALSE;   //if sw1 don't keep looping just do once
        }
		sensor_processing_mode = ESOS_SENSOR_ONE_SHOT; //for SW1 and SW2 runs
		if (esos_uiF14_isSW3Pressed()){
			start_menu:  //this is used lower in the loop if SW3 stops a SW2 continuous sample
            ESOS_ALLOCATE_CHILD_TASK(th_child_menu);
            ESOS_TASK_SPAWN_AND_WAIT(th_child_menu, menu);
        }
        ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Released() && esos_uiF14_isSW2Released());  /*wait for the release so
        sw1 can exit the loop when pressed again. The loop is fast enough to go around and sense sw1 still pressed
        and begin again*/
        do{         //claim the ADC, setup for CH2(potentiometer) and 3V3 for upper Vref
            ESOS_TASK_WAIT_ON_AVAILABLE_SENSOR(ESOS_SENSOR_CH03, ESOS_SENSOR_VREF_3V0);

            //do single read of ADC
            ESOS_TASK_WAIT_SENSOR_READ(u16_data, sensor_processing_mode, ESOS_SENSOR_FORMAT_BITS);
            //divides ADC return by 2^12 max value and uses equation from datasheet but x100 to avoid decimals
			u64_temp_data = (uint64_t)(((300000 * u16_data/4096) - 42400) / 625);
			u8_temp_data = (uint8_t)u64_temp_data;		//forces format smaller for readability	
			
			//ouput the data to the screen
            ESOS_ALLOCATE_CHILD_TASK(th_child_bar_graph);
            ESOS_TASK_SPAWN_AND_WAIT(th_child_bar_graph, barGraph_child, u8_temp_data);
                        
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
            if (esos_uiF14_isSW3Pressed()){  //sw3 will also stop the looping
                ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW3Released());
                b_keepLooping = FALSE;
				goto start_menu;
                
            }
			ESOS_TASK_WAIT_TICKS(LOOP_DELAY /2);  //the other half of the loop sw2 sensor reading delay
        }while(b_keepLooping); //keep doing the DO loop if sw2 initiated it
        
        //Release the potentiometer
        ESOS_SENSOR_CLOSE();        
    }  
    ESOS_TASK_END();
}

void user_init(void){
	//esos_uif14_flashLED3(250);
    config_esos_uiF14();
    esos_RegisterTask(loop);
}
