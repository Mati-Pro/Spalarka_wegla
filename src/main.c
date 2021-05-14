/*
 * Spalarka_wegla.cpp
 *
 * Created: 15.04.2021 08:36:48
 * Author : Pawel Rogoz
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "_DEBUG.h"
#include "sysClock.h"
#include "sysTWI.h"
#include "sysLCD.h"
#include "sysEnc.h"
#include "mainPRG.h"
#include "mainREGULATOR.h"
#include "sys1Wire.h"
#include "sysSPI.h"
#include "USART.h"

uint8_t SysStatus;
uint8_t RTC_odczyt;
uint8_t RTC_reg[8];

extern struct _HL_type OneWire_SENSOR[5];
extern struct OneWire_st OneWire_STATUS;
extern volatile uint8_t nrBajtuISP;

extern uint8_t CO_krok;



int main(void)
{
   // Przygotowanie pinow INT0 i INT1 (input pullup)
   DDRD &= ~((1 << DDD3) | (1 << DDD2)); //DDC3 i DDC2 jako input (zero na DDD3 i DDD2)
   PORTD |=   (1 << PD3) | (1 << PD2);	//PD3 i PD2 pullup
   
   //Przygotowanie przerwania INT0
   EICRA |= (1 << ISC01) | (1 << ISC00);	//przerwania INT0 przy zboczu rosn¹cym
   
   sei();
   
#ifdef _DEBUG_
	initUSART();
#endif // _DEBUG_

	OneWire_setup();	//inicjacja dla portów DS18
	Enc_setup();		//inicjalizacja portow encodera
	Time_setup();   //inicjacja zegara systemowego
	TWI_setup();		//inicjacja interfejsu i2c
	LCD_i2c_setup();	//inicjacja pinów dla wyswietlacza lcd_i2c
	Regulator_setup();	//inicjacja pinow dla sterowania podajnikiem i wentylatorem
   
	SysStatus = 0;
	RTC_odczyt = 0;
   
   
   while(SysStatus < 7)
   {
	   TWI_handling();				//obs³uga i2c
	   LCD_initiation();
	   RTC_setup();
   }
   
   SPI_setup();		//inicjacja pinow dla SPI
   
   LCD_clear_bufor();
   Ekran_glowny();
   OneWire_wyswietl_pomiar(1, LCD_row3, 2,  &OneWire_SENSOR[0].H, &OneWire_SENSOR[0].L, OneWire_STATUS.przeterminowany & 0x01);	//kociol zasilanie
   OneWire_wyswietl_pomiar(1, LCD_row4, 2,  &OneWire_SENSOR[1].H, &OneWire_SENSOR[1].L, OneWire_STATUS.przeterminowany & 0x02);	//kociol powrot
   OneWire_wyswietl_pomiar(0, LCD_row2, 13, &OneWire_SENSOR[2].H, &OneWire_SENSOR[2].L, OneWire_STATUS.przeterminowany & 0x04);	//CWU
   OneWire_wyswietl_pomiar(0, LCD_row3, 13, &OneWire_SENSOR[3].H, &OneWire_SENSOR[3].L, OneWire_STATUS.przeterminowany & 0x20);	//grzejniki zasilanie
   OneWire_wyswietl_pomiar(0, LCD_row4, 13, &OneWire_SENSOR[4].H, &OneWire_SENSOR[4].L, OneWire_STATUS.przeterminowany & 0x40);	//podlogowka
   LCD_wyslij_bufor();
   
   
    while (1) 
    {
		OneWire_sekwencjaOdczyt();	//obs³uga sekwencji odczytu z OneWire
		OneWire_handling();			//ogs³uga OneWire
		Enc_handling();				//obs³uga przycisku encodera
		TWI_handling();				//obs³uga i2c
		LCD_handling();				//obsluga lcd
		SPI_handling();				//obsluga SPI
		
		Regulator_handling();		//regulator CO CWU
		
		Program_glowny();			//MENU + Ekran Glowny + RTC
    }
}


ISR(INT0_vect)	//przerwanie od SQ RTC
{
	if(RTC_odczyt == 5)
	RTC_odczyt = 6;	//6 - odczyt RTC, 5 - neutralne
	
	nrBajtuISP = 1;
}


ISR(INT1_vect)	//przerwanie od synchronizacji AC
{
	//start licznika T1 do odliczenia przesuniecia fazowego
}


ISR(TWI_vect)	//TWI
{
	TWI_isr();
}


ISR(TIMER2_COMPA_vect)	//sysClock
{
	Time_isr();
}


ISR(PCINT0_vect)	//A
{
	Enc_isrA();
}


ISR(PCINT2_vect)	//B
{
	Enc_isrB();
}


ISR(PCINT1_vect)	//E
{
	Enc_isrE();
}


ISR(SPI_STC_vect)	//SPI
{
	SPI_isr();
}
