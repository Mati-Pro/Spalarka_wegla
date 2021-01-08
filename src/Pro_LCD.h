/*
 * Pro_LCD.h
 *
 * Created: 22.10.2020 15:22:42
 *  Author: Pro
 */ 


#ifndef PRO_LCD_H_
#define PRO_LCD_H_

#define LCD_kolejka_max		5

#define LCD_RS 8
#define LCD_EN 9
#define LCD_RW 6
#define LCD_INIT_time1 50000
#define LCD_INIT_time2 5000
#define LCD_INIT_time3 150
#define LCD_INIT_timeR 50
#define LCD_INIT_timeEN 10

#define LCD_Display_Clear	0x01
#define LCD_Return_Home		0x02
#define LCD_Entry_Mode		0x04
#define przesuwanie_w_prawo	0x02
#define przesuwanie_w_lewo	0x00
#define przesuwanie_okna	0x01	
#define przesuwanie_kurora	0x00

#define LCD_Display_Control	0x08
#define display_on			0x04
#define cursor_on			0x02
#define blink_on			0x01

#define LCD_Display_Cursor_Shift	0x10
#define display_shift		0x08
#define cursor_shift		0x00
#define shift_right			0x04
#define shift_left			0x00

#define LCD_Function_Set	0x20
#define data_lenght_8		0x10
#define data_lenght_4		0x00
#define lines_2				0x08
#define lines_1				0x00
#define dots_10				0x04
#define dots_8				0x00

#define LCD_Set_CGRAM		0x40
#define LCD_Set_DDRAM		0x80

//deklaracja funkcji
void lcd_setup(void);
void lcd_i2c_setup(void);
void LCD_initiation(void);
void lcd_data(uint8_t wartosc, uint8_t typB, uint8_t *aktywacja, uint8_t *step);
void lcd_send(void);
void lcd_i2c_send(void);
void LCDprint(uint8_t mode, uint8_t r1_pointer, uint8_t r1_lenght, char *row1);
void lcd_print(void);

//------------------------------------------LCD---------------------

struct LCD_ini {
	uint8_t active;
	uint8_t step;
};

struct LCD_ini LCD_INIT = {1, 1};

struct LCD_pr {
	uint8_t active;
	uint8_t step;
	uint8_t ddram;
};

struct LCD_pr LCD_PRINT = {0, 0, 1};

struct LCD_rs {
	uint8_t active;
	uint8_t step;
	uint8_t rozkaz;
	uint8_t typ;
	uint8_t *koniec;
};

struct LCD_rs LCD_SEND = {0, 1, 0, 8};

struct LCD_ctr {
	uint8_t mode;
	uint8_t pointer1;
	uint8_t lenght1;
	char *row1;
};

struct LCD_ctr LCD_CONTROL = {0, 0, 0};
struct LCD_ctr LCD_kolejka[LCD_kolejka_max];	//bufor operacji dla lcd_print

uint8_t LCD_kolejkaPointer;      //wskaŸnik bufora operacji

char LCD_row1[20];
char LCD_row2[20];
char LCD_row3[20];
char LCD_row4[20];


//------------------------------------------I2C_LCD---------------------
#define LCD_I2C_ADRES 0x3F  //20 27 - 38 3F
#define LCD_I2C_LED   0x08
#define LCD_I2C_EN    0x04
#define LCD_I2C_RW    0x02
#define LCD_I2C_RS    0x01

uint8_t LCD_I2C_DATA;


//--------------------------------Procedury obd³ugi LCD--------------------------
//--
//-- lcd_setup      - inicjacja portów bitowych dla lcd
//-- lcd_initiation - inicjacja i reset wyswietlacza lcd
//-- lcd_data       - procedura wys³ania danej do wyswietlacza lcd
//-- lcd_send       -
//-- lcd_i2c_send   -
//-- LCDprint       - procedura wydruku na wyswietlaczu lcd
//-- lcd_print      -
//--
//--------------------------------------------------------------------------------


//------------------------------Inicjacja portów bitowych dla lcd-----------------------------
void LCD_setup(void)
{
	/*
	pinMode(LCD_RS, OUTPUT);
	//digitalWrite(LCD_RS, LOW);
	
	pinMode(LCD_EN, OUTPUT);
	//digitalWrite(LCD_EN, LOW);
	
	DDRD = 0xF0;  //b³ad - dla 8 bit powinno byæ inaczej
	PORTD = 0x00;
	LCD_kolejkaPointer = 0;
	*/
}


//------------------------------Inicjacja portów bitowych dla i2c_lcd-----------------------------
void LCD_i2c_setup(void)
{
	i2c_access(LCD_I2C_ADRES, 5, LCD_I2C_LED, 0, 0, 0);
	LCD_kolejkaPointer = 0;
}


//------------------------------Procedura LCD_handling-----------------------------
void LCD_handling(void)
{
	//lcd_send();		//obs³uga wysylania komend do lcd (magistrala rownolegla)
	lcd_i2c_send();     //obs³uga wysylania komend do lcd_i2c
	lcd_print();		//obs³uga drukowania na wyswietlaczu lcd
}


//----------------------------Procedura czyszczenia bufora dla LCD---------------
void LCD_clear_bufor(void)
{
	for(uint8_t i=0; i < 20; i++)
	{
		LCD_row1[i] = 0x20;
		LCD_row2[i] = 0x20;
		LCD_row3[i] = 0x20;
		LCD_row4[i] = 0x20;
	}
}

//----------------------------Procedura wyslania ca³ego bufora do LCD---------------
void LCD_wyslij_bufor(void)
{
	LCDprint(1, 0, 20, LCD_row1);
	LCDprint(2, 0, 20, LCD_row2);
	LCDprint(3, 0, 20, LCD_row3);
	LCDprint(4, 0, 20, LCD_row4);
}

//------------------------------Procedura lcd_initiation-----------------------------
void LCD_initiation(void)
{
	if(LCD_INIT.active == 1) {
		switch (LCD_INIT.step) {
			
			case 1:	//opoznienie po zalaczeniu zaslania 50ms
			sysClock_delay1(LCD_INIT_time1, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 2:	//Function_Set - podejscie 1, startujemy z trybu 8-bitowego
			lcd_data((LCD_Function_Set | data_lenght_8), 8, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 3:	//opoznienie 1 5ms
			sysClock_delay1(LCD_INIT_time2, &LCD_INIT.active, &LCD_INIT.step);
			break;
			
			case 4:	//Function_Set - podejscie 2
			lcd_data((LCD_Function_Set | data_lenght_8), 8, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 5:	//opoznienie 2 5ms
			sysClock_delay1(LCD_INIT_time2, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 6:	//Function_Set - podejscie 3
			lcd_data((LCD_Function_Set | data_lenght_8), 8, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 7:	//opoznienie 3 150us
			sysClock_delay1(LCD_INIT_time3, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 8: //LCD_Function_Set - podejscie 4, finalnie 4-bit 
			lcd_data((LCD_Function_Set | data_lenght_4), 8, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 9: //Function set - 4-bit, dwie linie, matryca 5x8
			lcd_data((LCD_Function_Set | data_lenght_4 | lines_2 | dots_8), 4, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 10: //Display ON
			lcd_data((LCD_Display_Control | display_on), 4, &LCD_INIT.active, &LCD_INIT.step);
			break;
			
			case 11: //Display clear
			lcd_data(LCD_Display_Clear, 4, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 12:
			sysClock_delay1(2000, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 13: //Entry mode set
			lcd_data((LCD_Entry_Mode | przesuwanie_w_prawo | przesuwanie_kurora), 4, &LCD_INIT.active, &LCD_INIT.step);
			break;
			
			case 14:	//zakonczenie inicjacji wyswietlacza LCD
			LCD_INIT.active = 0;
			LCD_INIT.step = 0;
			SysStatus |= 1;
			break;
		}
	}
	LCD_handling();
}


//------------------------------Procedura lcd_data-----------------------------
//  lcd_data(wartosc, rozB, *aktywacja, *step)
//
//  wartosc - wartosc danej-rozkazu do wyslania dl lcd
//  typB    - dlugosc danej dla wyswietlacza lcd
//          4 - wartosc 4-bitowa
//          8 - wartosc 8-bitowa
//  *aktywacja  - wskaznik do aktywacji po wyslaniu danej do lcd
//  *step       - wskaznik do zwiekszenia kroku dla sekwencera
//
//-----------------------------------------------------------------------------
void lcd_data(uint8_t wartosc, uint8_t typB, uint8_t *aktywacja, uint8_t *step)
{
	*aktywacja = 0;
	(*step)++;
	
	LCD_SEND.active = 1;
	LCD_SEND.step = 1;
	LCD_SEND.rozkaz = wartosc;
	LCD_SEND.typ = typB;
	LCD_SEND.koniec = aktywacja;
}


//------------------------------Procedura LCD_SEND-----------------------------
//  LCD_SEND.typ
//  8,9 - 8 bitowe, 8 - rozkaz, 9 - dana
//  4,5 - 4 bitowe, 4 - rozkaz, 5 - dana
//
//--------------------------------------------------------------------------------
void lcd_send(void)
{
	/*
	if(LCD_SEND.active) {

		if(LCD_SEND.typ & 0x01)  // jeœli instrukcja RS = 0, jeœli dana RS = 1
		digitalWrite(LCD_RS, HIGH);
		else
		digitalWrite(LCD_RS, LOW);
		
		switch (LCD_SEND.step) {
			
			case 1:
			PORTD = LCD_SEND.rozkaz & 0xF0; //b³ad - dla 8-bit przesy³a tylko najstarsza czesc
			digitalWrite(LCD_EN, HIGH);
			sysClock_delay1(10, &LCD_SEND.active, &LCD_SEND.step);
			break;

			case 2:
			if(LCD_SEND.typ & 0x08) //jesli transmisja 8 - bitowa
			{
				LCD_SEND.step = 4;
				break;
			}
			digitalWrite(LCD_EN, LOW);
			sysClock_delay1(LCD_INIT_timeEN, &LCD_SEND.active, &LCD_SEND.step);
			break;

			case 3:
			LCD_SEND.rozkaz = LCD_SEND.rozkaz & 0x0F;
			PORTD = LCD_SEND.rozkaz << 4;
			digitalWrite(LCD_EN, HIGH);
			sysClock_delay1(LCD_INIT_timeEN, &LCD_SEND.active, &LCD_SEND.step);
			break;

			case 4:
			digitalWrite(LCD_EN, LOW);
			sysClock_delay1(LCD_INIT_timeR, &LCD_SEND.active, &LCD_SEND.step);
			break;

			case 5:
			LCD_SEND.active = false;
			LCD_SEND.step = 1;
			*LCD_SEND.koniec = true;
			break;
		}
	}
	*/
}


//------------------------------Procedura LCD_i2c_SEND - tylko 4 bit ----------------------------
//  LCD_SEND.typ
//  0 - wyslanie instrukcji
//  1 - wyslanie danej
//
//--------------------------------------------------------------------------------
void lcd_i2c_send(void)
{
	if(LCD_SEND.active == 1) {
		switch (LCD_SEND.step) {
			
			case 1:
			LCD_SEND.active = 0;
			LCD_SEND.step++;
			LCD_I2C_DATA = (LCD_SEND.rozkaz & 0xF0) | LCD_I2C_LED | LCD_I2C_EN | (LCD_I2C_RS & LCD_SEND.typ);
			i2c_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
			break;

			case 2:
			if(LCD_SEND.typ & 0x08) //jesli transmisja 8 - bitowa
			{
				LCD_SEND.step = 4;
				break;
			}
			LCD_SEND.active = 0;
			LCD_SEND.step++;
			LCD_I2C_DATA = (LCD_SEND.rozkaz & 0xF0) | LCD_I2C_LED | (LCD_I2C_RS & LCD_SEND.typ);
			i2c_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
			break;
			
			case 3:
			LCD_SEND.active = 0;
			LCD_SEND.step++;
			LCD_SEND.rozkaz = LCD_SEND.rozkaz & 0x0F;
			LCD_SEND.rozkaz = LCD_SEND.rozkaz << 4;
			LCD_I2C_DATA = (LCD_SEND.rozkaz & 0xF0) | LCD_I2C_LED | LCD_I2C_EN | (LCD_I2C_RS & LCD_SEND.typ);
			i2c_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
			break;

			case 4: //tutaj
			LCD_SEND.active = 0;
			LCD_SEND.step++;
			LCD_I2C_DATA = (LCD_SEND.rozkaz & 0xF0) | LCD_I2C_LED | (LCD_I2C_RS & LCD_SEND.typ);
			i2c_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
			break;
			
			case 5:
			LCD_SEND.active = 0;
			LCD_SEND.step = 1;
			*LCD_SEND.koniec = 1;
			break;
		}
	}
}


//------------------------------Procedura drukowania na lcd_print-----------------------------
//  LCDprint(mode, r1_pointer, r1_lenght, r2_pointer, r2_lenght, *row1, *row2)
//
//  mode  - tryb wyswietlania lcd
//        1 - pierwszy wiersz
//        2 - drugi wiersz
//        3 - trzeci wuersz
//        4 - czwarty wiersz
//  r1_pointer  - pozycja w wierszu (liczona od 0)
//  r1_lenght   - ilosc znakow do wyswietlenia w wierszu
//  *row1 - wskaznik do tablicy - jeden wiersz wyswietlacza
//
//--------------------------------------------------------------------------------------------
void LCDprint(uint8_t mode, uint8_t r1_pointer, uint8_t r1_lenght, char *row1)
{
	LCD_kolejka[LCD_kolejkaPointer].mode = mode;
	LCD_kolejka[LCD_kolejkaPointer].pointer1 = r1_pointer;
	LCD_kolejka[LCD_kolejkaPointer].lenght1 = r1_lenght;
	LCD_kolejka[LCD_kolejkaPointer].row1 = row1;

	if((LCD_PRINT.step == 0) && (LCD_kolejkaPointer == 0)) //jesli jeden element w kolejce uruchom lcd_print
	{
		LCD_PRINT.active = 1;
		LCD_PRINT.step = 1;
		LCD_CONTROL.mode = mode;
		LCD_CONTROL.pointer1 = r1_pointer;
		LCD_CONTROL.lenght1 = r1_lenght;
		LCD_CONTROL.row1 = row1;
	}
	
	if(LCD_kolejkaPointer < LCD_kolejka_max)
		LCD_kolejkaPointer++;
	else
		LCD_kolejkaPointer = LCD_kolejka_max;
	
	while (LCD_kolejkaPointer >= LCD_kolejka_max)
	{
		I2C_handling();
		//lcd_send();		//obs³uga wysylania komend do lcd (magistrala rownolegla)
		lcd_i2c_send();     //obs³uga wysylania komend do lcd_i2c
		lcd_print();		//obs³uga drukowania na wyswietlaczu lcd
	}
}


//------------------------------Procedura lcd_print-----------------------------
void lcd_print(void)
{	
	if((LCD_PRINT.active == 1) && (LCD_INIT.active == 0) && (LCD_INIT.step == 0))
	{
		switch(LCD_PRINT.step) {
			
			case 1:// LCD adres DDRAM
			switch(LCD_CONTROL.mode)
			{
				case 2:
				LCD_PRINT.ddram = 0x40;
				break;

				case 3:
				LCD_PRINT.ddram = 0x14;
				break;

				case 4:
				LCD_PRINT.ddram = 0x54;
				break;
				
				case 1:
				default:
				LCD_PRINT.ddram = 0;
				break;
			}
			lcd_data((0x80+LCD_PRINT.ddram+LCD_CONTROL.pointer1), 4, &LCD_PRINT.active, &LCD_PRINT.step); //instrukcja
			break;

			case 2:
			lcd_data(LCD_CONTROL.row1[LCD_CONTROL.pointer1], 5, &LCD_PRINT.active, &LCD_PRINT.step);  //dana
			break;

			case 3:
			LCD_CONTROL.lenght1--;
			if(LCD_CONTROL.lenght1 > 0) {
				LCD_CONTROL.pointer1++;
				LCD_PRINT.step--;
			}
			else
			LCD_PRINT.step++;
			break;
			
			case 4:
			LCD_kolejkaPointer--;
			if(LCD_kolejkaPointer > 0)
			{
				for(uint8_t i = 0; i < LCD_kolejkaPointer; i++)
				LCD_kolejka[i] = LCD_kolejka[i+1];

				LCD_PRINT.step = 1;
				LCD_CONTROL.mode = LCD_kolejka[0].mode;
				LCD_CONTROL.pointer1 = LCD_kolejka[0].pointer1;
				LCD_CONTROL.lenght1 = LCD_kolejka[0].lenght1;
				LCD_CONTROL.row1 = LCD_kolejka[0].row1;
			}
			else
			{
				LCD_PRINT.active = 0;
				LCD_PRINT.step = 0;
			}
			break;
		}
	}
}

#endif /* PRO_LCD_H_ */