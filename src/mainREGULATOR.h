/*
 * mainREGULATOR.h
 *
 * Created: 24.04.2021 23:36:47
 *  Author: Pawel Rogoz
 */ 


#ifndef MAINREGULATOR_H_
#define MAINREGULATOR_H_

#include "avr/io.h"

//bity IO_control
#define Pompa_CWU	0
#define Pompa_CO	1
#define Miesz_Gh	2
#define Miesz_Gl	3
#define Pompa_P		4
#define ZesilaniePodajWent		5

//bity CO_status
#define CO_pracaPodawanie	0
#define CO_pracaPodtrzymanie	1
#define CO_50st		2
#define CO_55st		3

//--------------------------temperatury---------------------
struct status {
	uint8_t active;
	uint8_t status;
};


struct  Nas_temp {
	uint8_t war;
	uint8_t his;
};


struct _time_type1 {
	uint8_t sekundy;
	uint8_t msekundy;
};


struct _time_type2 {
	uint8_t minuty;
	uint8_t sekundy;
};


struct Param_spal {
	struct _time_type1	podawanie;
	struct _time_type2	przerwa;
};


void Regulator_setup(void);
void Regulator_handling(void);


#endif /* MAINREGULATOR_H_ */