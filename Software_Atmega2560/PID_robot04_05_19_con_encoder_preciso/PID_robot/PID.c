/*
 * PID.c
 *
 * Created: 27/04/2018 10:55:00
 *  Author: Studente
 */ 

#include "PID.h"
#include "Seriale.h"
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

double Xo4=0, Y4=0, Yp4=0, Yd4=0, Yi4=0;
double Xo1=0, Y1=0, Yp1=0, Yd1=0, Yi1=0;
double Xo2=0, Y2=0, Yp2=0, Yd2=0, Yi2=0;
double Xo3=0, Y3=0, Yp3=0, Yd3=0, Yi3=0;
int PID_asx(double ngiri,double voluti,double tc,int16_t pwm_attuale)
{
	/*Xo=encoder;*/
	if(voluti>-2 && voluti<2 && voluti !=0)
	{
		double encoder=voluti-ngiri;
		Yp1 = Kp * encoder;
		Yd1 = Kd * (encoder - Xo1) / tc;
		Xo1=encoder;
		Yi1 += Ki * tc * encoder;
		if (Yp1 > Ypmax)	{ Yp1 = Ypmax; }
		if (Yp1 < -Ypmax)	{ Yp1 = -Ypmax; }
		if (Yi1 > Yimax)	{ Yi1 = Yimax; }
		if (Yi1 < -Yimax)	{ Yi1 = -Yimax;}
		if (Yd1 > Ydmax)	{ Yd1 = Ydmax; }
		if (Yd1 < -Ydmax)	{ Yd1 = -Ydmax; }
		Y1 = Yp1 + Yd1 + Yi1 + pwm_attuale;
		return Y1>1023?1023:Y1<-1023?-1023:Y1;
	}
	else
		return voluti;
}
int PID_adx(double ngiri,double voluti,double tc,int16_t pwm_attuale)
{
	/*Xo=encoder;*/
	if(voluti>-2 && voluti<2 && voluti !=0)
	{
		double encoder=voluti-ngiri;
		Yp2 = Kp * encoder;
		Yd2 = Kd * (encoder - Xo2) / tc;
		Xo2=encoder;
		Yi2 += Ki * tc * encoder;
		if (Yp2 > Ypmax)	{ Yp2 = Ypmax; }
		if (Yp2 < -Ypmax)	{ Yp2 = -Ypmax; }
		if (Yi2 > Yimax)	{ Yi2 = Yimax; }
		if (Yi2 < -Yimax)	{ Yi2 = -Yimax;}
		if (Yd2 > Ydmax)	{ Yd2 = Ydmax; }
		if (Yd2 < -Ydmax)	{ Yd2 = -Ydmax; }
		Y2 = Yp2 + Yd2 + Yi2 + pwm_attuale;
		return Y2>1023?1023:Y2<-1023?-1023:Y2;
	}
	else
		return voluti;
}
int PID_pdx(double ngiri,double voluti,double tc,int16_t pwm_attuale)
{
	if(voluti>-2 && voluti<2 && voluti !=0)
	{
		double encoder=voluti-ngiri;
		Yp3 = Kp * encoder;
		Yd3 = Kd * (encoder - Xo3) / tc;
		Xo3=encoder;
		Yi3 += Ki * tc * encoder;
		if (Yp3 > Ypmax)	{ Yp3 = Ypmax; }
		if (Yp3 < -Ypmax)	{ Yp3 = -Ypmax; }
		if (Yi3 > Yimax)	{ Yi3 = Yimax; }
		if (Yi3 < -Yimax)	{ Yi3 = -Yimax;}
		if (Yd3 > Ydmax)	{ Yd3 = Ydmax; }
		if (Yd3 < -Ydmax)	{ Yd3 = -Ydmax; }
		Y3 = Yp3 + Yd3 + Yi3 + pwm_attuale;
		return Y3>1023?1023:Y3<-1023?-1023:Y3;
	}
	else
		return voluti;
}
int PID_psx(double ngiri,double voluti,double tc,int16_t pwm_attuale)
{
	if(voluti>-2 && voluti<2 && voluti !=0)
	{
		double encoder=voluti-ngiri;
		Yp4 = Kp * (voluti-ngiri);
		Yd4 = Kd * (encoder - Xo4) / tc;
		Xo4=encoder;
		Yi4 += Ki * tc * encoder;
		if (Yp4 > Ypmax)	{ Yp4 = Ypmax; }
		if (Yp4 < -Ypmax)	{ Yp4 = -Ypmax; }
		if (Yi4 > Yimax)	{ Yi4 = Yimax; }
		if (Yi4 < -Yimax)	{ Yi4 = -Yimax;}
		if (Yd4 > Ydmax)	{ Yd4 = Ydmax; }
		if (Yd4 < -Ydmax)	{ Yd4 = -Ydmax; }
		Y4 = Yp4 + Yd4 + Yi4 + pwm_attuale;
		return Y4>1023?1023:Y4<-1023?-1023:Y4;
	}
	else
		return voluti;
}