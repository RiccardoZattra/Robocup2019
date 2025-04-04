/*
 * Posizione.c
 *
 * Created: 22/12/2018 17:44:44
 *  Author: Riki
 */ 

#include "Posizione.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <util/atomic.h>
int32_t posizioni_sx[10],posizioni_dx[10];
int32_t *pos_sx,*pos_dx;
void add_posizioni(int32_t *p_dx, int32_t *p_sx)
{
	pos_sx=p_sx;
	pos_dx=p_dx;
}

void set_posizione(uint8_t pos)
{
	posizioni_sx[pos]=*pos_sx;
	posizioni_dx[pos]=*pos_dx;
}

double get_posizione(uint8_t pos)//distanza in cm
{
	//return (*pos_dx-posizioni_dx[pos])*0.086855;
	double adh=(( (*pos_dx-posizioni_dx[pos]) /*+ (*pos_sx-posizioni_sx[pos])*/ )/*/2.0*/)*0.02199357;//0.086855 /02170418

	return adh;
}