/*
 * sysOneWire.h
 *
 * Created: 17.04.2021 14:01:08
 *  Author: Pawel Rogoz
 */ 


#ifndef SYSONEWIRE_H_
#define SYSONEWIRE_H_

#include <avr/io.h>

#define IO_ADRES 0x20  //adres ekspandera IO I2C

#define OneWire_RESET_time1 500
#define OneWire_RESET_time2 70
#define OneWire_RESET_time3 500

#define OneWire_SEND_time1 5
#define OneWire_SEND_time2 70
#define OneWire_SEND_time3 70

#define OneWire_READ_time1 5
#define OneWire_READ_time2 5
#define OneWire_READ_time3 70


struct _HL_type {
	uint8_t  H;
	uint8_t  L;
};

struct oneWire_ini {
	uint8_t active;
	uint8_t step;
};

struct oneWire_rs {
	uint8_t active;
	uint8_t step;
	uint8_t AS;
};

struct OneWire_st {
	uint8_t przeterminowany;
	uint8_t niewiarygodny;
};


void OneWire_setup(void);
void OneWire_handling(void);
void OneWire_sekwencjaOdczyt(void);

void OneWire_SEND(uint8_t OneWire_AS, uint8_t rozkaz);
void OneWire_READ(uint8_t OneWire_AS, uint8_t* sensor1, uint8_t* sensor2, uint8_t* sensor3, uint8_t* sensor4, uint8_t* sensor5);

void OneWire_wyswietl_pomiar(uint8_t mode, char *LCDrow, uint8_t poz, uint8_t *sensorH, uint8_t *sensorL, uint8_t przeterminowanie);


#endif /* SYSONEWIRE_H_ */