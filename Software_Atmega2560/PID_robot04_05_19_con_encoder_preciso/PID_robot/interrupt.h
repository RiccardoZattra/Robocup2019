/*
 * interrupt.h
 *
 * Created: 09/06/2018 16:16:40
 *  Author: Riki
 */ 


#ifndef INTERRUPT_H_
#define INTERRUPT_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <util/atomic.h>
void init_interrupt(void);




#endif /* INTERRUPT_H_ */