/*
 * millis.c
 *
 * Created: 02/11/2017 16:29:12
 *  Author: Riccardo e Matteo
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include <util/atomic.h>
#include "millis.h"
#include "Seriale.h"
#define USART_BAUDRATE 9600
#define BAUDR 103
uint32_t var_millis;
//int registro_controllo[11][2];
uint32_t registro_controllo[11][2];

ISR(TIMER5_COMPA_vect)
{
	var_millis++;
}

void init_millis()
{
	OCR5A=2000;						// set MAX contatore: f=fclk_uP/(2 N (1+OCR1A)) => f=16000000/(2 1024 (1+7809))=1 (p 145)
	
	TCCR5A|=(0<<WGM51) | (0<<WGM50);			// set modo di funzionamento CTC : Clear timer on Compare match (P 145, 154, 156)
	TCCR5B|=(0<<WGM53) | (1<<WGM52);
	
	TIMSK5=(1<<OCIE5A) ;	// enable timer1 compare interrupt  (p 131)
	
	TCCR5B |= (0 << CS52) | (1<<CS51) | (0 << CS50);
	
	for(int l=0;l<11;l++)
	{
		registro_controllo[l][0]=0;
		registro_controllo[l][1]=0;
	}
}

uint32_t millis()
{
	//ATOMIC_BLOCK(ATOMIC_FORCEON)
	//{
		return var_millis;
	//}
}

void set_millis(int pos)
{
	registro_controllo[pos][0]=millis();//Nella prima riga di matrice salva valore
	registro_controllo[pos][1]=1;		//Nella seconda riga salva se settato(1) o eliminato (0)
}

uint32_t/*int*/ get_millis(int pos)
{
			//chiedo se è =1 con il ?		//prima parte se è vero			//seconda parte se è falso
	return registro_controllo[pos][1]==1 ? millis()-registro_controllo[pos][0] : 1000000;
}

void del_millis(int pos)//serve per cancellare millis
{
	registro_controllo[pos][1]=0;
}