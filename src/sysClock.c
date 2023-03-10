/*
 * sysClock.cpp
 *
 * Created: 15.04.2021 08:38:46
 *  Author: Pawel Rogoz
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "_DEBUG.h"
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
	
	if(SC_pozycjaKolejki == 0)	//jesli jeszcze nie bylo kolejki to stworz j¹
		sysClock = (struct SClk_ctr *)calloc(SC_pozycjaKolejki + 1, sizeof(struct SClk_ctr));
	else
		sysClock = (struct SClk_ctr *)realloc(sysClock, (SC_pozycjaKolejki+1)*sizeof(struct SClk_ctr));
	
	*aktywacja = 0;
	sysClock[SC_pozycjaKolejki].wskaznik_aktywacja = aktywacja;
	sysClock[SC_pozycjaKolejki].wskaznik_step = step;
	sysClock[SC_pozycjaKolejki].opoznienie = opoznienie;
	
	uint64_t opoznienie_tmp;
	
	opoznienie_tmp = opoznienie;
	
	if(SC_pozycjaKolejki > 0)	//jesli licznik juz pracuje, czyli wystepuja elementy kolejki - redudujemy opoznienia
	{	
		for(uint8_t i = 0; i < SC_pozycjaKolejki; i++)	//liczymy opoznienia dla wszystkich SCpozycjiKolejki
		{
			sysClock[i].opoznienie -= (TCNT2 * N_mnoznikLicznika);	//np: 2us -> 1 takt licznika
		
			//Musze jeszcze sprawdzic czy wieksze od zera
			if(opoznienie_tmp > sysClock[i].opoznienie)	//Szukanie najmniejszego opoznienia
				opoznienie_tmp = sysClock[i].opoznienie;
		}
	}
	
			
	SC_pozycjaKolejki++;
	TCNT2 = 1;
	OCR2A = 250;

	
	//Time_przeliczCykle;
	if(opoznienie_tmp >= 16000)
	{
		N_mnoznikLicznika = 64;
		TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
	}
	else
	{
		if (opoznienie_tmp >= 4000)
		{
			N_mnoznikLicznika = 16;
			TCCR2B = (1 << CS22) | (1 << CS21);  //wznowienie pracy T2 dzielna 32
		} 
		else
		{
			if (opoznienie_tmp >= 2000)
			{
				N_mnoznikLicznika = 8;
				TCCR2B = (1 << CS22) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
			}
			else
			{
				if (opoznienie_tmp >= 1000)
				{
					N_mnoznikLicznika = 4;
					TCCR2B = (1 << CS22);  //wznowienie pracy T2 dzielna 32
				}
				else
				{
					if (opoznienie_tmp < 500)
						OCR2A = (opoznienie_tmp / 2);

					N_mnoznikLicznika = 2;
					TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
				}
			}
		}
	}
}


/**********************************************************************************/
void Time_isr(void)
{

	TCCR2B = 0;  //zatrzymanie licznika

	for(uint8_t i = 0; i < SC_pozycjaKolejki; i++)	//liczymy opoznienia dla wszystkich SCpozycjiKolejki
	{	
		sysClock[i].opoznienie -= ((OCR2A) * N_mnoznikLicznika);	//np: 2us -> 1 takt licznika
		
		if(sysClock[i].opoznienie == 0)  //jesli licznik dla opoznienie[i] zakonczyl prace
		{
			(*sysClock[i].wskaznik_step)++;
			*sysClock[i].wskaznik_aktywacja = 1;
		}
	}
	
	
	uint8_t x = 0;
	uint8_t y = 0;
	while(x < SC_pozycjaKolejki)	//redukujemy kolejke o zakonczone opoznienia
	{
		if(sysClock[x].opoznienie == 0)  //wykryto zakonczony licznik
		{
			y = x;
			while(y < SC_pozycjaKolejki-1)	//redukcja wykonanego opoznienia
			{
				sysClock[y].opoznienie = sysClock[y+1].opoznienie;
				sysClock[y].wskaznik_aktywacja = sysClock[y+1].wskaznik_aktywacja;
				sysClock[y].wskaznik_step = sysClock[y+1].wskaznik_step;
				y++;
			}
			SC_pozycjaKolejki--;
		}
		else
		x++;
	}
	
	if(SC_pozycjaKolejki == 0)
		free(sysClock);
	else
		sysClock = (struct SClk_ctr *)realloc(sysClock, (SC_pozycjaKolejki+1)*sizeof(struct SClk_ctr));
	
	
	TCNT2 = 1;
	OCR2A = 250;
	
	
	if(SC_pozycjaKolejki > 0)	//Jesli ostatni delay kasujemy licznik T2
	{	
		uint64_t opoznienie_tmp;
		
		opoznienie_tmp = sysClock[0].opoznienie;
		
		for(uint8_t i = 1; i < SC_pozycjaKolejki; i++)	//szukamy najmniejszego opoznienia
		{
			if(opoznienie_tmp > sysClock[i].opoznienie)
				opoznienie_tmp = sysClock[i].opoznienie;
		}
		
		
		//Time_przeliczCykle;
		if(opoznienie_tmp >= 16000)
		{
			N_mnoznikLicznika = 64;
			TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
		}
		else
		{
			if (opoznienie_tmp >= 4000)
			{
				N_mnoznikLicznika = 16;
				TCCR2B = (1 << CS22) | (1 << CS21);  //wznowienie pracy T2 dzielna 32
			}
			else
			{
				if (opoznienie_tmp >= 2000)
				{
					N_mnoznikLicznika = 8;
					TCCR2B = (1 << CS22) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
				}
				else
				{
					if (opoznienie_tmp >= 1000)
					{
						N_mnoznikLicznika = 4;
						TCCR2B = (1 << CS22);  //wznowienie pracy T2 dzielna 32
					}
					else
					{
						if (opoznienie_tmp < 500)
							OCR2A = (opoznienie_tmp / 2);

						N_mnoznikLicznika = 2;
						TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
					}
				}
			}
		}
	}
}
