/*
 * EEPROM.h
 *
 * Created: 01/03/2018 14:31:57
 *  Author: Riccardo e Matteo
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

/*extern*/ void write_EEPROM(uint16_t address,uint8_t data);	//invio di indirizzo seguito dal dato dal salvare
/*extern */uint8_t read_EEPROM(uint16_t address);
/*int main(void)
    write_EEPROM(0,15);
	_delay_ms(1);
	int lettura=read_EEPROM(0);
	_delay_ms(1);
	Serial_int(lettura);*/

/*///////////////////////////////////////////////COSE IMPORTANTI DA TENER PRESENTE/////////////////////////////////////////////////
1)Quando si scrive nella EEPROM bisogna assicurarsi che la tensione di alimentazione del micro sia nella norma
2)Non deve essere presente nessun interrupt nel programma altrimenti si rischia una scrittura corrotta della EEPROM
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#endif /* EEPROM_H_ */