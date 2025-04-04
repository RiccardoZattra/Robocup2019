/*
 * Accessori.c
 *
 * Created: 28/10/2018 15:52:32
 *  Author: Riki
 */ 
#include "Accessori.h"

int pos_braccio_dx=39,pos_braccio_sx=3,pos_pinza=0,pos_pianale=0;//39-3
uint8_t f_first=1,f_tensione=0,f_millis=1;

void init_accessori()
{
	DDRC=0x01;
	DDRD=0x00;
	DDRG|=0x00;
	DDRL|=0x01;
	DDRB|=0x05;
	PORTC|=0b11000000;//finecorsa per ostacolo
	PORTC|=0b00110000;//finecorsa posteriori
	PORTC|=0b00001100;//finecorsa anteriori
	PORTC|=0b00000010;//contatti striscianti
	PORTG|=0b00000100;
	PORTG|=0b00000001;
	PORTD|=0b10000000;
	OCR2A=pos_pinza*22+14;
	OCR0B=pos_pianale*15+9;
	OCR2B=pos_braccio_dx;
	OCR0A=pos_braccio_sx;
}
uint8_t fin_ostacolo_dx()
{
	return !(PINC&0b01000000)==0? 0:1;
}
uint8_t fin_ostacolo_sx()
{
	return !(PINC&0b10000000)==0? 0:1;
}
uint8_t fin_dietro_sx()
{
	return !(PINC&0b00010000)==0? 0:1;
}
uint8_t fin_dietro_dx()
{
	return !(PINC&0b00100000)==0? 0:1;
}
uint8_t fin_davanti_dx()
{
	return !(PINC&0b00001000)==0? 0:1;
}
uint8_t fin_davanti_sx()
{
	return !(PINC&0b00000100)==0? 0:1;
}
uint8_t contatti_striscianti()
{
	return !(PINC&0b00000010)==0? 0:1;
}
uint8_t fin_pinza_dx()
{
	return !(PIND&0b10000000)==0? 0:1;
}
uint8_t fin_pinza_sx()
{
	return !(PING&0b00000100)==0? 0:1;
}
uint8_t contatto_pallina()
{
	return !(PING&0b00000001)==0? 0:1;
}
void led(uint8_t colore)
{
	switch(colore)
	{
		case R:
			PORTB|=0b00000001;
			PORTB&=0b11111011;
			PORTL&=0b11111110;
			break;
		case B:
			PORTB|=0b00000100;
			PORTB&=0b11111110;
			PORTL&=0b11111110;
			break;
		case G:
			PORTB&=0b11111010;
			PORTL|=0b00000001;
			break;
		case OFF_LED:
			PORTB&=0b11111010;
			PORTL&=0b11111110;
			break;
	}
}
uint8_t pinza(uint8_t azione)
{
	switch (azione)
	{
		case ON:{
			f_tensione=1;
			PORTC|=0b00000001;
			break;
		}
		case OFF:{
			f_tensione=0;
			PORTC&=0b11111110;
			break;
		}
		case APRI:{
			if(f_first==1)
			{
				if(f_millis)
				{
					OCR2A=pos_pinza*22+14;//servo pinza
					set_millis(M_PINZA);
					f_millis=0;
				}
				if(get_millis(M_PINZA)>=300)
				{
					PORTC|=0b00000001;
					f_first=0;
					f_millis=1;
					set_millis(M_PINZA);
				}
				else return 1;
			}
			if(OCR2A>14)
			{
				if(get_millis(M_PINZA)>=15)
				{
					OCR2A--;//servo pinza
					set_millis(M_PINZA);
				}
				return 1;
			}
			else
			{
				pos_pinza=0;
				f_first=1;
				if(!f_tensione)PORTC&=0b11111110;
				return 0;
			}
			break;
		}
		case CHIUDI:{
			if(f_first==1)
			{
				if(f_millis)
				{
					OCR2A=pos_pinza*22+14;//servo pinza//9
					set_millis(M_PINZA);
					f_millis=0;
				}
				if(get_millis(M_PINZA)>=300)
				{
					PORTC|=0b00000001;
					f_first=0;
					f_millis=1;///////
					set_millis(M_PINZA);
				}
				else return 1;
			}
			if(OCR2A<36)
			{
				if(get_millis(M_PINZA)>=20)
				{
					OCR2A++;//servo pinza
					set_millis(M_PINZA);
				}
				return 1;
			}
			else
			{
				pos_pinza=1;
				f_first=1;
				if(!f_tensione)PORTC&=0b11111110;
				return 0;
			}
			break;
		}
		case PIANALE_APRI:
		{
			if(f_first==1)
			{
				if(f_millis)
				{
					OCR0B=pos_pianale*15+9;//servo pinza
					set_millis(M_PINZA);
					f_millis=0;
				}
				if(get_millis(M_PINZA)>=300)
				{
					PORTC|=0b00000001;
					f_first=0;
					f_millis=1;/////////
					set_millis(M_PINZA);
				}
				else return 1;
			}
			if(OCR0B<24)
			{
				if(get_millis(M_PINZA)>=20)
				{
					OCR0B++;
					set_millis(M_PINZA);
				}
				return 1;
			}
			else
			{
				pos_pianale=1;
				f_first=1;
				if(!f_tensione)PORTC&=0b11111110;
				return 0;
			}
			break;
		}
		case PIANALE_CHIUDI:
		{
			if(f_first==1)
			{
				if(f_millis)
				{
					OCR0B=pos_pianale*15+9;//servo pinza
					set_millis(M_PINZA);
					f_millis=0;
				}
				if(get_millis(M_PINZA)>=300)
				{
					PORTC|=0b00000001;
					f_first=0;
					f_millis=1;////////
					set_millis(M_PINZA);
				}
				else return 1;
			}
			if(OCR0B>9)
			{
				if(get_millis(M_PINZA)>=20)
				{
					OCR0B--;
					set_millis(M_PINZA);
				}
				return 1;
			}
			else
			{
				pos_pianale=0;
				f_first=1;
				if(!f_tensione)PORTC&=0b11111110;
				return 0;
			}
			break;
		}
		default: 
		{
			if(f_first==1)
			{
				if(f_millis)
				{
					OCR2B=pos_braccio_dx;//servo destra
					OCR0A=pos_braccio_sx;//servo sinistra
					set_millis(M_PINZA);
					f_millis=0;
				}
				if(get_millis(M_PINZA)>=300)
				{
					PORTC|=0b00000001;
					f_first=0;
					f_millis=1;////////////
					set_millis(M_PINZA);
				}
				else return 1;
			}
			int diff_dx=pos_braccio_dx-((azione-POS)*0.29+10);
			
			
			if(diff_dx<0)
			{
				if(get_millis(M_PINZA)>=30)
				{
					pos_braccio_dx++;
					pos_braccio_sx--;
					OCR2B=pos_braccio_dx;//servo destra
					OCR0A=pos_braccio_sx;//servo sinistra
					
					/*Serial_print("d=");
					Serial_int(diff_dx);
					Serial_print("\tOC=");
					Serial_int(OCR0A);
					Serial_print(" ");
					Serial_int(OCR2B);
					Serial_println("");*/
					
					set_millis(M_PINZA);
				}
				return 1;
			}
			else if(diff_dx>0)
			{
				if(get_millis(M_PINZA)>=40)
				{
					pos_braccio_dx--;
					pos_braccio_sx++;
					OCR2B=pos_braccio_dx;//servo destra
					OCR0A=pos_braccio_sx;//servo sinistra
					
					/*Serial_print("d=");
					Serial_int(diff_dx);
					Serial_print("\tOC=");
					Serial_int(OCR0A);
					Serial_print(" ");
					Serial_int(OCR2B);
					Serial_println("");*/
					
					set_millis(M_PINZA);
				}
				return 1;
			}
			else
			{
				
				Serial_println("");
				Serial_print("\t\tEND!!");
				Serial_println("");
				
				f_first=1;
				if(!f_tensione)PORTC&=0b11111110;
				return 0;
			}
			break;
		}
	}
	return 0;
}

//////////////////////ESEMPIO USO PINZA//////////////////////////////////
/*
pinza(ON);
f_pinza=1;
while(f_pinza)
{
	f_pinza=pinza(POS+0);
}
f_pinza=1;
while(f_pinza)
{
	f_pinza=pinza(CHIUDI);
}
f_pinza=1;
while(f_pinza)
{
	f_pinza=pinza(POS+100);
}
f_pinza=1;
while(f_pinza)
{
	f_pinza=pinza(APRI);
}
*/
/////////////////////////////////////////////////////////////////////////