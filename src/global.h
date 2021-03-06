#define DEC_ACCURACY (100000) // decimal accuracy multiplier
#define CELL_RES_DIV ((q16_t)(0x1AAAA)) // cell resistor 1/divider ((15k+10k)/15k=1.66666) in q16 format
#define PACKHI_RES_DIV ((q16_t)(0x215555)) // pack hi measurement resistor 1/divider ((220k+6.8k)/6.8k=33.33333) in q16 format
#define PACKLO_RES_DIV ((q16_t)(0x215555)) // pack lo measurement resistor 1/divider ((220k+6.8k)/6.8k=33.33333) in q16 format
#define FLYBACK_CONSTANT ((q16_t)(0x33333)) // Cnstant of flybac converter (2*Lpri*160^2)/(T*1000)=3.2, where Lpri is primary inductance in uH (0.625),160 is full range of PWM, T is period of PWM in us (10), and 1000 is decadic correction of orders

#define ERROR 	(1)
#define OK 		(0)
#define FPWM 	(100000)	//pwm frequency
#define RPWM	(160) // pwm resolution - CAREFUL!! CLK/(FPWM*RPWM)-1 must be lower than 65536
#define CLK		(16000000) //MCU base clk

#define E2_ADDR ((uint32_t)(0x08080000)) // starting address of data flash (EEPROM)
#define PEKEY1 	((uint32_t)(0x89ABCDEF)) // protection key to write EEPROM
#define PEKEY2 	((uint32_t)(0x02030405)) // protection key to write EEPROM
#define VREFINT_CAL ((uint16_t*) ((uint32_t) 0x1FF80078)) // internal Vref calibration value
#define TEMP130_CAL ((uint16_t*) ((uint32_t) 0x1FF8007E)) // internal T=130degC calibration value
#define TEMP30_CAL  ((uint16_t*) ((uint32_t) 0x1FF8007A)) // internal T=30degC calibration value

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
extern q16_t vdda_meas; // MCU Vdda measured
extern q16_t vcell; // measured Vcell
extern q16_t temp; // measured temperature

extern uint32_t mode;

// CONST

extern const uint8_t nr_buff[];



// VOLATILE, BACKED UP IN EEPROM

extern uint32_t pwm1; //duty cycle
extern uint32_t wupe; // WakeUp Period (lptimer)
extern uint32_t ibal; // balancing current (in mA)
extern uint32_t tbal; // balancing time in msec

extern q16_t thl1; // low threshold 1
extern q16_t thl2; // low threshold 2
extern q16_t thh1; // high threshold 1
extern q16_t thh2; // high threshold 2
extern q16_t tmph; // temp threshold



