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

extern struct _HL_type OneWire_SENSOR[5];
extern struct OneWire_st OneWire_STATUS;

struct Nas_temp Temp_CWU= {31, 5};
struct Nas_temp Temp_CO = {60, 4};
struct Nas_temp Temp_Mx = {40, 1};

volatile struct Param_spac	Podawanie; 
volatile struct Param_spac	Podtrzymanie;

volatile struct Param_spac	Podawanie_tmp;
volatile struct Param_spac	Podtrzymanie_tmp;
	
uint8_t IO_control = 0;
uint8_t IO_control_tmp = 0;
uint8_t	CO_status = 0;



void Regulator_setup(void)
{
	//ustawienie pinow dla podajnika i wentylatora
	//start zegara 0.1s
	
}


void Regulator_handling(void)
{
	//Regulator CO
	if(!(OneWire_STATUS.przeterminowany & 0x01))	//pomiar nie przeterminowany
	{
		if(OneWire_SENSOR[0].H >= Temp_CO.war)		//OFF
		clear_bit(CO_status, CO_pracaPodawanie);
		
		if(OneWire_SENSOR[0].H < Temp_CO.war - Temp_CO.his)	//ON
		set_bit(CO_status, CO_pracaPodawanie);
	}
	else
	clear_bit(CO_status, CO_pracaPodawanie);	//OFF poniewaz pomiar przeterminowany
	
	
	//Regulator CWU
	if(!(OneWire_STATUS.przeterminowany & 0x04))	//pomiar nie przeterminowany
	{
		if(OneWire_SENSOR[2].H >= Temp_CWU.war)		//OFF
		clear_bit(IO_control_tmp, Pompa_CWU);
		
		if(OneWire_SENSOR[2].H < Temp_CWU.war - Temp_CWU.his)	//ON
		set_bit(IO_control_tmp, Pompa_CWU);
	}
	else
	clear_bit(IO_control_tmp, Pompa_CWU);
	
	
	//Set control
	if(IO_control != IO_control_tmp)
	{
		IO_control = IO_control_tmp;
		TWI_access(IO_ADRES, 5, ~IO_control, 0, 0, 0); //przkaznik ON/OFF
	}
	
}


void Regulator_isr(void)
{
	//if(Podawanie_tmp.podawanie.minuty == 0)
		if(Podawanie_tmp.podawanie.sekundy == 0)
			if(Podawanie_tmp.podawanie.msekundy == 0)
				if(Podawanie_tmp.przerwa.minuty == 0)
					if(Podawanie_tmp.przerwa.sekundy == 0)
						if(Podawanie_tmp.przerwa.msekundy == 0)
						{
							//Podawanie_tmp.podawanie.minuty =	Podawanie.podawanie.minuty;
							Podawanie_tmp.podawanie.sekundy =	Podawanie.podawanie.sekundy;
							Podawanie_tmp.podawanie.msekundy =	Podawanie.podawanie.msekundy;
							
							Podawanie_tmp.przerwa.minuty =		Podawanie.przerwa.minuty;
							Podawanie_tmp.przerwa.sekundy =		Podawanie.przerwa.sekundy;
							Podawanie_tmp.przerwa.msekundy =	Podawanie.przerwa.msekundy; 
						}
	
	if(CO_status & 0x01)	//CO praca
	{
		if(Podawanie_tmp.podawanie.msekundy > 0)
			Podawanie_tmp.podawanie.msekundy -= 1;
		else
		{
			if(Podawanie_tmp.podawanie.sekundy > 0)
			{
				Podawanie_tmp.podawanie.sekundy -= 1;
				Podawanie_tmp.podawanie.msekundy = 9;
			}
			//else
			//kasowanie CO praca i start CO przerwa
		}
	}
	
	if(CO_status & 0x02)	//CO przerwa
	{
		if(Podawanie_tmp.przerwa.msekundy > 0)
			Podawanie_tmp.przerwa.msekundy -= 1;
		else
		{
			if(Podawanie_tmp.przerwa.sekundy > 0)
			{
				Podawanie_tmp.przerwa.sekundy -= 1;
				Podawanie_tmp.przerwa.msekundy = 9;
			}
			else
			{
				if(Podawanie_tmp.przerwa.minuty > 0)
				{
					Podawanie_tmp.przerwa.minuty -= 1;
					Podawanie_tmp.przerwa.sekundy = 9;
					Podawanie_tmp.przerwa.msekundy = 9;
				}
				//else
				//kasowanie CO przerwa i start CO praca
			}
		}
	}
	
}
