/*
 * Posizione.h
 *
 * Created: 22/12/2018 17:45:28
 *  Author: Riki
 */ 


#ifndef POSIZIONE_H_
#define POSIZIONE_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <util/atomic.h>
#include "Seriale.h"

void add_posizioni(int32_t *p_dx, int32_t *p_sx);
void set_posizione(uint8_t pos);
double get_posizione(uint8_t pos);


#endif /* POSIZIONE_H_ */