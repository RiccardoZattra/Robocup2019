/*
 * EEPROM.c
 *
 * Created: 01/03/2018 14:31:46
 *  Author: Riccardo e Matteo
 */ 
#include "EEPROM.h"
#include <avr/io.h>
#include <util/delay.h>

void write_EEPROM(uint16_t address,uint8_t data)
{
	while(EECR & (1<<EEPE));//aspetto che la scritura precedente sia finita
	EEAR=address;//carico l'indirizzo di memoria all'interno della quale voglio scrivere
	EEDR=data;//inserisco il dato da scrivere nella cella di memoria sopra dichiarata
	EECR|=(0<<EEPM0)|(0<<EEPM1)|(1<<EEMPE);//con i primi 2 bit abilito eliminazione dato e scrittura in un colpo solo con l'ultimo abiliti la scrittura
	EECR|=(1<<EEPE);//faccio cominciare la scrittura
}

uint8_t read_EEPROM(uint16_t address)
{
	while(EECR & (1<<EEPE));//aspetto che la scritura precedente sia finita
	EEAR=address;//carico l'indirizzo di memoria all'interno della quale voglio scrivere
	EECR=(1<<EERE);//faccio iniziare la lettura dalla cella di memoria sopra dichiarata
	return EEDR;//EDDR nella fase di lettura ritorna il dato letto
}
