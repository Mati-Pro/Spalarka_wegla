/*
 * sysTWI.cpp
 *
 * Created: 15.04.2021 08:39:39
 *  Author: Pawel Rogoz
 */ 

#include <stdlib.h>
#include "sysTWI.h"
#include "_PORT.h"

extern uint8_t SysStatus;


void TWI_setup(void)
{
	DDR_TWI &= ~((1 << DD_SCL) | (1 << DD_SDA));
	PORT_TWI |= (1 << P_SCL) | (1 << P_SDA);
	//DDRC &= ~((1 << DDC5) | (1 << DDC4));
	//PORTC |=   (1 << PC5) | (1 << PC4);
	
	TWBR = 72;
	TWSR = (0 << TWPS1) | (0 << TWPS0);
	TWI.stop = 0;
	TWI.busy = 0;

	TWI_kolejkaPointer = 0;
}

void TWI_access(uint8_t adres, uint8_t mode, uint8_t pointer, uint8_t *tab, uint8_t count, uint8_t *aktywacja)
{
	if(TWI_kolejkaPointer == 0)	//jesli jeszcze nie bylo kolejki to stworz j¹
		TWI_kolejka = (struct twi_bf *)calloc(TWI_kolejkaPointer + 1, sizeof(struct twi_bf));
	else
		TWI_kolejka = (struct twi_bf *)realloc(TWI_kolejka, (TWI_kolejkaPointer+1)*sizeof(struct twi_bf));
	
	TWI_kolejka[TWI_kolejkaPointer].adres = adres;
	TWI_kolejka[TWI_kolejkaPointer].mode = mode;
	TWI_kolejka[TWI_kolejkaPointer].data_pointer = pointer;
	TWI_kolejka[TWI_kolejkaPointer].reg = tab;
	TWI_kolejka[TWI_kolejkaPointer].data_count = count;
	TWI_kolejka[TWI_kolejkaPointer].f_aktywacja = aktywacja;

	//jesli jeden element w kolejce uruchom i2c
	if((TWI.stop == 0) && (TWI.busy == 0) && (TWI_kolejkaPointer == 0))
	{
		TWI.busy = 1;
		TWI.adres = adres;
		TWI.mode = mode;
		TWI.data_pointer = pointer;
		TWI.reg = tab;
		TWI.data_count = count;
		_TWISendStart();
	}
	
	TWI_kolejkaPointer++;
	
	/*
	if(TWI_kolejkaPointer < TWI_kolejka_max)
	TWI_kolejkaPointer++;
	else
	TWI_kolejkaPointer = TWI_kolejka_max;
	
	while(TWI_kolejkaPointer >= TWI_kolejka_max)
	{
		TWI_handling();
	}
	*/
}


void TWI_handling(void)
{
	if((TWI.stop == 1) && (TWI.busy == 1))
	{
		TWI.stop = 0;
		(*TWI_kolejka[0].f_aktywacja)++;
		
		TWI_kolejkaPointer--;
		
		if(TWI_kolejkaPointer > 0)
		{
			for(uint8_t i = 0; i < TWI_kolejkaPointer; i++)
				TWI_kolejka[i] = TWI_kolejka[i+1];
			
			//jesli kolejka ma zmniejszyc siê to zmniejsz tablice o kolejny element struktury	
			TWI_kolejka = (struct twi_bf *)realloc(TWI_kolejka, (TWI_kolejkaPointer+1)*sizeof(struct twi_bf));
			
			TWI.reg = TWI_kolejka[0].reg;
			TWI.adres = TWI_kolejka[0].adres;
			TWI.mode = TWI_kolejka[0].mode;
			TWI.data_pointer = TWI_kolejka[0].data_pointer;
			TWI.data_count = TWI_kolejka[0].data_count;
			_TWISendStart();
		}
		else
		{
			TWI.busy = 0;
			SysStatus |= 4;
			free(TWI_kolejka);	//zwolnij kolejke
		}
	}
}


void TWI_isr(void)
{
	switch (_TWI_SR)
	{
		//obsluga bledow TWI
		case TWI_ERROR:
		_TWISendStop();
		break;

		case TWI_MR_MTX_NACK:
		_TWISendStop();
		break;

		case TWI_MT_MTX_NACK:
		_TWISendStop();
		break;

		case TWI_MT_MTXD_NACK:
		_TWISendStop();
		break;
		
		//Start zosta³ wys³any -> wyslij SLA + R/W
		case TWI_START:   //0x08
		case TWI_REP_START: //0x10
		if((TWI.mode == 0) || (TWI.mode == 1))
		TWDR = (TWI.adres << 1) | 0x01; //Read mode, SLA+R
		if((TWI.mode == 2) || (TWI.mode == 3) || (TWI.mode == 4) || (TWI.mode == 5))
		TWDR = (TWI.adres << 1);  //Write mode, SLA+W
		_TWISendTransmit();
		break;

		
		// SLA+W wy³any, ACK odebrany -> rozpocznij wysy³anie, dana w TWDR
		case TWI_MT_MTX_ACK:  //0x18
		if((TWI.mode == 2) || (TWI.mode == 5))
		TWDR = TWI.data_pointer;
		if(TWI.mode == 3)
		{
			TWI.data_pointer = 0;
			TWDR = TWI.reg[TWI.data_pointer];
		}
		if(TWI.mode == 4)
		{
			TWDR = TWI.data_pointer;
			TWI.data_pointer = 0;
		}
		_TWISendTransmit();
		break;

		// Dana wys³ana, ACK wystawione przez SLAVE -> kontyn³uj wysy³anie lub wystaw RESTART lub STOP
		case TWI_MT_MTXD_ACK: //0x28
		
		if(TWI.mode == 2)
		{
			TWI.mode = 1;
			_TWISendStart();
		}
		
		if(TWI.mode == 3)
		{
			TWI.data_pointer++;
			if(TWI.data_pointer <= TWI.data_count)
			{
				TWDR = TWI.reg[TWI.data_pointer];
				_TWISendTransmit();
			}
			else
			{
				TWI.stop = 1;
				_TWISendStop();
			}
		}
		
		if(TWI.mode == 4)
		{
			if(TWI.data_pointer <= TWI.data_count)
			{
				TWDR = TWI.reg[TWI.data_pointer];
				TWI.data_pointer++;
				_TWISendTransmit();
			}
			else
			{
				TWI.stop = 1;
				_TWISendStop();
			}
		}

		if(TWI.mode == 5)
		{
			TWI.stop = 1;
			_TWISendStop();
		}
		break;

		
		// SLA+R wy³any, ACK odebrany -> rozpocznij pobieranie danych, po odebraniu wystaw ACK lub NACK (dla ostatniej danej)
		case TWI_MR_MTX_ACK:  //0x40
		if(TWI.mode == 0)
		_TWISendNACK();
		if(TWI.mode == 1)
		{
			TWI.data_pointer = 0;
			if(TWI.data_count > 0)
			_TWISendACK();
			else
			_TWISendNACK();
		}
		break;

		// Dane odebrane w TWDR, ACK wyslano w odpowiedzi -> kontynluj odbieranie, po odebraniu wystaw ACK lub NACK (dla ostatniej danej)
		case TWI_MR_MRX_ACK:  //0x50
		TWI.reg[TWI.data_pointer] = TWDR;
		TWI.data_pointer++;
		if(TWI.data_pointer < TWI.data_count)
		_TWISendACK();
		else
		_TWISendNACK();
		break;

		// Dane odebrane w TWDR, NACK wyslano w odpowiedzi -> wyslij STOP
		case TWI_MR_MRX_NACK: //0x58
		if(TWI.mode == 0)
		TWI.data_pointer = TWDR;
		if(TWI.mode == 1)
		{
			TWI.reg[TWI.data_pointer] = TWDR;
			TWI.stop = 1;
		}
		_TWISendStop();
		break;
	}
}