/*
 * Accessori.h
 *
 * Created: 28/10/2018 15:52:06
 *  Author: Riki
 */ 


#ifndef ACCESSORI_H_
#define ACCESSORI_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "Seriale.h"
#include "millis.h"

#define OFF 0
#define ON 1
#define APRI 2
#define CHIUDI 3
#define PIANALE_APRI 4
#define PIANALE_CHIUDI 5
#define POS 6
#define M_PINZA 6

#define R 0
#define G 1
#define B 2
#define OFF_LED 3


void init_accessori();
uint8_t fin_ostacolo_dx();
uint8_t fin_ostacolo_sx();
uint8_t fin_dietro_sx();
uint8_t fin_dietro_dx();
uint8_t fin_davanti_dx();
uint8_t fin_davanti_sx();
uint8_t contatti_striscianti();
uint8_t fin_pinza_dx();
uint8_t fin_pinza_sx();
uint8_t contatto_pallina();
void led(uint8_t colore);
uint8_t pinza(uint8_t azione);



#endif /* ACCESSORI_H_ */