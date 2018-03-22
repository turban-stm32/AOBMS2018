#include "stm32l011xx.h"
#include "global.h"


void LED_io_conf(void);
void RCC_Config(void);
void Butt_GPIO_Config(void);
void EXTI_Config(void);

void USART2_io_conf(void);
void USART2_en(void);
void USART2_dmaen(void);
void ADC_en(void);
void USART2_dis(void);
void ADC_dis(void);
void USART_DMA_char_conf(void);
void USART_DMA_conf(void);
void ADC_DMA_conf(void);
void TIM21_config(void);
