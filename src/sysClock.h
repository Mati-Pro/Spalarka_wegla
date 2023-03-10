/*
 * sysClock.h
 *
 * Created: 15.04.2021 08:38:28
 *  Author: Pawel Rogoz
 */ 


#ifndef SYSCLOCK_H_
#define SYSCLOCK_H_

#include <avr/io.h>

//#define SC_kolejka_max		6

volatile uint8_t SC_pozycjaKolejki;
volatile uint8_t N_mnoznikLicznika;

struct SClk_ctr {
	volatile uint64_t opoznienie;		//opoznienie w us
	volatile uint8_t *wskaznik_aktywacja;
	volatile uint8_t *wskaznik_step;
};

//volatile struct SClk_ctr sysClock[SC_kolejka_max];
//volatile struct SClk_ctr *sysClock;
struct SClk_ctr *sysClock;


void Time_setup(void);
void Time_delay(uint64_t opoznienie, uint8_t *aktywacja, uint8_t *step);
void Time_isr(void);


#endif /* SYSCLOCK_H_ */