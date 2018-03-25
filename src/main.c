#include "main.h"
#include "string.h"

// variables initialization
uint8_t tx_buff[] = "pocem";
uint8_t rx_buff[];
uint8_t cmd_buff[];
uint8_t rx_chbuff=0;
uint8_t tx_chbuff;
const uint8_t nr_buff[]="\n\r\0";
uint8_t machine, get;
uint8_t param_buff[4], val_buff_str[6], val_buff_str10k[10];
uint32_t par_i=9999;
uint16_t adc_vals[3];
uint32_t pwm1=1;
q16_t par_q=0;


// state definitions
void *init()
{
	static uint8_t n=0; // cmd buff counter


	if (((DMA1->ISR & DMA_ISR_TCIF5) == DMA_ISR_TCIF5) ) // rx is complete
	{



		if (rx_buff[0]=='\r')// kdyz tu bude enter, netisknout, skocit do parcmd
		{
			n=0; // reset counter
			DMA1->IFCR = DMA_IFCR_CTCIF5; // clear flag
			return parCmd; // goto parCmd
		}
		else if (rx_buff[0]==8 || rx_buff[0]==127)// kdyz bude backspace
		{
			DMA1->IFCR = DMA_IFCR_CTCIF5;/* Clear TC flag */
			n--; // decrement n
			tx_chbuff_f(rx_buff[0]); // send tx buff

		}

		else if (n<=(sizeof(cmd_buff)-5))// jinak kdyz je misto v bufferu
		{
			cmd_buff[n++]=rx_buff[0]; // copy entered character to cmdbuff and post increment n
			DMA1->IFCR = DMA_IFCR_CTCIF5;/* Clear TC flag */
			//ITOS(tx_buff,5,rx_buff[0]);
			tx_chbuff_f(rx_buff[0]); // send tx buff
			//tx_buff_f();
		}
		else
		{
			DMA1->IFCR = DMA_IFCR_CTCIF5;/* Clear TC flag */
			rx_chbuff_f();	// read next
		}
	}
	else if((DMA1->ISR & DMA_ISR_TCIF4) == DMA_ISR_TCIF4) // je tx dokonceno
	{
		DMA1->IFCR = DMA_IFCR_CTCIF4;/* Clear TC flag */
		rx_chbuff_f();	// read next
	}



	return init;
}
void *parCmd()
{

	rx_buff[0]=0; // reset last entered character (enter key)
	//strcpy((char*)tx_buff,"mesidz");
	memset(tx_buff,0,sizeof(tx_buff)); // clear txbuff
	//memset(param_buff,0,sizeof(param_buff)); // clear param buff
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end
	switch (cmd_buff[4])
	{
		case '?':
			strncpy((char*)param_buff,(const char*)cmd_buff,4);

			return gVal();
			break;
		case '=':
			strncpy((char*)param_buff,(const char*)cmd_buff,4);
			strncpy((char*)val_buff_str,(const char*)(cmd_buff+5),6); // shifts pointer by 5 bytes (value starts at 5th byte)
			return sVal();
			break;
		default:
			strcat((char*)tx_buff,"error\0");
			break;
	}
	memset(cmd_buff,0,sizeof(cmd_buff)); // clear cmdbuff
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end
	tx_buff_f(); // send complete txbuff
	return init;
}


void *gVal()
{

	GPIOA->ODR ^= (1<<9);

	memset(val_buff_str,0,sizeof(val_buff_str)); // clear valbuff
	//memset(tx_buff,0,sizeof(tx_buff)); // clear txbuff



	ADC_dmaread();
	GPIOA->ODR ^= (1<<9); //toggle led
	while((DMA1->ISR & DMA_ISR_TCIF1) == 0); // blocking until DMA is complete
	GPIOA->ODR ^= (1<<9); //toggle led
	DMA1->IFCR |= DMA_IFCR_CTCIF1; /* Clear the flag */

	DMA1_Channel1->CCR &= (uint32_t)(~DMA_CCR_EN); /* Disable DMA Channel 1 to write in CNDTR*/
	DMA1_Channel1->CNDTR = 3; /* Reload the number of DMA tranfer to be performs on DMA channel 1 */
	DMA1_Channel1->CCR |= DMA_CCR_EN; /* Enable again DMA Channel 1 */


	if(strncmp((char*)param_buff, "adc0", 4)==0) ITOS(val_buff_str, sizeof(val_buff_str), (uint32_t)adc_vals[0]);
	else if(strncmp((char*)param_buff, "adc1", 4)==0) ITOS(val_buff_str, sizeof(val_buff_str), (uint32_t)adc_vals[1]);
	else if(strncmp((char*)param_buff, "adc2", 4)==0) ITOS(val_buff_str, sizeof(val_buff_str), (uint32_t)adc_vals[2]);
	else
	{
		strcat((char*)val_buff_str, "error\0");
	}

	strncat((char*)tx_buff,(const char*)param_buff,4); // has to be with length spec. param buff is not null terminated
	strcat((char*)tx_buff, "=");

	//ITOS(val_buff_str, sizeof(val_buff_str), par_i);
	strcat((char*)tx_buff, (const char*)val_buff_str);
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end


	tx_buff_f(); // send complete txbuff
	memset(cmd_buff,0,sizeof(cmd_buff)); // clear cmdbuff
	memset(val_buff_str,0,sizeof(val_buff_str)); // clear valbuff
	return init;
}


void *sVal()
{
	//GPIOA->ODR ^= (1<<9);
	strncat((char*)tx_buff, (const char*)param_buff,4);
	strcat((char*)tx_buff, "=");

	//STOI(val_buff_str, sizeof(val_buff_str), &pwm1);
	STO100kI(val_buff_str, sizeof(val_buff_str),&par_i);
	memset(val_buff_str10k,0,sizeof(val_buff_str10k)); // clear valbuff
	I100kTOQ(par_i, &par_q);
	QTOS(val_buff_str10k, sizeof(val_buff_str10k), par_q);



	//TIM21->CCR2 = pwm1;

	//write_word_E2((E2_ADDR+0), pwm1);



	strncat((char*)tx_buff, (const char*)val_buff_str10k, 6);
	strcat((char*)tx_buff, " set");
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end
	tx_buff_f(); // send complete txbuff
	memset(cmd_buff,0,sizeof(cmd_buff)); // clear cmdbuff
	memset(val_buff_str,0,sizeof(val_buff_str)); // clear valbuff
	return init;


}


// main loop

int main(void)
{

	HW_Init();
	GPIOA->ODR &= ~(1<<9);
	stp_t stp = init;
	rx_chbuff_f();

	while(1)
	{
		stp = (stp_t)(*stp)();



		//Delay_ms(500);


	}

}

void HW_Init(void)
{
	RCC_Config();	// moje konfigurace hodinoveho stromu
	SysTick_Config(16000);
	LED_io_conf(); // moje konfigurace GPIO LED
	USART2_io_conf();
	USART_DMA_conf();
	USART2_dmaen();
	ADC_en();
	ADC_DMA_conf();

	//write_word_E2((E2_ADDR+0), (uint32_t)50);

	pwm1= *(uint32_t *)(E2_ADDR+0);

	TIM21_config();



}



