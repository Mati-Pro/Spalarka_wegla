/*
 * sysISP.c
 *
 * Created: 23.04.2021 17:46:47
 *  Author: Pawel Rogoz
 */ 

#include "sysSPI.h"
#include "sysClock.h"
#include "mainPRG.h"
#include "sysLCD.h"
#include "_DEBUG.h"
#include "_PORT.h"

extern struct men Menu;

volatile uint8_t nrBajtuISP;	//normalnie 2
volatile uint16_t MAX6670_reg;
uint16_t tempSpalin;
uint8_t tempSpalin_niewiarygodny = 1;
uint8_t Display_offset;


void SPI_setup(void)
{
	// wejscie SS, MOSI, SCK
	//DDR_SPI |= (1 << DD_SS) | (1<<DD_MOSI) | (1<<DD_SCK);	//SS, MOSI, SCK -> OUTPUT
	//PORT_SPI &= ~(1 << P_SS);	//SS -> low
	
	DDRB |= (1<< PB5) | (1<< PB3) | ( 1 << PB2);
	
	SPCR = (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << CPHA) | (1 << SPR0);	//startSPI;
	//SPDR = 0xff;
}


void SPI_handling(void)
{	
	if(nrBajtuISP == 4)
	{		
		tempSpalin_niewiarygodny = (MAX6670_reg >> 1) & 0x01;
		tempSpalin = (MAX6670_reg >> 3) & 0xFFF;
		tempSpalin /= 4;
		
		SPI_wyswietl_pomiar();
		nrBajtuISP = 0;
		//Time_delay(1000000, (uint8_t *)&nrBajtuISP, 0);
	}
	

	if(nrBajtuISP == 1)
	{
		PORT_SPI &= ~(1 << P_SS);	//SS -> low
		nrBajtuISP++;
		SPDR = 0xff;
	}
}


void SPI_isr(void)
{
	switch(nrBajtuISP)
	{
		case 2:
		MAX6670_reg = SPDR << 8;	//MSB
		nrBajtuISP++;
		SPDR = 0xff;
		break;
		
		case 3:
		MAX6670_reg |= SPDR;	//LSB
		nrBajtuISP++;
		PORT_SPI |= (1 << P_SS);	//SS -> high
	}
}


void SPI_wyswietl_pomiar(void)
{
	if(Menu.aktywacja == 0)
	{
		if(tempSpalin_niewiarygodny)
		{
			LCD_row2[3] = 0x2D;		//setki
			LCD_row2[4] = 0x2D;		//dziesiatki
			LCD_row2[5] = 0x2D;		//jednostki
			LCD_row2[6] = 0x20;
		}
		else
		{
			LCD_row2[3] = (tempSpalin / 1000);  //tysiace
			tempSpalin = tempSpalin - LCD_row2[3] * 1000;
			LCD_row2[3] += 0x30;
			if(LCD_row2[3] == 0x30)
			Display_offset = 0;
			else
			Display_offset = 1;
			
			LCD_row2[3+Display_offset] = (tempSpalin / 100);  //setki
			tempSpalin = tempSpalin - LCD_row2[3+Display_offset] * 100;
			LCD_row2[3+Display_offset] += 0x30;
			//if(LCDrow[poz] == 0x30) LCDrow[poz] = 0x20;
			
			LCD_row2[4+Display_offset] = (tempSpalin / 10);   //dziesiatki
			tempSpalin = tempSpalin - LCD_row2[4+Display_offset] * 10;
			LCD_row2[4+Display_offset] += 0x30;
			
			LCD_row2[5+Display_offset] = tempSpalin;          //jednostki
			LCD_row2[5+Display_offset] += 0x30;
		}
		LCD_print(2, 4, 4, LCD_row2);
	}
}
