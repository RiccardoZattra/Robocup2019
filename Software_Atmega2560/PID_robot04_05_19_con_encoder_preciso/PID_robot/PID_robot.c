/*
 * PID_robot.c
 *
 * Created: 09/06/2018 16:13:12
 *  Author: Riki
 */ 


#define F_CPU 16000000UL
//define RETROAZIONE --> PID --> MOTORI
#define T0 0
#define T1 1
#define RIP 2
#define AVANTI 1
#define INDIETRO -1
#define NG_LETT_PREC 0
#define NG_ERR 1
#define PDX 0
#define ASX 1
#define PSX 2
#define ADX 3
#define NUOVO 0
#define VECCHIO 1
#define PIANO 0
#define SALITA 1
#define DISCESA 2
#define OFF 0
#define ON 1
#define APRI 2
#define CHIUDI 3
#define PIANALE_APRI 4
#define PIANALE_CHIUDI 5
#define POS 6
#define SX 1
#define DX 0
#define M_ACUTA 9
#define M_STOP 8
#define M_BUMP 7
#define M_PINZA 6
#define M_DELAY_STANZA 5
#define M_DELAY_PAL 4
#define M_CALC_PID 2
#define M_ENTRATA_STANZA 3
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <util/atomic.h>
#include "PID.h"
#include "Seriale.h"
#include "millis.h"
#include "interrupt.h"
#include "PWM.h"
#include "PID_linea.h"
#include "BNO055.h"
#include "Accessori.h"
#include "EEPROM.h"
#include "Posizione.h"
#include "Stanza.h"
#include "VL6180x.h"
//variables RETROAZIONE --> PID --> MOTORI
uint8_t c_impulso_pdx=0,c_impulso_asx=0,c_impulso_psx=0,c_impulso_adx=0;
int32_t c_distanza_dx=0,c_distanza_sx=0;
uint16_t c_riporto_pdx=0,c_riporto_asx=0,c_riporto_psx=0,c_riporto_adx=0;
uint16_t t_calcolo_giri_pdx[3]={0,0,0},t_calcolo_giri_asx[3]={0,0,0},t_calcolo_giri_psx[3]={0,0,0},t_calcolo_giri_adx[3]={0,0,0};
uint8_t f_attivo_calc_asx=0,f_attivo_calc_pdx=0,f_attivo_calc_psx=0,f_attivo_calc_adx=0;
double ng_pdx=0.0,ng_asx=0.0,ng_psx=0.0,ng_adx=0.0;
uint32_t t_tc_pdx=0,t_tc_asx=0,t_tc_psx=0,t_tc_adx=0;
int16_t t_pwm[4];
int8_t f_direz_pdx=AVANTI,f_direz_asx=AVANTI,f_direz_psx=AVANTI,f_direz_adx=AVANTI,f_direz_asx_1=AVANTI,f_direz_adx_1=AVANTI;
uint8_t f_pend=PIANO;
double t_ng_pdx[2]={0,1},t_ng_asx[2]={0,1},t_ng_psx[2]={0,1},t_ng_adx[2]={0,1};
uint8_t f_pinza;
int16_t livello=0,codice_stanza=0;//codice_stanza serve per salvare il codice d'entrata della stanza
uint8_t f_not_entrato=1,f_filtro=0;
//variables STANZA
#define E_CORTO_DX 0
#define E_CORTO_SX 1
#define E_LUNGO_DX 2
#define E_LUNGO_SX 3
#define E_CORTO_CX 4
#define E_LUNGO_CX_VERSO_DX 5
#define E_LUNGO_CX_VERSO_SX 6

uint8_t f_stanza,f_tipo_entrata,disattiva_livello=0;//disattiva_livello serve per disabilitare il livello e ricevere il codice di entrata stanza

ISR(INT2_vect)//pdx
{
	c_impulso_pdx++;
	if(c_impulso_pdx==1)
	{
		t_calcolo_giri_pdx[T0]=TCNT3;
		c_riporto_pdx=0;
	}
	if(c_impulso_pdx>3)
	{
		if(c_impulso_pdx==4)
		{
			f_direz_pdx=(PINA&0b10000000)!=0?AVANTI:INDIETRO;
			t_calcolo_giri_pdx[T1]=TCNT3;
			t_calcolo_giri_pdx[RIP]=c_riporto_pdx;
			//c_distanza_dx+=f_direz_adx;
			f_attivo_calc_pdx=1;
		}
		c_impulso_pdx=0;
	}
}
ISR(INT3_vect)//asx
{
	c_distanza_sx+=f_direz_asx_1;
	f_direz_asx_1=(PINA&0b00100000)==0?AVANTI:INDIETRO;
	c_impulso_asx++;
	if(c_impulso_asx==1)
	{
		t_calcolo_giri_asx[T0]=TCNT3;
		c_riporto_asx=0;
	}
	if(c_impulso_asx>3)
	{
		if(c_impulso_asx==4)
		{
			f_direz_asx=(PINA&0b00100000)==0?AVANTI:INDIETRO;
			t_calcolo_giri_asx[T1]=TCNT3;
			t_calcolo_giri_asx[RIP]=c_riporto_asx;
			//c'era una volta...
			f_attivo_calc_asx=1;
		}
		c_impulso_asx=0;
	}
}
ISR(INT4_vect)//psx
{
	c_impulso_psx++;
	if(c_impulso_psx==1)
	{
		t_calcolo_giri_psx[T0]=TCNT3;
		c_riporto_psx=0;
	}
	if(c_impulso_psx>3)
	{
		if(c_impulso_psx==4)
		{
			f_direz_psx=(PINA&0b01000000)==0?AVANTI:INDIETRO;
			t_calcolo_giri_psx[T1]=TCNT3;
			t_calcolo_giri_psx[RIP]=c_riporto_psx;
			f_attivo_calc_psx=1;
		}
		c_impulso_psx=0;
	}
}
ISR(INT5_vect)//adx
{
	c_distanza_dx+=f_direz_adx_1;
	f_direz_adx_1=(PINA&0b00010000)!=0?AVANTI:INDIETRO;
	c_impulso_adx++;
	if(c_impulso_adx==1)
	{
		t_calcolo_giri_adx[T0]=TCNT3;
		c_riporto_adx=0;
	}
	if(c_impulso_adx>3)
	{
		if(c_impulso_adx==4)
		{
			f_direz_adx=(PINA&0b00010000)!=0?AVANTI:INDIETRO;
			t_calcolo_giri_adx[T1]=TCNT3;
			t_calcolo_giri_adx[RIP]=c_riporto_adx;
			//c'era una volta...
			f_attivo_calc_adx=1;
		}
		c_impulso_adx=0;
	}
}
ISR(TIMER3_COMPA_vect)
{
	c_riporto_pdx++;
	c_riporto_asx++;
	c_riporto_psx++;
	c_riporto_adx++;
}
void esegui_PID_stanza()
{
	if(f_attivo_calc_pdx)
	{
		ng_pdx=f_direz_pdx*(100000/(double)(((uint64_t)(t_calcolo_giri_pdx[RIP])*499+t_calcolo_giri_pdx[T1]-t_calcolo_giri_pdx[T0])*2.0625));
		f_attivo_calc_pdx=0;
	}
	if(f_attivo_calc_asx)
	{
		ng_asx=f_direz_asx*(100000/(double)(((uint64_t)(t_calcolo_giri_asx[RIP])*499+t_calcolo_giri_asx[T1]-t_calcolo_giri_asx[T0])*2.0625));
		f_attivo_calc_asx=0;
	}
	if(f_attivo_calc_psx)
	{
		ng_psx=f_direz_psx*(100000/(double)(((uint64_t)(t_calcolo_giri_psx[RIP])*499+t_calcolo_giri_psx[T1]-t_calcolo_giri_psx[T0])*2.0625));
		f_attivo_calc_psx=0;
	}
	if(f_attivo_calc_adx)
	{
		ng_adx=f_direz_adx*(100000/(double)(((uint64_t)(t_calcolo_giri_adx[RIP])*499+t_calcolo_giri_adx[T1]-t_calcolo_giri_adx[T0])*2.0625));
		f_attivo_calc_adx=0;
	}
	int16_t m_calc_pid=get_millis(M_CALC_PID);
	if(m_calc_pid>=2)
	{
		PID_stanza(livello);
		m_asx(PID_asx(ng_asx,get_PIDsx(),m_calc_pid/1000.0,t_pwm[ASX]),t_pwm);
		m_adx(PID_adx(ng_adx,get_PIDdx(),m_calc_pid/1000.0,t_pwm[ADX]),t_pwm);
		m_pdx(PID_pdx(ng_pdx,get_PIDdx(),m_calc_pid/1000.0,t_pwm[PDX]),t_pwm);
		m_psx(PID_psx(ng_psx,get_PIDsx(),m_calc_pid/1000.0,t_pwm[PSX]),t_pwm);
		set_millis(M_CALC_PID);
	}
}
int main(void)
{
	USART_0(BAUDR0);
	USART_3(BAUDR3);
	USART_2(BAUDR2);
	
	for(int k=0;k<5;k++)
		USART_Tx_3('l');
	for(int k=0;k<5;k++)
		USART_Tx_3('j');
	
	init_millis();
	init_pwm0();
	start_pwm0();
	init_pwm1();
	start_pwm1();
	init_pwm2();
	start_pwm2();
	init_pwm4();
	start_pwm4();
	init_interrupt();
	init_accessori();
	Init_gyro();
	
	init_laser();
	
	sei();
	set_millis(M_CALC_PID);
	set_millis(M_ENTRATA_STANZA);
	int lettura=read_EEPROM(1)<<8 |read_EEPROM(2);
	if(read_EEPROM(0)==1)
	{
		lettura*=-1;
	}
	_delay_ms(1);
	Serial_print("letturaeeprom=");
	Serial_int(lettura);
	Serial_println("");
	set_millis(1);
	calibraPendenza();
	add_posizioni(&c_distanza_dx,&c_distanza_sx);
	set_posizione(0);
	
	set_millis(10);
	livello=0;
	///Valori da modificare///
	f_tipo_entrata=E_CORTO_SX;
	f_stanza=0;
	inizializza_valori_eeprom();
	//////////////////////////
	set_posizione(0);
	pinza(ON);
	_delay_ms(1000);
	pinza(OFF);
	while(1)
	{
		flush_buffer();
		while(!f_stanza)
		{
			if(disattiva_livello==0)
			{
				receive_rasp(&livello);
			}
			else if (disattiva_livello)
			{
				receive_rasp(&codice_stanza);
				if(codice_stanza==20000)
				{
					Serial_println("\t\tcodice_STANZA");
					f_stanza=1;
				}
			}
			if(get_millis(10)>30000){set_millis(10); pinza(OFF);}
			else if(get_millis(10)>29800) pinza(ON);
			//CALCULATION OF REVOLUTIONS when the interrupt tell me do it
			if(f_attivo_calc_pdx)
			{
				ng_pdx=f_direz_pdx*(100000/(double)(((uint64_t)(t_calcolo_giri_pdx[RIP])*499+t_calcolo_giri_pdx[T1]-t_calcolo_giri_pdx[T0])*2.0625));
				f_attivo_calc_pdx=0;
			}
			if(f_attivo_calc_asx)
			{
				ng_asx=f_direz_asx*(100000/(double)(((uint64_t)(t_calcolo_giri_asx[RIP])*499+t_calcolo_giri_asx[T1]-t_calcolo_giri_asx[T0])*2.0625));
				f_attivo_calc_asx=0;
			}
			if(f_attivo_calc_psx)
			{
				ng_psx=f_direz_psx*(100000/(double)(((uint64_t)(t_calcolo_giri_psx[RIP])*499+t_calcolo_giri_psx[T1]-t_calcolo_giri_psx[T0])*2.0625));
				f_attivo_calc_psx=0;
			}
			if(f_attivo_calc_adx)
			{
				ng_adx=f_direz_adx*(100000/(double)(((uint64_t)(t_calcolo_giri_adx[RIP])*499+t_calcolo_giri_adx[T1]-t_calcolo_giri_adx[T0])*2.0625));
				f_attivo_calc_adx=0;
			}
			if(pendenza()<-10)
				f_pend=DISCESA;
			else if(pendenza()>10)
				f_pend=SALITA;
			else
				f_pend=PIANO;
			if (contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000 || f_not_entrato))
			{
				f_filtro++;
				if(f_filtro>9)
				{
					Serial_print("\t\t\tcontatti");
					USART_Tx_3('s');
					livello=2000;
					disattiva_livello=1;
					f_not_entrato=0;
					f_filtro=0;
				}
			}
			if(fin_ostacolo_dx()||fin_ostacolo_sx())//per ostacolo
			{
				GetSetOstacolo(1);
				USART_Tx_3('i');
			}
			//PID every 2 millisecond
			if(get_millis(M_CALC_PID)>=2)
			{
				if(!GetSetOstacolo(2))
					PID_linea(livello,f_pend);
				else
					PID_ostacolo(SX);
				m_asx(PID_asx(ng_asx,get_PIDsx(),0.002,t_pwm[ASX]),t_pwm);
				m_adx(PID_adx(ng_adx,get_PIDdx(),0.002,t_pwm[ADX]),t_pwm);
				m_pdx(PID_pdx(ng_pdx,get_PIDdx(),0.002,t_pwm[PDX]),t_pwm);
				m_psx(PID_psx(ng_psx,get_PIDsx(),0.002,t_pwm[PSX]),t_pwm);
				set_millis(M_CALC_PID);
			}
		}
		
		flush_buffer();
		while(f_stanza)
		{
			Serial_println("\t\tSTANZA");
			switch(f_tipo_entrata)
			{
				case E_CORTO_SX://entrata lato corto a sinistra
				
					set_millis(M_CALC_PID);
					set_millis(M_ENTRATA_STANZA);
					dichiara_variabili_da_eeprom();
					while(f_stanza)
					{
						programma_movimento_e_corto_sx();
						livello=get_azione();
						esegui_PID_stanza();
						if(contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000))
						{
							f_filtro++;
							if(f_filtro>9)
							{
								USART_Tx_3('l');
								f_stanza=0;
								disattiva_livello=0;
								livello=0;
								f_tipo_entrata=0;
								set_millis(M_ENTRATA_STANZA);
								f_filtro=0;
							}
						}
						else
							f_filtro=0;
					}
					break;
				
				case E_CORTO_DX://entrata lato corto a destra
				
					set_millis(M_CALC_PID);
					set_millis(M_ENTRATA_STANZA);
					dichiara_variabili_da_eeprom();
					while(f_stanza)
					{
						programma_movimento_e_corto_dx();
						livello=get_azione();
						esegui_PID_stanza();
						if(contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000))
						{
							f_filtro++;
							if(f_filtro>9)
							{
								USART_Tx_3('l');
								f_stanza=0;
								disattiva_livello=0;
								livello=0;
								f_tipo_entrata=0;
								set_millis(M_ENTRATA_STANZA);
								f_filtro=0;
							}
						}
						else
							f_filtro=0;
					}
					break;
				
				case E_LUNGO_SX://entrata in fondo a sinistra lato lungo muro sulla sua sinistra
				
					set_millis(M_CALC_PID);
					set_millis(M_ENTRATA_STANZA);
					dichiara_variabili_da_eeprom();
					while(f_stanza)
					{
						programma_movimento_e_lungo_sx();
						livello=get_azione();
						esegui_PID_stanza();
						if(contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000))
						{
							f_filtro++;
							if(f_filtro>9)
							{
								USART_Tx_3('l');
								f_stanza=0;
								disattiva_livello=0;
								livello=0;
								f_tipo_entrata=0;
								set_millis(M_ENTRATA_STANZA);
								f_filtro=0;
							}
						}
						else
							f_filtro=0;
					}
					break;
				
				case E_LUNGO_DX://entrata in fondo a destra lato lungo muro sulla sua destra
				
					set_millis(M_CALC_PID);
					set_millis(M_ENTRATA_STANZA);
					dichiara_variabili_da_eeprom();
					while(f_stanza)
					{
						programma_movimento_e_lungo_dx();
						livello=get_azione();
						esegui_PID_stanza();
						if(contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000))
						{
							f_filtro++;
							if(f_filtro>9)
							{
								USART_Tx_3('l');
								f_stanza=0;
								disattiva_livello=0;
								livello=0;
								f_tipo_entrata=0;
								set_millis(M_ENTRATA_STANZA);
								f_filtro=0;
							}
						}
						else
							f_filtro=0;
					}
					break;
				
				case E_CORTO_CX:
				
					set_millis(M_CALC_PID);
					set_millis(M_ENTRATA_STANZA);
					dichiara_variabili_da_eeprom();
					while(f_stanza)
					{
						programma_movimento_e_corto_cx();
						livello=get_azione();
						esegui_PID_stanza();
						if(contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000))
						{
							f_filtro++;
							if(f_filtro>9)
							{
								USART_Tx_3('l');
								f_stanza=0;
								disattiva_livello=0;
								livello=0;
								f_tipo_entrata=0;
								set_millis(M_ENTRATA_STANZA);
								f_filtro=0;
							}
						}
						else
							f_filtro=0;
					}
					break;
				
				case E_LUNGO_CX_VERSO_DX:
				
					set_millis(M_CALC_PID);
					set_millis(M_ENTRATA_STANZA);
					dichiara_variabili_da_eeprom();
					while(f_stanza)
					{
						programma_movimento_e_lungo_cx_verso_dx();
						livello=get_azione();
						esegui_PID_stanza();
						if(contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000))
						{
							f_filtro++;
							if(f_filtro>9)
							{
								USART_Tx_3('l');
								f_stanza=0;
								disattiva_livello=0;
								livello=0;
								f_tipo_entrata=0;
								set_millis(M_ENTRATA_STANZA);
								f_filtro=0;
							}
						}
						else
							f_filtro=0;
					}
					break;
				
				case E_LUNGO_CX_VERSO_SX:
				
					set_millis(M_CALC_PID);
					set_millis(M_ENTRATA_STANZA);
					dichiara_variabili_da_eeprom();
					while(f_stanza)
					{
						programma_movimento_e_lungo_cx_verso_sx();
						livello=get_azione();
						esegui_PID_stanza();
						if(contatti_striscianti() && (get_millis(M_ENTRATA_STANZA)>10000))
						{
							f_filtro++;
							if(f_filtro>9)
							{
								USART_Tx_3('l');
								f_stanza=0;
								disattiva_livello=0;
								livello=0;
								f_tipo_entrata=0;
								set_millis(M_ENTRATA_STANZA);
								f_filtro=0;
							}
						}
						else
							f_filtro=0;
					}
					break;
				
			}
		}
		livello=2200;
	}
}
