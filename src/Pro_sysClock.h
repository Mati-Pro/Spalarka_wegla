/*
 * Pro_sysClock.h
 *
 * Created: 25.10.2020 17:53:07
 *  Author: Pro
 */ 


#ifndef PRO_SYSCLOCK_H_
#define PRO_SYSCLOCK_H_


//-------------------------------------Obs³uga sys_Clock---------------------------------
//--
//-- sysClock_setup() procedura inicjacji warunki poczatkowe zegara systemowego
//-- sysClock_delay1() procedura wywolania opoznienia nr 1
//-- sysClock_delay2() procedura wywolania opoznienia nr 2
//-- ISR(TIMER2_COMPA_vect) procedura obs³ugi przerwania dla zegara Timer2
//--
//--------------------------------------------------------------------------------

#define OPOZNIENIE_DELAY3	50000	//stale opoznienie (us) dla delay3
#include <avr/io.h>

//------------------------------------------sysClock---------------------
struct SClk_ctr {
	uint8_t praca;				//znacznik pracy timera 0-stop 1-praca
	uint8_t cykl;				//wartosc kwantow licznika T2 do odliczenia
	uint8_t komparator;			//wartosc rejestru komparatora
	unsigned long opoznienie;	//opoznienie w us
	uint8_t *aktywacja;			//wskaznik aktywujacy funkcje po odliczeniu zadanego czasu
};

volatile struct SClk_ctr sysClock1 = {0, 0, 0, 0};
volatile struct SClk_ctr sysClock2 = {0, 0, 0, 0};
volatile struct SClk_ctr sysClock3 = {0, 0, 0, 0};

uint8_t sysClock_exe;


//----------------------------Inicjacja sysClock----------------------------------
void SysClock_setup(void)
{
	TCCR2A = 0;  //licznik pracuje w normal mode
	TCCR2B = 0;  //licznik zatrzymany
	TIMSK2 = (1 << OCIE2A); //wlaczenie przerwan od komparatora 2A
}


//----------------------------Wywo³anie opoznienia delay----------------------------------
//  sysClock_delay1(unsigned int opoznienie, struct LCD_rs *p)
//
//  opoznienia  - opoznienia w us, najmniejsza wartosc to 10us
//  *aktywacja  - wskaznik do aktywacji po odliczeniu zadanego czasu
//  *step - zwiekszenie kroku dla sekwencji krokow
//
//--------------------------------------------------------------------------------
void sysClock_delay1(unsigned long opoznienie, uint8_t *aktywacja, uint8_t *step)
{
	uint8_t kompR2;
	
	*aktywacja = 0;
	(*step)++;
	
	sysClock1.aktywacja = aktywacja;
	sysClock1.praca = 1;
	sysClock1.opoznienie = opoznienie / 2;
	//dzielimy przez 2 w celu przeliczenia z us na kwanty licznika T2
	//dla 500us to 250 kwantow licznika
	//dla 100us to 500 kwantow czyli 2x250 kwantow
	
	if(sysClock1.opoznienie > 250)
		sysClock1.cykl = 250;
	else
		sysClock1.cykl = sysClock1.opoznienie;
	
	if(sysClock2.praca == 0) //jesli licznik dla delay2 nie pracuje
	{
		sysClock_exe = 1;
		sysClock1.komparator = sysClock1.cykl;
		OCR2A = sysClock1.cykl; //wartosc dla komparatora licznika T2
		TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
	}
	else  //jesli licznik dla delay2 pracuje
	{
		TCCR2B = 0; // zatrzymanie licznika
		//cli();
		kompR2 = sysClock2.komparator - TCNT2;  //roznica do osiagniecia wartosci komparatora dla delay2
		
		if(kompR2 > sysClock1.cykl)	//jesli roznica wieksza od ilosci kwantow dla delay1
		{
			sysClock1.komparator = TCNT2 + sysClock1.cykl; //wartosc dla komparatora dla delay1
			OCR2A = sysClock1.komparator;
			sysClock_exe = 1;
		}

		if(kompR2 < sysClock1.cykl)
		{
			if((TCNT2 + sysClock1.cykl) > 250)	//wartosc dla komparatora nie moze byc wieksza niz 250
			{
				sysClock1.cykl = 250 - TCNT2;
				sysClock1.komparator = 250;
			}
			else
			sysClock1.komparator = TCNT2 + sysClock1.cykl;
		}

		if(kompR2 == sysClock1.cykl)
		{
			sysClock_exe = 3;
			sysClock1.komparator = sysClock2.komparator;
		}

		//sei();
		TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
	}
}


void sysClock_delay2(unsigned long opoznienie, uint8_t *aktywacja, uint8_t *step)
{
	uint8_t kompR1;
	
	*aktywacja = 0;
	(*step)++;
	
	sysClock2.aktywacja = aktywacja;
	sysClock2.praca = 1;
	sysClock2.opoznienie = opoznienie / 2;
	
	if(sysClock2.opoznienie > 250)
		sysClock2.cykl = 250;
	else
		sysClock2.cykl = sysClock2.opoznienie;

	if(sysClock1.praca == 0) //jesli licznik dla delay1 nie pracuje
	{
		sysClock_exe = 2;
		sysClock2.komparator = sysClock2.cykl;
		OCR2A = sysClock2.cykl; //wartosc dla komparatora licznika T2
		TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
	}
	else  //jesli licznik dla delay1 pracuje
	{
		TCCR2B = 0; // zatrzymanie licznika
		//cli();
		kompR1 = sysClock1.komparator - TCNT2;  //roznica do osiagniecia komparatora dla delay2
		
		if(kompR1 > sysClock2.cykl)
		{
			sysClock2.komparator = TCNT2 + sysClock2.cykl; //wartosc dla komparatora dla delay1
			OCR2A = sysClock2.komparator;
			sysClock_exe = 2;
		}

		if(kompR1 < sysClock2.cykl)
		{
			if((TCNT2 + sysClock2.cykl) > 250)
			{
				sysClock2.cykl = 250 - TCNT2;
				sysClock2.komparator = 250;
			}
			else
			sysClock2.komparator = TCNT2 + sysClock2.cykl;
		}

		if(kompR1 == sysClock2.cykl)
		{
			sysClock_exe = 3;
			sysClock2.komparator = sysClock1.komparator;
		}

		//sei();
		TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie T2 dzielna 32
	}
}


//----------------------------Wywo³anie opoznienia delay----------------------------------
//  sysClock_delay
//
//  opoznienia  - opoznienia 50ms (50000)
//  *aktywacja  - wskaznik do aktywacji po odliczeniu zadanego czasu
//  *step - zwiekszenie kroku dla sekwencji krokow
//
//--------------------------------------------------------------------------------
void sysClock_delay3(uint8_t *aktywacja, uint8_t *step)
{
	uint8_t kompR1 = 250;
	uint8_t kompR2 = 250;
	uint8_t kompRx = 0;
	
	*aktywacja = 0;
	(*step)++;
	
	sysClock3.aktywacja = aktywacja;
	sysClock3.praca = 1;
	sysClock3.opoznienie = OPOZNIENIE_DELAY3 / 2;
	//dzielimy przez 2 w celu przeliczenia z us na kwanty licznika T2
	//dla 500us to 250 kwantow licznika
	//dla 100us to 500 kwantow czyli 2x250 kwantow
	
	if(sysClock3.opoznienie > 250)
		sysClock3.cykl = 250;
	else
		sysClock3.cykl = sysClock3.opoznienie;
	
	if(sysClock1.praca == 0 && sysClock2.praca == 0) //jesli licznik dla delay1 i delay2 nie pracuja
	{
		sysClock_exe = 0x04;
		sysClock3.komparator = sysClock3.cykl;
		TCNT2 = 0; //kasowanie licznika T2
		OCR2A = sysClock3.komparator; //wartosc dla komparatora licznika T2
		TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
	}
	
	if(sysClock1.praca == 1 || sysClock2.praca == 1)  //jesli pracuje delay1 lub delay2
	{
		TCCR2B = 0; // zatrzymanie licznika
		
		if(sysClock1.praca == 1)
			{
				kompR1 = sysClock1.komparator - TCNT2;  //roznica do osiagniecia wartosci komparatora dla delay1
				kompRx = kompR1;
			}
			
		if(sysClock2.praca == 1)
			{
				kompR2 = sysClock2.komparator - TCNT2;  //roznica do osiagniecia wartosci komparatora dla delay2
				if(kompR2 < kompR1)	//szukamy mniejszej roznicy
					kompRx = kompR2;
			}
			
		if(kompRx > sysClock3.cykl)	//jesli roznica wieksza od ilosci kwantow dla delay3
		{
			sysClock3.komparator = TCNT2 + sysClock3.cykl; //wartosc dla komparatora dla delay1
			OCR2A = sysClock3.komparator;
			sysClock_exe = 0x04;
		}

		if(kompRx < sysClock3.cykl)
		{
			if((TCNT2 + sysClock3.cykl) > 250)	//wartosc dla komparatora nie moze byc wieksza niz 250
			{
				sysClock3.cykl = 250 - TCNT2;
				sysClock3.komparator = 250;
			}
			else
				sysClock3.komparator = TCNT2 + sysClock3.cykl;
		}

		if(kompRx == sysClock3.cykl)
		{	
			sysClock_exe = 0x04;
			
			if((sysClock1.praca == 1) && (sysClock3.cykl == kompR1))
			{
				sysClock_exe |= 0x01;
				sysClock3.komparator = sysClock1.komparator;
				OCR2A = sysClock3.komparator;
			}
			
			if((sysClock2.praca == 1) && (sysClock3.cykl == kompR2))
			{
				sysClock_exe |= 0x02;
				sysClock3.komparator = sysClock2.komparator;
				OCR2A = sysClock3.komparator;
			}
		}

		TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
	}	
}


//----------------------------Wywo³ania przerwania zegara T2----------------------------------
ISR(TIMER2_COMPA_vect)
{
	unsigned int delayX_mode;
	
	TCCR2B = 0;  //wylaczenie zegara dla Timera2
	
	switch (sysClock_exe) {
		
		//Przerwanie dla delay1
		case 1:
		sysClock1.opoznienie = sysClock1.opoznienie - sysClock1.cykl;
		
		//Licznik dla delay1 zakonczyl prace
		if((sysClock1.praca == 1) && (sysClock1.opoznienie == 0))
		{
			sysClock1.praca = 0;
			*sysClock1.aktywacja = 1;
		}
		
		//Licznik delay1 pracuje - to znaczy,ze komparator ustawiony byl na 250
		if(sysClock1.praca == 1)
		{
			TCNT2 = 0; //kasowanie Timera2
			
			if(sysClock1.opoznienie > 250)
				sysClock1.cykl = 250;
			else
				sysClock1.cykl = sysClock1.opoznienie;

			sysClock_exe = 0x01;
			sysClock1.komparator = sysClock1.cykl;
			OCR2A = sysClock1.komparator;		//wartosc dla komparatora licznika T2
		}
		
		//Licznik delay2 pracuje
		if((sysClock2.praca == 1) && (sysClock3.praca == 0))
		{
			sysClock_exe = 0x02;
			OCR2A = sysClock2.komparator;		//wartosc dla komparatora licznika T2
		}
		
		//Licznik delay3 pracuje
		if((sysClock2.praca == 0) && (sysClock3.praca == 1))
		{
			sysClock_exe = 0x04;
			OCR2A = sysClock3.komparator;		//wartosc dla komparatora licznika T2
		}
		
		//Pracuje tylko delay2 i delay3
		if((sysClock2.praca == 1) && (sysClock3.praca == 1))
		{
			if(sysClock3.komparator < sysClock2.komparator)
			{
				sysClock_exe = 0x04;
				OCR2A = sysClock3.komparator;		//wartosc dla komparatora licznika T2
			}
			else
			{
				sysClock_exe = 0x02;
				OCR2A = sysClock2.komparator;		//wartosc dla komparatora licznika T2
			}
			
			if(sysClock2.komparator == sysClock3.komparator)
				{
					sysClock_exe = 0x04 | 0x02;
					OCR2A = sysClock2.komparator;
				}
		}
		
		//Zaden delay nie prauje
		if((sysClock1.praca == 0) && (sysClock2.praca == 0) && (sysClock3.praca == 0))
		{
			TCNT2 = 0; //kasowanie Timera2
			sysClock_exe = 0;
		}
		else
			TCCR2B = (1 << CS21) | (1 << CS20); //uruchomienie T2 dzielna 32
		
		break;


		//Przerwanie dla delay2
		case 2:
		sysClock2.opoznienie = sysClock2.opoznienie - sysClock2.cykl;
		
		//Licznik dla delay1 zakonczyl prace
		if((sysClock2.praca == 1) && (sysClock2.opoznienie == 0))
		{
			sysClock2.praca = 0;
			*sysClock2.aktywacja = 1;
		}

		//Licznik delay2 pracuje - to znaczy,ze komparator ustawiony byl na 250
		if(sysClock2.praca == 1)
		{
			TCNT2 = 0; //kasowanie Timera2
			
			if(sysClock2.opoznienie > 250)
				sysClock2.cykl = 250;
			else
				sysClock2.cykl = sysClock2.opoznienie;

			sysClock_exe = 0x02;
			sysClock2.komparator = sysClock2.cykl;
			OCR2A = sysClock2.cykl; //wartosc dla komparatora licznika T2
		}
		
		//Licznik delay1 pracuje
		if((sysClock1.praca == 1) && (sysClock3.praca == 0))
		{
			sysClock_exe = 0x01;
			OCR2A = sysClock1.komparator;		//wartosc dla komparatora licznika T2
		}
		
		//Licznik delay3 pracuje
		if((sysClock1.praca == 0) && (sysClock3.praca == 1))
		{
			sysClock_exe = 0x04;
			OCR2A = sysClock3.komparator;		//wartosc dla komparatora licznika T2
		}
		
		//Pracuje tylko delay1 i delay3
		if((sysClock1.praca == 1) && (sysClock3.praca == 1))
		{
			if(sysClock3.komparator < sysClock1.komparator)
			{
				sysClock_exe = 0x04;
				OCR2A = sysClock3.komparator;		//wartosc dla komparatora licznika T2
			}
			else
			{
				sysClock_exe = 0x01;
				OCR2A = sysClock1.komparator;		//wartosc dla komparatora licznika T2
			}
			
			if(sysClock1.komparator == sysClock3.komparator)
			{
				sysClock_exe = 0x04 | 0x01;
				OCR2A = sysClock1.komparator;
			}
		}
		
		//Zaden delay nie prauje
		if((sysClock1.praca == 0) && (sysClock2.praca == 0) && (sysClock3.praca == 0))
		{
			TCNT2 = 0; //kasowanie Timera2
			sysClock_exe = 0;
		}
		else
			TCCR2B = (1 << CS21) | (1 << CS20); //uruchomienie T2 dzielna 32
		
		break;


		//Przerwanie dla delay3
		case 4:
		sysClock3.opoznienie = sysClock3.opoznienie - sysClock3.cykl;
		
		//Licznik dla delay3 zakonczyl prace
		if((sysClock3.praca == 1) && (sysClock3.opoznienie == 0))
		{
			sysClock3.praca = 0;
			*sysClock3.aktywacja = 1;
		}

		//Licznik delay3 pracuje - to znaczy,ze komparator ustawiony byl na 250
		if(sysClock3.praca == 1)
		{
			TCNT2 = 0; //kasowanie Timera2
			
			if(sysClock3.opoznienie > 250)
			sysClock3.cykl = 250;
			else
			sysClock3.cykl = sysClock3.opoznienie;

			sysClock_exe = 0x04;
			sysClock3.komparator = sysClock3.cykl;
			OCR2A = sysClock3.cykl; //wartosc dla komparatora licznika T2
		}
		
		//Licznik delay1 pracuje
		if((sysClock1.praca == 1) && (sysClock2.praca == 0))
		{
			sysClock_exe = 0x01;
			OCR2A = sysClock1.komparator;		//wartosc dla komparatora licznika T2
		}
		
		//Licznik delay2 pracuje
		if((sysClock1.praca == 0) && (sysClock2.praca == 1))
		{
			sysClock_exe = 0x02;
			OCR2A = sysClock2.komparator;		//wartosc dla komparatora licznika T2
		}
		
		//Pracuje tylko delay1 i delay2
		if((sysClock1.praca == 1) && (sysClock2.praca == 1))
		{
			if(sysClock2.komparator < sysClock1.komparator)
			{
				sysClock_exe = 0x02;
				OCR2A = sysClock2.komparator;		//wartosc dla komparatora licznika T2
			}
			else
			{
				sysClock_exe = 0x01;
				OCR2A = sysClock1.komparator;		//wartosc dla komparatora licznika T2
			}
			
			if(sysClock1.komparator == sysClock2.komparator)
			{
				sysClock_exe = 0x02 | 0x01;
				OCR2A = sysClock1.komparator;
			}
		}
		
		//Zaden delay nie prauje
		if((sysClock1.praca == 0) && (sysClock2.praca == 0) && (sysClock3.praca == 0))
		{
			TCNT2 = 0; //kasowanie Timera2
			sysClock_exe = 0;
		}
		else
			TCCR2B = (1 << CS21) | (1 << CS20); //uruchomienie T2 dzielna 32
		
		break;
		
		
		//Przerwanie wspolne dla delay1, delay2 i delay3
		case 3: case 5: case 6: case 7:	
		
		if(sysClock1.praca == 1)
			sysClock1.opoznienie = sysClock1.opoznienie - sysClock1.cykl;
		
		if(sysClock2.praca == 1)
			sysClock2.opoznienie = sysClock2.opoznienie - sysClock2.cykl;
		
		if(sysClock3.praca == 1)
			sysClock3.opoznienie = sysClock3.opoznienie - sysClock3.cykl;

		if((sysClock1.opoznienie == 0) && (sysClock1.praca == 1))  //licznik dla delay1 zakonczyl prace
		{
			sysClock1.praca = 0;
			*sysClock1.aktywacja = 1;
			sysClock_exe &= ~(0x01);
		}

		if((sysClock2.opoznienie == 0) && (sysClock2.praca == 1))  //licznik dla delay2 zakonczyl prace
		{
			sysClock2.praca = 0;
			*sysClock2.aktywacja = 1;
			sysClock_exe &= ~(0x02);
		}
		
		if((sysClock3.opoznienie == 0) && (sysClock3.praca == 1))  //licznik dla delay3 zakonczyl prace
		{
			sysClock3.praca = 0;
			*sysClock3.aktywacja = 1;
			sysClock_exe &= ~(0x04);
		}

		if(sysClock1.praca == 1)
		{
			if(sysClock1.opoznienie > 250)
				sysClock1.cykl = 250;
			else
				sysClock1.cykl = sysClock1.opoznienie;
			
			sysClock1.komparator = sysClock1.cykl;
		}
		
		if(sysClock2.praca == 1)
		{
			if(sysClock2.opoznienie > 250)
				sysClock2.cykl = 250;
			else
				sysClock2.cykl = sysClock2.opoznienie;
			
			sysClock2.komparator = sysClock2.cykl;
		}
		
		if(sysClock3.praca == 1)
		{
			if(sysClock3.opoznienie > 250)
			sysClock3.cykl = 250;
			else
				sysClock3.cykl = sysClock3.opoznienie;
			
			sysClock3.komparator = sysClock3.cykl;		
		}
		
		//Rozne liczniki pracuja
		delayX_mode = (sysClock3.praca << 2) | (sysClock2.praca << 1) | sysClock1.praca;
		switch (delayX_mode)
		{
			//Pracuje tylko delay1
			case 1:
				sysClock_exe = 0x01;
				OCR2A = sysClock1.cykl;
			break;
			
			//Pracuje tylko delay2
			case 2:
				sysClock_exe = 0x02;
				OCR2A = sysClock2.cykl;
			break;
			
			//Pracuje tylko delay3
			case 4:
				sysClock_exe = 0x04;
				OCR2A = sysClock3.cykl;
			break;
			
			//Pracuje tylko delay1 i delay2
			case 3:
				if(sysClock2.komparator < sysClock1.komparator)
				{
					sysClock_exe = 0x02;
					OCR2A = sysClock2.komparator;		//wartosc dla komparatora licznika T2
				}
				else
				{
					sysClock_exe = 0x01;
					OCR2A = sysClock1.komparator;		//wartosc dla komparatora licznika T2
				}
				
				if(sysClock1.komparator == sysClock2.komparator)
				{
					sysClock_exe = 0x02 | 0x01;
					OCR2A = sysClock1.komparator;
				}
			break;
			
			//Pracuje tylko delay1 i delay3
			case 5:
			if(sysClock3.komparator < sysClock1.komparator)
			{
				sysClock_exe = 0x04;
				OCR2A = sysClock3.komparator;		//wartosc dla komparatora licznika T2
			}
			else
			{
				sysClock_exe = 0x01;
				OCR2A = sysClock1.komparator;		//wartosc dla komparatora licznika T2
			}
			
			if(sysClock1.komparator == sysClock3.komparator)
			{
				sysClock_exe = 0x04 | 0x01;
				OCR2A = sysClock1.komparator;
			}
			break;
			
			//Pracuje tylko delay2 i delay3
			case 6:
			if(sysClock2.komparator < sysClock3.komparator)
			{
				sysClock_exe = 0x02;
				OCR2A = sysClock2.komparator;		//wartosc dla komparatora licznika T2
			}
			else
			{
				sysClock_exe = 0x04;
				OCR2A = sysClock3.komparator;		//wartosc dla komparatora licznika T2
			}
			
			if(sysClock3.komparator == sysClock2.komparator)
			{
				sysClock_exe = 0x04 | 0x02;
				OCR2A = sysClock2.komparator;
			}
			break;
			
			//Pracuje tylko delay1, delay2 i delay3
			case 7:
			if((sysClock3.komparator == sysClock2.komparator) && (sysClock3.komparator == sysClock1.komparator))
			{
				sysClock_exe = 0x04 | 0x02 | 0x01;
				OCR2A = sysClock1.komparator;
			}
			break;
		}
		
		//Koniec pracy
		if((sysClock1.praca == 0) && (sysClock2.praca == 0) && (sysClock3.praca == 0))  
		{
			TCNT2 = 0; //kasowanie Timera2
			sysClock_exe = 0;
		}
		else
			TCCR2B = (1 << CS21) | (1 << CS20); //uruchomienie T2 dzielna 32
		
		break;
	}

}

#endif /* PRO_SYSCLOCK_H_ */

/*
void sysClock_delay1(unsigned long opoznienie, uint8_t *aktywacja, uint8_t *step)
{
	uint8_t kompR2;
	
	*aktywacja = 0;
	(*step)++;
	
	sysClock1.aktywacja = aktywacja;
	sysClock1.praca = 1;
	sysClock1.opoznienie = opoznienie / 2;
	//dzielimy przez 2 w celu przeliczenia z us na kwanty licznika T2
	//dla 500us to 250 kwantow licznika
	//dla 100us to 500 kwantow czyli 2x250 kwantow
	
	if(sysClock1.opoznienie > 250)
	sysClock1.cykl = 250;
	else
	sysClock1.cykl = sysClock1.opoznienie;
	
	if(sysClock2.praca == 0) //jesli licznik dla delay2 nie pracuje
	{
		sysClock_exe = 1;
		sysClock1.komparator = sysClock1.cykl;
		OCR2A = sysClock1.cykl; //wartosc dla komparatora licznika T2
		TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
	}
	else  //jesli licznik dla delay2 pracuje
	{
		TCCR2B = 0; // zatrzymanie licznika
		//cli();
		kompR2 = sysClock2.komparator - TCNT2;  //roznica do osiagniecia wartosci komparatora dla delay2
		
		if(kompR2 > sysClock1.cykl)	//jesli roznica wieksza od ilosci kwantow dla delay1
		{
			sysClock1.komparator = TCNT2 + sysClock1.cykl; //wartosc dla komparatora dla delay1
			OCR2A = sysClock1.komparator;
			sysClock_exe = 1;
		}

		if(kompR2 < sysClock1.cykl)
		{
			if((TCNT2 + sysClock1.cykl) > 250)	//wartosc dla komparatora nie moze byc wieksza niz 250
			{
				sysClock1.cykl = 250 - TCNT2;
				sysClock1.komparator = 250;
			}
			else
			sysClock1.komparator = TCNT2 + sysClock1.cykl;
		}

		if(kompR2 == sysClock1.cykl)
		{
			sysClock_exe = 3;
			sysClock1.komparator = sysClock2.komparator;
		}

		//sei();
		TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie pracy T2 dzielna 32
	}
}


void sysClock_delay2(unsigned long opoznienie, uint8_t *aktywacja, uint8_t *step)
{
	uint8_t kompR1;
	
	*aktywacja = 0;
	(*step)++;
	
	sysClock2.aktywacja = aktywacja;
	sysClock2.praca = 1;
	sysClock2.opoznienie = opoznienie / 2;
	
	if(sysClock2.opoznienie > 250)
	sysClock2.cykl = 250;
	else
	sysClock2.cykl = sysClock2.opoznienie;

	if(sysClock1.praca == 0) //jesli licznik dla delay1 nie pracuje
	{
		sysClock_exe = 2;
		sysClock2.komparator = sysClock2.cykl;
		OCR2A = sysClock2.cykl; //wartosc dla komparatora licznika T2
		TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
	}
	else  //jesli licznik dla delay1 pracuje
	{
		TCCR2B = 0; // zatrzymanie licznika
		//cli();
		kompR1 = sysClock1.komparator - TCNT2;  //roznica do osiagniecia komparatora dla delay2
		
		if(kompR1 > sysClock2.cykl)
		{
			sysClock2.komparator = TCNT2 + sysClock2.cykl; //wartosc dla komparatora dla delay1
			OCR2A = sysClock2.komparator;
			sysClock_exe = 2;
		}

		if(kompR1 < sysClock2.cykl)
		{
			if((TCNT2 + sysClock2.cykl) > 250)
			{
				sysClock2.cykl = 250 - TCNT2;
				sysClock2.komparator = 250;
			}
			else
			sysClock2.komparator = TCNT2 + sysClock2.cykl;
		}

		if(kompR1 == sysClock2.cykl)
		{
			sysClock_exe = 3;
			sysClock2.komparator = sysClock1.komparator;
		}

		//sei();
		TCCR2B = (1 << CS21) | (1 << CS20);  //wznowienie T2 dzielna 32
	}
}

//----------------------------Wywo³ania przerwania zegara T2----------------------------------
ISR(TIMER2_COMPA_vect)
{
	TCCR2B = 0;  //wylaczenie zegara dla Timera2
	
	switch (sysClock_exe) {
		
		case 1:	//przerwanie dla delay1
		sysClock1.opoznienie = sysClock1.opoznienie - sysClock1.cykl;
		
		if((sysClock1.opoznienie == 0) && (sysClock1.praca == 1))  //licznik dla delay1 zakonczyl prace
		{
			sysClock1.praca = 0;
			*sysClock1.aktywacja = 1;
			
			if(sysClock2.praca == 1)	//jesli pracuje jeszcze licznik dla delay2
			{
				sysClock_exe = 2;
				OCR2A = sysClock2.komparator; //wartosc dla komparatora licznika T2
				TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
			}
			else
			{
				TCNT2 = 0; //kasowanie Timera2
				sysClock_exe = 0;
			}
		}

		if((sysClock1.praca == 1) && (sysClock2.praca == 0))  //licznik pracowa³ tylko dla delay1, delay2 nie pracowal
		{
			TCNT2 = 0; //kasowanie Timera2
			
			if(sysClock1.opoznienie > 250)
			sysClock1.cykl = 250;
			else
			sysClock1.cykl = sysClock1.opoznienie;

			sysClock_exe = 1;
			sysClock1.komparator = sysClock1.cykl;
			OCR2A = sysClock1.cykl; //wartosc dla komparatora licznika T2
			TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
		}
		break;


		case 2: //przerwanie dla delay2
		sysClock2.opoznienie = sysClock2.opoznienie - sysClock2.cykl;
		
		if((sysClock2.opoznienie == 0) && (sysClock2.praca == 1))  //licznik dla delay1 zakonczyl prace
		{
			sysClock2.praca = 0;
			*sysClock2.aktywacja = 1;
			if(sysClock1.praca == 1)	//jesli pracuje jeszcze licznik dla delay1
			{
				sysClock_exe = 1;
				OCR2A = sysClock1.komparator; //wartosc dla komparatora licznika T2
				TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
			}
			else
			{
				TCNT2 = 0; //kasowanie Timera2
				sysClock_exe = 0;
			}
		}

		if((sysClock2.praca == 1) && (sysClock1.praca == 0))  //jesli nie pracowa³ licznik dla delay2
		{
			TCNT2 = 0; //kasowanie Timera2
			
			if(sysClock2.opoznienie > 250)
			sysClock2.cykl = 250;
			else
			sysClock2.cykl = sysClock2.opoznienie;

			sysClock_exe = 2;
			sysClock2.komparator = sysClock2.cykl;
			OCR2A = sysClock2.cykl; //wartosc dla komparatora licznika T2
			TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
		}
		break;


		case 3:	//przerwanie dla delay1 i delay2
		sysClock1.opoznienie = sysClock1.opoznienie - sysClock1.cykl;
		sysClock2.opoznienie = sysClock2.opoznienie - sysClock2.cykl;

		if((sysClock1.opoznienie == 0) && (sysClock1.praca == 1))  //licznik dla delay1 zakonczyl prace
		{
			sysClock1.praca = 0;
			*sysClock1.aktywacja = 1;
		}

		if((sysClock2.opoznienie == 0) && (sysClock2.praca == 1))  //licznik dla delay2 zakonczyl prace
		{
			sysClock2.praca = 0;
			*sysClock2.aktywacja = 1;
		}

		if((sysClock1.praca == 0) && (sysClock2.praca == 0))  //koniec pracy
		{
			TCNT2 = 0; //kasowanie Timera2
			sysClock_exe = 0;
		}

		if((sysClock1.praca == 1) && (sysClock2.praca == 1))
		{
			if(sysClock1.opoznienie > 250)
			sysClock1.cykl = 250;
			else
			sysClock1.cykl = sysClock1.opoznienie;

			if(sysClock2.opoznienie > 250)
			sysClock2.cykl = 250;
			else
			sysClock2.cykl = sysClock2.opoznienie;

			if(sysClock1.cykl == sysClock2.cykl)
			{
				sysClock_exe = 3;
				sysClock1.komparator = sysClock1.cykl;
				sysClock2.komparator = sysClock2.cykl;
				OCR2A = sysClock1.cykl; //wartosc dla komparatora licznika T2
				TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
			}

			if(sysClock1.cykl > sysClock2.cykl)
			{
				sysClock_exe = 2;
				sysClock2.komparator = sysClock2.cykl;
				OCR2A = sysClock2.cykl; //wartosc dla komparatora licznika T2
				TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
			}

			if(sysClock1.cykl < sysClock2.cykl)
			{
				sysClock_exe = 1;
				sysClock1.komparator = sysClock1.cykl;
				OCR2A = sysClock1.cykl; //wartosc dla komparatora licznika T2
				TCCR2B = (1 << CS21) | (1 << CS20);  //uruchomienie T2 dzielna 32
			}
		}
		break;
	}
	//sei();
}
*/