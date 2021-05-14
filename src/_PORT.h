/*
 * PORT.h
 *
 * Created: 24.04.2021 20:43:59
 *  Author: Pawel
 */ 


#ifndef PORT_H_
#define PORT_H_

//	PORTB
#define PORT_SPI		PORTB
#define P_SS		PB2

//	DDRB
#define DDR_SPI		DDRB
#define DD_SS	DDB2
#define DD_MOSI	DDB3
#define DD_MISO	DDB4
#define DD_SCK	DDB5


//	PORTC
#define PORT_TWI	PORTC
#define P_SDA	PC4
#define P_SCL	PC5

//	DDRC
#define DDR_TWI		DDRC
#define	DD_SDA	DDC4
#define DD_SCL	DDC5


#endif /* PORT_H_ */