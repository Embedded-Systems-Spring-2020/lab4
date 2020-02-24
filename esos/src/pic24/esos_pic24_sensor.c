/*
 * "Copyright (c) 2008 Robert B. Reese, Bryan A. Jones, J. W. Bruce ("AUTHORS")"
 * All rights reserved.
 * (R. Reese, reese_AT_ece.msstate.edu, Mississippi State University)
 * (B. A. Jones, bjones_AT_ece.msstate.edu, Mississippi State University)
 * (J. W. Bruce, jwbruce_AT_ece.msstate.edu, Mississippi State University)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice, the following
 * two paragraphs and the authors appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE "AUTHORS" BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE "AUTHORS"
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE "AUTHORS" SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE "AUTHORS" HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Please maintain this header in its entirety when copying/modifying
 * these files.
 *
 *
 */

/**
 * \addtogroup ESOS_Task_Sensor_Service
 * @{
 */
 
/*** I N C L U D E S *************************************************/
#include "esos_pic24_sensor.h"

/*** T H E   C O D E *************************************************/

/*********************************************************
 * Public functions intended to be called by other files *
 *********************************************************/

/** \file
 *  Sensor support functions. \see pic24_sensor.h for details.
 */

/**
Configure and enable the sensor module for pic24 hardware.
\param e_senCh   specifies sensor channel
\param e_senVRef specifies sensor voltage reference
\hideinitializer
 */

 
void esos_sensor_config_hw (esos_sensor_ch_t e_senCh, esos_sensor_vref_t e_senVRef){
	if (esos_IsUserFlagClear(__ESOS_SYS_ADC_IS_BUSY)){
		esos_SetUserFlag(__ESOS_SYS_ADC_IS_BUSY);
		AD1CON1bits.ADON = 0; //disables ADC while progranning
		AD1CON1 = ADC_CLK_AUTO | ADC_AUTO_SAMPLING_OFF;
		AD1CON1bits.AD12B = 1;  // sets ADC for 10 bit sampling
		if (e_senCh == ESOS_SENSOR_CH02){
			CONFIG_RB2_AS_ANALOG(); //for potentiometer analog input
			
		}else if (e_senCh == ESOS_SENSOR_CH03){
			CONFIG_RB0_AS_ANALOG();  //for 3V input on Vref+
			CONFIG_RB3_AS_ANALOG(); //for thermometer input
			AD1CON2 = ADC_VREF_EXT_AVSS; //uses Vref+ on pin 0 instead of VDD
		}
		if (e_senVRef == ESOS_SENSOR_VREF_3V3){
			AD1CON2 = ADC_VREF_AVDD_AVSS; //uses VDD and VSS
		}else {
			AD1CON2 = ADC_VREF_EXT_AVSS; //uses Vref+ on pin 0 instead of VDD
		}
		AD1CHS0 = ADC_CH0_NEG_SAMPLEA_VREFN | e_senCh;
		// Use internal clock, set sample time to 31 (bitmask logic)
		AD1CON3 = ADC_CONV_CLK_INTERNAL_RC | ADC_SAMPLE_TIME_31;
		AD1CON1bits.ADON = 1;
	}
}

/**
Determine truth of: the sensor is currently converting
\hideinitializer
 */
BOOL esos_sensor_is_converting_hw (void){
	return(!AD1CON1bits.DONE);
}

/**
Initiate a conversion for a configured sensor
\hideinitializer
 */
void esos_sensor_initiate_conversion_hw (void){
	SET_SAMP_BIT_ADC1();		// applies a bitmask to AD1CON1 to start sampling
}

/**
Receive the value from a conversion that has already been initiated
\hideinitializer
 */
uint16_t esos_sensor_getvalue_u16_hw (void){
	uint16_t u16_sensor_value;
	u16_sensor_value = ADC1BUF0;		// get value from ADC out of buffer
	AD1CON1bits.DONE = 0;				// signal that we are done
	return u16_sensor_value;
}

/**
Release any pending conversions for the sensor
\hideinitializer
 */
void esos_sensor_release_hw (void){
	esos_ClearUserFlag(__ESOS_SYS_ADC_IS_BUSY);

	AD1CON1bits.ADON = 0; // turn off sensor
}
