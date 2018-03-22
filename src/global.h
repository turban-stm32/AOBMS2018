#define DEC_ACCURACY 5

#define ERROR 	(1)
#define OK 		(0)
#define FPWM 	(2)
#define RPWM	(1000) // CLK*FPWM/RPWM must be lower than 65536
#define CLK		(16000000)






extern uint8_t tx_buff[32];
extern uint8_t rx_buff[16];
extern uint8_t rx_chbuff;
extern uint8_t tx_chbuff;
extern uint8_t cmd_buff[16];
extern uint8_t nr_buff[];
extern uint8_t nr[];
extern uint8_t r[];
extern uint16_t adc_vals[3];
extern uint8_t error_flag;
extern uint32_t pwm1;

//typedef
typedef uint32_t q16_t; // defines signed fixpoint type, point precedes 16th digit (2^0)
