

#include "hw_init.h"
#include "handlers.h"
#include "util.h"
#include "global.h"




//extern uint8_t rx_chbuff;

uint8_t intbuff[10];
uint8_t fracbuff[10];
uint32_t timestamp;

void HW_Init(void);

//state prototypes
void *init(void);
void *gVal(void);
void *sVal(void);
void *parCmd(void);
void *autoLoop(void);
void *meas(void);
void *com(void);
void *balance(void);


// State pointer
typedef void *(*stp_t)();
