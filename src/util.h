#include "stm32l011xx.h"

void Delay_ms(uint32_t t);
void USART2_putchar(uint8_t ch);
void USART2_txs(uint8_t *data, uint8_t length);
uint8_t ITOS(uint8_t *s, uint8_t l, uint32_t ini);
uint8_t STOI(uint8_t *s, uint8_t l, uint32_t *outi);
uint8_t CHTOI(uint8_t inch, uint8_t outi);
uint8_t STO100kI(uint8_t *s, uint8_t l, uint32_t *outi);
void USART2_rxcharen(void);
uint32_t gTicks(void);
void USART2_rxchardis(void);
void ADC_dmaread(void);

void tx_chbuff_f(uint8_t ch);
void rx_chbuff_f(void);
void tx_buff_f(void);
void rx_buff_f(void);
void tx_nbuff_f(uint8_t n);
