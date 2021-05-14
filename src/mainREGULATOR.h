/*
 * mainREGULATOR.h
 *
 * Created: 24.04.2021 23:36:47
 *  Author: Pawel
 */ 


#ifndef MAINREGULATOR_H_
#define MAINREGULATOR_H_

#include "avr/io.h"

#define Pompa_CWU	1
#define Pompa_CO	2
#define Miesz_Gh	3
#define Miesz_Gl	4
#define Pompa_P		5
#define Zes_PW		6

#define CO_pracaPodawanie	0
#define CO_pracaPrzerwa		1
#define CO_50st		2
#define CO_55st		3

//--------------------------temperatury---------------------
struct  Nas_temp {
	uint8_t war;
	uint8_t his;
};


struct _time_type {
	uint8_t minuty;
	uint8_t sekundy;
	uint8_t msekundy;
};


struct Param_spac {
	struct _time_type	podawanie;
	struct _time_type	przerwa;
};


void Regulator_setup(void);
void Regulator_handling(void);
void Regulator_isr(void);


#endif /* MAINREGULATOR_H_ */