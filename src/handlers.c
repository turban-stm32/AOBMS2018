#include "handlers.h"
#include "util.h"
#include "global.h"

uint8_t tick_led_status=0;
uint8_t sid=S_INIT;

extern uint32_t ticks;

void SysTick_Handler(void)
{
	static uint16_t count=0;
	ticks++;  // kazde preruseni (1ms) inkrementuje
	if (count-- ==0)
	{
		GPIOA->ODR ^= (1 << 10);

		if(tick_led_status==0) count=NOERROR_BL;
		else if (tick_led_status==1) count=AWAIT_BL;
		else count=ERROR;
	}
}


void EXTI4_15_IRQHandler(void)
{
	//
	if((EXTI->PR & EXTI_PR_PR13) !=0)
	{
		EXTI->PR |= EXTI_PR_PR13;
		//GPIOC->ODR ^= (1<<1);
		sid=BUTT_PRESSED;

	}
}

void USART2_IRQHandler(void)
{
	if((USART2->ISR & USART_ISR_RXNE) == USART_ISR_RXNE) // kdyz rx buffer neni prazdny
	{
		rx_chbuff=(uint8_t)USART2->RDR;
		sid=CHAR_RCVD;
		USART2_putchar(rx_chbuff);
	}
	else
	{
		tick_led_status=2;
	}
}
