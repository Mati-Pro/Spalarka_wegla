/*
 * Pro_Encoder.h
 *
 * Created: 14.11.2020 23:49:14
 *  Author: Pro
 */ 


#ifndef PRO_ENCODER_H_
#define PRO_ENCODER_H_

volatile uint8_t Enc_pinA;
volatile uint8_t Enc_tmpA = 1;
volatile uint8_t Enc_tmpB = 1;
volatile uint8_t Enc_tmpE = 1;
volatile uint8_t Enc_kierunek = 0;
uint8_t Enc_x = 0;


void Enc_setup(void)
{
	// wejscie B
	DDRD &= ~(1 << DDD7);
	PORTD |= (1 << PD7);
	
	// wejscie A
	DDRB &= ~(1 << DDB0);
	PORTB |= (1 << PB0);
	
	// wejscie E
	DDRC &= ~(1 << DDC3);
	PORTC |= (1 << PC3);
	
	// Ustawienie flag rejestrow PCINT
	PCMSK0 = (1 << PCINT0);		//A
	PCMSK1 = (1 << PCINT11);	//E
	PCMSK2 = (1 << PCINT23);	//B
	
	PCICR = (1 << PCIE2) | (1 << PCIE1) | (1 << PCIE0);
}


ISR(PCINT0_vect)	//A
{
	PCMSK0 &= ~(1 << PCINT0);	//A disable
	PCMSK2 = (1 << PCINT23);	//B enable
	
	Enc_tmpB = ((PIND & 0x80) >> 7);	//tmpB=B
}

ISR(PCINT2_vect)	//B
{
	PCMSK0 = (1 << PCINT0);		//A enable
	PCMSK2 &= ~(1 << PCINT23);	//B disable
	
	Enc_pinA = (PINB & 0x01);
	
	if (Enc_pinA == 0 && Enc_tmpA == 1 && Enc_tmpB == 0)
			Enc_kierunek = 1;	//lewo
	
	if (Enc_pinA == 1 && Enc_tmpA == 0 && Enc_tmpB == 0)
			Enc_kierunek = 2;	//prawo
	
	Enc_tmpA = Enc_pinA;
}

ISR(PCINT1_vect)	//E
{
	PCMSK1 &= ~(1 << PCINT11);	//E disable
	
	if(Enc_tmpE == 1)
		Enc_kierunek = 8;	//przycisk nacisniety
	else
		Enc_kierunek = 16;	//przycisk odpuszczony
}

void Enc_handling(void)
{
	if(Enc_kierunek == 8)
	{
		Enc_kierunek = 4;
		sysClock_delay3(&Enc_x, 0);
	}
	
	if(Enc_kierunek == 16)
	{
		Enc_kierunek = 0;
		sysClock_delay3(&Enc_x, 0);
	}
	
	if(Enc_x == 1)
	{
		Enc_x = 0;
		Enc_tmpE = ((PINC & 0x08) >> 3);	//sprawdzenie stanu pinu E
		PCMSK1 = (1 << PCINT11);			//E enable
	}
}
#endif /* PRO_ENCODER_H_ */