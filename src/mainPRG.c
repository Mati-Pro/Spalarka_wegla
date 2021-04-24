/*
 * mainPRG.c
 *
 * Created: 17.04.2021 12:23:09
 *  Author: Pawel
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "mainPRG.h"
#include "sysTWI.h"
#include "sysLCD.h"
#include "sysEnc.h"
#include "sys1Wire.h"
#include "sysSPI.h"
#include "_DEBUG.h"

extern uint8_t SysStatus;
extern uint8_t RTC_odczyt;
extern uint8_t RTC_reg[8];
extern uint8_t Enc_kierunek;

extern struct Nas_temp Temp_CWU;
extern struct Nas_temp Temp_CO;
extern struct Nas_temp Temp_Mx;

extern struct _HL_type OneWire_SENSOR[5];
extern struct OneWire_st OneWire_STATUS;

struct zegar RTC_zegar;
struct data RTC_data;
struct men Menu = {0};

const char* dniTygodnia[] = {"PN", "WT", "SR", "CZ", "PT", "SO", "ND"};
const char* Menu_poz[] = {"Wyjscie        ","Temp. CWU      ","Temp. kotla    ","Temp. mieszacza","Godzina        ","Data           "};


void Ekran_glowny(void)
{
	LCD_row2[1] = 0x53; LCD_row2[12] = 0x43;	//S C
	LCD_row3[0] = 0x7F; LCD_row3[1] = 0x4B;  LCD_row3[12] = 0x47;	//<K	G
	LCD_row4[0] = 0x7E; LCD_row4[1] = 0x4B;  LCD_row4[12] = 0x50;	//>K	P
	OneWire_wyswietl_pomiar(0, LCD_row2, 16, &(Temp_CWU.war), 0, 0);
	OneWire_wyswietl_pomiar(0, LCD_row3, 7, &(Temp_CO.war), 0, 0);
	OneWire_wyswietl_pomiar(0, LCD_row3, 16, &(Temp_Mx.war), 0, 0);
}


void Program_glowny(void)
{
	if (RTC_odczyt == 6)	//RTC_odczyt = 2 - bylo przerwanie od SQ uk³adu RTC -> wywolanie odczytu ukladu RTC
	{
		RTC_odczyt = 7;
		TWI_access(RTC_ADRES, 2, 0, RTC_reg, 6, &RTC_odczyt);
	}
	
	
	if(RTC_odczyt == 8)	//Odczytano uklad RTC, dane w tabeli RTC_reg[]
	{
		RTC_odczyt = 5;
		RTC_konwertuj();
		
		if(Menu.aktywacja == 0)
		{
			RTC_kalk_Time(LCD_row1, 0);
			LCD_print(1, 0, 11, LCD_row1);
		}
		
		if((Menu.aktywacja & 0x02) && (Menu.pozTab == 4))	//wyswietlanie jesli jestes w ustaweniach godziny
		{
			RTC_kalk_Time(LCD_row2, 4);
			LCD_print(2, 4, 11, LCD_row2);
		}
	}
	
	
	if (Menu.aktywacja == 0)
	{
		//wejscie do Menu
		if (Enc_kierunek == 4) //jesli wcisnieto SELECT i nie jestes w MENU (aktywacja MENU)
		{
			Enc_kierunek = 0;
			Menu.pozEkranu = 1;
			Menu.pozWys = 0;
			Menu.pozTab = 0;
			Menu.aktywacja  = 0x01;	//Menu aktywne
			Menu.aktywacja |= 0x04;	//drukuj raz
			LCD_clear_bufor();
		}
	}
	

	if((Menu.aktywacja & 0x01))		//MENU Glownym, ale nie Ustawienia
	{
		if((Menu.aktywacja & 0x04))	//drukujRaz
		{
			Menu.aktywacja &= ~(0x04);	//drukujRaz = false;
			LCD_row1[8] = 0x4D; LCD_row1[9] = 0x45; LCD_row1[10] = 0x4E; LCD_row1[11] = 0x55;	//napis MENU
			
			for(uint8_t i=0; i < 15; i++)
			{
				LCD_row2[3+i] = *(Menu_poz[Menu.pozWys]+i);
				LCD_row3[3+i] = *(Menu_poz[Menu.pozWys+1]+i);
				LCD_row4[3+i] = *(Menu_poz[Menu.pozWys+2]+i);
			}
			
			switch (Menu.pozEkranu)	//Pozycja strzalki
			{
				case 1:
				LCD_row2[1] = 0x7E;
				LCD_row3[1] = 0x20;
				LCD_row4[1] = 0x20;
				break;
				
				case 2:
				LCD_row2[1] = 0x20;
				LCD_row3[1] = 0x7E;
				LCD_row4[1] = 0x20;
				break;
				
				case 3:
				LCD_row2[1] = 0x20;
				LCD_row3[1] = 0x20;
				LCD_row4[1] = 0x7E;
				break;
			}
			LCD_wyslij_bufor();
		}
		
		switch (Enc_kierunek)
		{
			case 1:   //przewijanie pozycji Menu w lewo
			Enc_kierunek = 0;
			
			if((Menu.pozEkranu == 1) && (Menu.pozWys > 0))
			Menu.pozWys--;
			
			if(Menu.pozEkranu > 1)
			Menu.pozEkranu--;
			
			Menu.aktywacja |= 0x04;	//drukuj raz
			break;
			
			case 2:    //przewijanie pozycji Menu w prawo
			Enc_kierunek = 0;
			
			if((Menu.pozEkranu == 3) && (Menu.pozWys < 3))
			Menu.pozWys++;
			
			if(Menu.pozEkranu < 3)
			Menu.pozEkranu++;
			
			Menu.aktywacja |= 0x04;	//drukuj raz
			break;

			case 4:   //wejscie do pozycji Menu
			Enc_kierunek = 0;
			Menu.pozTab = Menu.pozWys + Menu.pozEkranu -1;
			Menu.aktywacja  = 0x02;
			Menu.aktywacja |= 0x04;
			Menu.pozUstaw = 1;
			LCD_clear_bufor();
			break;
		}
	}

	if (Menu.aktywacja & 0x02) //jesli jestes w MENU w ustawieniach wewnetrznych
	{
		if ((Menu.aktywacja & 0x04) && (Menu.pozTab > 0))
		{
			for(uint8_t i=0; i < 15; i++)
			LCD_row1[i] = *(Menu_poz[Menu.pozTab]+i);
		}
		
		switch (Menu.pozTab)  {
			case 1:   //ustawiene temperatury CWU
			Menu_tempX(Menu.pozTab,  9, &OneWire_SENSOR[2], OneWire_STATUS.przeterminowany & 0x04, &Temp_CWU);
			break;

			case 2:   //ustawiene temperatury Kotla
			Menu_tempX(Menu.pozTab, 11, &OneWire_SENSOR[0], OneWire_STATUS.przeterminowany & 0x01, &Temp_CO);
			break;

			case 3:   //ustawiene temperatury Mieszczacza
			Menu_tempX(Menu.pozTab, 11, &OneWire_SENSOR[3], OneWire_STATUS.przeterminowany & 0x20, &Temp_Mx);
			break;

			case 4:   //ustawiene godziny
			Menu_godzina();
			break;

			case 5:   //ustawienie daty
			Menu_data();
			break;

			case 0:   //wyjscie z MENU (jesli wybrano WYJSCIE)
			Menu.aktywacja  = 0;	//Menu nieaktywne
			LCD_clear_bufor();
			RTC_kalk_Time(LCD_row1, 0);
			Ekran_glowny();
			OneWire_wyswietl_pomiar(1, LCD_row3, 2,  &OneWire_SENSOR[0].H, &OneWire_SENSOR[0].L, OneWire_STATUS.przeterminowany & 0x01);	//kociol zasilanie
			OneWire_wyswietl_pomiar(1, LCD_row4, 2,  &OneWire_SENSOR[1].H, &OneWire_SENSOR[1].L, OneWire_STATUS.przeterminowany & 0x02);	//kociol powrot
			OneWire_wyswietl_pomiar(0, LCD_row2, 13, &OneWire_SENSOR[2].H, &OneWire_SENSOR[2].L, OneWire_STATUS.przeterminowany & 0x04);	//CWU
			OneWire_wyswietl_pomiar(0, LCD_row3, 13, &OneWire_SENSOR[3].H, &OneWire_SENSOR[3].L, OneWire_STATUS.przeterminowany & 0x20);	//grzejniki zasilanie
			OneWire_wyswietl_pomiar(0, LCD_row4, 13, &OneWire_SENSOR[4].H, &OneWire_SENSOR[4].L, OneWire_STATUS.przeterminowany & 0x40);	//podlogowka
			SPI_wyswietl_pomiar();	//spaliny
			LCD_wyslij_bufor();
			break;
		}
	}
}


void Menu_tempX(uint8_t poz_tab, uint8_t poz_pom, struct _HL_type *sensor, uint8_t przetermin, struct Nas_temp *tempX)
{
	if (Menu.aktywacja & 0x04)
	{
		Menu.aktywacja &= ~(0x04);	//drukujRaz = false;
		
		LCD_row1[poz_pom] = 0x20; LCD_row1[poz_pom+1] = 0x3D;	// = temp
		OneWire_wyswietl_pomiar(1, LCD_row1, poz_pom+2, &(sensor->H), &(sensor->L), przetermin);	//CWU
		LCD_row2[3] = 0x54; LCD_row2[5] = 0x3D;	//T =
		OneWire_wyswietl_pomiar(0, LCD_row2, 6, &(tempX->war), 0, 0);
		LCD_row3[3] = 0x68; LCD_row3[5] = 0x3D;	//h =
		OneWire_wyswietl_pomiar(2, LCD_row3, 6, &(tempX->his), 0, 0);
		LCD_row4[3] = 0x57; LCD_row4[4] = 0x79; LCD_row4[5] = 0x6A; LCD_row4[6] = 0x73;
		LCD_row4[7] = 0x63; LCD_row4[8] = 0x69; LCD_row4[9] = 0x65;	//Wyjscie
		
		switch (Menu.pozUstaw & 0x03)
		{
			case 2:
			if(Menu.pozUstaw & 0x04)
			{
				LCD_row2[1] = 0x20;
				LCD_row2[10] = 0x7F;
			}
			else
			{
				LCD_row2[10] = 0x20;
				LCD_row2[1] = 0x7E;
				LCD_row3[1] = 0x20;
				LCD_row4[1] = 0x20;
			}
			break;
			
			case 3:
			if(Menu.pozUstaw & 0x04)
			{
				LCD_row3[1] = 0x20;
				LCD_row3[10] = 0x7F;
			}
			else
			{
				LCD_row3[10] = 0x20;
				LCD_row2[1] = 0x20;
				LCD_row3[1] = 0x7E;
				LCD_row4[1] = 0x20;
			}
			break;
			
			case 1:
			LCD_row2[1] = 0x20;
			LCD_row3[1] = 0x20;
			LCD_row4[1] = 0x7E;
			break;
		}
		LCD_wyslij_bufor();
	}
	
	switch (Enc_kierunek)
	{
		case 1:   //przewijanie pozycji Menu Ustawien w lewo
		Enc_kierunek = 0;
		if(Menu.pozUstaw & 0x04)
		{
			if((tempX->war > 1) && ((Menu.pozUstaw & 0x03) == 2))
			tempX->war--;
			
			if((tempX->his > 1) && ((Menu.pozUstaw & 0x03) == 3))
			tempX->his--;
		}
		else
		{
			if(Menu.pozUstaw > 1)
			Menu.pozUstaw--;
			else
			Menu.pozUstaw = 3;
		}
		Menu.aktywacja |= 0x04;	//drukuj raz
		break;
		
		case 2:    //przewijanie pozycji Menu w prawo
		Enc_kierunek = 0;
		if(Menu.pozUstaw & 0x04)
		{
			if((tempX->war < 99) && ((Menu.pozUstaw & 0x03) == 2))
			tempX->war++;
			
			if((tempX->his < 10) && ((Menu.pozUstaw & 0x03) == 3))
			tempX->his++;
		}
		else
		{
			if(Menu.pozUstaw < 3)
			Menu.pozUstaw++;
			else
			Menu.pozUstaw = 1;
		}
		
		Menu.aktywacja |= 0x04;	//drukuj raz
		break;

		case 4:   //wejscie do pozycji Menu
		Enc_kierunek = 0;
		
		if((Menu.pozUstaw & 0x03) == 1)
		{
			RTC_reg[0] = tempX->war;	//zapis wartosci do pamieci RTC_RAM
			RTC_reg[1] = tempX->his;
			TWI_access(RTC_ADRES, 4, 6+(poz_tab*2), RTC_reg, 1, 0);	//RAM od rejestru 0x08 RTC
			
			Menu.aktywacja = 0x01;
			LCD_clear_bufor();
		}
		else
		{
			if(Menu.pozUstaw & 0x04)
			Menu.pozUstaw &= ~0x04;
			else
			Menu.pozUstaw |= 0x04;
		}
		Menu.aktywacja |= 0x04;
		break;
	}
}



void Menu_godzina(void)
{
	if (Menu.aktywacja & 0x04)
	{
		Menu.aktywacja &= ~(0x04);	//drukujRaz = false;
		RTC_kalk_Time(LCD_row2, 4);
		
		LCD_row4[4] = 0x57; LCD_row4[5] = 0x79; LCD_row4[6] = 0x6A; LCD_row4[7] = 0x73;
		LCD_row4[8] = 0x63; LCD_row4[9] = 0x69; LCD_row4[10] = 0x65;	//Wyjscie
		
		switch (Menu.pozUstaw & 0x07)
		{
			case 2:	//Strzalka lub gwiazdka dniu tygodnia
			if(Menu.pozUstaw & 0x08)
			{
				LCD_row3[5] = 0x2A;
			}
			else
			{
				LCD_row3[5] = 0x5E;
				LCD_row3[8] = 0x20;
				LCD_row4[2] = 0x20;
			}
			break;
			
			case 3:	//Strzalka lub gwiazdka na godzinie
			if(Menu.pozUstaw & 0x08)
			{
				LCD_row3[8] = 0x2A;
			}
			else
			{
				LCD_row3[8] = 0x5E;
				LCD_row3[11] = 0x20;
				LCD_row3[5] = 0x20;
			}
			break;
			
			case 4:	//Strzalka lub gwiazdka na minucie
			if(Menu.pozUstaw & 0x08)
			{
				LCD_row3[11] = 0x2A;
			}
			else
			{
				LCD_row3[11] = 0x5E;
				LCD_row3[14] = 0x20;
				LCD_row3[8] = 0x20;
			}
			break;
			
			case 5: //Strzalka na sekundzie
			LCD_row3[14] = 0x5E;
			LCD_row4[2] = 0x20;
			LCD_row3[11] = 0x20;
			break;
			
			case 1:
			LCD_row4[2] = 0x7E;
			LCD_row3[5] = 0x20;
			LCD_row3[14] = 0x20;
			break;
		}
		LCD_wyslij_bufor();
	}
	
	switch (Enc_kierunek)
	{
		case 1:   //przewijanie pozycji Menu Ustawien w lewo
		Enc_kierunek = 0;
		if(Menu.pozUstaw & 0x08)
		{
			if((Menu.pozUstaw & 0x07) == 2)	//ustawianie dnia tygodnia
			{
				if(RTC_data.nrDnia > 1)
				RTC_data.nrDnia--;
				else
				RTC_data.nrDnia = 7;
				
				RTC_reg[0] = RTC_data.nrDnia;
				TWI_access(RTC_ADRES, 4, 3, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 3)	//ustawianie godziny
			{
				if(RTC_zegar.godzina.j > 0)
				RTC_zegar.godzina.j--;
				else
				{
					RTC_zegar.godzina.j = 9;
					if(RTC_zegar.godzina.d > 0)
					RTC_zegar.godzina.d--;
					else
					{
						RTC_zegar.godzina.d = 2;
						RTC_zegar.godzina.j = 3;
					}
				}
				RTC_reg[0] = ((RTC_zegar.godzina.d & 0x03) << 4) | RTC_zegar.godzina.j;
				TWI_access(RTC_ADRES, 4, 2, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 4)	//ustawianie mniut
			{
				if(RTC_zegar.minuta.j > 0)
				RTC_zegar.minuta.j--;
				else
				{
					RTC_zegar.minuta.j = 9;
					if(RTC_zegar.minuta.d > 0)
					RTC_zegar.minuta.d--;
					else
					RTC_zegar.minuta.d = 5;
				}
				
				RTC_reg[0] = (RTC_zegar.minuta.d << 4) | RTC_zegar.minuta.j;
				TWI_access(RTC_ADRES, 4, 1, RTC_reg, 0, 0);
			}
			/*
			if((Menu.pozUstaw & 0x07) == 5)	//kasowanie sekund
			{
				RTC_zegar.sekunda.j = 0;
				RTC_zegar.sekunda.d = 0;
				RTC_reg[0] = 0;
				TWI_access(RTC_ADRES, 4, 0, RTC_reg, 0, 0);
			}*/
		}
		else
		{
			if(Menu.pozUstaw > 1)
			Menu.pozUstaw--;
			else
			Menu.pozUstaw = 5;
		}
		Menu.aktywacja |= 0x04;	//drukuj raz
		break;
		
		case 2:    //przewijanie pozycji Menu w prawo
		Enc_kierunek = 0;
		if(Menu.pozUstaw & 0x08)
		{
			if((Menu.pozUstaw & 0x07) == 2)		//ustawianie dnia tygodnia
			{
				if(RTC_data.nrDnia < 7)
				RTC_data.nrDnia++;
				else
				RTC_data.nrDnia = 1;
				
				RTC_reg[0] = RTC_data.nrDnia;
				TWI_access(RTC_ADRES, 4, 3, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 3)	//ustawianie godziny
			{
				if((RTC_zegar.godzina.d == 2) && (RTC_zegar.godzina.j == 3))
				{
					RTC_zegar.godzina.d = 0;
					RTC_zegar.godzina.j = 0;
				}
				else
				{
					if(RTC_zegar.godzina.j < 9)
					RTC_zegar.godzina.j++;
					else
					{
						RTC_zegar.godzina.j = 0;
						RTC_zegar.godzina.d++;
					}
				}
				RTC_reg[0] = ((RTC_zegar.godzina.d & 0x03) << 4) | RTC_zegar.godzina.j;
				TWI_access(RTC_ADRES, 4, 2, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 4)	//ustawianie minut
			{
				if(RTC_zegar.minuta.j < 9)
				RTC_zegar.minuta.j++;
				else
				{
					RTC_zegar.minuta.j = 0;
					if(RTC_zegar.minuta.d < 5)
					RTC_zegar.minuta.d++;
					else
					RTC_zegar.minuta.d = 0;
				}
				RTC_reg[0] = (RTC_zegar.minuta.d << 4) | RTC_zegar.minuta.j;
				TWI_access(RTC_ADRES, 4, 1, RTC_reg, 0, 0);
			}
			/*
			if((Menu.pozUstaw & 0x07) == 5)	//kasowanie sekund
			{
				RTC_zegar.sekunda.j = 0;
				RTC_zegar.sekunda.d = 0;
				RTC_reg[0] = 0;
				TWI_access(RTC_ADRES, 4, 0, RTC_reg, 0, 0);
			}*/
		}
		else
		{
			if(Menu.pozUstaw < 5)
			Menu.pozUstaw++;
			else
			Menu.pozUstaw = 1;
		}
		Menu.aktywacja |= 0x04;	//drukuj raz
		break;
		
		case 4:   //wejscie do pozycji Menu
		Enc_kierunek = 0;
		
		if((Menu.pozUstaw & 0x07) == 1)	//wyjscie z ustawien godziny
		{
			Menu.aktywacja = 0x01;
			LCD_clear_bufor();
		}
		
		if((Menu.pozUstaw & 0x07) == 5)	//kasowanie sekund
		{
			RTC_zegar.sekunda.j = 0;
			RTC_zegar.sekunda.d = 0;
			RTC_reg[0] = 0;
			TWI_access(RTC_ADRES, 4, 0, RTC_reg, 0, 0);
		}
		else
		{
			Menu.pozUstaw ^= 0x08;
			/*
			if(Menu.pozUstaw & 0x08)
			Menu.pozUstaw &= ~0x08;
			else
			Menu.pozUstaw |= 0x08;
			*/
		}

		Menu.aktywacja |= 0x04;
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////
//
//	Procedura ustawiania daty
//
///////////////////////////////////////////////////////////////////////////////////////

void Menu_data(void)
{
	if (Menu.aktywacja & 0x04)
	{
		Menu.aktywacja &= ~(0x04);	//drukujRaz = false;
		RTC_kalk_Data(LCD_row2, 4);
		
		LCD_row4[4] = 0x57; LCD_row4[5] = 0x79; LCD_row4[6] = 0x6A; LCD_row4[7] = 0x73;
		LCD_row4[8] = 0x63; LCD_row4[9] = 0x69; LCD_row4[10] = 0x65;	//Wyjscie
		
		switch (Menu.pozUstaw & 0x07)
		{
			case 2:
			if(Menu.pozUstaw & 0x08)
			{
				LCD_row3[7] = 0x2A;
			}
			else
			{
				LCD_row3[7] = 0x5E;
				LCD_row3[10] = 0x20;
				LCD_row4[2] = 0x20;
			}
			break;
			
			case 3:
			if(Menu.pozUstaw & 0x08)
			{
				LCD_row3[10] = 0x2A;
			}
			else
			{
				LCD_row3[10] = 0x5E;
				LCD_row3[13] = 0x20;
				LCD_row3[7] = 0x20;
			}
			break;
			
			case 4:
			if(Menu.pozUstaw & 0x08)
			{
				LCD_row3[13] = 0x2A;
			}
			else
			{
				LCD_row3[13] = 0x5E;
				LCD_row4[2] = 0x20;
				LCD_row3[10] = 0x20;
			}
			break;
			
			case 1:
			LCD_row4[2] = 0x7E;
			LCD_row3[7] = 0x20;
			LCD_row3[13] = 0x20;
			break;
		}
		LCD_wyslij_bufor();
	}
	
	switch (Enc_kierunek)
	{
		case 1:   //przewijanie pozycji Menu Ustawien w lewo
		Enc_kierunek = 0;
		if(Menu.pozUstaw & 0x08)
		{
			if((Menu.pozUstaw & 0x07) == 2)	//ustawianie roku
			{
				if(RTC_data.rok.j > 0)
				RTC_data.rok.j--;
				else
				{
					RTC_data.rok.j = 9;
					
					if(RTC_data.rok.d > 0)
					RTC_data.rok.d--;
					else
					RTC_data.rok.d = 9;
				}
				RTC_reg[0] = (RTC_data.rok.d << 4) | RTC_data.rok.j;
				TWI_access(RTC_ADRES, 4, 6, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 3)	//ustawianie miesiaca
			{
				if((RTC_data.miesiac.d == 0) && (RTC_data.miesiac.j <= 1))
				{
					RTC_data.miesiac.d = 1;
					RTC_data.miesiac.j = 2;
				}
				else
				{
					if(RTC_data.miesiac.j > 0)
					RTC_data.miesiac.j--;
					else
					{
						RTC_data.miesiac.j = 9;
						RTC_data.miesiac.d--;
					}
				}
				RTC_reg[0] = (RTC_data.miesiac.d << 4) | RTC_data.miesiac.j;
				TWI_access(RTC_ADRES, 4, 5, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 4)	//ustawianie dnia
			{
				if((RTC_data.dzien.d == 0) && (RTC_data.dzien.j <= 1))
				{
					RTC_data.dzien.d = ileDniMiesiaca()/10;
					RTC_data.dzien.j = ileDniMiesiaca() % 10;
				}
				else
				{
					if(RTC_data.dzien.j > 0)
					RTC_data.dzien.j--;
					else
					{
						RTC_data.dzien.j = 9;
						RTC_data.dzien.d--;
					}
				}
				RTC_reg[0] = (RTC_data.dzien.d << 4) | RTC_data.dzien.j;
				TWI_access(RTC_ADRES, 4, 4, RTC_reg, 0, 0);
			}
		}
		else
		{
			if(Menu.pozUstaw > 1)
			Menu.pozUstaw--;
			else
			Menu.pozUstaw = 4;
		}
		Menu.aktywacja |= 0x04;	//drukuj raz
		break;
		
		case 2:    //przewijanie pozycji Menu w prawo
		Enc_kierunek = 0;
		if(Menu.pozUstaw & 0x08)
		{
			if((Menu.pozUstaw & 0x07) == 2)	//ustawianie roku
			{
				if(RTC_data.rok.j < 9)
				RTC_data.rok.j++;
				else
				{
					RTC_data.rok.j = 0;
					
					if(RTC_data.rok.d < 9)
					RTC_data.rok.d++;
					else
					RTC_data.rok.d = 0;
				}
				RTC_reg[0] = (RTC_data.rok.d << 4) | RTC_data.rok.j;
				TWI_access(RTC_ADRES, 4, 6, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 3)	//ustawianie miesiaca
			{
				if((RTC_data.miesiac.d == 1) && (RTC_data.miesiac.j) >= 2)
				{
					RTC_data.miesiac.d = 0;
					RTC_data.miesiac.j = 1;
				}
				else
				{
					if(RTC_data.miesiac.j < 9)
					RTC_data.miesiac.j++;
					else
					{
						RTC_data.miesiac.j = 0;
						RTC_data.miesiac.d++;
					}
				}
				RTC_reg[0] = (RTC_data.miesiac.d << 4) | RTC_data.miesiac.j;
				TWI_access(RTC_ADRES, 4, 5, RTC_reg, 0, 0);
			}
			
			if((Menu.pozUstaw & 0x07) == 4)	//ustawianie dnia
			{
				if((RTC_data.dzien.d*10 + RTC_data.dzien.j) >= ileDniMiesiaca())
				{
					RTC_data.dzien.d = 0;
					RTC_data.dzien.j = 1;
				}
				else
				{
					if(RTC_data.dzien.j < 9)
					RTC_data.dzien.j++;
					else
					{
						RTC_data.dzien.j = 0;
						RTC_data.dzien.d++;
					}
				}
				RTC_reg[0] = (RTC_data.dzien.d << 4) | RTC_data.dzien.j;
				TWI_access(RTC_ADRES, 4, 4, RTC_reg, 0, 0);
			}
		}
		else
		{
			if(Menu.pozUstaw < 4)
			Menu.pozUstaw++;
			else
			Menu.pozUstaw = 1;
		}
		Menu.aktywacja |= 0x04;	//drukuj raz
		break;
		
		case 4:   //wejscie do pozycji Menu
		Enc_kierunek = 0;
		
		if((Menu.pozUstaw & 0x07) == 1)	//wyjscie z ustawien godziny
		{
			Menu.aktywacja = 0x01;
			LCD_clear_bufor();
		}
		else
		{
			if(Menu.pozUstaw & 0x08)
			Menu.pozUstaw &= ~0x08;
			else
			Menu.pozUstaw |= 0x08;
		}

		Menu.aktywacja |= 0x04;
		break;
	}
}


void RTC_setup()
{
	//Inicjacja RTC
	if(RTC_odczyt == 0)
	{
		RTC_odczyt = 1;
		TWI_access(RTC_ADRES, 2, 0, RTC_reg, 7, &RTC_odczyt);
	}
	
	if(RTC_odczyt == 2)
	{
		if(RTC_reg[0] & 0x80)	//Sprawdzenie bitu CH
		{
			RTC_reg[0] &= 0x7F;
			TWI_access(RTC_ADRES, 4, 0, RTC_reg, 0, 0);
		}
		
		if(RTC_reg[2] & 0x40)	//Sprawdzenie bitu 12/24
		{
			RTC_reg[0] = RTC_reg[2] & 0x3F;
			TWI_access(RTC_ADRES, 4, 2, RTC_reg, 0, 0);
		}
		
		if((RTC_reg[7] & 0x10) == 0)	//uruhomienie wyjscia SQ w RTC
		{
			RTC_reg[0] = 0x10;
			TWI_access(RTC_ADRES, 4, 7, RTC_reg, 0, 0);
		}
		RTC_odczyt = 3;
		TWI_access(RTC_ADRES, 2, 8, RTC_reg, 5, &RTC_odczyt);	//odczyt pamieci RTC_RAM z parametrami
	}
	
	if(RTC_odczyt == 4)
	{
		Temp_CWU.war = RTC_reg[0];
		Temp_CWU.his = RTC_reg[1];
		Temp_CO.war = RTC_reg[2];
		Temp_CO.his = RTC_reg[3];
		Temp_Mx.war = RTC_reg[4];
		Temp_Mx.his = RTC_reg[5];
		RTC_odczyt = 5;
		SysStatus |= 2;
		EIMSK = (1 << INT0);	//uruchomienie przewania dla INT0
	}
}


void RTC_konwertuj(void)
{
	RTC_zegar.sekunda.j = (RTC_reg[0] & 0x0f);
	RTC_zegar.sekunda.d = ((RTC_reg[0] & 0xf0) >> 4);
	
	RTC_zegar.minuta.j = (RTC_reg[1] & 0x0f);
	RTC_zegar.minuta.d = ((RTC_reg[1] & 0xf0) >> 4);

	RTC_zegar.godzina.j = (RTC_reg[2] & 0x0f);
	RTC_zegar.godzina.d = ((RTC_reg[2] & 0x30) >> 4);

	RTC_data.dzien.j = (RTC_reg[4] & 0x0f);
	RTC_data.dzien.d = ((RTC_reg[4] & 0xf0) >> 4);

	RTC_data.miesiac.j = (RTC_reg[5] & 0x0f);
	RTC_data.miesiac.d = ((RTC_reg[5] & 0xf0) >> 4);

	RTC_data.rok.j = (RTC_reg[6] & 0x0f);
	RTC_data.rok.d = ((RTC_reg[6] & 0xf0) >> 4);
	
	RTC_data.nrDnia = RTC_reg[3] & 0x0f;
}


void RTC_kalk_Time(char *tab, uint8_t poz)
{
	tab[poz] = *dniTygodnia[RTC_data.nrDnia-1];
	tab[poz+1] = *(dniTygodnia[RTC_data.nrDnia-1]+1);
	
	tab[poz+3] = RTC_zegar.godzina.d + 0x30;
	tab[poz+4] = RTC_zegar.godzina.j + 0x30;
	tab[poz+5] = 0x3A;
	tab[poz+6] = RTC_zegar.minuta.d + 0x30;
	tab[poz+7] = RTC_zegar.minuta.j + 0x30;
	tab[poz+8] = 0x3A;
	tab[poz+9] = RTC_zegar.sekunda.d + 0x30;
	tab[poz+10] = RTC_zegar.sekunda.j + 0x30;
}


void RTC_kalk_Data(char *tab, uint8_t poz)
{
	tab[poz] = 0x32;
	tab[poz+1] = 0x30;
	tab[poz+2] = RTC_data.rok.d + 0x30;
	tab[poz+3] = RTC_data.rok.j + 0x30;
	tab[poz+4] = 0x2E;
	tab[poz+5] = RTC_data.miesiac.d + 0x30;
	tab[poz+6] = RTC_data.miesiac.j + 0x30;
	tab[poz+7] = 0x2E;
	tab[poz+8] = RTC_data.dzien.d + 0x30;
	tab[poz+9] = RTC_data.dzien.j + 0x30;
}


/*------------------czy rok przestepny i ile dni ma dany miesiac--------------------------------*/
uint8_t ileDniMiesiaca(void)
{
	uint16_t rok;
	uint8_t miesiac;

	miesiac = RTC_data.miesiac.d * 10 + RTC_data.miesiac.j;

	switch (miesiac)  {
		case 1: case 3: case 5: case 7: case 8: case 10:  case 12:
		return 31;
		break;

		case 4: case 6: case 9: case 11:
		return 30;
		break;

		case 2:
		rok = 2000 + RTC_data.rok.d * 10 + RTC_data.rok.j;
		if (((rok % 4 == 0) && (rok % 100 != 0)) || (rok % 400 == 0))
		return 29; // badamy czy rok jest rokiem przestepnym
		else
		return 28;
		break;
		
		default:
		return 30;
		break;
	}
}