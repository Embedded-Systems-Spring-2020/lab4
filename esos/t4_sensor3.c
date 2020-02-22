  *Task 2
  
  
  
  if (output_continuous) {
                // output every 1 second, until the output_state flag is unset
                ESOS_ALLOCATE_CHILD_TASK(read_adc);
                ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_ON_AVAILABLE_SENSOR, TEMP_CHANNEL, ESOS_SENSOR_VREF_3V0);
                do {
                    ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_SENSOR_READ, &pu16_out, output_state,
                                             ESOS_SENSOR_FORMAT_VOLTAGE);
                    static uint32_t pu32_out;
                    static uint32_t temp32_ipart;
                    static uint32_t temp32_fpart;

                    pu32_out = (uint32_t)pu16_out * 1000; // convert to not use decimals
                    pu32_out = (pu32_out - 424000) / 625; // millimillivolts to temp
                    temp32_ipart = pu32_out / 100; // convert to get the integer part
                    temp32_fpart = pu32_out - temp32_ipart * 100; // subtract out the integer part to get the decimal
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
                    if (temp32_fpart >= 0 && temp32_fpart <= 9) {
                        ESOS_TASK_WAIT_ON_SEND_UINT8('0');
                    }
                    convert_uint32_t_to_str(temp32_fpart, temp32_str, 12, 10);
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
                // ESOS_TASK_WAIT_ON_SEND_STRING("WE ARE OUTPUTTING STUFF NOW");
                ESOS_ALLOCATE_CHILD_TASK(read_adc);
                ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_ON_AVAILABLE_SENSOR, TEMP_CHANNEL, ESOS_SENSOR_VREF_3V0);
                // ESOS_TASK_WAIT_ON_SEND_STRING("_wait_sensor_availble");
                ESOS_TASK_SPAWN_AND_WAIT(read_adc, _WAIT_SENSOR_READ, &pu16_out, output_state,
                                         ESOS_SENSOR_FORMAT_VOLTAGE); // could change this to percent to get a cool
                                                                      // output
                // ESOS_TASK_WAIT_ON_SEND_STRING("_wait_sensor_read");
                ESOS_SENSOR_CLOSE();

                // Vo = (6.25mV * T) + 424mV
                // T ~= (pu16_out - 424) / 6

                static uint32_t pu32_out;
                static uint32_t temp32_ipart;
                static uint32_t temp32_fpart;

                pu32_out = (uint32_t)pu16_out * 1000; // convert to not use decimals
                pu32_out = (pu32_out - 424000) / 625; // millimillivolts to temp
                temp32_ipart = pu32_out / 100; // convert to get the integer part
                temp32_fpart = pu32_out - temp32_ipart * 100; // subtract out the integer part to get the decimal part

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
                convert_uint32_t_to_str(temp32_fpart, temp32_str, 12, 10);
                if (temp32_fpart >= 0 && temp32_fpart <= 9) {
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
