#define DEC_ACCURACY 100000 // decimal accuracy multiplier

#define ERROR 	(1)
#define OK 		(0)
#define FPWM 	(2)	//pwm frequency
#define RPWM	(1000) // pwm resolution - CAREFUL!! CLK*FPWM/RPWM must be lower than 65536
#define CLK		(16000000) //MCU base clk

#define E2_ADDR ((uint32_t)(0x08080000)) // starting address of data flash (EEPROM)
#define PEKEY1 	((uint32_t)(0x89ABCDEF))
#define PEKEY2 	((uint32_t)(0x02030405))

//typedef
typedef uint32_t q16_t; // defines signed fixpoint type, point precedes 16th digit (2^0)


// VOLATILE

extern uint8_t tx_buff[32];
extern uint8_t rx_buff[16];
extern uint8_t rx_chbuff;
extern uint8_t tx_chbuff;
extern uint8_t cmd_buff[16];
extern uint16_t adc_vals[3];
extern uint8_t error_flag;

// CONST

extern const uint8_t nr_buff[];



// VOLATILE, BACKED UP IN EEPROM

extern uint32_t pwm1;
extern q16_t thl1;
extern q16_t thl2;
extern q16_t thh1;
extern q16_t thh2;



