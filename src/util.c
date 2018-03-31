#include "util.h"
#include "global.h"
#include "string.h"

volatile uint32_t ticks;


//****************************************************************************************
// SysTick Functions
//****************************************************************************************

void Delay_ms(uint32_t t) // blocking
{
	uint32_t tickstart;
	tickstart = ticks;
	while(ticks < (t+tickstart))
	{
	}
}


uint32_t gTicks(void)
{
	return ticks;
}


//****************************************************************************************
// UART FUNCTIONS
//****************************************************************************************


void tx_chbuff_f(uint8_t ch)
{
	tx_buff[0]=ch;
	DMA1_Channel4->CCR &=~ DMA_CCR_EN; // disable dma
	DMA1_Channel4->CNDTR = 1;/* Data size */
	DMA1_Channel4->CCR |= DMA_CCR_EN; // enable dma

}
void rx_chbuff_f(void)
{
	DMA1_Channel5->CCR &=~ DMA_CCR_EN; // disable dma
	DMA1_Channel5->CNDTR = 1;/* Data size */
	DMA1_Channel5->CCR |= DMA_CCR_EN; // enable dma
}

void tx_buff_f(void)
{
	DMA1_Channel4->CCR &=~ DMA_CCR_EN; // disable dma
	DMA1_Channel4->CNDTR = sizeof(tx_buff);/* Data size */
	DMA1_Channel4->CCR |= DMA_CCR_EN; // enable dma
}
void rx_buff_f(void)
{
	DMA1_Channel5->CCR &=~ DMA_CCR_EN; // disable dma
	DMA1_Channel5->CNDTR = sizeof(rx_buff);/* Data size */
	DMA1_Channel5->CCR |= DMA_CCR_EN; // enable dma
}
//void tx_nbuff_f(uint8_t n)
//{
//	DMA1_Channel4->CCR &=~ DMA_CCR_EN; // disable dma
//	DMA1_Channel4->CNDTR = n;/* Data size */
//	DMA1_Channel4->CCR |= DMA_CCR_EN; // enable dma
//}

//void USART2_txs(uint8_t *data, uint8_t length)
//{
//	for (uint8_t chn = 0; chn < length; ++chn) {
//		USART2_putchar(data[chn]);
//		//GPIOC->ODR ^=(1<<0);
//	}
//	//GPIOC->ODR ^=(1<<6);
//}
//
//void USART2_putchar(uint8_t ch)
//{
//	USART2->TDR = ch;
//	while ((USART2->ISR & USART_ISR_TC)!=USART_ISR_TC)
//	{
//		//GPIOC->ODR ^=(1<<3);
//	}
//}

//void USART2_rxchardis(void)
//{
//	USART2->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE);
//	NVIC_DisableIRQ(USART2_IRQn); // zakaz irq
//}
//void USART2_rxcharen(void)
//{
//	USART2->CR1 |= (USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE);
//
//	NVIC_SetPriority(USART2_IRQn, 0); // nastaveni irq priority=0
//	NVIC_EnableIRQ(USART2_IRQn); // povoleni irq
//
//}

//****************************************************************************************
// ADC FUNCTIONS
//****************************************************************************************

void ADC_dmaread(void)
{
	//GPIOA->ODR |= (1<<9);
	ADC1->CR |= ADC_CR_ADSTART;
	while ((ADC1->ISR & ADC_ISR_EOS)==0);
	ADC1->ISR |= ADC_ISR_EOS;

}

q16_t getTemp(q16_t vdda, uint16_t meas)
{
	q16_t temp;
	temp=qmul(qdiv(vdda, i16toq(3)),i16toq(meas));
	temp+=qmul(qdiv(i16toq(3),i16toq(10)), i16toq(((*TEMP130_CAL)-(*TEMP30_CAL))<<4));
	temp-=i16toq((*TEMP30_CAL)<<4);
	temp =qmul(temp, qdiv(i16toq(100),i16toq((*TEMP130_CAL-*TEMP30_CAL)<<4)));
	return temp;
}

//****************************************************************************************
// NVM FUNCTIONS
//****************************************************************************************

uint8_t write_word_E2(uint32_t address, uint32_t ini)
{

	//unlock

	/* (1) Wait till no operation is on going */
	/* (2) Check if the PELOCK is unlocked */
	/* (3) Perform unlock sequence */
	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
		/* For robust implementation, add here time-out management */
	}
	if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0) /* (2) */
	{
		FLASH->PEKEYR = PEKEY1; /* (3) */
		FLASH->PEKEYR = PEKEY2;
	}

	// write

	*(uint32_t *)(address) = ini;


	//lock

	/* (1) Wait till no operation is on going */
	/* (2) Locks the NVM by setting PELOCK in PECR */
	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
	/* For robust implementation, add here time-out management */
	}
	FLASH->PECR |= FLASH_PECR_PELOCK; /* (2) */



	return OK;
}


//****************************************************************************************
// Integer, string, Q FUNCTIONS
//****************************************************************************************

uint8_t FTOS(uint8_t *s, uint8_t l, uint32_t ini)
{
	uint8_t n;
	//uint8_t zcnt=0;
	uint32_t div = DEC_ACCURACY/10;

	for( n = 0; n < l; n++ )
	{

		s[n]=(uint8_t)((ini/div) + '0');
		ini %=div;
		div /=10;
		if ((s[n]=='0') && (ini==0))
		{
			s[n]=0;

		}
	}

	return OK;
}


uint8_t ITOS(uint8_t *s, uint8_t l, uint32_t ini)
{
	uint8_t n;
	uint8_t zcnt=0;


	for( n = l; n > 0; n-- )
	{

		s[n-1]=(uint8_t)(ini%10) + '0';
		ini /=10;
		if ((s[n-1]=='0') && (ini==0))
		{
			zcnt++;

		}
		//n--;
	}
	for (n=0; n<l; n++) // remove insignificant zeroes, realign the string
	{
		if ((zcnt+n)<l) s[n]=s[(zcnt+n)];
		else s[n]=0;
	}
	if (zcnt==l) s[0]='0'; //add one zero if whole string is zero
	//strcpy(temp, s);
	return OK;
}

uint8_t CHTOI(uint8_t inch, uint8_t outi)
{
	if ((inch>0x2f)&&(inch<0x3a)) // kdyz je vlozeny znak cislo
	{
		outi= (inch-'0');
		return OK;
	}
	else
	{
		return ERROR;
		// spatne zadani
	}

}

uint8_t STOI(uint8_t *s, uint8_t l, uint32_t *outi) // least significant digit is at the end of the string array (s[l-1])
{
	if (l>9) return ERROR; // if the string is longer than 9 decimal places (999999999) return error
	uint8_t n=l;
	uint32_t out=0;
	uint32_t exp=1;
	while (n>0)
	{
		n--;

		if ((s[n]>0x2f)&&(s[n]<0x3a)) // if a character is a number in ascii
		{
			out += (s[n]-'0')*exp;
			exp *=10; // increase decimal order
		}
	}
	*outi= out;
	return OK;
}

uint8_t STO100kI(uint8_t *s, uint8_t l, uint32_t *outi)
{
	uint8_t n=l;
	uint64_t out=0;
	uint32_t exp1=1;
	uint32_t exp2=DEC_ACCURACY;
	while (n>0)
	{
		n--;
		if ((s[n]>0x2f)&&(s[n]<0x3a)) // if a character is a number in ascii
		{
			out += (s[n]-'0')*exp1;
			exp1 *=10; // increase decimal order

		}
		else if (s[n]=='.')
		{
			exp2 /=exp1;
		}
	}
	out *=exp2;
	if (out > 0xFFFFFFFF) return ERROR; // if uint32 is overflown, leave outi at its original value
	else
	{
		*outi=out;
		return OK;
	}
}

uint8_t I100kTOQ(uint32_t ini, q16_t *outq)
{

	uint32_t fraci=0;

	uint32_t div=DEC_ACCURACY;

	*outq = i16toq((uint16_t)(ini/DEC_ACCURACY)); // whole integer part
	fraci = (ini%DEC_ACCURACY); // fractional part

	for(uint8_t n=(16); n>0; n--)
	{
		div /=2;
		*outq |= (fraci/div << (n-1));
		fraci %= div;
		if (fraci==0) break;
	}

	return OK;
}

uint8_t QTOS(uint8_t *s, uint8_t l, q16_t inq)
{
	uint8_t wholes[6];
	uint32_t fraci=0;
	uint8_t fracs[6];
	uint32_t div=DEC_ACCURACY;
	memset(fracs,0,sizeof(fracs)); // clear fracs


	ITOS(wholes, sizeof(wholes), (uint32_t)qtoi16(inq)); //whole ineger part
	for(uint8_t n=16; n>0; n--)
	{
		div /=2;
		fraci=fraci + (((inq>>(n-1)) & 1)*div);
	}

	FTOS(fracs, sizeof(fracs), fraci);

	strcat((char*)s,(const char*)wholes);
	strcat((char*)s, ".");
	strcat((char*)s,(const char*)fracs);

	return OK;
}



// otestovat


q16_t i16toq(uint16_t inp)
{
	return (q16_t)(inp << 16);
}

uint16_t qtoi16(q16_t inq)
{
	return (uint16_t)(inq >> 16);
	//return 0;
}


q16_t qmul(q16_t ina, q16_t inb)
{
	return ((ina>>(16/2))*(inb>>(16/2)));
}
q16_t qdiv(q16_t ina, q16_t inb)
{

	 return ((ina/(inb>>(16/2)))<<(16/2));
}

q16_t parVal(uint8_t *inp, uint8_t len)
{



	return 0;
}

