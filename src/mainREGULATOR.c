/*
 * mainREGULATOR.c
 *
 * Created: 24.04.2021 23:37:10
 *  Author: Pawel Rogoz
 */ 

#include "mainREGULATOR.h"
#include "macros.h"
#include "sys1Wire.h"
#include "sysTWI.h"
#include "sysClock.h"
#include "USART.h"

extern struct _HL_type OneWire_CZUJNIK[5];
extern struct OneWire_st OneWire_STATUS;

struct Nas_temp Temp_CWU;
struct Nas_temp Temp_CO;
struct Nas_temp Temp_Mx;

struct Param_spal	Podawanie; 
struct Param_spal	Podtrzymanie;

struct Param_spal	Podawanie_tmp;
struct Param_spal	Podtrzymanie_tmp;

struct status	Podajnik;
	
uint8_t IO_control = 0;
uint8_t IO_control_tmp = 0;
uint8_t	CO_status = 0;



void Regulator_setup(void)
{
	
	Podajnik.status = 0;
	Podajnik.active = 0;
	
	Podawanie.podawanie.sekundy = 2;
	Podawanie.podawanie.msekundy = 0;
	
	Podawanie.przerwa.sekundy = 4;
	
	Podtrzymanie.podawanie.sekundy = 10;
	
	Podtrzymanie.przerwa.minuty = 1;
	
	//ustawienie pinow dla podajnika i wentylatora
}


void Regulator_handling(void)
{
		
	//Pompa - regulator CWU - ON/OFF
	//############################################################################################################
	if(!(OneWire_STATUS.przeterminowany & 0x04))	//pomiar nie przeterminowany
	{
		if((OneWire_CZUJNIK[2].H * 10 + OneWire_CZUJNIK[2].L) >= Temp_CWU.war * 10)		//OFF
		clear_bit(IO_control_tmp, Pompa_CWU);
		
		if((OneWire_CZUJNIK[2].H < Temp_CWU.war - Temp_CWU.his) && (CO_status & 0x04))	//ON jeœli 50st osiagniete
		set_bit(IO_control_tmp, Pompa_CWU);
	}
	else
	clear_bit(IO_control_tmp, Pompa_CWU);
	
	/*
	//Podajnik - regulator CO - ON/OFF
	// ****************************************************************************************
	if(!(OneWire_STATUS.przeterminowany & 0x01))	//pomiar nie przeterminowany
	{
		//Zezwolenie 50st ********************************************************************
		if(!(CO_status & 0x04))	// nie by³o 50st
			if(((OneWire_CZUJNIK[0].H * 10) + OneWire_CZUJNIK[0].L) >= (50 * 10))	//ON CO_50st
			{
				set_bit(CO_status, CO_50st);
			}
		
		//Zezwolenie 55st ********************************************************************
		if(!(CO_status & 0x08))	// nie by³o 55st
			if(((OneWire_CZUJNIK[0].H * 10) + OneWire_CZUJNIK[0].L) >= (55 * 10))	//ON CO_50st
			{
				set_bit(CO_status, CO_55st);
				set_bit(IO_control_tmp, Pompa_CO);
			}
		
		//Regulator podawania **************************************************************
		if(((OneWire_CZUJNIK[0].H * 10) + OneWire_CZUJNIK[0].L) >= (Temp_CO.war * 10))	//OFF
		{
			if(CO_status & 0x01)
			{
				clear_bit(CO_status, CO_pracaPodawanie);
				//wy³¹cz podajnik
				clear_bit(IO_control_tmp, ZesilaniePodajWent);
				//wylacz wentylator
				
				
				//printString("t11 >=  podawanie off pompa off");
				//printString("  \r\n");
			}
			
			if(!(CO_status & 0x02))
			{
				set_bit(CO_status, CO_pracaPodtrzymanie);
				Podajnik.status = 0;
				Podajnik.active = 0;
				
				//printString("t12 >=  podtrzymanie on");
				//printString("  \r\n");
			}
			
		}
		
		if(OneWire_CZUJNIK[0].H < (Temp_CO.war - Temp_CO.his))	//ON
		{
			//printString("t2");
			//printString("  \r\n");
			//printWord((OneWire_SENSOR[0].H * 10) + OneWire_SENSOR[0].L);
			//printString("  \r\n");
			
			if(!(CO_status & 0x01))
			{
				set_bit(CO_status, CO_pracaPodawanie);
				Podajnik.status = 0;
				Podajnik.active = 0;
				
				//printString("t21 <  podawanie on");
				//printString("  \r\n");
			}
			
			if(CO_status & 0x02)
			{
				clear_bit(CO_status, CO_pracaPodtrzymanie);
				
				//printString("t22 <  podtrzymanie off");
				//printString("  \r\n");
			}
		}
		
	}
	else
	{
		Podajnik.status = 0;
		Podajnik.active = 0;
		
		clear_bit(CO_status, CO_pracaPodawanie);	//OFF poniewaz pomiar przeterminowany
		clear_bit(CO_status, CO_pracaPodtrzymanie);	//OFF poniewaz pomiar przeterminowany
		//wy³aczenie podajnika
		clear_bit(IO_control_tmp, ZesilaniePodajWent);
		//wy³aczenie wentylatora
		
		//printString("t3   podawanie off podtrzymanie off pompa off");
		//printString("  \r\n");
	}
	
	
	if(CO_status & 0x01)	//CO pracuje kocio³ i podaje wegiel
	{
		switch (Podajnik.status)
		{	
			case 0:
			Podawanie_tmp.podawanie.sekundy = 0;
			Podawanie_tmp.podawanie.msekundy =	0;
			Podawanie_tmp.przerwa.sekundy =	0;
			Podajnik.status = 1;
			
			//printString("case 0");
			//printString("  \r\n");
			
			//zalaczamy podajnik
			set_bit(IO_control_tmp, ZesilaniePodajWent);
			Time_delay(100000, &Podajnik.active, 0);
			break;
			
			case 1:
			if(Podajnik.active == 1)
			{
				//printString("case 1");
				//printString("  \r\n");
				Podawanie_tmp.podawanie.msekundy +=	1;
				if(Podawanie_tmp.podawanie.msekundy >= 10)
				{
					Podawanie_tmp.podawanie.msekundy = 0;
					Podawanie_tmp.podawanie.sekundy += 1;
					//printString("case 11");
					//printString("  \r\n");
				}
				
				if((Podawanie_tmp.podawanie.msekundy >= Podawanie.podawanie.msekundy) && (Podawanie_tmp.podawanie.sekundy >= Podawanie.podawanie.sekundy))
				{
					Podajnik.status = 2;
					//printString("case 12");
					//printString("  \r\n");
					//wylaczamy podajnik
					clear_bit(IO_control_tmp, ZesilaniePodajWent);
					Time_delay(1000000,&Podajnik.active,0);
				}
				else
					Time_delay(100000,&Podajnik.active,0);
			}
			break;
			
			case 2:
			if(Podajnik.active == 1)
			{
				//printString("case 2");
				//printString("  \r\n");
				
				Podawanie_tmp.przerwa.sekundy += 1;
				if(Podawanie_tmp.przerwa.sekundy >= Podawanie.przerwa.sekundy)
				{
					Podajnik.status = 0;
					Podajnik.active = 0;
					//printString("case 21");
					//printString("  \r\n");
				}
				else
					Time_delay(1000000,&Podajnik.active,0);
			}
			break;
		}
	}
	
	
	if(CO_status & 0x02)	//CO przerwa
	{
		switch (Podajnik.status)
		{
			case 0:
			Podtrzymanie_tmp.podawanie.sekundy = 0;
			Podtrzymanie_tmp.przerwa.sekundy =	0;
			Podtrzymanie_tmp.przerwa.minuty =	0;
			Podajnik.status = 1;
			
			//wylacz podajnik
			clear_bit(IO_control_tmp, ZesilaniePodajWent);
			Time_delay(1000000, &Podajnik.active, 0);	//sekunda
			break;
			
			case 1:
			if(Podajnik.active == 1)
			{
				//printString("case 1");
				//printString("  \r\n");
				Podtrzymanie_tmp.przerwa.sekundy +=	1;
				if(Podtrzymanie_tmp.przerwa.sekundy >= 60)
				{
					Podtrzymanie_tmp.przerwa.sekundy = 0;
					Podtrzymanie_tmp.przerwa.minuty += 1;
				}
				
				if((Podtrzymanie_tmp.przerwa.minuty >= Podtrzymanie.przerwa.minuty) && (Podtrzymanie_tmp.przerwa.sekundy = 0))
				{
					Podajnik.status = 2;
					//printString("case 12");
					//printString("  \r\n");
					//w³acz podajnik
					set_bit(IO_control_tmp, ZesilaniePodajWent);
				}
				
				Time_delay(1000000,&Podajnik.active,0);	//sekunda
			}
			break;
			
			case 2:
			if(Podajnik.active == 1)
			{
				//printString("case 2");
				//printString("  \r\n");
				
				Podtrzymanie_tmp.podawanie.sekundy += 1;
				if(Podtrzymanie_tmp.podawanie.sekundy >= Podtrzymanie.podawanie.sekundy)
				{
					Podajnik.status = 0;
					Podajnik.active = 0;
					//printString("case 21");
					//printString("  \r\n");
				}
				else
				Time_delay(1000000,&Podajnik.active,0);	//sekunda
			}
			break;
		}
	}
	*/
	
	//Set control - sterowanie przekaznikami
	//############################################################################################################
	if(IO_control != IO_control_tmp)
	{
		IO_control = IO_control_tmp;
		TWI_access(IO_ADRES, 5, ~IO_control, 0, 0, 0); //przkaznik ON/OFF
	}
	
}

