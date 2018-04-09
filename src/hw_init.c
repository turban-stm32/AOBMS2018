

#include "hw_init.h"
//#include "stm32l053xx.h"


void ADC_en(void)
{
	//Enable APB clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	//ADC1->CR |=ADC_CR_ADVREGEN;


	// PCLK/4 (CKMODE =0b10)
	ADC1->CFGR2 = (ADC1->CFGR2 & ~(ADC_CFGR2_CKMODE)) | ADC_CFGR2_CKMODE_1;
	//Oversampling shift 4bit (OVSS=0b0100)
	ADC1->CFGR2 = (ADC1->CFGR2 & ~(ADC_CFGR2_OVSS)) | ADC_CFGR2_OVSS_2;
	//Oversampling ratio 256x (OVSR=0b111)
	//ADC1->CFGR2 = (ADC1->CFGR2 & ~(ADC_CFGR2_OVSR)) | ADC_CFGR2_OVSR_0 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_2;
	ADC1->CFGR2 |=ADC_CFGR2_OVSR;
	//Oversampler enable
	ADC1->CFGR2 |= ADC_CFGR2_OVSE;
	//Sampling rate max (160,5clk) (SMP=0b111)
	//ADC1->SMPR = (ADC1->SMPR & ~(ADC_SMPR_SMP)) | ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2;
	ADC1->SMPR |= ADC_SMPR_SMP;
	// channel select (PA5=ADC_IN5, PA6=ADC_IN6, PA7=ADC_IN7)
	ADC1->CHSELR = ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6 | ADC_CHSELR_CHSEL7 | ADC_CHSELR_CHSEL17 | ADC_CHSELR_CHSEL18;
	// auto off, overrun activation
	ADC1->CFGR1 |= ADC_CFGR1_AUTOFF | ADC_CFGR1_OVRMOD;
	//ADC1->CFGR1 |= ADC_CFGR1_OVRMOD;
	ADC->CCR |=ADC_CCR_VREFEN;
	ADC->CCR |=ADC_CCR_TSEN;
	// calibrate adc
	ADC1->CR |= ADC_CR_ADCAL;
	while ((ADC1->ISR & ADC_ISR_EOCAL)==0);
	ADC1->ISR |= ADC_ISR_EOCAL;
	// initialize adc (not needed - autooff activated)
//	ADC1->ISR |= ADC_ISR_ADRDY;
//	ADC1->CR |= ADC_CR_ADEN;
//	while ((ADC1->ISR & ADC_ISR_ADRDY)==0);



}

void ADC_DMA_conf(void)
{
	  /* (1) Enable the peripheral clock on DMA */
	  /* (2) Enable DMA transfer on ADC - DMACFG is kept at 0 for one shot mode */
	  /* (3) Configure the peripheral data register address */
	  /* (4) Configure the memory address */
	  /* (5) Configure the number of DMA tranfer to be performs on DMA channel 1 */
	  /* (6) Configure increment, size and interrupts */
	  /* (7) Enable DMA Channel 1 */
	  RCC->AHBENR |= RCC_AHBENR_DMA1EN; /* (1) */
	  ADC1->CFGR1 |= ADC_CFGR1_DMAEN; /* (2) */
	  DMA1_Channel1->CPAR = (uint32_t) (&(ADC1->DR)); /* (3) */
	  DMA1_Channel1->CMAR = (uint32_t)(adc_vals); /* (4) */
	  //DMA1_Channel1->CNDTR = (sizeof(adc_vals)/sizeof(uint16_t)); /* (5) */
	  DMA1_Channel1->CNDTR = 5; /* (5) */

	  DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0;
	  DMA1_Channel1->CCR |= DMA_CCR_EN; /* (7) */
}






void ADC_dis(void){

}



void LED_io_conf(void)
{
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE10)) | (GPIO_MODER_MODE10_0); // PA10 as output (green led)
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE9)) | (GPIO_MODER_MODE9_0); // PA9 as output (red led)

	//GPIOC->MODER = (GPIOC->MODER & ~(0x0000FFFF)) | (0x00005555); // PC0-PC7 as outputs (red leds)
}

void LPTIM_conf(void)
{
	RCC->CSR |=RCC_CSR_LSION; // Enable internal 37kHz RC oscillator
	while ((RCC->CSR & RCC_CSR_LSIRDY)==0); // wait until LSI is ready
	RCC->CCIPR |= RCC_CCIPR_LPTIM1SEL_0; //LSI selected for LPTIM

	RCC->APB1ENR |=RCC_APB1ENR_LPTIM1EN;

	LPTIM1->CFGR |= LPTIM_CFGR_PRESC_0 | LPTIM_CFGR_PRESC_2; // LPTIM prescaler =32
	LPTIM1->CR |=LPTIM_CR_ENABLE;
	LPTIM1->ARR = 10000; // ~ 5000ms

	//LPTIM1->CR |=LPTIM_CR_SNGSTRT;

}

void STOP_mode_conf(void)
{
	/* (1) Set SLEEPDEEP bit of Cortex System Control Register */
	/* (2) Clear PDDS bit and set LPDS bit in PWR_CR
	clear WUF in PWR_CSR by setting CWUF in PWR_CR */
	/* (3) Select HSI as clock while exiting stop mode */
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; /* (1) */
	PWR->CR = (PWR->CR & (uint32_t)(~(PWR_CR_PDDS)))
	| PWR_CR_LPSDSR | PWR_CR_CWUF; /* (2) */
	RCC->CFGR |= RCC_CFGR_STOPWUCK; /* (3) */


	LPTIM1->IER |= LPTIM_IER_ARRMIE;

	EXTI->EMR |= EXTI_EMR_EM29;

	DBGMCU->CR |=DBGMCU_CR_DBG_STOP;
	DBGMCU->CR |=DBGMCU_CR_DBG_SLEEP;
	DBGMCU->CR |=DBGMCU_CR_DBG_STANDBY;
}


void RCC_Config(void)
{


	RCC->IOPENR |= RCC_IOPENR_GPIOAEN ; // porta povolit hodiny
	//RCC->IOPENR |= RCC_IOPENR_GPIOCEN ; // portC povolit hodiny

	RCC->CR |=  RCC_CR_HSION; // enable HSI
	while ((RCC->CR & RCC_CR_HSIRDY)==0); // wait until HSI is ready


	RCC->CFGR |= RCC_CFGR_SW_HSI; // sysclk=HSI
	RCC->APB1ENR |= (RCC_APB1ENR_PWREN); // enable powercontrol
	PWR->CR = (PWR->CR & ~(PWR_CR_VOS)) | PWR_CR_VOS_0; // set voltage range
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // enable SYSCFG

}

void Butt_GPIO_Config(void)
{

	GPIOC->MODER = (GPIOC->MODER & ~(GPIO_MODER_MODE13)); // 00 on PC13 position of MODER - input
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE1)); // PA1 as input - isolator power enable
}


void EXTI_Config(void)
{
	SYSCFG->EXTICR[3] = (SYSCFG->EXTICR[3] & ~(SYSCFG_EXTICR4_EXTI13)) | (SYSCFG_EXTICR4_EXTI13_PC); //EXTI na PC13
	EXTI->IMR |= EXTI_IMR_IM13;
	EXTI->FTSR |= EXTI_FTSR_FT13;

	NVIC_SetPriority(EXTI4_15_IRQn, 0);
	NVIC_EnableIRQ(EXTI4_15_IRQn);

}

void USART2_io_conf(void)
{
	 GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0|GPIO_MODER_MODE2)) | (GPIO_MODER_MODE0_1 | GPIO_MODER_MODE2_1); // alternativni funkce na GPIOA pin 2 a 3
	 GPIOA->AFR[0]= (GPIOA->AFR[0]& ~(GPIO_AFRL_AFRL0 | GPIO_AFRL_AFRL2)) | ((0 << GPIO_AFRL_AFRL0_Pos) | (4 << GPIO_AFRL_AFRL2_Pos)); // AF1 na pin 0 a AF4 na pin 2 GPIOA
	 GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE1)); // PA1 as input - isolator power enable
}


void USART2_en(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // enable periph clock usart2
	USART2->BRR = 16000000/9600; //clk / baudrate
	USART2->CR1 = USART_CR1_TE | USART_CR1_UE | USART_CR1_RE; // usart enable, transmitter enable, receiver enable
	while((USART2->ISR & USART_ISR_TC) != USART_ISR_TC) //pocka nez odejde idle frame
	{
	    /* add time out here for a robust application */
	}
	USART2->ICR = USART_ICR_TCCF;/* clear TC flag */
	//USART2->CR1 |= USART_CR1_TCIE;/* enable TC interrupt */
	//NVIC_SetPriority(USART2_IRQn, 0); // nastaveni irq priority=0
	//NVIC_EnableIRQ(USART2_IRQn); // povoleni irq
	//GPIOA->ODR |= (1<<9); // red led on

}


void USART_DMA_char_conf(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMAEN; // enable clock


	 /* DMA1 Channel4 USART2_TX config */
	 /* (1)  Map USART2_TX DMA channel */
	 /* (2)  Peripheral address */
	 /* (3)  Memory address */
	 /* (4)  Memory increment */
	 /*      Memory to peripheral */
	 /*      8-bit transfer */

	 DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR & ~DMA_CSELR_C4S) | (4 << DMA_CSELR_C4S_Pos);
	 DMA1_Channel4->CPAR = (uint32_t)&(USART2->TDR); /* (2) */
	 DMA1_Channel4->CMAR = (uint32_t)&tx_chbuff; /* (3) */
	 DMA1_Channel4->CCR = DMA_CCR_MINC | DMA_CCR_DIR; /* (4) */


	 /* DMA1 Channel5 USART_RX config */
	  /* (5)  Map USART2_RX DMA channel */
	  /* (6)  Peripheral address */
	  /* (7)  Memory address */
	  /* (8)  Data size */
	  /* (9)  Memory increment */
	  /*      Peripheral to memory*/
	  /*      8-bit transfer */
	  /*      Transfer complete IT */
	  DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR & ~DMA_CSELR_C5S) | (4 << DMA_CSELR_C5S_Pos); /* (5) */
	  DMA1_Channel5->CPAR = (uint32_t)&(USART2->RDR); /* (6) */
	  DMA1_Channel5->CMAR = (uint32_t)&rx_chbuff; /* (7) */
	  DMA1_Channel5->CNDTR = sizeof(rx_chbuff); /* (8) */
	  DMA1_Channel5->CCR = DMA_CCR_MINC | DMA_CCR_EN; /* (9) */


}
void USART_DMA_conf(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMAEN; // enable clock


	 /* DMA1 Channel4 USART2_TX config */
	 /* (1)  Map USART2_TX DMA channel */
	 /* (2)  Peripheral address */
	 /* (3)  Memory address */
	 /* (4)  Memory increment */
	 /*      Memory to peripheral */
	 /*      8-bit transfer */

	 DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR & ~DMA_CSELR_C4S) | (4 << DMA_CSELR_C4S_Pos);
	 DMA1_Channel4->CPAR = (uint32_t)&(USART2->TDR); /* (2) */
	 DMA1_Channel4->CMAR = (uint32_t)tx_buff; /* (3) */

	 DMA1_Channel4->CCR = DMA_CCR_MINC | DMA_CCR_DIR; /* (4) */


	 /* DMA1 Channel5 USART_RX config */
	  /* (5)  Map USART2_RX DMA channel */
	  /* (6)  Peripheral address */
	  /* (7)  Memory address */
	  /* (8)  Data size */
	  /* (9)  Memory increment */
	  /*      Peripheral to memory*/
	  /*      8-bit transfer */
	  /*      Transfer complete IT */
	  DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR & ~DMA_CSELR_C5S) | (4 << DMA_CSELR_C5S_Pos); /* (5) */
	  DMA1_Channel5->CPAR = (uint32_t)&(USART2->RDR); /* (6) */
	  DMA1_Channel5->CMAR = (uint32_t)rx_buff; /* (7) */
	  DMA1_Channel5->CNDTR = sizeof(rx_buff); /* (8) */

	  DMA1_Channel5->CCR = DMA_CCR_MINC | DMA_CCR_EN; /* (9) */


}
void TIM21_config(void)
{
	/* (1) Enable the peripheral clock of Timer x */
	/* (2) Enable the peripheral clock of GPIOA */
	/* (3) Select alternate function mode on GPIOA pin 9 */
	/* (4) Select AF5 on PA9 in AFRH for TIM21_CH1 */

	RCC->APB2ENR |= RCC_APB2ENR_TIM21EN; /* (1) */
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN; /* (2) */
	GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE9)) | (GPIO_MODER_MODE9_1); /* (3) */
	GPIOA->AFR[1] |= (0x5 << GPIO_AFRH_AFRH1_Pos); /* (4) */

	/* (1) Set prescaler to 15, so APBCLK/16 i.e 1MHz */
	/* (2) Set ARR = 8, as timer clock is 1MHz the period is 9 us */
	/* (3) Set CCRx = 4, , the signal will be high during 4 us */
	/* (4) Select PWM mode 1 on OC1  (OC1M = 110),
		 enable preload register on OC1 (OC1PE = 1) */
	/* (5) Select active high polarity on OC1 (CC1P = 0, reset value),
		 enable the output on OC1 (CC1E = 1)*/
	/* (6) Enable output (MOE = 1)*/
	/* (7) Enable counter (CEN = 1)
		 select edge aligned mode (CMS = 00, reset value)
		 select direction as upcounter (DIR = 0, reset value) */
	/* (8) Force update generation (UG = 1) */


	TIM21->PSC = (CLK/(RPWM*FPWM))-1; /* (1) */
	TIM21->ARR = RPWM-1; /* (2) */
	TIM21->CCR2 = pwm1; /* (3) */
	TIM21->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE; /* (4) */
	TIM21->CCER |= TIM_CCER_CC2E; /* (5) */
	TIM21->CR1 |= TIM_CR1_CEN; /* (7) */
	TIM21->EGR |= TIM_EGR_UG; /* (8) */
}




void USART2_dmaen(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // enable periph clock usart2
	USART2->BRR = 16000000/9600; //clk / baudrate

	USART2->CR3 = USART_CR3_DMAT | USART_CR3_DMAR; //DMA rx, tx enable

	USART2->CR1 = USART_CR1_TE | USART_CR1_UE | USART_CR1_RE; // usart enable, transmitter enable, receiver enable

	while((USART2->ISR & USART_ISR_TC) != USART_ISR_TC) //pocka nez odejde idle frame
	{
		/* add time out here for a robust application */
	}
	USART2->ICR = USART_ICR_TCCF;/* clear TC flag */
	//GPIOA->ODR |= (1<<9); // red led on
}


void USART2_dis(void)
{
	USART2->CR1 &= ~(USART_CR1_TE | USART_CR1_UE | USART_CR1_RE);
	RCC->APB1ENR &= ~(RCC_APB1ENR_USART2EN);
	//GPIOA->ODR &= ~(1<<9); // red led off
}


void init_params(void)
{
	pwm1= *(uint32_t *)(E2_ADDR+0);
	thl1= *(uint32_t *)(E2_ADDR+4);
	thl2= *(uint32_t *)(E2_ADDR+8);
	thh1= *(uint32_t *)(E2_ADDR+12);
	thh2= *(uint32_t *)(E2_ADDR+16);
}

