
#include "esp8266_sleep.h"
#include "driver/onewire.h"

#include "gpio.h"
#include "ets_sys.h"

LOCAL uint8 onewire_pin;    
LOCAL unsigned char ROM_NO[8];
LOCAL uint8 LastDiscrepancy;
LOCAL uint8 LastFamilyDiscrepancy;
LOCAL uint8 LastDeviceFlag;

void onewire_init (uint8_t pin)
	{


	onewire_pin = pin;
	
	// set the selected gpio pin to output and pull it high
	gpio_init();

	GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),1);

	esp8266_sleep(500);

	
	#if ONEWIRE_SEARCH
	onewire_resetsearch();
	#endif


	}

uint8 onewire_reset(void) 
	{

		uint8 r;
		uint8 retries = 125;

		GPIO_DIS_OUTPUT(GPIO_ID_PIN(onewire_pin));

		while (!GPIO_INPUT_GET(GPIO_ID_PIN(onewire_pin)))
		{
			if (--retries==0)
			{
				return 0;
			}
			esp8266_sleep(2);
		}

		ETS_UART_INTR_DISABLE();
		ETS_FRC1_INTR_DISABLE();

		GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),0);
		esp8266_sleep(480);

		GPIO_DIS_OUTPUT(GPIO_ID_PIN(onewire_pin));
		esp8266_sleep(40);
		r = !GPIO_INPUT_GET(GPIO_ID_PIN(onewire_pin));

		ETS_UART_INTR_ENABLE();
		ETS_FRC1_INTR_ENABLE();

		esp8266_sleep(410);

		GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),1);

		return r;

	}

void onewire_writebit(uint8 v)
{
	if (v & 1)
	{
		ETS_UART_INTR_DISABLE();
		ETS_FRC1_INTR_DISABLE();
		GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),0);
		esp8266_sleep(10);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),1);
		ETS_UART_INTR_ENABLE();
		ETS_FRC1_INTR_ENABLE();
		esp8266_sleep(55);
	}
	else
	{
		ETS_UART_INTR_DISABLE();
		ETS_FRC1_INTR_DISABLE();
		GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),0);
		esp8266_sleep(65);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),1);
		ETS_UART_INTR_ENABLE();
		ETS_FRC1_INTR_ENABLE();
		esp8266_sleep(5);
	}
}

uint8 onewire_readbit(void)
{
	uint8 r;

	ETS_UART_INTR_DISABLE();
	ETS_FRC1_INTR_DISABLE();
	
	GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),0);
	esp8266_sleep(3);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(onewire_pin));
	esp8266_sleep(10);
	r = GPIO_INPUT_GET(GPIO_ID_PIN(onewire_pin));

	ETS_UART_INTR_ENABLE();
	ETS_FRC1_INTR_ENABLE();

	esp8266_sleep(53);

	GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),1);

	return r;

}

void onewire_write(uint8 v, bool power)
{
	uint8 bitMask;

	for (bitMask = 0x01; bitMask; bitMask <<= 1) 
	{
		onewire_writebit( (bitMask & v)?1:0);
    }
    if (!power)
    {
    	GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),0);
    }
}

void onewire_writebytes(uint16 *str, uint8 length, bool power)
{
	uint16 i;

	for (i = 0 ; i < length ; i++)
	{
		onewire_write(str[i],1);
	}
    
  if (!power)
  {
  	GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),0);	
  }

}

uint8 onewire_read(void)
{
	uint8 bitMask;
    uint8 r = 0;

      for (bitMask = 0x01; bitMask; bitMask <<= 1)
      {
      	if ( onewire_readbit())
      	{
      		r |= bitMask;
      	}
      		
      }

    return r;
}

void onewire_readbytes(uint16 *str, uint8 length)
{
	uint8 i;
	for (i = 0 ; i < length ; i++)
	{
		str[i] = onewire_read();
	}
   

}

void onewire_select(uint16 rom[8])
{
	uint8 i;

    onewire_write(0x55,1);           // Choose ROM

    for (i = 0; i < 8; i++) 
    {
    	onewire_write(rom[i],1);
    }
}

void onewire_skip(void)
{
	onewire_write(0xCC,1);           // Skip ROM
}

void onewire_depower(void)
{
	GPIO_OUTPUT_SET(GPIO_ID_PIN(onewire_pin),0);
}

void onewire_resetsearch(void)
{
	uint8 i;

	// reset the search state
	LastDiscrepancy = 0;
	LastDeviceFlag = FALSE;
	LastFamilyDiscrepancy = 0;
	for(i = 7; i > 0 ; i--) 
	{
    	ROM_NO[i] = 0;
  	}
}

void onewire_targetsearch(uint8 family_code)
{

	uint8 i;

	ROM_NO[0] = family_code;
   	for (i = 1; i < 8; i++)
      	ROM_NO[i] = 0;
   	LastDiscrepancy = 64;
   	LastFamilyDiscrepancy = 0;
   	LastDeviceFlag = FALSE;
}

uint8 onewire_search(uint16 *newAddr)
{

	uint8 id_bit_number;
	uint8 last_zero, rom_byte_number, search_result;
	uint8 id_bit, cmp_id_bit, i;

	unsigned char rom_byte_mask, search_direction;

   // initialize for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;

	// if the last call was not the last one
	if (!LastDeviceFlag)
	{
      // 1-Wire reset
    	if (onewire_reset())
    	{
        	// reset the search
        	LastDiscrepancy = 0;
        	LastDeviceFlag = FALSE;
        	LastFamilyDiscrepancy = 0;
        	return FALSE;
    	}

    	// issue the search command
    	onewire_write(0xF0,1);

    	// loop to do the search
     	do
    	{
         	// read a bit and its complement
        	id_bit = onewire_readbit();
        	cmp_id_bit = onewire_readbit();

        	// check for no devices on 1-wire
        	if ((id_bit == 1) && (cmp_id_bit == 1))
            	break;
        	else
        	{
            	// all devices coupled have 0 or 1
            	if (id_bit != cmp_id_bit)
            		search_direction = id_bit;  // bit write value for search
            	else
            	{
            		// if this discrepancy if before the Last Discrepancy
            		// on a previous next then pick the same as last time
            		if (id_bit_number < LastDiscrepancy)
            			search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
             		else
             			// if equal to last pick 1, if not then pick 0
                		search_direction = (id_bit_number == LastDiscrepancy);

            	// if 0 was picked then record its position in LastZero
             	if (search_direction == 0)
             	{
                	last_zero = id_bit_number;

                	// check for Last discrepancy in family
                	if (last_zero < 9)
                    	LastFamilyDiscrepancy = last_zero;
             		}
            	}

            	// set or clear the bit in the ROM byte rom_byte_number
            	// with mask rom_byte_mask
            	if (search_direction == 1)
              		ROM_NO[rom_byte_number] |= rom_byte_mask;
            	else
              		ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            	// serial number search direction write bit
            	onewire_writebit(search_direction);

            	// increment the byte counter id_bit_number
            	// and shift the mask rom_byte_mask
            	id_bit_number++;
            	rom_byte_mask <<= 1;

            	// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            	if (rom_byte_mask == 0)
            	{
                	rom_byte_number++;
                	rom_byte_mask = 1;
            	}
        	}	
    	}
    	while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

    	// if the search was successful then
    	if (!(id_bit_number < 65))
    	{
        	// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
        	LastDiscrepancy = last_zero;

        	// check for last device
        	if (LastDiscrepancy == 0)
        		LastDeviceFlag = TRUE;

        	search_result = TRUE;
    	}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !ROM_NO[0])
	{
     	LastDiscrepancy = 0;
    	LastDeviceFlag = FALSE;
    	LastFamilyDiscrepancy = 0;
    	search_result = FALSE;
   	}
	for (i = 0; i < 8; i++) newAddr[i] = ROM_NO[i];
	return search_result;
}
