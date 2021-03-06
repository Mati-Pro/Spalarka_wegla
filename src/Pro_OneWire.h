/*
 * Pro_OneWire.h
 *
 * Created: 25.10.2020 19:30:04
 *  Author: Pro
 */ 


#ifndef PRO_ONEWIRE_H_
#define PRO_ONEWIRE_H_

//------------------------------------------OneWire---------------------
#define OneWire_RESET_time1 500
#define OneWire_RESET_time2 70
#define OneWire_RESET_time3 500

#define OneWire_SEND_time1 5
#define OneWire_SEND_time2 70
#define OneWire_SEND_time3 70

#define OneWire_READ_time1 5
#define OneWire_READ_time2 5
#define OneWire_READ_time3 70

struct oneWire_ini {
	uint8_t active;
	uint8_t step;
};
struct oneWire_ini OneWire_SEKWENCJA = {1, 1};
	
struct oneWire_rs {
	uint8_t active;
	uint8_t step;
	uint8_t AS;
};
struct oneWire_rs OneWire_RESET = {0, 1, 0};
	
struct _HL_type {
	uint8_t  H;
	uint8_t  L;
};

struct _HL_type OneWire_SENSOR[5];

uint8_t OneWire_SENSOR_NC[5] = {0,0,0,0,0};	//nieczulosc pomiaru
	
struct OneWire_st {
	uint8_t przeterminowany;
	uint8_t niewiarygodny;
} OneWire_STATUS = {0b1100111, 0b1100111};


//--------------------------temperatury---------------------
struct  Nas_temp {
	uint8_t war;
	uint8_t his;
	};

struct Nas_temp Temp_CWU= {31, 5};
struct Nas_temp Temp_CO = {60, 4};
struct Nas_temp Temp_Mx = {40, 1};

uint8_t IO_control = 0;
uint8_t IO_control_tmp = 0;

#define Pompa_CWU	1
#define Pompa_CO	2
#define Miesz_Gh	3
#define Miesz_Gl	4
#define Pompa_P		5
#define Zes_PW		6


//--------------------------------Procedury obd�ugi OneWire----------------------
//--
//-- OneWire_RESET  inicjacja i reset magistrali OneWire
//-- OneWire_SEND   wys�anie bajtu do urzadzenia OneWire
//-- OneWire_READ   odebranie bajtu z urzadzenia OneWire
//--
//--------------------------------------------------------------------------------


void OneWire_wyswietl_pomiar(uint8_t mode, char *LCDrow, uint8_t poz, uint8_t *sensorH, uint8_t *sensorL, uint8_t przeterminowanie)
{
	uint8_t tempH;
	uint8_t tempL;
	
	tempH = *sensorH;
	tempL  =  0;
	
	if(przeterminowanie)
	{
		LCDrow[poz] = 0x20;		//setki
		LCDrow[poz+1] = 0x2D;	//dziesiatki
		LCDrow[poz+2] = 0x2D;	//jednostki
		
		if(mode == 1)
		{
			LCDrow[poz+3] = 0x2E; //kropka
			LCDrow[poz+4] = 0x2D; //dziesietne
		}
		
		*sensorL = 0;
		*sensorH = 0;
	}
	else
	{
		LCDrow[poz] = (tempH / 100);  //setki
		tempH = tempH - LCDrow[poz] * 100;
		LCDrow[poz] += 0x30;
		if(LCDrow[poz] == 0x30) LCDrow[poz] = 0x20;
	
		LCDrow[poz+1] = (tempH / 10);   //dziesiatki
		tempH = tempH - LCDrow[poz+1] * 10;
		LCDrow[poz+1] += 0x30;
		if((LCDrow[poz+1] == 0x30) && (mode == 2)) LCDrow[poz+1] = 0x20;
	
		LCDrow[poz+2] = tempH;          //jednostki
		LCDrow[poz+2] += 0x30;
	
		if(mode == 1)	//dziesietne
		{
			tempL  =  6 * (0x01 &  *sensorL);
			tempL += 12 * (0x01 & (*sensorL >> 1));
			tempL += 25 * (0x01 & (*sensorL >> 2));
			tempL += 50 * (0x01 & (*sensorL >> 3));
		
			LCDrow[poz+3] = 0x2E;   //kropka
			LCDrow[poz+4] = (tempL / 10);		//dziesietne
			LCDrow[poz+4] += 0x30;
		}
	}
}


void OneWire_setup(void) {
	
	DDRC &= ~((1 << DDC2) | (1 << DDC1) | (1 << DDC0));
	PORTC |=  (1 << PC2) | (1 << PC1) | (1 << PC0);
	
	DDRD &= ~((1 << DDD6) | (1 << DDD5));
	PORTD |=  (1 << PD6) | (1 << PD5);
	
	OneWire_SENSOR[0].L = 0; OneWire_SENSOR[0].H = 0;
	OneWire_SENSOR[1].L = 0; OneWire_SENSOR[1].H = 0;
	OneWire_SENSOR[2].L = 0; OneWire_SENSOR[2].H = 0;
	OneWire_SENSOR[3].L = 0; OneWire_SENSOR[3].H = 0;
	OneWire_SENSOR[4].L = 0; OneWire_SENSOR[4].H = 0;
}



//------------------------------Procedura OneWire_READ----------------------------
void OneWire_READ(uint8_t OneWire_AS, uint8_t* sensor1, uint8_t* sensor2, uint8_t* sensor3, 
					uint8_t* sensor4, uint8_t* sensor5)	//AS - aktywny sensor
{
	if(OneWire_AS & 0x01)
		*sensor1 = 0;
	
	if(OneWire_AS & 0x02)
		*sensor2 = 0;
	
	if(OneWire_AS & 0x04)
		*sensor3 = 0;
		
	if(OneWire_AS & 0x20)
		*sensor4 = 0;
	
	if(OneWire_AS & 0x40)
		*sensor5 = 0;
		
	for (uint8_t i = 0; i < 8; i++)
	{
		if(OneWire_AS & 0x01)
		{
			PORTC &= ~(1 << PC0);
			DDRC |= (1 << DDC0);
		}
			
		if(OneWire_AS & 0x02)
		{
			PORTC &= ~(1 << PC1);
			DDRC |= (1 << DDC1);
		}
			
		if(OneWire_AS & 0x04)
		{
			PORTC &= ~(1 << PC2);
			DDRC |= (1 << DDC2);
		}
		
		if(OneWire_AS & 0x20)
		{
			PORTD &= ~(1 << PD5);
			DDRD |= (1 << DDD5);
		}
		
		if(OneWire_AS & 0x40)
		{
			PORTD &= ~(1 << PD6);
			DDRD |= (1 << DDD6);
		}

		_delay_us(OneWire_READ_time1);

		if(OneWire_AS & 0x01)
		{
			PORTC |= (1 << PC0);
			DDRC &= ~(1 << DDC0);
		}
			
		if(OneWire_AS & 0x02)
		{
			PORTC |= (1 << PC1);
			DDRC &= ~(1 << DDC1);
		}
		
		if(OneWire_AS & 0x04)
		{
			PORTC |= (1 << PC2);
			DDRC &= ~(1 << DDC2);
		}
		
		if(OneWire_AS & 0x20)
		{
			PORTD |= (1 << PD5);
			DDRD &= ~(1 << DDD5);
		}
		
		if(OneWire_AS & 0x40)
		{
			PORTD |= (1 << PD6);
			DDRD &= ~(1 << DDD6);
		}

		_delay_us(OneWire_READ_time2);

		if(OneWire_AS & 0x01)
		{
			if (PINC & (1 << PINC0))
			*sensor1 |= 1 << i;
		}
		
		if(OneWire_AS & 0x02)
		{
			if (PINC & (1 << PINC1))
			*sensor2 |= 1 << i;
		}
		
		if(OneWire_AS & 0x04)
		{
			if (PINC & (1 << PINC2))
			*sensor3 |= 1 << i;
		}
		
		if(OneWire_AS & 0x20)
		{
			if (PIND & (1 << PIND5))
			*sensor4 |= 1 << i;
		}
		
		if(OneWire_AS & 0x40)
		{
			if (PIND & (1 << PIND6))
			*sensor5 |= 1 << i;
		}
		
		_delay_us(OneWire_READ_time3);
	}
}


//------------------------------Procedura OneWire_SEND----------------------------
void OneWire_SEND(uint8_t OneWire_AS, uint8_t rozkaz)
{
	uint8_t pom;
		 
	for (uint8_t i = 0; i < 8; i++)
	{
		pom = (rozkaz >> i) & 0x01;
			 
		PORTC &= ~(((1 << PC2) | (1 << PC1) | (1 << PC0)) & OneWire_AS);	//piny na 0
		DDRC |= (((1 << DDC2) | (1 << DDC1) | (1 << DDC0)) & OneWire_AS);
		
		PORTD &= ~(((1 << PD6) | (1 << PD5)) & OneWire_AS);	//piny na 0
		DDRD |= (((1 << DDD6) | (1 << DDD5)) & OneWire_AS);
		
		_delay_us(OneWire_SEND_time1);
			 
		if(OneWire_AS & 0x01)
		{
			if (pom == 1)
				PORTC |= (1 << PC0);
			else
				PORTC &= ~(1 << PC0);
		}
			 
		if(OneWire_AS & 0x02)
		{
			if (pom == 1)
			PORTC |= (1 << PC1);
			else
			PORTC &= ~(1 << PC1);
		}
			 
		if(OneWire_AS & 0x04)
		{
			if (pom == 1)
			PORTC |= (1 << PC2);
			else
			PORTC &= ~(1 << PC2);
		}
		
		if(OneWire_AS & 0x20)
		{
			if (pom == 1)
			PORTD |= (1 << PD5);
			else
			PORTD &= ~(1 << PD5);
		}
		
		if(OneWire_AS & 0x40)
		{
			if (pom == 1)
			PORTD |= (1 << PD6);
			else
			PORTD &= ~(1 << PD6);
		}
		
		_delay_us(OneWire_SEND_time2);
			 
		PORTC |= (1 << PC2) | (1 << PC1) | (1 << PC0);
		PORTD |= (1 << PD6) | (1 << PD5);
			 
		_delay_us(OneWire_SEND_time3);
	}
}
//------------------------------Procedura OneWire_sekwencjaOdczytu----------------
void OneWire_sekwencjaOdczyt(void) {
  
  if(OneWire_SEKWENCJA.active) {
    switch (OneWire_SEKWENCJA.step) {
    case 1:
    OneWire_SEKWENCJA.active = 0;
    OneWire_SEKWENCJA.step++;
	OneWire_RESET.AS = 0b1100111;
    OneWire_RESET.active = 1;
    OneWire_RESET.step = 1;
    break;

    case 2:
    if(OneWire_RESET.AS > 0)  //1 - RESET NOT OK
	{
		//RESET udany (jedynka w AS oznacza reset udany)
		OneWire_SEND(OneWire_RESET.AS, 0xCC);
		OneWire_SEND(OneWire_RESET.AS, 0x44);
	}
	else
		OneWire_SEKWENCJA.step = 4;

	sysClock_delay2(860000, &OneWire_SEKWENCJA.active, &OneWire_SEKWENCJA.step);
    break;

    case 3:
    OneWire_SEKWENCJA.active = 0;
    OneWire_SEKWENCJA.step++;
    OneWire_RESET.active = 1;
    OneWire_RESET.step = 1;
    break;

    case 4:
	OneWire_SEKWENCJA.step++;
    if(OneWire_RESET.AS > 0)  //1
      {
		OneWire_SEND(OneWire_RESET.AS, 0xCC);
		OneWire_SEND(OneWire_RESET.AS, 0xBE);
		OneWire_READ(OneWire_RESET.AS, &OneWire_SENSOR[0].L, &OneWire_SENSOR[1].L, &OneWire_SENSOR[2].L, &OneWire_SENSOR[3].L, &OneWire_SENSOR[4].L);
		OneWire_READ(OneWire_RESET.AS, &OneWire_SENSOR[0].H, &OneWire_SENSOR[1].H, &OneWire_SENSOR[2].H, &OneWire_SENSOR[3].H, &OneWire_SENSOR[4].H);
      }
    break;
	
	case 5:
	OneWire_STATUS.niewiarygodny = 0b1100111 ^ OneWire_RESET.AS;
	
	//Sensor 1
	if(OneWire_STATUS.niewiarygodny & 0x01)
		OneWire_SENSOR_NC[0]++;
	else
		OneWire_SENSOR_NC[0] = 0;
	
	//Sensor 2
	if(OneWire_STATUS.niewiarygodny & 0x02)
		OneWire_SENSOR_NC[1]++;
	else
		OneWire_SENSOR_NC[1] = 0;
	
	//Sensor 3
	if(OneWire_STATUS.niewiarygodny & 0x04)
		OneWire_SENSOR_NC[2]++;
	else
		OneWire_SENSOR_NC[2] = 0;
	
	//Sensor 4
	if(OneWire_STATUS.niewiarygodny & 0x20)
		OneWire_SENSOR_NC[3]++;
	else
		OneWire_SENSOR_NC[3] = 0;
	
	//Sensor 5
	if(OneWire_STATUS.niewiarygodny & 0x40)
		OneWire_SENSOR_NC[4]++;
	else
		OneWire_SENSOR_NC[4] = 0;
	
	//Sensor 1
	if(OneWire_SENSOR_NC[0] > 2)
	{
		OneWire_SENSOR_NC[0] = 0;
		OneWire_STATUS.przeterminowany |= (OneWire_STATUS.niewiarygodny & 0x01);
	}
	
	//Sensor 2
	if(OneWire_SENSOR_NC[1] > 2)
	{
		OneWire_SENSOR_NC[1] = 0;
		OneWire_STATUS.przeterminowany |= (OneWire_STATUS.niewiarygodny & 0x02);
	}
	
	//Sensor 3
	if(OneWire_SENSOR_NC[2] > 2)
	{
		OneWire_SENSOR_NC[2] = 0;
		OneWire_STATUS.przeterminowany |= (OneWire_STATUS.niewiarygodny & 0x04);
	}
	
	//Sensor 4
	if(OneWire_SENSOR_NC[3] > 2)
	{
		OneWire_SENSOR_NC[3] = 0;
		OneWire_STATUS.przeterminowany |= (OneWire_STATUS.niewiarygodny & 0x20);
	}
	
	//Sensor 5
	if(OneWire_SENSOR_NC[4] > 2)
	{
		OneWire_SENSOR_NC[4] = 0;
		OneWire_STATUS.przeterminowany |= (OneWire_STATUS.niewiarygodny & 0x40);
	}
	
	OneWire_STATUS.przeterminowany &= OneWire_STATUS.niewiarygodny;
	
	if(OneWire_RESET.AS & 0x01)
	{
		//temperatura K zasilania
		OneWire_SENSOR[0].H = OneWire_SENSOR[0].H << 4;
		OneWire_SENSOR[0].H |=(OneWire_SENSOR[0].L & 0xF0) >> 4;
		OneWire_SENSOR[0].H = OneWire_SENSOR[0].H & 0x7F;
		OneWire_SENSOR[0].L = OneWire_SENSOR[0].L & 0x0F;
	}
	
	if(OneWire_RESET.AS & 0x02)
	{
		//temperatura K powrot
		OneWire_SENSOR[1].H = OneWire_SENSOR[1].H << 4;
		OneWire_SENSOR[1].H |=(OneWire_SENSOR[1].L & 0xF0) >> 4;
		OneWire_SENSOR[1].H = OneWire_SENSOR[1].H & 0x7F;
		OneWire_SENSOR[1].L = OneWire_SENSOR[1].L & 0x0F;
	}
	
	if(OneWire_RESET.AS & 0x04)
	{
		//temperatura CWU zasilania
		OneWire_SENSOR[2].H = OneWire_SENSOR[2].H << 4;
		OneWire_SENSOR[2].H |= (OneWire_SENSOR[2].L & 0xF0) >> 4;
		OneWire_SENSOR[2].H = OneWire_SENSOR[2].H & 0x7F;
		OneWire_SENSOR[2].L = OneWire_SENSOR[2].L & 0x0F;
	}
	
	if(OneWire_RESET.AS & 0x20)
	{
		//temperatura G zasilania
		OneWire_SENSOR[3].H = OneWire_SENSOR[3].H << 4;
		OneWire_SENSOR[3].H |=(OneWire_SENSOR[3].L & 0xF0) >> 4;
		OneWire_SENSOR[3].H = OneWire_SENSOR[3].H & 0x7F;
		OneWire_SENSOR[3].L = OneWire_SENSOR[3].L & 0x0F;
	}
	
	if(OneWire_RESET.AS & 0x40)
	{
		//temperatura P zasilania
		OneWire_SENSOR[4].H = OneWire_SENSOR[4].H << 4;
		OneWire_SENSOR[4].H |=(OneWire_SENSOR[4].L & 0xF0) >> 4;
		OneWire_SENSOR[4].H = OneWire_SENSOR[4].H & 0x7F;
		OneWire_SENSOR[4].L = OneWire_SENSOR[4].L & 0x0F;
	}
	
	//Regulator CWU
	if(OneWire_SENSOR[2].H >= Temp_CWU.war)
		clear_bit(IO_control_tmp, Pompa_CWU);
		
	if(OneWire_SENSOR[2].H < Temp_CWU.war - Temp_CWU.his)
		set_bit(IO_control_tmp, Pompa_CWU);
		
	if(IO_control != IO_control_tmp)
	{
		IO_control = IO_control_tmp;
		i2c_access(IO_ADRES, 5, ~IO_control, 0, 0, 0); //przkaznik 1 ON/OFF
	}
	
	if(Menu.aktywacja == 0)
	{
		OneWire_wyswietl_pomiar(1, LCD_row3, 2,  &OneWire_SENSOR[0].H, &OneWire_SENSOR[0].L, OneWire_STATUS.przeterminowany & 0x01);	//kociol zasilanie
		OneWire_wyswietl_pomiar(1, LCD_row4, 2,  &OneWire_SENSOR[1].H, &OneWire_SENSOR[1].L, OneWire_STATUS.przeterminowany & 0x02);	//kociol powrot
		OneWire_wyswietl_pomiar(0, LCD_row2, 13, &OneWire_SENSOR[2].H, &OneWire_SENSOR[2].L, OneWire_STATUS.przeterminowany & 0x04);	//CWU
		OneWire_wyswietl_pomiar(0, LCD_row3, 13, &OneWire_SENSOR[3].H, &OneWire_SENSOR[3].L, OneWire_STATUS.przeterminowany & 0x20);	//grzejniki zasilanie
		OneWire_wyswietl_pomiar(0, LCD_row4, 13, &OneWire_SENSOR[4].H, &OneWire_SENSOR[4].L, OneWire_STATUS.przeterminowany & 0x40);	//podlogowka
		
			LCDprint(2, 13, 3, LCD_row2);
			LCDprint(3, 2, 14, LCD_row3);
			LCDprint(4, 2, 14, LCD_row4);
	}
	
	if(Menu.aktywacja & 0x02)
		Menu.aktywacja |= 0x04;	//drukuj raz
	
    OneWire_SEKWENCJA.step = 1;
    break;
    }
  }
}


//------------------------------Procedura OneWire_RESET-------------------------
void OneWire_handling(void) {
  
  if(OneWire_RESET.active) {
	     
    switch (OneWire_RESET.step) {
    case 1:
	PORTC &= ~((1 << PC2) |(1 << PC1) | (1 << PC0));		//piny na 0
	DDRC |= (1 << DDC2) | (1 << DDC1) | (1 << DDC0);
	
	PORTD &= ~((1 << PD6) |(1 << PD5));		//piny na 0
	DDRD |= (1 << DDD6) | (1 << DDD5);
	
    sysClock_delay2(OneWire_RESET_time1, &OneWire_RESET.active, &OneWire_RESET.step);
    break;

    case 2:
	PORTC |= (1 << PC2) | (1 << PC1) | (1 << PC0);		//piny na 1 pullup
	DDRC &= ~((1 << DDC2) | (1 << DDC1) | (1 << DDC0));
	
	PORTD |= (1 << PD6) | (1 << PD5);		//piny na 1 pullup
	DDRD &= ~((1 << DDD6) | (1 << DDD5));
	
	_delay_us(OneWire_RESET_time2);
	OneWire_RESET.AS ^= 0b1100111;
	OneWire_RESET.AS |= PINC & ((1 << PINC2) | (1 << PINC1) | (1 << PINC0));
	OneWire_RESET.AS |= PIND & ((1 << PIND6) | (1 << PIND5));
	
	if (OneWire_RESET.AS < 0b1100111)	//jedynka to zle
      OneWire_RESET.step++;				//jezeli OneWire_RESET.AS == 0x07 -> RESET nieudany
    else
    {
      OneWire_RESET.AS = 0;
	  OneWire_RESET.active = 0; //0 - RESET NOT OK
      OneWire_RESET.step = 1; 
      OneWire_SEKWENCJA.active = 1;
    }
    break;

    case 3:
    sysClock_delay2(OneWire_RESET_time3, &OneWire_RESET.active, &OneWire_RESET.step);
    break;

    case 4:
	OneWire_RESET.AS ^= 0b1100111; 
	OneWire_RESET.AS = OneWire_RESET.AS & ((PINC & 0b0000111) | (PIND & 0b1100000));
	
	if (OneWire_RESET.AS > 0)		//RESET udany (jedynka w AS oznacza reset udany)
    {
      OneWire_RESET.active = 0;
	  OneWire_RESET.step = 0;		//1 - RESET OK
    }
    else
    {
      OneWire_RESET.active = 0;
      OneWire_RESET.step = 1;		//0 - RESET NOT OK
    }
	OneWire_SEKWENCJA.active = 1;
    break;
    }
  }
}

/* Kopia - pojedynczy odczyty z PC0
//------------------------------Procedura OneWire_sekwencjaOdczytu----------------
void OneWire_sekwencjaOdczyt(void) {
	
	if(OneWire_SEKWENCJA.active) {
		switch (OneWire_SEKWENCJA.step) {
			case 1:
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_RESET.active = 1;
			OneWire_RESET.step = 1;
			break;

			case 2:
			if(OneWire_RESET.step == 0)  //1 - RESET OK
			{
				OneWire_SEKWENCJA.step++;
			}
			else                        //0 - RESET NOT OK
			{
				OneWire_SEKWENCJA.step = 1;
			}
			break;

			case 3:
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_SEND.active = 1;
			OneWire_SEND.wartosc = 0xCC;
			break;
			
			case 4:
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_SEND.active = 1;
			OneWire_SEND.wartosc = 0x44;
			break;

			case 5:
			sysClock_delay2(860000, &OneWire_SEKWENCJA.active, &OneWire_SEKWENCJA.step);
			break;

			case 6:
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_RESET.active = 1;
			OneWire_RESET.step = 1;
			break;

			case 7:
			if(OneWire_RESET.step == 0)  //1
			{
				OneWire_SEKWENCJA.step++;
			}
			else                        //0
			{
				OneWire_SEKWENCJA.step = 1;
			}
			break;
			
			case 8:
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_SEND.active = 1;
			OneWire_SEND.wartosc = 0xCC;
			break;
			
			case 9:
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_SEND.active = 1;
			OneWire_SEND.wartosc = 0xBE;
			break;

			case 10:
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_READ.active = 1;
			break;

			case 11:
			OneWire_TEMP.L = OneWire_READ.wartosc;
			
			OneWire_SEKWENCJA.active = 0;
			OneWire_SEKWENCJA.step++;
			OneWire_READ.active = 1;
			break;

			case 12:
			OneWire_TEMP.H = OneWire_READ.wartosc;
			
			tempH = OneWire_TEMP.H << 4;
			tempH |= (OneWire_TEMP.L & 0xF0) >> 4;
			
			OneWire_TEMP.L &= 0x0F;
			tempL = 0;
			tempL  =  6 * (0x01 &  OneWire_TEMP.L);
			tempL += 12 * (0x01 & (OneWire_TEMP.L >> 1));
			tempL += 25 * (0x01 & (OneWire_TEMP.L >> 2));
			tempL += 50 * (0x01 & (OneWire_TEMP.L >> 3));
			LCD_row3[5] = (tempL / 10);
			
			LCD_row3[1] = (tempH / 100);  //setki
			tempH = tempH - LCD_row3[1] * 100;
			
			LCD_row3[2] = (tempH / 10);   //dziesiatki
			tempH = tempH - LCD_row3[2] * 10;
			
			LCD_row3[3] = tempH;          //jednostki
			
			LCD_row3[1] += 0x30;
			LCD_row3[2] += 0x30;
			LCD_row3[3] += 0x30;
			LCD_row3[4] = 0x2E;   //kropka
			LCD_row3[5] += 0x30;
			if(LCD_row3[1] == 0x30) LCD_row3[1] = 0x20;
			
			LCDprint(3, 1, 15, LCD_row3);
			OneWire_SEKWENCJA.step = 1;
			break;
		}
	}
}
//------------------------------Procedura OneWire_RESET-------------------------
void OneWire_handling(void) {
	
	if(OneWire_RESET.active) {
		
		switch (OneWire_RESET.step) {
			case 1:
			PORTC &= ~(1 << PC0);		//pin na 0
			DDRC |= (1 << DDC0);
			
			sysClock_delay2(OneWire_RESET_time1, &OneWire_RESET.active, &OneWire_RESET.step);
			break;

			case 2:
			PORTC |= (1 << PC0);		//pin na 1 pullup
			DDRC &= ~(1 << DDC0);
			
			_delay_us(OneWire_RESET_time2);
			
			if ((PINC & (1 << PINC0)) == 0)
			OneWire_RESET.step = 3;
			else
			{
				OneWire_RESET.active = 0;
				OneWire_RESET.step = 1; //0 - RESET NOT OK
				OneWire_SEKWENCJA.active = 1;
			}
			break;

			case 3:
			sysClock_delay2(OneWire_RESET_time3, &OneWire_RESET.active, &OneWire_RESET.step);
			break;

			case 4:
			OneWire_SEKWENCJA.active = 1;
			
			if ((PINC & (1 << PINC0)) == 1)
			{
				OneWire_RESET.active = 0;
				OneWire_RESET.step = 0; //1 - RESET OK
			}
			else
			{
				OneWire_RESET.active = 0;
				OneWire_RESET.step = 1; //0 - RESET NOT OK
			}
			break;
		}
	}


	//------------------------------Procedura OneWire_SEND-------------------------
	if(OneWire_SEND.active) {

		uint8_t pom;
		for (uint8_t i = 0; i < 8; i++)
		{
			pom = (OneWire_SEND.wartosc >> i) & 0x01;
			
			PORTC &= ~(1 << PC0);
			DDRC |= (1 << DDC0);
			_delay_us(OneWire_SEND_time1);
			
			if (pom == 1)
			PORTC |= (1 << PC0);
			else
			PORTC &= ~(1 << PC0);
			
			_delay_us(OneWire_SEND_time2);
			PORTC |= (1 << PC0);
			_delay_us(OneWire_SEND_time3);
		}
		OneWire_SEND.active = 0;
		OneWire_SEKWENCJA.active = 1;
	}


	//------------------------------Procedura OneWire_READ-------------------------
	if(OneWire_READ.active) {

		OneWire_READ.wartosc = 0;
		for (uint8_t i = 0; i < 8; i++)
		{
			PORTC &= ~(1 << PC0);
			DDRC |= (1 << DDC0);

			_delay_us(OneWire_READ_time1);

			PORTC |= (1 << PC0);
			DDRC &= ~(1 << DDC0);

			_delay_us(OneWire_READ_time2);

			if ((PINC & (1 << PINC0)) == 1)
			OneWire_READ.wartosc |= 0x01 << i;
			
			_delay_us(OneWire_READ_time3);
		}
		OneWire_READ.active = 0;
		OneWire_SEKWENCJA.active = 1;
	}
}




NOWE HANDLING 1
//------------------------------Procedura OneWire_RESET-------------------------
void OneWire_handling(void) {
	
	uint8_t OneWire_TMP = 0;
  
  if(OneWire_RESET.active & 0x01) {
	     
    switch (OneWire_RESET.step) {
    case 1:
	PORTC &= ~((1 << PC2) |(1 << PC1) | (1 << PC0));		//piny na 0
	DDRC |= (1 << DDC2) | (1 << DDC1) | (1 << DDC0);
	
    sysClock_delay2(OneWire_RESET_time1, &OneWire_RESET.active, &OneWire_RESET.step);
    break;

    case 2:
	PORTC |= (1 << PC2) | (1 << PC1) | (1 << PC0);		//piny na 1 pullup
	DDRC &= ~((1 << DDC2) | (1 << DDC1) | (1 << DDC0));
	
	_delay_us(OneWire_RESET_time2);
	
	OneWire_RESET.active = ((PINC & ( (1 << PINC2) | (1 << PINC1) | (1 << PINC0) )) << 1) | 0x01;
	
	if ((OneWire_RESET.active & 0x0E) < 0x0E)	//0x0E = (1 << PINC2) | (1 << PINC1) | (1 << PINC0) << 1, jedynka to zle
      OneWire_RESET.step = 3;					//jezeli OneWire_RESET.active == 0X0E -> RESET nieudany
    else
    {
      OneWire_RESET.active = 0; //0 - RESET NOT OK
      OneWire_RESET.step = 1; 
      OneWire_SEKWENCJA.active = 1;
    }
    break;

    case 3:
    sysClock_delay2(OneWire_RESET_time3, &OneWire_RESET.active, &OneWire_RESET.step);
    break;

    case 4:
    OneWire_SEKWENCJA.active = 1;
	
	OneWire_TMP = (~(OneWire_RESET.active >> 1) & 0x07);
		
	OneWire_RESET.active = ((OneWire_TMP & PINC & ( (1 << PINC2) | (1 << PINC1) | (1 << PINC0) )) << 1);
	
	if (OneWire_RESET.active > 1)		//RESET udany
    {
      OneWire_RESET.step = 0; //1 - RESET OK
    }
    else
    {
      OneWire_RESET.active = 0;
      OneWire_RESET.step = 1; //0 - RESET NOT OK
    }
    break;
    }  
  }


  //------------------------------Procedura OneWire_SEND-------------------------
  if(OneWire_SEND.active) {

    uint8_t pom;
	
	OneWire_TMP = (OneWire_RESET.active >> 1) & 0x07;
	
    for (uint8_t i = 0; i < 8; i++)
    {
      pom = (OneWire_SEND.wartosc >> i) & 0x01;
     
		if(OneWire_TMP $ 0x01)
		{
			PORTC &= ~(1 << PC0);	//pin0 na 0
			DDRC |= (1 << DDC0);
		}
		
		if(OneWire_TMP $ 0x02)
		{
			PORTC &= ~(1 << PC1);	//pin1 na 0
			DDRC |= (1 << DDC1);
		}
		
		if(OneWire_TMP $ 0x04)
		{
			PORTC &= ~(1 << PC2);	//pin1 na 0
			DDRC |= (1 << DDC2);
		}
		
      _delay_us(OneWire_SEND_time1);
		
		if(OneWire_TMP $ 0x01)
		{
			if (pom == 1)
				PORTC |= (1 << PC0);
			else
				PORTC &= ~(1 << PC0);
		}
		
		if(OneWire_TMP $ 0x02)
		{
			if (pom == 1)
				PORTC |= (1 << PC1);
			else
				PORTC &= ~(1 << PC1);
		}
		
		if(OneWire_TMP $ 0x04)
		{
			if (pom == 1)
				PORTC |= (1 << PC2);
			else
				PORTC &= ~(1 << PC2);
		}
		
	  _delay_us(OneWire_SEND_time2);
	  
		if(OneWire_TMP $ 0x01)
			PORTC |= (1 << PC0);
			
		if(OneWire_TMP $ 0x02)
			PORTC |= (1 << PC1);
			
		if(OneWire_TMP $ 0x04)
			PORTC |= (1 << PC1);
	  
	  _delay_us(OneWire_SEND_time3);
    }
      OneWire_SEND.active = 0;
      OneWire_SEKWENCJA.active = 1;
  }


  //------------------------------Procedura OneWire_READ-------------------------
  if(OneWire_READ.active) {

    OneWire_READ.wartosc = 0;
	OneWire_TMP = (OneWire_RESET.active >> 1) & 0x07;
	
    for (uint8_t i = 0; i < 8; i++)
    {
		if(OneWire_TMP $ 0x01)
		{
			PORTC &= ~(1 << PC0);
			DDRC |= (1 << DDC0);
		}
		
		if(OneWire_TMP $ 0x02)
		{
			PORTC &= ~(1 << PC1);
			DDRC |= (1 << DDC1);
		}
		
		if(OneWire_TMP $ 0x04)
		{
			PORTC &= ~(1 << PC2);
			DDRC |= (1 << DDC2);
		}

	  _delay_us(OneWire_READ_time1);

		if(OneWire_TMP $ 0x01)
		{
			PORTC |= (1 << PC0);
			DDRC &= ~(1 << DDC0);
		}
		
		if(OneWire_TMP $ 0x02)
		{
			PORTC |= (1 << PC1);
			DDRC &= ~(1 << DDC1);
		}
		
		if(OneWire_TMP $ 0x04)
		{
			PORTC |= (1 << PC2);
			DDRC &= ~(1 << DDC2);
		}

	  _delay_us(OneWire_READ_time2);

		if(OneWire_TMP $ 0x01)
		{
			if ((PINC & (1 << PINC0)) == 1)
				OneWire_READ.wartosc[0] |= 0x01 << i;
		}
		
		if(OneWire_TMP $ 0x02)
		{
			if ((PINC & (1 << PINC1)) == 0x02)
				OneWire_READ.wartosc[1] |= 0x01 << i;
		}
		
		if(OneWire_TMP $ 0x04)
		{
			if ((PINC & (1 << PINC2)) == 0x04)
				OneWire_READ.wartosc[2] |= 0x01 << i;
		}
		
      _delay_us(OneWire_READ_time3);
    }
      OneWire_READ.active = 0;
      OneWire_SEKWENCJA.active = 1;
  } 
}


*/



/*    tempH = OneWire_TEMP.H << 4;
    tempH |= (OneWire_TEMP.L & 0xF0) >> 4;
    
    OneWire_TEMP.L &= 0x0F;
    tempL = 0;
    tempL  =  6 * (0x01 &  OneWire_TEMP.L);
    tempL += 12 * (0x01 & (OneWire_TEMP.L >> 1));
    tempL += 25 * (0x01 & (OneWire_TEMP.L >> 2));
    tempL += 50 * (0x01 & (OneWire_TEMP.L >> 3));
    LCD_row3[5] = (tempL / 10);
    
    LCD_row3[1] = (tempH / 100);  //setki
    tempH = tempH - LCD_row3[1] * 100;
        
    LCD_row3[2] = (tempH / 10);   //dziesiatki
    tempH = tempH - LCD_row3[2] * 10;
    
    LCD_row3[3] = tempH;          //jednostki
    
    LCD_row3[1] += 0x30;   
    LCD_row3[2] += 0x30;
    LCD_row3[3] += 0x30;
    LCD_row3[4] = 0x2E;   //kropka
    LCD_row3[5] += 0x30;
    if(LCD_row3[1] == 0x30) LCD_row3[1] = 0x20;
*/ 

#endif /* PRO_ONEWIRE_H_ */