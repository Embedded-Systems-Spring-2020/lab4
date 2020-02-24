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
BOOL b_keepLooping;

b_keepLooping = FALSE;

ESOS_CHILD_TASK(menu) {

    static char proc_mode_buffer[8];    // buffer for holding the desired processing mode 
    static char num_samples_buffer[8];  // buffer for holding the desired number of samples

    // neither of these will be used across yield or waits, so no need for static(-ness)
    char proc_mode;
    char num_samples;

    ESOS_TASK_BEGIN();
    for (;;) {
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
        ESOS_TASK_WAIT_ON_GET_STRING(proc_mode_buffer);     // write the user's input to the aforementioned buffer
        ESOS_TASK_SIGNAL_AVAILABLE_IN_COMM();       // let everyone else know we are done with the in stream

        // do the above process, but for output (so the user can see what they typed)
        ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
        ESOS_TASK_WAIT_ON_SEND_STRING(proc_mode_buffer);
        ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

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
        ESOS_TASK_WAIT_ON_GET_STRING(num_samples_buffer);
        ESOS_TASK_SIGNAL_AVAILABLE_IN_COMM();

        ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
        ESOS_TASK_WAIT_ON_SEND_STRING(num_samples_buffer);
        ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

        // we only need the first character of each buffer, so doing this now
        // will save me a lot of typing
        proc_mode = proc_mode_buffer[0];
        num_samples = num_samples_buffer[0];

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
    }
    ESOS_TASK_END();
}

ESOS_USER_TASK(loop) {

    static uint16_t u16_data;
    ESOS_TASK_HANDLE th_child;

    ESOS_TASK_BEGIN(); {
        for (;;) {
            ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Pressed() || esos_uiF14_isSW2Pressed());

            if (esos_uiF14_isSW2Pressed()) {
                b_keepLooping = TRUE;
            }

            else {
                b_keepLooping = FALSE;
            }

            ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Released() && esos_uiF14_isSW2Released());

            do {                    
                //claim the ADC
                ESOS_TASK_WAIT_ON_AVAILABLE_SENSOR(ESOS_SENSOR_CH02, ESOS_SENSOR_VREF_3V3);

                //Now we have exclusive use, grab data
                ESOS_TASK_WAIT_SENSOR_QUICK_READ(u16_data);

                //Grab the output
                ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
                ESOS_TASK_WAIT_ON_SEND_STRING("Hex: ");
                ESOS_TASK_WAIT_ON_SEND_UINT32_AS_HEX_STRING(u16_data);
                ESOS_TASK_WAIT_ON_SEND_STRING("\n");
                ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

                ESOS_TASK_WAIT_TICKS(LOOP_DELAY / 2);

                if (esos_uiF14_isSW1Pressed()) {
                    ESOS_TASK_WAIT_UNTIL(esos_uiF14_isSW1Released());
                    b_keepLooping = FALSE;
                }

                if (esos_uiF14_isSW2Pressed()) {
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
