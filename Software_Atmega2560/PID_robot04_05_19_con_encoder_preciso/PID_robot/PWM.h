/*
 * PWM.h
 *
 * Created: 09/06/2018 16:19:16
 *  Author: Riki
 */ 


#ifndef PWM_H_
#define PWM_H_
#define F_CPU 16000000UL
#define PDX 0
#define ASX 1
#define PSX 2
#define ADX 3
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <util/atomic.h>

void init_pwm1 (void);
void start_pwm1 (void);
void init_pwm4 (void);
void start_pwm4 (void);
void m_psx(int16_t duty,int16_t pwm_attuale[]);
void m_asx(int16_t duty,int16_t pwm_attuale[]);
void m_adx(int16_t duty,int16_t pwm_attuale[]);
void m_pdx(int16_t duty,int16_t pwm_attuale[]);
void init_pwm0();
void start_pwm0 (void);
void init_pwm2();
void start_pwm2 (void);


#endif /* PWM_H_ */