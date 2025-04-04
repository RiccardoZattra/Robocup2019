/*
 * Seriale_new.c
 *
 * Created: 01/06/2018 17:33:42
 *  Author: Riki
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "Seriale.h"

int a=-1;
int messaggio[5]={0,0};//azzerare tutto a parte ultima posizione
int i=0;
uint8_t parity_check=0,errore=0;

FILE mystdout = FDEV_SETUP_STREAM(Usart_tx_IO,NULL, _FDEV_SETUP_WRITE);

void USART_0( unsigned int BAUD) //inizializza seriale
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(BAUD>>8);  // 9600 bit/s
	UBRR0L = (unsigned char)BAUD;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit, none parity  "9600,N,8,1" */
	UCSR0C = 0b00000110;
	stdout=&mystdout;
} // USART_Init
void Usart_tx_IO(char data,FILE *stream)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0=data;//0b00001111 ;
}
void USART_3( unsigned int BAUD) //inizializza seriale
{
	/* Set baud rate */
	UBRR3H = (unsigned char)(BAUD>>8);  // 9600 bit/s
	UBRR3L = (unsigned char)BAUD;
	/* Enable receiver and transmitter */
	UCSR3B = (1<<RXEN3)|(1<<TXEN3);
	/* Set frame format: 8data, 1stop bit, none parity  "9600,N,8,1" */
	UCSR3C = 0b00100110;//0b00000110
	DDRB&=0b01111111;
	DDRB|=0b01000000;
	PORTB|=0b10000000;
} // USART_Init
void USART_Tx_3( /*unsigned char*/ uint8_t data )//Trasmissione seriale (un carattere)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR3A & (1<<UDRE3)) );
	/* Put data into buffer, sends the data */
	UDR3=data;//0b00001111 ;
}
void USART_2( unsigned int BAUD)
{
	UBRR2H = (unsigned char)(BAUD>>8);
	UBRR2L = (unsigned char)BAUD;
	UCSR2B = (1<<RXEN2)|(1<<TXEN2);
	UCSR2C = 0b00000110;
}
void USART_Tx_2(uint8_t data )
{
	while ( !( UCSR2A & (1<<UDRE2)) );
	UDR2=data;
}
void flush_buffer(void)
{
	unsigned char cestino;
	while(UCSR3A & (1<<RXC3))
	cestino=UDR3;
}
int USART_Rx_3( void )//ricezione seriale (un carattere)
{
	if (UCSR3A & (1<<RXC3))
	{
		if(UCSR3A & (1<<UPE3))
		return -2;
		else
		return UDR3;
	}
	else
	return -1;
}
void receive_rasp (int16_t *dato)
{
	//dato[VECCHIO]=dato[NUOVO];
	a=USART_Rx_3();
	if(a!=-1)
	{
		if(a!=-2)
		{
			messaggio[i]=a;
			/*Serial_int(messaggio[i]);
			Serial_print("\t");
			Serial_int(i);
			Serial_println("");*/
		}
		else
		{
			parity_check++;
			Serial_print("p++");
		}
		i++;
	}
	if(i==2)
	{
		if(parity_check==0)
		{
			//int16_t valore=(messaggio[1]<<8)|messaggio[0];
			/*Serial_int(messaggio[1]<<8,'u',10);
			Serial_print("\t");
			Serial_int(messaggio[0],'u',10);
			Serial_println("");*/
			*dato/*[NUOVO]*/=(messaggio[1]<<8)|messaggio[0];
			//Serial_int(valore,'s',10);
			flush_buffer();
			//USART_Tx_3(65);
			messaggio[0]=0;
			messaggio[1]=0;
			//errore=0;
			i=0;
		}
		else
		{
			//USART_Tx_3(66);
			flush_buffer();
			Serial_print("\t\terrore");
			Serial_println("");
			parity_check=0;
			i=0;
			errore=1;
		}
	}
}
uint8_t getError()
{
	return errore;
}
void set_val_error()
{
	errore=0;
}
void Serial_print(const char string[])
{
	int h=0;
	while(string[h])
	{
		USART_Tx_2(string[h]);//per spedire con bt
		//putchar(string[h]);//per spedire con cavo
		h++;
	}
	
}
void Serial_println(const char string[])
{
	int k=0;
	while(string[k])
	{
		USART_Tx_2(string[k]);//per spedire con bt
		//putchar(string[k]);//per spedire con cavo
		k++;
	}
	//putchar('\n');//per spedire con cavo
	USART_Tx_2('\n');//per spedire con bt
	USART_Tx_2(13);
}
void Serial_float(double num/*,uint8_t cifre*/,uint8_t digit)
{
	char buffer[33];
	dtostrf(num,-1/*cifre*/,digit,buffer);
	Serial_print(buffer);
}
void Serial_int (int num/*,char type,uint8_t base*/)
{
	char buffer[33]={0};
	//if(type=='s' || type=='S')
	//{
		ltoa(num,buffer,10/*base*/);
		
		//ultoa((uint64_t)num,buffer,10);//bisogna mettere uint32_t, è stato modificato per vedere il problema della virata veloce
		Serial_print(buffer);
	//}
	//else if(type=='u' || type=='U')
	//{
		//ultoa((uint16_t)num,buffer,base);//bisogna mettere uint32_t, è stato modificato per vedere il problema della virata veloce
		//Serial_print(buffer);
	//}
	
}