/*
 * Seriale_new.h
 *
 * Created: 01/06/2018 17:34:00
 *  Author: Riki
 */ 


#ifndef SERIALE_NEW_H_
#define SERIALE_NEW_H_
#include <stdlib.h>
#include <stdio.h>
#define USART_BAUDRATE 9600
#define BAUDR3 1
#define BAUDR0 8//3,250000k--8,115200
#define BAUDR2 8
#define NUOVO 0
#define VECCHIO 1

void USART_0( unsigned int BAUD);
void USART_3( unsigned int BAUD);
void USART_Tx_3( unsigned char data );
void USART_2( unsigned int BAUD);
void USART_Tx_2(uint8_t data );
void flush_buffer(void);
int USART_Rx_3( void );
void receive_rasp (int16_t *dato);
void Serial_print(const char string[]);
void Serial_int (int num/*,char type,uint8_t base*/);
void Serial_float(double num/*,uint8_t cifre*/,uint8_t digit);
void Usart_tx_IO(char data,FILE *stream);
void Serial_println(const char string[]);
void set_val_error();
uint8_t getError();




#endif /* SERIALE_NEW_H_ */