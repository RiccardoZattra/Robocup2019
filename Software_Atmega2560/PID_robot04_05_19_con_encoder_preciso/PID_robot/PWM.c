/*
 * PWM.c
 *
 * Created: 09/06/2018 16:19:04
 *  Author: Riki
 */ 
#include "PWM.h"
#include "Seriale.h"
void init_pwm0()
{
	DDRB|=(1<<DDB7);
	DDRG|=(1<<DDG5);
	TCCR0A|= ( 1<<WGM00 ) | ( 1<<WGM01 );
	TCCR0B|= ( 0<<WGM02 );
	
	TCCR0A|= (0<<COM0A0) | (1<<COM0A1) | ( 0<<COM0B0 ) | ( 1<<COM0B1);
}
void start_pwm0 (void)
{
	TCCR0B|= ( 1<<CS02 ) | (0<<CS01) | (1<<CS00);
}
void init_pwm2()
{
	DDRH|=(1<<DDH6);
	DDRB|=(1<<DDB4);
	TCCR2A|= ( 1<<WGM20 ) | ( 1<<WGM21 );
	TCCR2B|= ( 0<<WGM22 );
	
	TCCR2A|= (0<<COM2A0) | (1<<COM2A1) | ( 0<<COM2B0 ) | ( 1<<COM2B1);
}
void start_pwm2 (void)
{
	TCCR2B|= ( 1<<CS22 ) | (1<<CS21) | (1<<CS20);
}
void init_pwm1 (void)
{
	DDRB|=(1<<DDB5);//per abilitare pwm
	
	
	TCCR1A|= ( 1<<WGM10 ) | ( 1<<WGM11 );						//attiva  PWM 10 bit
	TCCR1B|= ( 1<<WGM12 ) | ( 0<<WGM13 );
	
	TCCR1A|= (0<<COM1A0) | (1<<COM1A1) | ( 0<<COM1B0 ) | ( 1<<COM1B1) ;//0c1a-0c1b-0c1c
	
}
void start_pwm1 (void){
	TCCR1B|= ( 0<<CS12 ) | (1<<CS11) | (1<<CS10);//250 Hz
}
void init_pwm4 (void)
{
	DDRH|=(1<<DDH5)|(1<<DDH4)|(1<<DDH3);//per abilitare pwm
	TCCR4A|= ( 1<<WGM40 ) | ( 1<<WGM41 );						//attiva  PWM 10 bit
	TCCR4B|= ( 1<<WGM42 ) | ( 0<<WGM43 );
	
	TCCR4A|= (0<<COM4A0) | (1<<COM4A1) | ( 0<<COM4B0 ) | ( 1<<COM4B1)|( 0<<COM4C0 ) | ( 1<<COM4C1) ;//0c1a-0c1b-0c1c
	
}
void start_pwm4 (void)
{
	DDRA|=(1<<DDA3)|(1<<DDA2)|(1<<DDA1)|(1<<DDA0);//per abilitare direzioni
	TCCR4B|= ( 0<<CS42 ) | (1<<CS41) | (1<<CS40);//250 Hz
}
void m_asx(int16_t duty,int16_t pwm_attuale[])
{
	pwm_attuale[ASX]=duty;
	if(duty<0)
	{
		PORTA|=0b00001000;
		duty*=-1;
	}
	else
	{
		PORTA&=0b11110111;
	}
	OCR4A=duty;
}
void m_adx(int16_t duty,int16_t pwm_attuale[])
{
	pwm_attuale[ADX]=duty;
	if(duty<0)
	{
		PORTA|=0b00000100;
		duty*=-1;
	}
	else
	{
		PORTA&=0b11111011;
	}
	OCR4B=duty;
}
void m_pdx(int16_t duty,int16_t pwm_attuale[])
{
	pwm_attuale[PDX]=duty;
	if(duty<0)
	{
		PORTA|=0b00000010;
		duty*=-1;
	}
	else
	{
		PORTA&=0b11111101;
	}
	OCR4C=duty;
}
void m_psx(int16_t duty,int16_t pwm_attuale[])
{
	pwm_attuale[PSX]=duty;
	if(duty<0)
	{
		PORTA|=0b00000001;
		duty*=-1;
	}
	else
	{
		PORTA&=0b11111110;
	}
	OCR1A=duty;
}
