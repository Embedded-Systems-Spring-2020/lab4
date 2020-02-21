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



uint16_t uint16_POT = 0;
uint8_t u8_autoSampleTime = 31;
uint16_t u16_ch0PositiveMask = 2; 
	
ESOS_USER_TASK(demoADC){
	ESOS_TASK_BEGIN();
		
		CONFIG_RB2_AS_ANALOG();
		AD1CON1bits.ADON = 0;
		AD1CON1 = ADC_CLK_AUTO | ADC_AUTO_SAMPLING_OFF;
		AD1CON1bits.AD12B = 0;
		AD1CON2 = ADC_VREF_AVDD_AVSS;
		AD1CHS0 = ADC_CH0_NEG_SAMPLEA_VREFN | u16_ch0PositiveMask;
		AD1CON3 = ADC_CONV_CLK_INTERNAL_RC | (u8_autoSampleTime<<8);
		AD1CON1bits.ADON = 1;
		while(1){
			AD1CON1bits.SAMP = 1;                    //start sampling
			NOP();                                  //takes one clock to clear previous DONE flag, delay before checking.
			WAIT_UNTIL_CONVERSION_COMPLETE_ADC1();   //waits for AD1CON1bits.DONE
			uint16_POT = ADC1BUF0;
			printf("%u,\n", uint16_POT);
			DELAY_MS(500);
			AD1CON1bits.DONE = 0;
			
		}	
	ESOS_TASK_END();
}

void user_init(void){
	config_esos_uiF14();
	esos_RegisterTask(demoADC);
}

