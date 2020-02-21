#include <p33EP512GP806.h>
#include <stdio.h>
#include <stdlib.h>
#include "pic24_all.h"
#include <string.h>
#include "pic24_ports_config.h"
#include <esos.h>
#include "esos_pic24.h"
#include "esos_comm.h"
#include "esos_f14ui.h"
#include "esos_sensor.h"

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

ESOS_USER_TASK(prettyMenu) {

    static char proc_mode_buffer[8];    // buffer for holding the desired processing mode 
    static char num_samples_buffer[8];  // buffer for holding the desired number of samples

    ESOS_TASK_BEGIN();
    for (;;) {

        // on either a press or a release of SW3 (RPG), pull up the menu
        if (esos_uiF14_isSW3Pressed() || esos_uiF14_isSW3Released()) {
            ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();     // wait until we can grab the output stream
            // print a very pretty menu
            ESOS_TASK_WAIT_ON_SEND_STRING(
                "\n\
                    +===========================+\n\
                    |  Select Processing mode   |\n\
                    |   1. one-shot             |\n\
                    |   2. average              |\n\
                    |   3. minimum              |\n\
                    |   4. maximum              |\n\
                    |   5. median               |\n\
                    +===========================+\n\n\
                    >"
            );
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
            ESOS_TASK_WAIT_ON_SEND_STRING(
                "\n\   
                    +===========================+\n\
                    |  Select Number of Samples |\n\
                    |   1. two                  |\n\
                    |   2. four                 |\n\
                    |   3. eight                |\n\
                    |   4. sixteen              |\n\
                    |   5. thirty-two           |\n\
                    |   6. sixty-four           |\n\
                    |   7. one-shot             |\n\
                    +===========================+\n\n\
                    >"
            );
            ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

            ESOS_TASK_WAIT_ON_AVAILABLE_IN_COMM();
            ESOS_TASK_WAIT_ON_GET_STRING(num_samples_buffer);
            ESOS_TASK_SIGNAL_AVAILABLE_IN_COMM();

            ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
            ESOS_TASK_WAIT_ON_SEND_STRING(num_samples_buffer);
            ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

            // we only need the first character of each buffer, so doing this now
            // will save me a lot of typing
            char proc_mode = proc_mode_buffer[0];
            char num_samples = num_samples_buffer[0];

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
        }
        ESOS_TASK_YIELD();
    }
    ESOS_TASK_END();
}

void user_init(void){
    config_esos_uiF14();
    esos_RegisterTask(prettyMenu);
}
