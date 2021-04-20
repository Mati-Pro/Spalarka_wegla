/*
 * sysClock.h
 *
 * Created: 15.04.2021 08:38:28
 *  Author: Pawel Rogoz
 */ 


#ifndef SYSCLOCK_H_
#define SYSCLOCK_H_

#include <avr/io.h>

#define SC_kolejka_max		4

volatile uint8_t SC_pozycjaKolejki;

struct SClk_ctr {
	volatile uint8_t cykl;				//wartosc kwantow licznika T2 do odliczenia
	volatile uint8_t komparator;			//wartosc rejestru komparatora
	volatile uint64_t opoznienie;		//opoznienie w us
	volatile uint8_t *wskaznik_aktywacja;
	volatile uint8_t *wskaznik_step;
};

volatile struct SClk_ctr sysClock[SC_kolejka_max];


void Time_setup(void);
void Time_delay(uint64_t opoznienie, uint8_t *aktywacja, uint8_t *step);
void Time_isr(void);


#endif /* SYSCLOCK_H_ */