/*
 * sysLCD.h
 *
 * Created: 15.04.2021 09:32:54
 *  Author: Pawel Rogoz
 */ 


#ifndef SYSLCD_H_
#define SYSLCD_H_

#include <avr/io.h>

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


struct LCD_ini{
	uint8_t active;
	uint8_t step;
};

struct LCD_pr {
	uint8_t active;
	uint8_t step;
	uint8_t ddram;
};

struct LCD_rs {
	uint8_t active;
	uint8_t step;
	uint8_t rozkaz;
	uint8_t typ;
	uint8_t *koniec;
};

struct LCD_ctr {
	uint8_t mode;
	uint8_t pointer1;
	uint8_t lenght1;
	char *row1;
};


uint8_t LCD_kolejkaPointer;      //wskaünik bufora operacji

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


void LCD_i2c_setup(void);
void LCD_handling(void);
void LCD_clear_bufor(void);
void LCD_wyslij_bufor(void);
void LCD_initiation(void);
void LCD_print(uint8_t mode, uint8_t r1_pointer, uint8_t r1_lenght, char *row1);

void lcd_data(uint8_t wartosc, uint8_t typB, uint8_t *aktywacja, uint8_t *step);
void lcd_i2c_send(void);
void lcd_print(void);


#endif /* SYSLCD_H_ */