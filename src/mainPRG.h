/*
 * mainPRG.h
 *
 * Created: 16.04.2021 21:32:45
 *  Author: Pawel Rogoz
 */ 


#ifndef MAINPRG_H_
#define MAINPRG_H_

#include <avr/io.h>
#include "sys1Wire.h"
#include "mainREGULATOR.h"

#define RTC_ADRES 0x68  //adres RTC I2C
#define IO_ADRES 0x20  //adres ekspandera IO I2C

//------------------------------------------RTC---------------------
struct _dj_type {
	uint8_t d; //dziesiatki
	uint8_t j; //jednostki
};

struct zegar {
	struct _dj_type godzina;
	struct _dj_type minuta;
	struct _dj_type sekunda;
};

struct data {
	struct _dj_type rok;
	struct _dj_type miesiac;
	struct _dj_type dzien;
	uint8_t	nrDnia;
};

//------------------------------------------MENU---------------------
struct men {
	uint8_t aktywacja;
	uint8_t pozEkranu;
	uint8_t pozWys;
	uint8_t pozTab;
	uint8_t pozUstaw;
};


void Ekran_glowny(void);
void Program_glowny(void);
void Menu_godzina(void);
void Menu_data(void);
void Menu_tempX(uint8_t poz_tab, uint8_t poz_pom, struct _HL_type *sensor, uint8_t przetermin, struct Nas_temp *tempX);

uint8_t ileDniMiesiaca(void);

void RTC_setup();
void RTC_konwertuj(void);
void RTC_kalk_Time(char *tab, uint8_t poz);
void RTC_kalk_Data(char *tab, uint8_t poz);



#endif /* MAINPRG_H_ */