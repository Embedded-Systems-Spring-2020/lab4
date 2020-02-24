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

ESOS_USER_TASK(display_output)
{
    ESOS_TASK_BEGIN();
    ESOS_TASK_WAIT_ON_AVAILABLE_SENSOR(ESOS_SENSOR_CH03, ESOS_SENSOR_VREF_3V3);
    while(TRUE)
    {
        static uint32_t pu32_out; //CONVERSION VARIABLE FOR ADC SENSOR DATA
        static uint32_t temp32_int; //TEMPATURE CONVERSATION VARIABLE
        static uint32_t temp32_dec; //TEMPATURE FINSIHED PRODUCT VARAIBLE
   
        //Math logic
        // Vo = (6.25mV * T) + 424mV
        // T ~= (pu16_out - 424) / 6

        // pu16_out is still raw ADC sensor data meaning So multiplying it by the top voltage value which 3V and then divide by 1048.
        pu32_out = (uint32_t)(3*pu16_out/1048) * 1000; // convert to not use decimals
        pu32_out = (pu32_out - 424000) / 625; // millimillivolts to temp
        temp32_int = pu32_out / 100; // convert to get the integer part
        temp32_dec = pu32_out - temp32_int * 100; // subtract out the integer part to get the decimal
                                                                        // part
        static char buffer[12];
        ESOS_TASK_WAIT_ON_AVAILABLE_OUT_COMM();                  
        sprintf(buffer, "%d\n", temp32_dec); //Print converted ADC Sensor reading  
        ESOS_TASK_SIGNAL_AVAILABLE_OUT_COMM();
        ESOS_TASK_WAIT_TICKS(1000);
    }
    
    ESOS_SENSOR_CLOSE(); //TURN OFF ADC AND SIGNAL
    
    ESOS_TASK_YIELD();
    ESOS_TASK_END();
}

void user_init(void)
{
    config_esos_uiF14();
    esos_RegisterTask(menu);
    esos_RegisterTask(display_output);
}
