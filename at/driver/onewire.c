
#include "esp8266_sleep.h"
#include "driver/onewire.h"

#include "gpio.h"

LOCAL uint8_t onewire_pin;

void onewire_init (uint8_t pin) { 


	onewire_pin = pin;
	
	// set the selected gpio pin to output and pull it high
	gpio_init();

	GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),1);

	esp8266_sleep(50*1000);

	
	#if ONEWIRE_SEARCH
	onewire_reset_search();
	#endif


	}