/*
 * sysTWI.h
 *
 * Created: 15.04.2021 08:39:24
 *  Author: Pawel Rogoz
 */ 


#ifndef SYSTWI_H_
#define SYSTWI_H_

#include <avr/io.h>

#define _TWISendStart()    (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA)|(1<<TWIE))
#define _TWISendStop()     (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO))
#define _TWISendTransmit() (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE))
#define _TWISendACK()      (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA))
#define _TWISendNACK()     (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE))
#define _TWI_SR            (TWSR & 0xF8)

#define TWI_ERROR 0x00
#define TWI_START 0x08
#define TWI_REP_START 0x10
//Master-Nadawanie
#define TWI_MT_MTX_ACK 0x18     //Master transmit mode, SLA+W has been transmitted, ACK has been received
#define TWI_MT_MTX_NACK 0x20    //Master transmit mode, SLA+W has been transmitted, NOT ACK has been received
#define TWI_MT_MTXD_ACK 0x28    //Master transmit mode, DATA has been transmitted, ACK has been received
#define TWI_MT_MTXD_NACK  0x30  //Master transmit mode, DATA has been transmitted, NOT ACK has been received
//Master-Odbior
#define TWI_MR_MTX_ACK 0x40
#define TWI_MR_MTX_NACK 0x48
#define TWI_MR_MRX_ACK 0x50
#define TWI_MR_MRX_NACK 0x58

#define TWI_ARBITRATION 0x38

//#define  TWI_kolejka_max	6


uint8_t TWI_kolejkaPointer;      //wskaŸnik bufora operacji

struct twi_rs {
	uint8_t adres;
	uint8_t mode;
	uint8_t *reg;
	uint8_t data_pointer;
	uint8_t data_count;
	uint8_t stop;
	uint8_t busy;
};

volatile struct twi_rs TWI;

struct twi_bf {
	uint8_t adres;
	uint8_t mode;
	uint8_t *reg;
	uint8_t data_pointer;
	uint8_t data_count;
	uint8_t *f_aktywacja;
};

//struct twi_bf TWI_kolejka[TWI_kolejka_max]; //bufor operacji dla i2c
struct twi_bf *TWI_kolejka;


void TWI_setup(void);
void TWI_access(uint8_t adres, uint8_t mode, uint8_t pointer, uint8_t *tab, uint8_t count, uint8_t *aktywacja);
void TWI_handling(void);
void TWI_isr(void);

#endif /* SYSTWI_H_ */