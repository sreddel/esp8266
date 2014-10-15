// implement a sleep function

#include "ets_sys.h"

#define US_TO_RTC_TIMER_TICKS(t)          \
    ((t) ?                                   \
     (((t) > 0x35A) ?                   \
      (((t)>>2) * ((APB_CLK_FREQ>>4)/250000) + ((t)&0x3) * ((APB_CLK_FREQ>>4)/1000000))  :    \
      (((t) *(APB_CLK_FREQ>>4)) / 1000000)) :    \
     0)


LOCAL uint8_t sleep_state = 0;

LOCAL void
sleep_tim1_intr_handler(void)
{
    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);

    sleep_state = 1;
}


void esp8266_sleep(int16_t time) 
{

	ETS_FRC_TIMER1_INTR_ATTACH(sleep_tim1_intr_handler, NULL);
    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();

    int16_t rtc_ticks = US_TO_RTC_TIMER_TICKS(time);

    sleep_state = 0;

    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, rtc_ticks);

    while (sleep_state == 0)
    {
    }

    ETS_FRC1_INTR_DISABLE();

}