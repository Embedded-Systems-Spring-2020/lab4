//Start of Task 2
#include <p33EP512GP806.h>
#include <stdio.h>
#include <stdlib.h>
#include "pic24_all.h"
#include <string.h>
#include "pic24_ports_config.h"
#include <esos.h>
#include "esos_pic24.h"
#include <esos_task.h>
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

ESOS_CHILD_TASK(menu) {

    static char proc_mode_buffer[8];    // buffer for holding the desired processing mode 
    static char num_samples_buffer[8];  // buffer for holding the desired number of samples

    // neither of these will be used across yield or waits, so no need for static(-ness)
    char proc_mode;
    char num_samples;

    ESOS_TASK_BEGIN();
    for (;;) {                                                   // part
        ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();     // wait until we can grab the output stream
        // print a very pretty menu
        ESOS_TASK_WAIT_ON_SEND_STRING(
            "\n"
                "+===========================+\n\
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
            "\n"  
                "+===========================+\n\
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

ESOS_USER_TASK(placeholderTask) {
    ESOS_TASK_HANDLE th_child;

    ESOS_TASK_BEGIN();
    ESOS_TASK_SPAWN_AND_WAIT(th_child, menu);
    ESOS_TASK_END();
    
}

void user_init(void){
    config_esos_uiF14();
    esos_RegisterTask(placeholderTask);
}
//End of Task 2

ESOS_USER_TASK(display_output)
{
    static ESOS_TASK_HANDLE read_adc;
    ESOS_TASK_BEGIN();
    while (TRUE) 
    {
        if (output_continuous) 
            {
                // output every 1 second, until the output_state flag is unset
                ESOS_ALLOCATE_CHILD_TASK(read_adc);
                ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_ON_AVAILABLE_SENSOR, TEMP_CHANNEL, ESOS_SENSOR_VREF_3V0);
                do {
                    ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_SENSOR_READ, &pu16_out, output_state,
                                             ESOS_SENSOR_FORMAT_VOLTAGE);
                    static uint32_t pu32_out;
                    static uint32_t temp32_ipart;
                    static uint32_t temp32_dpart;
   
                    // pu16_out is still raw ADC sensor data meaning it is a number between 0 and 2^10. 
                    // So multiply it by the top voltage value which 3V and then divide by 1048.
                    pu32_out = (uint32_t)(3*pu16_out/1048) * 1000; // convert to not use decimals
                    pu32_out = (pu32_out - 424000) / 625; // millimillivolts to temp
                    temp32_ipart = pu32_out / 100; // convert to get the integer part
                    temp32_dpart = pu32_out - temp32_ipart * 100; // subtract out the integer part to get the decimal
                                                                  // part

                    static uint32_t i = 0;
                    static char temp32_str[12];

                    ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
                    ESOS_TASK_WAIT_ON_SEND_UINT32_AS_HEX_STRING(pu32_out);
                    ESOS_TASK_WAIT_ON_SEND_UINT8(' ');
                    ESOS_TASK_WAIT_ON_SEND_UINT8('=');
                    ESOS_TASK_WAIT_ON_SEND_UINT8(' ');
                    convert_uint32_t_to_str(temp32_ipart, temp32_str, 12, 10);
                    ESOS_TASK_WAIT_ON_SEND_STRING(temp32_str);
                    ESOS_TASK_WAIT_ON_SEND_UINT8('.');
                    if (temp32_dpart >= 0 && temp32_dpart <= 9) {
                        ESOS_TASK_WAIT_ON_SEND_UINT8('0');
                    }
                    convert_uint32_t_to_str(temp32_dpart, temp32_str, 12, 10);
                    ESOS_TASK_WAIT_ON_SEND_STRING(temp32_str);
                    ESOS_TASK_WAIT_ON_SEND_UINT8(' ');
                    ESOS_TASK_WAIT_ON_SEND_UINT8('C');
                    ESOS_TASK_WAIT_ON_SEND_UINT8('\n');
                    ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();
                    ESOS_TASK_WAIT_TICKS(1000);
                } while (output_continuous == TRUE);
                ESOS_SENSOR_CLOSE();
            } else {
                // output once, then go to yield
                ESOS_ALLOCATE_CHILD_TASK(read_adc);
                ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_ON_AVAILABLE_SENSOR, TEMP_CHANNEL, ESOS_SENSOR_VREF_3V0);
                ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_SENSOR_READ, &pu16_out, output_state,
                                         ESOS_SENSOR_FORMAT_VOLTAGE); // could change this to percent to get a cool
                                                                      // output
                ESOS_SENSOR_CLOSE();

                // Vo = (6.25mV * T) + 424mV
                // T ~= (pu16_out - 424) / 6

                static uint32_t pu32_out;
                static uint32_t temp32_ipart;
                static uint32_t temp32_dpart;

                pu32_out = (uint32_t)(3*pu16_out/1048) * 1000; // convert to not use decimals
                pu32_out = (pu32_out - 424000) / 625; // millimillivolts to temp
                temp32_ipart = pu32_out / 100; // convert to get the integer part
                temp32_dpart = pu32_out - temp32_ipart * 100; // subtract out the integer part to get the decimal part

                static uint32_t i = 0;
                static char temp32_str[12];

                ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
                ESOS_TASK_WAIT_ON_SEND_UINT32_AS_HEX_STRING(pu32_out);
                ESOS_TASK_WAIT_ON_SEND_UINT8(' ');
                ESOS_TASK_WAIT_ON_SEND_UINT8('=');
                ESOS_TASK_WAIT_ON_SEND_UINT8(' ');
                convert_uint32_t_to_str(temp32_ipart, temp32_str, 12, 10);
                ESOS_TASK_WAIT_ON_SEND_STRING(temp32_str);
                ESOS_TASK_WAIT_ON_SEND_UINT8('.');
                convert_uint32_t_to_str(temp32_dpart, temp32_str, 12, 10);
                if (temp32_dpart >= 0 && temp32_dpart <= 9) {
                    ESOS_TASK_WAIT_ON_SEND_UINT8('0');
                }
                ESOS_TASK_WAIT_ON_SEND_STRING(temp32_str);
                ESOS_TASK_WAIT_ON_SEND_UINT8(' ');
                ESOS_TASK_WAIT_ON_SEND_UINT8('C');
                ESOS_TASK_WAIT_ON_SEND_UINT8('\n');
                ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();
                output_enabled = FALSE;
            }
        }

        ESOS_TASK_YIELD();
    }
    ESOS_TASK_END();
}
ESOS_USER_TASK(adc_interface)
{
    ESOS_TASK_BEGIN();
    while (TRUE) {
        if (sw1_state != esos_uiF14_isSW1Pressed()) {
            sw1_state = esos_uiF14_isSW1Pressed();
            if (sw1_state) {
                if (output_continuous == TRUE) {
                    output_continuous = FALSE; // if continuous is running, stop it.
                    output_enabled = FALSE;
                } else {
                    output_state = ESOS_SENSOR_ONE_SHOT; // otherwise print a single value out.
                    output_enabled = TRUE;
                }
            }
        }
        if (sw2_state != esos_uiF14_isSW2Pressed()) {
            sw2_state = esos_uiF14_isSW2Pressed();
            if (sw2_state) {
                if (output_continuous == TRUE) {
                    output_continuous = FALSE; // if continuous is running, stop it.
                    output_enabled = FALSE;
                } else {
                    output_state = ESOS_SENSOR_ONE_SHOT; // otherwise print a single value out.
                    output_enabled = TRUE;
                    output_continuous = TRUE;
                }
            }
        }
        if (sw3_state != esos_uiF14_isSW3Pressed()) {
            sw3_state = esos_uiF14_isSW3Pressed();
            if (sw3_state) {
                if (output_continuous == TRUE) {
                    output_continuous = FALSE; // if continuous is running, stop it.
                    output_enabled = FALSE;
                } else {
                    // Print menu options
                    ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
                    ESOS_TASK_WAIT_ON_SEND_STRING(str_m_PROCESS_SETTINGS);
                    ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_PROCESS_ONE);
                    ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_PROCESS_AVG);
                    ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_PROCESS_MIN);
                    ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_PROCESS_MAX);
                    ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_PROCESS_MED);
                    ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

                    // Wait for selection
                    ESOS_TASK_WAIT_ON_AVAILABLE_IN_COMM();
                    ESOS_TASK_WAIT_ON_GET_UINT8(u8_proctype);
                    ESOS_TASK_SIGNAL_AVAILABLE_IN_COMM();

                    if (u8_proctype != '1') {
                        ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();
                        ESOS_TASK_WAIT_ON_SEND_STRING(str_m_SAMPLE_SETTINGS);
                        ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_SAMPLE_2);
                        ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_SAMPLE_4);
                        ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_SAMPLE_8);
                        ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_SAMPLE_16);
                        ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_SAMPLE_32);
                        ESOS_TASK_WAIT_ON_SEND_STRING(str_madc_SAMPLE_64);
                        ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();

                        // Wait for selection
                        ESOS_TASK_WAIT_ON_AVAILABLE_IN_COMM();
                        ESOS_TASK_WAIT_ON_GET_UINT8(u8_procnum);
                        ESOS_TASK_SIGNAL_AVAILABLE_IN_COMM();
                    }

                    // Set processing state based on selection
                    output_state =
                        (u8_proctype == '2' && u8_procnum == '1') ?
                            ESOS_SENSOR_AVG2 :
                            (u8_proctype == '2' && u8_procnum == '2') ?
                            ESOS_SENSOR_AVG4 :
                            (u8_proctype == '2' && u8_procnum == '3') ?
                            ESOS_SENSOR_AVG8 :
                            (u8_proctype == '2' && u8_procnum == '4') ?
                            ESOS_SENSOR_AVG16 :
                            (u8_proctype == '2' && u8_procnum == '5') ?
                            ESOS_SENSOR_AVG32 :
                            (u8_proctype == '2' && u8_procnum == '6') ?
                            ESOS_SENSOR_AVG64 :
                            (u8_proctype == '3' && u8_procnum == '1') ?
                            ESOS_SENSOR_AVG2 :
                            (u8_proctype == '3' && u8_procnum == '2') ?
                            ESOS_SENSOR_AVG4 :
                            (u8_proctype == '3' && u8_procnum == '3') ?
                            ESOS_SENSOR_AVG8 :
                            (u8_proctype == '3' && u8_procnum == '4') ?
                            ESOS_SENSOR_AVG16 :
                            (u8_proctype == '3' && u8_procnum == '5') ?
                            ESOS_SENSOR_AVG32 :
                            (u8_proctype == '3' && u8_procnum == '6') ?
                            ESOS_SENSOR_AVG64 :
                            (u8_proctype == '4' && u8_procnum == '1') ?
                            ESOS_SENSOR_AVG2 :
                            (u8_proctype == '4' && u8_procnum == '2') ?
                            ESOS_SENSOR_AVG4 :
                            (u8_proctype == '4' && u8_procnum == '3') ?
                            ESOS_SENSOR_AVG8 :
                            (u8_proctype == '4' && u8_procnum == '4') ?
                            ESOS_SENSOR_AVG16 :
                            (u8_proctype == '4' && u8_procnum == '5') ?
                            ESOS_SENSOR_AVG32 :
                            (u8_proctype == '4' && u8_procnum == '6') ?
                            ESOS_SENSOR_AVG64 :
                            (u8_proctype == '5' && u8_procnum == '1') ?
                            ESOS_SENSOR_AVG2 :
                            (u8_proctype == '5' && u8_procnum == '2') ?
                            ESOS_SENSOR_AVG4 :
                            (u8_proctype == '5' && u8_procnum == '3') ?
                            ESOS_SENSOR_AVG8 :
                            (u8_proctype == '5' && u8_procnum == '4') ?
                            ESOS_SENSOR_AVG16 :
                            (u8_proctype == '5' && u8_procnum == '5') ?
                            ESOS_SENSOR_AVG32 :
                            (u8_proctype == '5' && u8_procnum == '6') ?
                            ESOS_SENSOR_AVG64 :
                            (u8_proctype == '6' && u8_procnum == '1') ?
                            ESOS_SENSOR_AVG2 :
                            (u8_proctype == '6' && u8_procnum == '2') ?
                            ESOS_SENSOR_AVG4 :
                            (u8_proctype == '6' && u8_procnum == '3') ?
                            ESOS_SENSOR_AVG8 :
                            (u8_proctype == '6' && u8_procnum == '4') ?
                            ESOS_SENSOR_AVG16 :
                            (u8_proctype == '6' && u8_procnum == '5') ?
                            ESOS_SENSOR_AVG32 :
                            (u8_proctype == '6' && u8_procnum == '6') ? ESOS_SENSOR_AVG64 : ESOS_SENSOR_ONE_SHOT;

                    output_enabled = TRUE;
                } // end if continuous
            } // end if sw3 true
        } // end if sw3pressed

        ESOS_TASK_YIELD();
    } // endwhile
    ESOS_TASK_END();
}

void user_init()
{
    u8_procnum = '1';
    u8_proctype = '1'; // doesn't matter
    output_enabled = FALSE;
    output_continuous = FALSE;
    output_state = ESOS_SENSOR_ONE_SHOT;
    sw1_state = FALSE;
    sw2_state = FALSE;
    sw3_state = FALSE;
    esos_RegisterTimer(heartbeat, 250);

    config_esos_uiF14();
    esos_RegisterTask(adc_interface);
    esos_RegisterTask(placeholderTask);
    esos_RegisterTask(display_output);
}
