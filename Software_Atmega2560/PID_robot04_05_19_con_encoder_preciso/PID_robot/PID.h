/*
 * PID.h
 *
 * Created: 27/04/2018 10:55:11
 *  Author: Studente
 */ 


#ifndef PID_H_
#define PID_H_
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

//per giri molto bassi come 0.35 da impostare per altri tipi di zona
#define Kp	150//150
#define Kd	7//7
#define Ki	2//2

#define Ypmax	200//200
#define Ydmax	500//500
#define Yimax	0.15//0.15


int PID_asx(double ngiri,double voluti,double tc,int16_t pwm_attuale);
int PID_pdx(double ngiri,double voluti,double tc,int16_t pwm_attuale);
int PID_adx(double ngiri,double voluti,double tc,int16_t pwm_attuale);
int PID_psx(double ngiri,double voluti,double tc,int16_t pwm_attuale);

#endif /* PID_H_ */