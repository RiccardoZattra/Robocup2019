/*
 * PID_linea.c
 *
 * Created: 23/08/2018 15:33:20
 *  Author: Riki
 */ 
#define F_CPU 16000000UL
#define PIANO 0
#define SALITA 1
#define DISCESA 2
#include <avr/io.h>
#include <util/delay.h>
#include <avr/common.h>
#include "PID_linea.h"
#include "millis.h"
#include "BNO055.h"
#include "EEPROM.h"
#include "Stanza.h"
#include "Posizione.h"
double Yp_asx_psx=0,Yp_adx_pdx=0;
uint8_t f_start_acuta=0,f_stop=0,f_start_gira=0,f_end_gira=0,f_possibile_dv=1,f_bumper=0,f_esegui_bumper=0;
double result_dx=0,result_sx=0;
uint8_t f_step_ost=0,f_funz=0,f_ostacolo=0,f_dopo_ostacolo=0,f_start_pos=0,f_fin_pos=0,f_start_gira_180=0,f_gira_corr=0,f_gira_dv=0,f_start_stop=0,fase_discendente=0,contANG_CORR=0,f_gira_stop=0;
uint16_t azione_ostacolo=0;
float res_stop=0,kp=0,res_prec=0,result_gl=0,lim_stop=0;
double mot=0;



/////////////////////////////////////////////////PID_linea/////////////////////////////////////////////////
void PID_linea(int16_t differenza, uint8_t pend)
{
	if(differenza>=4000 && differenza<5000 && f_possibile_dv==1 && pend==PIANO)
	{
		if(f_start_gira==0)
		{
			calibraAngolo();
			f_start_gira=1;
		}
		PID_gira(differenza-4500,0.5);
		result_sx=PID_gira_sx();
		result_dx=PID_gira_dx();
		if(f_end_gira==1)
		{
			USART_Tx_3('F');
			f_start_gira=0;
			f_end_gira=0;
			f_possibile_dv=0;
		}
	}
	if(differenza>=6000 && differenza<7000 && f_possibile_dv==1 && pend==PIANO)
	{
		if(f_start_gira==0)
		{
			//Serial_int(differenza);
			//Serial_println(" S_Corr");
			calibraAngolo();
			f_start_gira=1;
			f_gira_corr=1;
			led(R);
		}
		if(differenza>6500 && f_gira_corr)
		{
			//Serial_println("I_Corr");
			PID_gira(differenza-6680,0.5);
			result_sx=PID_gira_sx();
			result_dx=PID_gira_dx();
		}
		else if(f_gira_corr)
		{
			//Serial_println("I_Corr");
			PID_gira(differenza-6320,0.5);
			result_sx=PID_gira_sx();
			result_dx=PID_gira_dx();
		}
		if((f_end_gira || differenza==6680 || differenza==6320) && f_gira_corr)
		{
			//Serial_println("E_Corr");
			f_start_pos=1;
			f_end_gira=0;
			f_gira_corr=0;
		}
		if(f_start_pos)
		{
			//Serial_println("S_pos");
			led(B);
			set_posizione(1);
			f_start_pos=0;
			f_fin_pos=1;
			result_dx=0.5;
			result_sx=0.5;
		}
		if(get_posizione(1)>=8 && f_fin_pos)
		{
			//Serial_println("E_pos");
			f_fin_pos=0;
			result_sx=0;
			result_dx=0;
			f_start_gira_180=1;
		}
		if(f_start_gira_180)
		{
			//Serial_println("S_180");
			led(G);
			calibraAngolo();
			f_start_gira_180=0;
			f_gira_dv=1;
		}
		if(f_gira_dv)
		{
			//Serial_println("I_180");
			PID_gira(180,0.7);
			result_sx=PID_gira_sx();
			result_dx=PID_gira_dx();
		}
		if(f_gira_dv && f_end_gira)
		{
			//Serial_println("E_180");
			f_start_gira_180=0;
			f_start_gira=0;
			f_gira_corr=0;
			f_end_gira=0;
			f_gira_dv=0;
			f_start_pos=0;
			f_fin_pos=0;
			
			result_dx=0;
			result_sx=0;
			
			USART_Tx_3('F');
			f_possibile_dv=0;
			led(OFF_LED);
		}
	}
	else if((differenza<4000 || differenza>=5000) && (differenza<6000 || differenza>=7000) && pend==PIANO)
	{
		f_possibile_dv=1;
		switch(differenza)
		{
			case 1000://acuta dx
			{
				if(f_start_acuta==0)
				{
					f_start_acuta=1;
					set_millis(M_ACUTA);
					result_dx=0.5;
					result_sx=-0.5;
				}
				else if(get_millis(M_ACUTA)>=450)//150 sx / 300 avanti / giro dx fino a livello alto
					result_dx=-0.5;
				else if(get_millis(M_ACUTA)>=200)
					result_sx=0.5;
				break;
			}
			case 1100://acuta sx
			{
				if(f_start_acuta==0)
				{
					f_start_acuta=1;
					set_millis(M_ACUTA);
					result_dx=-0.5;
					result_sx=0.5;
				}
				else if(get_millis(M_ACUTA)>=450)//150 sx / 300 avanti / giro dx fino a livello alto
					result_sx=-0.5;
				else if(get_millis(M_ACUTA)>=200)
					result_dx=0.5;
				break;
			}
			case 2000://stop
			{
				if(f_stop==0)
				{
					f_stop=1;
					set_millis(M_STOP);
					result_dx=-0.5;
					result_sx=-0.5;
				}
				else if(get_millis(M_STOP)>35)
				{
					result_dx=0;
					result_sx=0;
				}
				break;
			}
			case 2100://indietro
			{
				result_dx=-0.5;
				result_sx=-0.5;
				break;
			}
			case 2200://rilascio motori
			{
				result_dx=0;
				result_sx=0;
				break;
			}
			case 3000://serve per girare sul proprio perno (usi in gap e verde_dx)
			{
				result_dx=-0.5;
				result_sx=0.5;
				break;
			}
			case 3100://serve per girare sul proprio perno (usi in gap e verde_sx)
			{
				result_dx=0.5;
				result_sx=-0.5;
				break;
			}
			default://segui linea in condizioni normali
			{
				f_start_acuta=0;
				f_stop=0;
				if((f_dopo_ostacolo==1 && differenza>=0) || (f_dopo_ostacolo==2 && differenza<=0))
					differenza=0;
				else if(f_dopo_ostacolo==1 || f_dopo_ostacolo==2)
					f_dopo_ostacolo=0;
				if(differenza >= 0 && differenza<=383)//Parte di SX
				{
					Yp_asx_psx=k_avanti*(double)differenza+0.5;//era 0.5
				}
				else if(differenza>=-383)
				{
					if(differenza>=-100)
					{
						Yp_asx_psx=k1_indietro*(double)differenza+0.5;
					}
					else
					{
						Yp_asx_psx=k2_indietro*(double)(differenza+100);
					}
				}
				result_sx=Yp_asx_psx>1.7?1.7:Yp_asx_psx<-1.7?-1.7:Yp_asx_psx;
				
				differenza*=-1;
				if(differenza >= 0 && differenza<=383)//Parte di DX
				{
					Yp_adx_pdx=k_avanti*(double)differenza+0.5;//era 0.5
				}
				else if(differenza>=-383 && differenza<=383)
				{
					if(differenza>=-100)
					{
						Yp_adx_pdx=k1_indietro*(double)differenza+0.5;
					}
					else
					{
						Yp_adx_pdx=k2_indietro*(double)(differenza+100);
					}
				}
				else//controlliamo che non si verifichi l'errore per cui gira molto velocemente
				{
					Serial_println("\t\t\t fuoriscala=");
					write_EEPROM(0,0);
					_delay_ms(1);
					if (differenza>0)
					{
						write_EEPROM(0,1);
						_delay_ms(1);
						//differenza*=-1;
					}
					else
					{
						write_EEPROM(0,0);
						_delay_ms(1);
						differenza*=-1;
					}
					write_EEPROM(1,(differenza&0xFF00)>>8);
					_delay_ms(1);
					write_EEPROM(2,differenza&0x00FF);
					_delay_ms(1);
					//write_EEPROM(0,15);
					//_delay_ms(1);
					Serial_int(-differenza);
				}
				result_dx=Yp_adx_pdx>1.7?1.7:Yp_adx_pdx<-1.7?-1.7:Yp_adx_pdx;
				break;
			}
		}
	}
	else if(pend == DISCESA)
	{
		f_possibile_dv=1;
		USART_Tx_3('F');
		f_start_acuta=0;
		f_stop=0;
		
		if(differenza >= 0)//Parte di SX
		{
			Yp_asx_psx=k_avanti_discesa*(double)differenza+0.4;
		}
		else
		{
			Yp_asx_psx=k1_indietro_discesa*(double)differenza+0.4;
		}
		result_sx=Yp_asx_psx>1.7?1.7:Yp_asx_psx<-1.7?-1.7:Yp_asx_psx;
		
		differenza*=-1;
		if(differenza >= 0)//Parte di DX
		{
			Yp_adx_pdx=k_avanti_discesa*(double)differenza+0.4;
		}
		else
		{
			Yp_adx_pdx=k1_indietro_discesa*(double)differenza+0.4;
		}
		result_dx=Yp_adx_pdx>1.7?1.7:Yp_adx_pdx<-1.7?-1.7:Yp_adx_pdx;
	}
	else if(pend == SALITA)
	{
		f_possibile_dv=1;
		f_start_acuta=0;
		f_stop=0;
		if(!get_bumper() && !f_bumper)
		{
			if(differenza >= 0)//Parte di SX
			{
				Yp_asx_psx=k_avanti*(double)differenza+0.5;
			}
			else
			{
				if(differenza>=-100)
				{
					Yp_asx_psx=k1_indietro*(double)differenza+0.5;
				}
				else
				{
					Yp_asx_psx=k2_indietro*(double)(differenza+100);
				}
			}
			
			differenza*=-1;
			if(differenza >= 0)//Parte di DX
			{
				Yp_adx_pdx=k_avanti*(double)differenza+0.5;
			}
			else
			{
				if(differenza>=-100)
				{
					Yp_adx_pdx=k1_indietro*(double)differenza+0.5;
				}
				else
				{
					Yp_adx_pdx=k2_indietro*(double)(differenza+100);
				}
			}
		}
		else
		{
			if(!f_bumper)
			{
				set_millis(M_BUMP);
				Yp_adx_pdx=-0.3;
				Yp_asx_psx=-0.3;
				f_bumper=1;
			}
			if(get_millis(M_BUMP)>=761 && !f_esegui_bumper)
			{
				Yp_adx_pdx=120*pow(get_millis(M_BUMP),2);
				Yp_asx_psx=120*pow(get_millis(M_BUMP),2);
				f_esegui_bumper=1;
				set_millis(M_BUMP);
			}
			else if(get_millis(M_BUMP)>=400 && f_esegui_bumper)
			{
				Yp_adx_pdx=0.5;
				Yp_asx_psx=0.5;
				f_esegui_bumper=0;
				f_bumper=0;
			}
		}
		result_dx=Yp_adx_pdx>1.7?1.7:Yp_adx_pdx<-1.7?-1.7:Yp_adx_pdx;
		result_sx=Yp_asx_psx>1.7?1.7:Yp_asx_psx<-1.7?-1.7:Yp_asx_psx;
	}
}

double get_PIDsx()
{
	return result_sx;
}
double get_PIDdx()
{
	return result_dx;
}





/////////////////////////////////////////////////////////ostacolo//////////////////////////////////////////////////
void PID_esegui_ostacolo()
{
	if(azione_ostacolo>=4000 && azione_ostacolo<5000)
	{
		if(f_start_gira==0)
		{
			calibraAngolo();
			f_start_gira=1;
		}
		PID_gira((int)(azione_ostacolo-4500),0.5);
		result_sx=PID_gira_sx();
		result_dx=PID_gira_dx();
		if(f_end_gira==1)
		{
			f_start_gira=0;
			f_end_gira=0;
			f_step_ost++;
		}
	}
	if(azione_ostacolo==2100)//indietro
	{
		result_dx=-0.5;
		result_sx=-0.5;
	}
	else if(azione_ostacolo==0)//avanti
	{
		result_sx=0.5;
		result_dx=0.5;
	}
}
void esegui_mv(int f_tipo_movimento)
{
	if(f_funz==f_step_ost)
	{
		switch(f_tipo_movimento)
		{
			case RESET:
			{
				f_step_ost=0;
				f_ostacolo=0;
				f_dopo_ostacolo=1;
				USART_Tx_3('f');
				break;
			}
			case POS_ENCODER:
			{
				set_posizione(0);
				f_step_ost++;
				break;
			}
			default:
			{
				if(f_tipo_movimento<=AVANTI_MISURA+500 && f_tipo_movimento>=AVANTI_MISURA-500)
				{
					if(get_posizione(0)<f_tipo_movimento-AVANTI_MISURA)
					{
						azione_ostacolo=0;
						if(get_posizione(0)>(f_tipo_movimento-AVANTI_MISURA)/10.0)//33.5
						f_step_ost++;
					}
					else
					{
						azione_ostacolo=2100;
						if(get_posizione(0)<(f_tipo_movimento-AVANTI_MISURA)/10.0)//33.5
						f_step_ost++;
					}
				}
				else if(f_tipo_movimento<=ANGOLO_VARIABILE+90 && f_tipo_movimento>=ANGOLO_VARIABILE-90)
				{
					azione_ostacolo=f_tipo_movimento;
				}
				break;
			}
		}
		Serial_println("");
	}
	f_funz++;
}
void PID_ostacolo(uint8_t direzione)
{
	if(direzione==DX)
	{
		Serial_println("ostacoloDX");
		f_dopo_ostacolo=2;
		f_funz=0;
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA-100);
		esegui_mv(ANGOLO_VARIABILE+60);
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA+220);
		esegui_mv(ANGOLO_VARIABILE-60);
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA+230);
		esegui_mv(ANGOLO_VARIABILE-70);
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA+50);
		esegui_mv(RESET);
		PID_esegui_ostacolo();
	}
	else
	{
		Serial_println("ostacoloSX");
		f_dopo_ostacolo=1;
		f_funz=0;
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA-100);
		esegui_mv(ANGOLO_VARIABILE-60);
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA+220);
		esegui_mv(ANGOLO_VARIABILE+60);
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA+230);
		esegui_mv(ANGOLO_VARIABILE+70);
		esegui_mv(POS_ENCODER);
		esegui_mv(AVANTI_MISURA+50);
		esegui_mv(RESET);
		PID_esegui_ostacolo();	
	}
}
uint8_t GetSetOstacolo(uint8_t az)
{
	if(az==1 || az==0)
		f_ostacolo=az;
	return f_ostacolo;
}



///////////////////////////////////////////////////Giroscopio///////////////////////////////////////////////
double PID_gira_sx()
{
	/*Serial_print("\tmot=");
	Serial_float(mot,2);*/
	return mot;
}
double PID_gira_dx()
{
	return -mot;
}
void PID_gira(float angolo_voluto,double ng_iniz)
{
	if(angolo_voluto!=0)//controllo idiot prove
	{
		if(f_gira_stop==0)
		{
			float angle=angolo(angolo_voluto<0?SX:DX);
			if(angolo_voluto>0)//filtro per angolo negativo (es. 359.9 >> -0.1)
				angle=angle>270?360-angle:angle;
			else
				angle=-(angle>270?360-angle:angle);
			
			float diff_ang=angolo_voluto-angle;//calcolo giri da differenza
			mot = (ng_iniz/30-1/150)*diff_ang+(diff_ang>0?0.2:-0.2);
		
			if(mot>ng_iniz)mot=ng_iniz;//filtro per giri massimi
			else if(mot<-ng_iniz)mot=-ng_iniz;
		
			if(angolo_voluto>0 && mot<0.2)mot=0.2;//filtro per giri minimi
			else if(angolo_voluto<0 && mot>-0.2)mot=-0.2;
		
			/*Serial_print("d=");
			Serial_float(diff_ang,1);
			Serial_println("");*/
			if((angolo_voluto>0 && diff_ang<=-0.4) || (angolo_voluto<0 && diff_ang>=0.4))//condizione di inizio stop
			{
				mot*=-0.5;
				set_millis(M_STOP);
				f_gira_stop=1;
				//Serial_println("\t\tSTOP");
			}
		}
		else if(get_millis(M_STOP)>=25)//condizione di fine
		{
			mot=0;
			f_end_gira=1;
			f_gira_stop=0;
			//Serial_println("\t\tEND");
		}
		
	}
	else
		f_end_gira=1;
}







//////////////////////////////////////////////////////PID stanza////////////////////////////////////////////////////////
void PID_stanza(int16_t differenza)
{
	if(differenza>=4000 && differenza<5000)
	{
		if(f_start_gira==0)
		{
			calibraAngolo();
			f_start_gira=1;
		}
		PID_gira(differenza-4500,0.5);
		result_sx=PID_gira_sx();
		result_dx=PID_gira_dx();
		if(f_end_gira==1)
		{
			incrementa_step();
			f_start_gira=0;
			f_end_gira=0;
		}
	}
	else if((differenza<4000 || differenza>=5000))
	{
		switch(differenza)
		{
			case 1000:
			{
				break;
			}
			case 2000://stop
			{
				if(f_stop==0)
				{
					f_stop=1;
					set_millis(M_STOP);
					result_dx=-0.5;
					result_sx=-0.5;
				}
				else if(get_millis(M_STOP)>35)
				{
					result_dx=0;
					result_sx=0;
					f_stop=0;
					incrementa_step();
				}
				break;
			}
			case 2050://stop per indietro
			{
				if(f_stop==0)
				{
					f_stop=1;
					set_millis(M_STOP);
					result_dx=0.5;
					result_sx=0.5;
				}
				else if(get_millis(M_STOP)>35)
				{
					result_dx=0;
					result_sx=0;
					f_stop=0;
					incrementa_step();
				}
				break;
			}
			case 2100://indietro
			{
				result_dx=-0.8;
				result_sx=-0.8;
				break;
			}
			case 2200:
			{
				result_dx=0;
				result_sx=0;
				break;
			}
			case 3000://serve per girare sul proprio perno (usi in gap e verde_dx)
			{
				result_dx=-0.5;
				result_sx=0.5;
				break;
			}
			case 3100://serve per girare sul proprio perno (usi in gap e verde_sx)
			{
				result_dx=0.5;
				result_sx=-0.5;
				break;
			}
			case 3200://avanti molto veloce
			{
				result_sx=1.4;
				result_dx=1.4;
				break;
			}
			case 3300://imposto un pwm per avanzare e andare a prendere la pallina cosi da togliere il PID
			{
				result_sx=170;
				result_dx=170;
				break;
			}
			case 3400://imposto una velocità di avanzamento con il PID per le palline vicino al muro
			{
				result_dx=0.4;
				result_sx=0.4;
				break;
			}
			default:
			{
				f_start_acuta=0;
				f_stop=0;
				
				result_sx=0.8;
				result_dx=0.8;
				break;
			}
		}
	}
}