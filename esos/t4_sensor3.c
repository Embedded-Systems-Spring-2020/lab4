  *Task 2
  {
  
                    static uint32_t pu32_out;
                    static uint32_t temp32_ipart;
                    static uint32_t temp32_fpart;
   
// pu16_out is still raw ADC sensor data meaning it is a number between 0 and 2^10. 
// So multiply it by the top voltage value which 3V and then divide by 1048.
                    pu32_out = (uint32_t)(3*pu16_out/1048) * 1000; // convert to not use decimals
                    pu32_out = (pu32_out - 424000) / 625; // millimillivolts to temp
                    temp32_ipart = pu32_out / 100; // convert to get the integer part
                    temp32_fpart = pu32_out - temp32_ipart * 100; // subtract out the integer part to get the decimal
                                                                  // part

            } else {
                

                // Vo = (6.25mV * T) + 424mV
                // T ~= (pu16_out - 424) / 6

                static uint32_t pu32_out;
                static uint32_t temp32_ipart;
                static uint32_t temp32_fpart;

                pu32_out = (uint32_t)(3*pu16_out/1048) * 1000; // convert to not use decimals
                pu32_out = (pu32_out - 424000) / 625; // millimillivolts to temp
                temp32_ipart = pu32_out / 100; // convert to get the integer part
                temp32_fpart = pu32_out - temp32_ipart * 100; // subtract out the integer part to get the decimal part

            }
