/*
 * sysClock.cpp
 *
 * Created: 15.04.2021 08:38:46
 *  Author: Pawel Rogoz
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "DEBUG.h"
#include "sysClock.h"


void Time_setup(void)
{
	TCCR2A = 0;  //licznik pracuje w normal mode
	TCCR2B = 0;  //licznik zatrzymany
	TIMSK2 = (1 << OCIE2A); //wlaczenie przerwan od komparatora 2A
	SC_pozycjaKolejki = 0;
}


void Time_delay(uint64_t opoznienie, uint8_t *aktywacja, uint8_t *step)
{
	TCCR2B = 0; // zatrzymanie licznika
	//cli();

#ifdef _DEBUG_sysClock_1
	printString("  \r\n");
	printString("Time_delay 1 -> SC_pozycjaKolejki =");
	printByte(SC_pozycjaKolejki);
	printString(" \r\n");
#endif // _DEBUG_sysClock_1
	
	if(SC_pozycjaKolejki == 0)	//Jesli pierwszy delay kasujemy licznik T2
	{
		OCR2A = 250;
		TCNT2 = 0;
	}
	
	*aktywacja = 0;
	sysClock[SC_pozycjaKolejki].wskaznik_aktywacja = aktywacja;
	
	sysClock[SC_pozycjaKolejki].wskaznik_step = step;
	
	sysClock[SC_pozycjaKolejki].opoznienie = opoznienie / 2;
	//dzielimy przez 2 w celu przeliczenia z us na kwanty licznika T2
	//dla 500us to 250 kwantow licznika (wartosc rejestru licznika)
	//dla 1000us to 500 kwantow czyli 2x250 kwantow
	
	if(sysClock[SC_pozycjaKolejki].opoznienie > 250)
	sysClock[SC_pozycjaKolejki].cykl = 250;
	else
	sysClock[SC_pozycjaKolejki].cykl = sysClock[SC_pozycjaKolejki].opoznienie;
	
	if((TCNT2 + sysClock[SC_pozycjaKolejki].cykl) > 250)	//czy cykl jest wiekszy niz roznica licznika do 250
	{
		sysClock[SC_pozycjaKolejki].cykl = 250 - TCNT2;
		sysClock[SC_pozycjaKolejki].komparator = 250;
	}
	else
	sysClock[SC_pozycjaKolejki].komparator = TCNT2 + sysClock[SC_pozycjaKolejki].cykl;
	
	SC_pozycjaKolejki++;
	
	for(uint8_t i = 0; i < SC_pozycjaKolejki; i++)	//liczymy nowy komparator
	{
		if(OCR2A > sysClock[i].komparator)
		{
			OCR2A = sysClock[i].komparator;
		}
	}

#ifdef _DEBUG_sysClock_1
printString("Time_delay 2 -> SC_pozycjaKolejki =");
printByte(SC_pozycjaKolejki);
printString(" \r\n");
#endif // _DEBUG_sysClock_1

	//sei();
	TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
}


/**********************************************************************************/
void Time_isr(void)
{
	TCCR2B = 0;  //zatrzymanie licznika
	
	for(uint8_t i = 0; i < SC_pozycjaKolejki; i++)	//najpierw liczymy opoznienia, nowe cykle, nowe komparatory
	{
		sysClock[i].opoznienie = sysClock[i].opoznienie - (OCR2A - (sysClock[i].komparator - sysClock[i].cykl)); //nowe opoznienie
		// zle liczone opiznienie

#ifdef _DEBUG_sysClock_2
printString("  \r\n");
printString("Time_isr 1 -> opoznienie[");
printByte(i);
printString("]=");
printWord(sysClock[i].opoznienie);
printString(" \r\n");
#endif // _DEBUG_sysClock_2

		if(sysClock[i].opoznienie == 0)  //licznik dla opoznienie[i] zakonczyl prace
		{
			(*sysClock[i].wskaznik_step)++;
			*sysClock[i].wskaznik_aktywacja = 1;
		}
		else
		{
			if(sysClock[i].opoznienie > 250)	//wyliczamy nowy cykl
			sysClock[i].cykl = 250;
			else
			sysClock[i].cykl = sysClock[i].opoznienie;
			
			sysClock[i].komparator = sysClock[i].cykl;	//wyliczamy nowy komarator
		}
	}
	
	
	uint8_t x = 0;
	uint8_t y = 0;
	while(x < SC_pozycjaKolejki)	//redukujemy kolejke o zakonczone opuznienia
	{
		if(sysClock[x].opoznienie == 0)  //wykryto zakonczony licznik
		{
			y = x;
			while(y < SC_pozycjaKolejki-1)	//redukcja wykonanego opoznienia
			{
				sysClock[y].opoznienie = sysClock[y+1].opoznienie;
				sysClock[y].cykl = sysClock[y+1].cykl;
				sysClock[y].komparator = sysClock[y+1].komparator;
				sysClock[y].wskaznik_aktywacja = sysClock[y+1].wskaznik_aktywacja;
				sysClock[y].wskaznik_step = sysClock[y+1].wskaznik_step;
				y++;
			}
			SC_pozycjaKolejki--;
			
			#ifdef _DEBUG_sysClock_2
			printString("  \r\n");
			printString("Time_isr 1 -> SC_pozycjaKolejki =");
			printByte(SC_pozycjaKolejki);
			printString(" \r\n");
			#endif // _DEBUG_sysClock_2
		}
		else
		x++;
	}
	
	if(SC_pozycjaKolejki == 0)	//Jesli pierwszy delay kasujemy licznik T2
		TCNT2 = 0;
		
	OCR2A = 250;
	
	for(uint8_t i = 0; i < SC_pozycjaKolejki; i++)	//liczymy nowy komparator
	{
		if(OCR2A > sysClock[i].komparator)
		{
			OCR2A = sysClock[i].komparator;
		}
	}
	
	if(SC_pozycjaKolejki > 0)
		TCCR2B = (1 << CS21) | (1 << CS20); //uruchomienie T2 dzielna 32
}