/*
 * sysISP.h
 *
 * Created: 23.04.2021 17:46:25
 *  Author: Pawel Rogoz
 */ 


#ifndef SYSISP_H_
#define SYSISP_H_

#include <avr/io.h>


void SPI_setup(void);
void SPI_handling(void);
void SPI_isr(void);

void SPI_wyswietl_pomiar(void);

#endif /* SYSISP_H_ */