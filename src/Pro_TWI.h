/*
 * Pro_TWI.h
 *
 * Created: 21.10.2020 10:23:37
 *  Author: Pro
 */ 


#ifndef PRO_TWI_H_
#define PRO_TWI_H_

void i2c_setup(void);
void i2c_service(void);
void i2c_access(uint8_t adres, uint8_t mode, uint8_t pointer, uint8_t *tab, uint8_t count, uint8_t *aktywacja);
void I2C_handling(void);

//------------------------------------------I2C---------------------
#define _TWISendStart()    (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA)|(1<<TWIE))
#define _TWISendStop()     (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO))
#define _TWISendTransmit() (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE))
#define _TWISendACK()      (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA))
#define _TWISendNACK()     (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE))
#define _TWI_SR            (TWSR & 0xF8)

#define TWI_ERROR 0x00
#define TWI_START 0x08
#define TWI_REP_START 0x10
//Master-Nadawanie
#define TWI_MT_MTX_ACK 0x18     //Master transmit mode, SLA+W has been transmitted, ACK has been received
#define TWI_MT_MTX_NACK 0x20    //Master transmit mode, SLA+W has been transmitted, NOT ACK has been received
#define TWI_MT_MTXD_ACK 0x28    //Master transmit mode, DATA has been transmitted, ACK has been received
#define TWI_MT_MTXD_NACK  0x30  //Master transmit mode, DATA has been transmitted, NOT ACK has been received
//Master-Odbior
#define TWI_MR_MTX_ACK 0x40
#define TWI_MR_MTX_NACK 0x48
#define TWI_MR_MRX_ACK 0x50
#define TWI_MR_MRX_NACK 0x58

#define TWI_ARBITRATION 0x38

struct twi_rs {
	volatile uint8_t adres;
	volatile uint8_t mode;
	volatile uint8_t *reg;
	volatile uint8_t data_pointer;
	volatile uint8_t data_count;
	volatile uint8_t stop;
	volatile uint8_t busy;
};

volatile struct twi_rs TWI;

struct twi_bf {
	uint8_t adres;
	uint8_t mode;
	uint8_t *reg;
	uint8_t data_pointer;
	uint8_t data_count;
	uint8_t *f_aktywacja;
};

#define  TWI_kolejka_max	5
struct twi_bf TWI_kolejka[TWI_kolejka_max]; //bufor operacji dla i2c

uint8_t TWI_kolejkaPointer;      //wskaŸnik bufora operacji



//-------------------------------------Obs³uga i2c---------------------------------
//--
//-- i2c_setup() procedura inicjacji interfejsu TWI
//-- i2c_access() procedura wywo³ania obs³ugi TWI
//-- i2c_service() procedura realizacji kolejki obs³ugi TWI
//-- ISR(TWI_vect) procedura obs³ugi przerwania TWI
//--
//--------------------------------------------------------------------------------

//----------------------------Inicjalizacja interfejsu TWI----------------------------------
void I2C_setup(void)
{
	DDRC &= ~((1 << DDC5) | (1 << DDC4));
	PORTC |=   (1 << PC5) | (1 << PC4);
	
	TWBR = 72;
	TWSR = (0 << TWPS1) | (0 << TWPS0);
	TWI.stop = 0;
	TWI.busy = 0;

	TWI_kolejkaPointer = 0;
}


//---------------------------Wywo³ania obs³ugi TWI-----------------------------------------------
//  i2c_access(adres, mode, pointer, *tab, count, *aktywacja)
//
//  adres - adres urz¹dzenia i2c
//  mode  - tryb dzia³ania i2c
//        0 - odczyt 1-bajtu, bajt we wkazniku
//        1 - odczyt n-bajtów
//        2 - odczyt n-bajtów ze wskaŸnikiem (pointer = adres wewnêtrzny dla urz¹dzenia i2c)
//        3 - zapis n-bajtów
//        4 - zapis n-bajtów ze wskaŸnikiem (pointer = adres wewnêtrzny dla urz¹dzenia i2c)
//        5 - zapis 1-bajtu, bajt we wskaŸniku (pointer)
//  pointer - wskaŸnik-adres wewnêtrzny dla urz¹dzenia i2c
//  *tab  - wskaŸnik do tablicy danych odczytu-zapisu
//  count - liczba bajtów do zapisu-odczytu z urz¹dzenia i2c
//  *aktywacja  - wskaŸnik do zmiennej aktywowanej po zakonczeniu obs³ugi i2c
//
//--------------------------------------------------------------------------------
void i2c_access(uint8_t adres, uint8_t mode, uint8_t pointer, uint8_t *tab, uint8_t count, uint8_t *aktywacja)
{
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
	
	if(TWI_kolejkaPointer < TWI_kolejka_max)
		TWI_kolejkaPointer++;
	else
		TWI_kolejkaPointer = TWI_kolejka_max;
	
	while(TWI_kolejkaPointer >= TWI_kolejka_max)
	{
		I2C_handling();
	}
}


//----------------------------Obs³uga kolejki TWI----------------------------------
void I2C_handling(void)
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
		}
	}
}


//---------------------------Wywo³ania przerwania TWI-----------------------------------------------
//  TWI.mode  - tryb dzia³ania i2c
//        0 - odczyt 1-bajtu, bajt we wkazniku
//        1 - odczyt n-bajtów
//        2 - odczyt n-bajtów ze wskaŸnikiem (pointer = adres wewnêtrzny dla urz¹dzenia i2c)
//        3 - zapis n-bajtów
//        4 - zapis n-bajtów ze wskaŸnikiem (pointer = adres wewnêtrzny dla urz¹dzenia i2c)
//        5 - zapis 1-bajtu, bajt we wskaŸniku (pointer)
//---------------------------------------------------------------------------------
ISR(TWI_vect)
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

#endif /* PRO_TWI_H_ */