
#include "esp8266_sleep.h"
#include "driver/onewire.h"

#include "gpio.h"
#include "ets_sys.h"

LOCAL uint8_t onewire_pin;

void onewire_init (uint8_t pin)
	{


	onewire_pin = pin;
	
	// set the selected gpio pin to output and pull it high
	gpio_init();

	GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),1);

	esp8266_sleep(50*1000);

	
	#if ONEWIRE_SEARCH
	onewire_reset_search();
	#endif


	}

uint8 onewire_reset(void) 
	{

		uint8 r;
		uint8 retries = 125;

		GPIO_DIS_OUTPUT(onewire_pin);

		while (!GPIO_INPUT_GET(onewire_pin))
		{
			if (--retries==0)
			{
				return 0;
			}
			esp8266_sleep(2);
		}

		ETS_UART_INTR_DISABLE();
		ETS_FRC1_INTR_DISABLE();
		ETS_GPIO_INTR_DISABLE();

		GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),0);
		esp8266_sleep(480);

		GPIO_DIS_OUTPUT(onewire_pin);
		esp8266_sleep(70);
		r = !GPIO_INPUT_GET(onewire_pin);

		ETS_UART_INTR_ENABLE();
		ETS_FRC1_INTR_ENABLE();
		ETS_GPIO_INTR_ENABLE();

		esp8266_sleep(410);

		GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),1);

		return r;

	}

void onewire_writebit(uint8 v)
{
	if (v & 1)
	{
		ETS_UART_INTR_DISABLE();
		ETS_FRC1_INTR_DISABLE();
		ETS_GPIO_INTR_DISABLE();
		GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),0);
		esp8266_sleep(10);
		GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),1);
		ETS_UART_INTR_ENABLE();
		ETS_FRC1_INTR_ENABLE();
		ETS_GPIO_INTR_ENABLE();
		esp8266_sleep(55);
	}
	else
	{
		ETS_UART_INTR_DISABLE();
		ETS_FRC1_INTR_DISABLE();
		ETS_GPIO_INTR_DISABLE();
		GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),0);
		esp8266_sleep(65);
		GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),1);
		ETS_UART_INTR_ENABLE();
		ETS_FRC1_INTR_ENABLE();
		ETS_GPIO_INTR_ENABLE();
		esp8266_sleep(5);
	}
}

uint8 onewire_readbit(void)
{
	uint8 r;

	ETS_UART_INTR_DISABLE();
	ETS_FRC1_INTR_DISABLE();
	ETS_GPIO_INTR_DISABLE();
	
	GPIO_OUTPUT_SET(GPIO_PIN_ADDR(onewire_pin),0);
	esp8266_sleep(3);
	GPIO_DIS_OUTPUT(onewire_pin);
	esp8266_sleep(10);
	r = GPIO_INPUT_GET(onewire_pin);

	ETS_UART_INTR_ENABLE();
	ETS_FRC1_INTR_ENABLE();
	ETS_GPIO_INTR_ENABLE();

	esp8266_sleep(53);

	return r;

}