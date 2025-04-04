/*
 * interrupt.c
 *
 * Created: 09/06/2018 16:16:54
 *  Author: Riki
 */ 
#include "interrupt.h"
void init_interrupt(void)
{
	DDRA|=(0<<DDA4)|(0<<DDA5)|(0<<DDA6)|(0<<DDA7);
	//PORTA|=0b11110000;
	//interrupt esterni
	DDRD|=(0<<DDD3)|(0<<DDD2);//abilito in input il primo bit della porta D (INT2/3)
	DDRE|=(0<<DDE4)|(0<<DDE5);//abilito in input il primo bit della porta D (INT4/5)
	PORTE|=(1<<PE4)|(1<<PE5);//attivo r di pull-up per rispettivi int
	PORTD|=(1<<PD3)|(1<<PD2);//attivo r di pull-up per rispettivi int
	EICRA|=(1<<ISC21)|(1<<ISC20)|(1<<ISC31)|(1<<ISC30);
	EICRB|=(1<<ISC41)|(1<<ISC40)|(1<<ISC51)|(1<<ISC50);
	EIMSK|=(1<<INT5)|(1<<INT2)|(1<<INT3)|(1<<INT4);
	
	//set timer
	OCR3A=499;
	TCCR3A|=(0<<WGM31) | (0<<WGM30);
	TCCR3B|=(0<<WGM33) | (1<<WGM32);
	TIMSK3=(1<<OCIE3A);
	TCCR3B |= (0 << CS32) | (0<<CS31) | (1<< CS30);
}
