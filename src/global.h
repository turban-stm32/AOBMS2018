#define DEC_ACCURACY (100000) // decimal accuracy multiplier
#define CELL_RES_DIV ((q16_t)(0x999B)) // cell resistor divider (15k/(15k+10k)=0.6) in q16 format

#define ERROR 	(1)
#define OK 		(0)
#define FPWM 	(10)	//pwm frequency
#define RPWM	(10000) // pwm resolution - CAREFUL!! CLK*FPWM/RPWM must be lower than 65536
#define CLK		(16000000) //MCU base clk

#define E2_ADDR ((uint32_t)(0x08080000)) // starting address of data flash (EEPROM)
#define PEKEY1 	((uint32_t)(0x89ABCDEF))
#define PEKEY2 	((uint32_t)(0x02030405))
#define VREFINT_CAL ((uint16_t*) ((uint32_t) 0x1FF80078))
#define TEMP130_CAL ((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TEMP30_CAL  ((uint16_t*) ((uint32_t) 0x1FF8007A))

//typedef
typedef uint32_t q16_t; // defines signed fixpoint type, point precedes 16th digit (2^0)


// VOLATILE

extern uint8_t tx_buff[32];
extern uint8_t rx_buff[16];
extern uint8_t rx_chbuff;
extern uint8_t tx_chbuff;
extern uint8_t cmd_buff[16];
extern uint16_t adc_vals[5];
extern uint8_t error_flag;
extern q16_t vdda_meas;
extern q16_t vcell;
extern q16_t temp;

// CONST

extern const uint8_t nr_buff[];



// VOLATILE, BACKED UP IN EEPROM

extern uint32_t pwm1;
extern uint32_t wupe;
extern q16_t thl1;
extern q16_t thl2;
extern q16_t thh1;
extern q16_t thh2;



