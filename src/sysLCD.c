/*
 * sysLCD.c
 *
 * Created: 15.04.2021 09:38:09
 *  Author: Pawel Rogoz
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include "sysClock.h"
#include "sysTWI.h"
#include "sysLCD.h"
#include "USART.h"

extern uint8_t SysStatus;

struct LCD_ini LCD_init = {1,1};
struct LCD_pr LCD_PRINT = {0, 0, 1};
struct LCD_rs LCD_SEND = {0, 1, 0, 8};
struct LCD_ctr LCD_CONTROL = {0, 0, 0};
//struct LCD_ctr LCD_kolejka[LCD_kolejka_max];	//bufor operacji dla lcd_print
struct LCD_ctr *LCD_kolejka;

//------------------------------Inicjacja portów bitowych dla i2c_lcd-----------------------------
void LCD_i2c_setup(void)
{
	//LCD_init = {1,1};
		
	TWI_access(LCD_I2C_ADRES, 5, LCD_I2C_LED, 0, 0, 0);
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
	LCD_print(1, 0, 20, LCD_row1);
	LCD_print(2, 0, 20, LCD_row2);
	LCD_print(3, 0, 20, LCD_row3);
	LCD_print(4, 0, 20, LCD_row4);
}


//------------------------------Procedura drukowania na lcd_print-----------------------------
//  LCDprint(mode, r1_pointer, r1_lenght, *row1)
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
void LCD_print(uint8_t mode, uint8_t r1_pointer, uint8_t r1_lenght, char *row1)
{
	if(LCD_kolejkaPointer == 0)	//jesli jeszcze nie bylo kolejki to stworz j¹
		LCD_kolejka = (struct LCD_ctr *)calloc(LCD_kolejkaPointer + 1, sizeof(struct LCD_ctr));
	else
		LCD_kolejka = (struct LCD_ctr *)realloc(LCD_kolejka, (LCD_kolejkaPointer+1)*sizeof(struct LCD_ctr));
	
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
	
	LCD_kolejkaPointer++;
	
	/*
	if(LCD_kolejkaPointer < LCD_kolejka_max)
	LCD_kolejkaPointer++;
	else
	LCD_kolejkaPointer = LCD_kolejka_max;
	
	while (LCD_kolejkaPointer >= LCD_kolejka_max)
	{
		TWI_handling();
		//lcd_send();		//obs³uga wysylania komend do lcd (magistrala rownolegla)
		lcd_i2c_send();     //obs³uga wysylania komend do lcd_i2c
		lcd_print();		//obs³uga drukowania na wyswietlaczu lcd
	}
	*/
}



//------------------------------Procedura lcd_initiation-----------------------------
void LCD_initiation(void)
{
	if(LCD_init.active == 1) {
		
#ifdef _DEBUG_LCD_init_
		printString("LCD init =");
		printByte(LCD_init.step);
		printString(" \r\n");
#endif // _DEBUG_LCD_init_
				
		switch (LCD_init.step) {
			
			case 1:	//opoznienie po zalaczeniu zaslania 50ms
			Time_delay(LCD_INIT_time1, &LCD_init.active, &LCD_init.step);
			break;

			case 2:	//Function_Set - podejscie 1, startujemy z trybu 8-bitowego
			lcd_data((LCD_Function_Set | data_lenght_8), 8, &LCD_init.active, &LCD_init.step);
			break;

			case 3:	//opoznienie 1 5ms
			Time_delay(LCD_INIT_time2, &LCD_init.active, &LCD_init.step);
			break;
			
			case 4:	//Function_Set - podejscie 2
			lcd_data((LCD_Function_Set | data_lenght_8), 8, &LCD_init.active, &LCD_init.step);
			break;

			case 5:	//opoznienie 2 5ms
			Time_delay(LCD_INIT_time2, &LCD_init.active, &LCD_init.step);
			break;

			case 6:	//Function_Set - podejscie 3
			lcd_data((LCD_Function_Set | data_lenght_8), 8, &LCD_init.active, &LCD_init.step);
			break;

			case 7:	//opoznienie 3 150us
			Time_delay(LCD_INIT_time3, &LCD_init.active, &LCD_init.step);
			//SysClock_delay(LCD_INIT_time2, &LCD_INIT.active, &LCD_INIT.step);
			break;

			case 8: //LCD_Function_Set - podejscie 4, finalnie 4-bit
			lcd_data((LCD_Function_Set | data_lenght_4), 8, &LCD_init.active, &LCD_init.step);
			break;

			case 9: //Function set - 4-bit, dwie linie, matryca 5x8
			lcd_data((LCD_Function_Set | data_lenght_4 | lines_2 | dots_8), 4, &LCD_init.active, &LCD_init.step);
			break;

			case 10: //Display ON
			lcd_data((LCD_Display_Control | display_on), 4, &LCD_init.active, &LCD_init.step);
			break;
			
			case 11: //Display clear
			lcd_data(LCD_Display_Clear, 4, &LCD_init.active, &LCD_init.step);
			break;

			case 12:
			Time_delay(2000, &LCD_init.active, &LCD_init.step);
			break;

			case 13: //Entry mode set
			lcd_data((LCD_Entry_Mode | przesuwanie_w_prawo | przesuwanie_kurora), 4, &LCD_init.active, &LCD_init.step);
			break;
			
			case 14:	//zakonczenie inicjacji wyswietlacza LCD
			LCD_init.active = 0;
			SysStatus |= 1;
			break;
		}
	}
	LCD_handling();
}


//------------------------------Procedura lcd_print-----------------------------
void lcd_print(void)
{
	if((LCD_PRINT.active == 1) && (SysStatus & 0x01))
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
			//redukcja kolejki
			LCD_kolejkaPointer--;
			if(LCD_kolejkaPointer > 0)
			{
				for(uint8_t i = 0; i < LCD_kolejkaPointer; i++)
					LCD_kolejka[i] = LCD_kolejka[i+1];
				
				LCD_kolejka = (struct LCD_ctr *)realloc(LCD_kolejka, (LCD_kolejkaPointer+1)*sizeof(struct LCD_ctr));

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
				free(LCD_kolejka);
			}
			break;
		}
	}
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
			TWI_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
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
			TWI_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
			break;
			
			case 3:
			LCD_SEND.active = 0;
			LCD_SEND.step++;
			LCD_SEND.rozkaz = LCD_SEND.rozkaz & 0x0F;
			LCD_SEND.rozkaz = LCD_SEND.rozkaz << 4;
			LCD_I2C_DATA = (LCD_SEND.rozkaz & 0xF0) | LCD_I2C_LED | LCD_I2C_EN | (LCD_I2C_RS & LCD_SEND.typ);
			TWI_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
			break;

			case 4: //tutaj
			LCD_SEND.active = 0;
			LCD_SEND.step++;
			LCD_I2C_DATA = (LCD_SEND.rozkaz & 0xF0) | LCD_I2C_LED | (LCD_I2C_RS & LCD_SEND.typ);
			TWI_access(LCD_I2C_ADRES, 5, LCD_I2C_DATA, 0, 0, &LCD_SEND.active);
			break;
			
			case 5:
			LCD_SEND.active = 0;
			LCD_SEND.step = 1;
			*LCD_SEND.koniec = 1;
			break;
		}
	}
}




