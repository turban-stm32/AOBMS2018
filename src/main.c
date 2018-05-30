#include "main.h"
#include "string.h"

//****************************************************************************************
// INTEGER VARIABLES
//****************************************************************************************


uint8_t tx_buff[];
uint8_t rx_buff[];
uint8_t cmd_buff[];
uint8_t rx_chbuff=0;
uint8_t tx_chbuff;
const uint8_t nr_buff[]="\n\r";
uint8_t machine, get;
uint8_t param_buff[4], val_buff_str_in[6], val_buff_str_out[6], val_buff_str_100k[10];

uint32_t mode=0;
uint32_t fbal=0;

//mode0..temp threshold initialized
//mode1..tbal time initialized
//mode2..ibal current initialized
//mode3..wupe time initialized
//mode7..force balance mode



uint16_t adc_vals[5]; // adc dma buffer
uint32_t pwm1=30; // tim21 pwm
uint32_t wupe=5000; // lptim1 timeout (WakeUpPEriod)
uint32_t ibal=200; // balancing current in mA
uint32_t tbal=3000; //balancing time in msec

//****************************************************************************************
// FIXED POINT VARIABLES
//****************************************************************************************

q16_t par_q=0;
q16_t thl1=0;
q16_t thl2=0;
q16_t thh1=0;
q16_t thh2=0;
q16_t vdda_meas=0;
q16_t vcell=0;
q16_t vpack_hi=0;
q16_t vpack_lo=0;
q16_t vpack=0;
q16_t temp=0;
q16_t tmph =(99<<16); // first init tmph=99degC


//****************************************************************************************
// STEP VARIABLES
//****************************************************************************************

stp_t curr_s = autoLoop; // initial state definition
stp_t prev_s, next_s;

//****************************************************************************************
// STATE FUNCTIONS
//****************************************************************************************





void *parCmd() // Parse command state
{
	rx_buff[0]=0; // reset last entered character (enter key)
	//strcpy((char*)tx_buff,"mesidz");
	memset(tx_buff,0,sizeof(tx_buff)); // clear txbuff
	//memset(param_buff,0,sizeof(param_buff)); // clear param buff
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end
	switch (cmd_buff[4])
	{
		case '?': // get
			strncpy((char*)param_buff,(const char*)cmd_buff,4);
			prev_s=parCmd; //set previous state to identify where this jumps from
			return gVal();
			break;
		case '=': // set
			strncpy((char*)param_buff,(const char*)cmd_buff,4);
			strncpy((char*)val_buff_str_in,(const char*)(cmd_buff+5),6); // shifts pointer by 5 bytes (value starts at 5th byte)
			return sVal();
			break;
		default: // something else
			strcat((char*)tx_buff,"error\0");
			break;
	}
	memset(cmd_buff,0,sizeof(cmd_buff)); // clear cmdbuff
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end
	tx_buff_f(); // send complete txbuff
	return com;
}


void *gVal() // get value state
{
	if(prev_s==parCmd) //if this comes from parCmd
	{
		prev_s=gVal; // set this state as return state
		return meas; // perform measurement
	}

	memset(val_buff_str_in,0,sizeof(val_buff_str_in)); // clear valbuff


	if(strncmp((char*)param_buff, "vpah", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), vpack_hi);
	else if(strncmp((char*)param_buff, "vpal", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), vpack_lo);
	else if(strncmp((char*)param_buff, "vpck", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), vpack);
	else if(strncmp((char*)param_buff, "pwm1", 4)==0) ITOS(val_buff_str_in, sizeof(val_buff_str_in), pwm1);
	else if(strncmp((char*)param_buff, "ibal", 4)==0) ITOS(val_buff_str_in, sizeof(val_buff_str_in), ibal);
	else if(strncmp((char*)param_buff, "wupe", 4)==0) ITOS(val_buff_str_in, sizeof(val_buff_str_in), wupe);
	else if(strncmp((char*)param_buff, "tbal", 4)==0) ITOS(val_buff_str_in, sizeof(val_buff_str_in), tbal);
	else if(strncmp((char*)param_buff, "tmph", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), tmph);
	else if(strncmp((char*)param_buff, "thl1", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), thl1);
	else if(strncmp((char*)param_buff, "thl2", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), thl2);
	else if(strncmp((char*)param_buff, "thh1", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), thh1);
	else if(strncmp((char*)param_buff, "thh2", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), thh2);
	else if(strncmp((char*)param_buff, "vdda", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), vdda_meas);
	else if(strncmp((char*)param_buff, "temp", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), temp);
	else if(strncmp((char*)param_buff, "vcel", 4)==0) QTOS(val_buff_str_in, sizeof(val_buff_str_in), vcell);
	else if(strncmp((char*)param_buff, "mode", 4)==0) ITOS(val_buff_str_in, sizeof(val_buff_str_in), mode);
	else
	{
		strcat((char*)val_buff_str_in, "error\0");
	}

	strncat((char*)tx_buff,(const char*)param_buff,4); // has to be with length spec. param buff is not null terminated
	strcat((char*)tx_buff, "=");

	//ITOS(val_buff_str, sizeof(val_buff_str), par_i);
	strncat((char*)tx_buff, (const char*)val_buff_str_in, 6);
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end


	tx_buff_f(); // send complete txbuff
	memset(cmd_buff,0,sizeof(cmd_buff)); // clear cmdbuff
	memset(val_buff_str_in,0,sizeof(val_buff_str_in)); // clear valbuff
	return com;
}


void *sVal() // set value state
{
	strncat((char*)tx_buff, (const char*)param_buff,4);
	strcat((char*)tx_buff, "=");
	uint32_t temp_100k;
	next_s = com;


	if(strncmp((char*)param_buff, "pwm1", 4)==0)
	{
		STOI(val_buff_str_in, sizeof(val_buff_str_in), &pwm1);
		TIM21->CCR2 = pwm1;
		write_word_E2(E2_ADDR, pwm1);
		ITOS(val_buff_str_out, sizeof(val_buff_str_out), pwm1);
	}
	else if(strncmp((char*)param_buff, "thl1", 4)==0)
	{
		STO100kI(val_buff_str_in, sizeof(val_buff_str_in),&temp_100k);
		I100kTOQ(temp_100k, &thl1);
		// some action
		write_word_E2((E2_ADDR+4), (uint32_t)thl1);
		memset(val_buff_str_out,0,sizeof(val_buff_str_out)); // clear valbuff
		QTOS(val_buff_str_out, sizeof(val_buff_str_out), thl1);
		//ITOS(val_buff_str_100k, sizeof(val_buff_str_100k), temp_100k);
	}
	else if(strncmp((char*)param_buff, "thl2", 4)==0)
	{
		STO100kI(val_buff_str_in, sizeof(val_buff_str_in),&temp_100k);
		I100kTOQ(temp_100k, &thl2);
		// some action
		write_word_E2((E2_ADDR+8), (uint32_t)thl2);
		memset(val_buff_str_out,0,sizeof(val_buff_str_out)); // clear valbuff
		QTOS(val_buff_str_out, sizeof(val_buff_str_out), thl2);
	}
	else if(strncmp((char*)param_buff, "thh1", 4)==0)
	{
		STO100kI(val_buff_str_in, sizeof(val_buff_str_in),&temp_100k);
		I100kTOQ(temp_100k, &thh1);
		 //some action
		write_word_E2((E2_ADDR+12), (uint32_t)thh1);
		memset(val_buff_str_out,0,sizeof(val_buff_str_out)); // clear valbuff
		QTOS(val_buff_str_out, sizeof(val_buff_str_out), thh1);
	}
	else if(strncmp((char*)param_buff, "thh2", 4)==0)
	{
		STO100kI(val_buff_str_in, sizeof(val_buff_str_in),&temp_100k);
		I100kTOQ(temp_100k, &thh2);
		// some action
		write_word_E2((E2_ADDR+16), (uint32_t)thh2);
		memset(val_buff_str_out,0,sizeof(val_buff_str_out)); // clear valbuff
		QTOS(val_buff_str_out, sizeof(val_buff_str_out), thh2);
	}
	else if(strncmp((char*)param_buff, "tmph", 4)==0)
	{
		STO100kI(val_buff_str_in, sizeof(val_buff_str_in),&temp_100k);
		I100kTOQ(temp_100k, &tmph);
		mode |= (1<<0); //set first bit = temp threshold initialized
		write_word_E2((E2_ADDR+20), (uint32_t)mode);
		write_word_E2((E2_ADDR+24), (uint32_t)tmph);
		memset(val_buff_str_out,0,sizeof(val_buff_str_out)); // clear valbuff
		QTOS(val_buff_str_out, sizeof(val_buff_str_out), tmph);
	}
	else if(strncmp((char*)param_buff, "tbal", 4)==0)
	{
		STOI(val_buff_str_in, sizeof(val_buff_str_in), &tbal);
		mode |= (1<<1); //set second bit = balancing time initialized
		write_word_E2((E2_ADDR+20), (uint32_t)mode);
		write_word_E2((E2_ADDR+28), (uint32_t)tbal);
		ITOS(val_buff_str_out, sizeof(val_buff_str_out), tbal);
	}
	else if(strncmp((char*)param_buff, "ibal", 4)==0)
	{
		STOI(val_buff_str_in, sizeof(val_buff_str_in), &ibal);
		mode |= (1<<2); //set third bit = balancing current initialized
		write_word_E2((E2_ADDR+20), (uint32_t)mode);
		write_word_E2((E2_ADDR+32), (uint32_t)ibal);
		ITOS(val_buff_str_out, sizeof(val_buff_str_out), ibal);
	}
	else if(strncmp((char*)param_buff, "wupe", 4)==0)
	{
		STOI(val_buff_str_in, sizeof(val_buff_str_in), &wupe);
		mode |= (1<<3);
		write_word_E2((E2_ADDR+20), (uint32_t)mode);
		write_word_E2((E2_ADDR+36), (uint32_t)wupe);
		LPTIM1->ARR =wupe;
		ITOS(val_buff_str_out, sizeof(val_buff_str_out), wupe);
	}
	else if(strncmp((char*)param_buff, "fbal", 4)==0) // force balancing mode
	{
		STOI(val_buff_str_in, sizeof(val_buff_str_in), &fbal);

		if((fbal>>7)&1) next_s=balance;// seventh bit==1, goto balancing mode

		timestamp=gTicks();
		//write_word_E2((E2_ADDR+20), (uint32_t)mode);
		ITOS(val_buff_str_out, sizeof(val_buff_str_out), fbal);
	}
	else
	{
		strcat((char*)val_buff_str_out, "error\0");
	}

//	STO100kI(val_buff_str, sizeof(val_buff_str),&par_i);
//	memset(val_buff_str10k,0,sizeof(val_buff_str10k)); // clear valbuff
//	I100kTOQ(par_i, &par_q);
//	QTOS(val_buff_str10k, sizeof(val_buff_str10k), par_q);


	//strncat((char*)tx_buff, (const char*)val_buff_str_100k, 10);
	strncat((char*)tx_buff, (const char*)val_buff_str_out, 6);
	strcat((char*)tx_buff, " set");
	strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end
	tx_buff_f(); // send complete txbuff
	memset(cmd_buff,0,sizeof(cmd_buff)); // clear cmdbuff
	memset(val_buff_str_in,0,sizeof(val_buff_str_in)); // clear valbuff
	return next_s;


}


void *autoLoop() // default loop in automatic mode
{
	GPIOA->ODR &=~(1 << 10);// green led off
	if(!(GPIOA->IDR & GPIO_IDR_ID1))
	{
		USART2_dmaen();
		return com;
	}
	if ((LPTIM1->ISR & LPTIM_ISR_ARRM) != 0) /* Check ARR match */
	{
		prev_s=autoLoop;
		return meas;
	}
	//TIM21->EGR |= TIM_EGR_UG; // re-initializes counter to 0 (DIR=1 (downcounter))
	//Delay_ms(5000);
	__WFE(); // stop until LPTIM wakes the MCU up
	return autoLoop;
}

void *meas() // measure state
{

	GPIOA->ODR |= (1 << 10); //green led on

	ADC_dmaread();
	while((DMA1->ISR & DMA_ISR_TCIF1) == 0); // blocking until DMA is complete
	DMA1->IFCR |= DMA_IFCR_CTCIF1; /* Clear the flag */
	DMA1_Channel1->CCR &= (uint32_t)(~DMA_CCR_EN); /* Disable DMA Channel 1 to write in CNDTR*/
	DMA1_Channel1->CNDTR = 5; /* Reload the number of DMA tranfer to be performs on DMA channel 1 */
	DMA1_Channel1->CCR |= DMA_CCR_EN; /* Enable again DMA Channel 1 */

	LPTIM1->ICR |= LPTIM_ICR_ARRMCF; /* Clear ARR match flag */
	LPTIM1->CR |=LPTIM_CR_SNGSTRT; // restart lptim

	vdda_meas=qmul(qdiv(i16toq((*VREFINT_CAL)<<4),i16toq(adc_vals[3])),i16toq(3)); // vrefint_cal is in 12-bit resolution, needs to be multiplied by 2^4=16
	vcell=getVoltage(vdda_meas, adc_vals[0], CELL_RES_DIV);
	vpack_hi=getVoltage(vdda_meas, adc_vals[1], PACKHI_RES_DIV);
	vpack_lo=qmul(vdda_meas,PACKLO_RES_DIV)-getVoltage(vdda_meas, adc_vals[2], PACKLO_RES_DIV);
	vpack=vpack_hi+vpack_lo-vdda_meas;
	temp=getTemp(vdda_meas,adc_vals[4]);


	if(temp>tmph)
	{
		if (prev_s==autoLoop)
		{
			timestamp=gTicks();
			next_s=balance;
		}
		else if (prev_s==gVal)
		{
			memset(tx_buff,0,sizeof(tx_buff)); // clear txbuff
			strcat((char*)tx_buff, "\n\rOvertemperature! \n\r\0"); // overtemperature info
			tx_buff_f(); // send
			while ((DMA1->ISR & DMA_ISR_TCIF4) != DMA_ISR_TCIF4);// blocking until message is sent
			memset(tx_buff,0,sizeof(tx_buff)); // clear txbuff
			//strcat((char*)tx_buff, (const char*)nr_buff); // copy /n/r/ at the end
			next_s=prev_s;
		}


	}
	else next_s=prev_s; //get back where you come from (autoloop)


	return next_s; // go to previous state defined
}

void *com() // enable and control UART communication state
{
	GPIOA->ODR |=(1 << 10);// green led on

	static uint8_t n=0; // cmd buff counter

	if(!(GPIOA->IDR & GPIO_IDR_ID1))
	{
		rx_chbuff_f();

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

		return com;
	}
	else
	{
		USART2_dis();
		return autoLoop;
	}
}

void *balance() // state to perform balancing
{

	//GPIOA->ODR |=(1 << 9);// red led on
	GPIOA->ODR &=~(1 << 10);// green led off

	TIM21->CR1 |=TIM_CR1_CEN;

	next_s=balance;
	if(gTicks()>=(timestamp+tbal))
	{
		next_s=autoLoop;
		TIM21->CR1 &=~TIM_CR1_CEN;

	}

	//TIM2->CR1 |=TIM_CR1_CEN;
	//TIM2->EGR |= TIM_EGR_UG;
	//Delay_ms(5000);


	//TIM2->CR1 &=~(TIM_CR1_CEN);
	//TIM21->CR1 &=~(TIM_CR1_CEN);
	//Delay_ms(5000);
	mode &= ~(1<<7);// reset seventh bit of mode
	return next_s;
}


//****************************************************************************************
// MAIN
//****************************************************************************************

int main(void)
{

	HW_Init();
	while(1)
	{
		curr_s = (stp_t)(*curr_s)();
	}
}


//****************************************************************************************
// HW init procedure
//****************************************************************************************

void HW_Init(void)
{
	RCC_Config();
	SysTick_Config(16000);
	Butt_GPIO_Config();
	init_params(); // reads parameters from FLASH
	LED_io_conf();
	USART2_io_conf();
	USART_DMA_conf();
	ADC_en();
	ADC_DMA_conf();
	LPTIM_conf();
	STOP_mode_conf();
	TIM21_config();
}
