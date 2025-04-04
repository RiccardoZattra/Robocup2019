/*
 * Stanza.c
 *
 * Created: 28/12/2018 13:15:49
 *  Author: Riki
 */ 
#include "Stanza.h"
#define IS_DEBUG 1
#define NUM_PALLINE_ARGENTO 3
#define NUM_PALLINE_NERE 2
#define NUM_AVANZAMENTI_MAX 3

uint16_t azione=0;
uint8_t f_movimenti=0;//abbiamo 256 movimenti eseguibili
uint8_t f_num_virate=0,f_riciclo=0/*,f_num_avanzamenti*/,f_mil=0,f_dist_ang_compiuta=0,f_controlla_angolo=1,f_scandaglia=1,f_all_fin=0,f_scaricato=0,f_presa_pallina=1,f_muro_alle_spalle=0;
//////////////////////variabili da salvare su EEPROM///////////////////////////////////
uint8_t pos_raccolta=0;
uint8_t f_avanzamenti=0;
int8_t f_num_palline_argento=NUM_PALLINE_ARGENTO,f_num_palline_nere=NUM_PALLINE_NERE;
///////////////////////////////////////////////////////////////////////////////////////
uint8_t f_avanti_piano=0,f_num_scandagliate=0,f_avanti_misura0=0,f_filtro_angolo;
uint8_t f_contatto_muro=0,f_argento_vera=0,f_cerca_argento=1,f_cerca_nere=0,f_primo_180=1,f_raccolta=0,f_spostamento_palline=0;
uint8_t f_fine_prima_parte=0,cont_vibra=0,start_vibra=1;
int16_t angolo_rasp=VAL_INIZIALE,distanza_rasp=VAL_INIZIALE;
int8_t f_num_palline_pianale=0;
double distanza_percorsa=0;
uint16_t f_step[]={0,0,0,0},f_id[]={0,0,0,0};
uint8_t pos_gl=0,pos_salvato=0,f_verso_gl=0,f_avanza_pid=0;


int16_t get_azione()
{
	return azione;
}
void incrementa_step()
{
	f_step[pos_salvato]++;
	azione=1000;
	if(IS_DEBUG)
	{
		Serial_println("");
		Serial_print("\t\tinc\t");
		Serial_int(f_step[0]);
		Serial_print(" ");
		Serial_int(f_step[1]);
		Serial_print(" ");
		Serial_int(f_step[2]);
		Serial_print(" ");
		Serial_int(f_step[3]);
		Serial_println("");
	}
}




////////////////////////////////////////SALVATAGGIO////////////////////////////////////////////////////////
void inizializza_valori_eeprom()
{
	write_EEPROM(IND_EE_POS_RACCOLTA,pos_raccolta);
	write_EEPROM(IND_EE_NUMERO_AVANZAMENTI,f_avanzamenti);
	write_EEPROM(IND_EE_NUM_PAL_NERE,f_num_palline_nere);
	write_EEPROM(IND_EE_NUM_PAL_ARGENTO,f_num_palline_argento);
}
void dichiara_variabili_da_eeprom()
{
	pos_raccolta=read_EEPROM(IND_EE_POS_RACCOLTA);
	f_avanzamenti=read_EEPROM(IND_EE_NUMERO_AVANZAMENTI);
	f_num_palline_argento=read_EEPROM(IND_EE_NUM_PAL_ARGENTO);
	f_num_palline_nere=read_EEPROM(IND_EE_NUM_PAL_NERE);
	if(IS_DEBUG)
	{
		Serial_print("pos_raccolta=");
		Serial_int(pos_raccolta);
		Serial_print("\tf_avanzamenti=");
		Serial_int(f_avanzamenti);
		Serial_print("\tf_num_palline_nere=");
		Serial_int(f_num_palline_nere);
		Serial_print("\tf_num_palline_argento=");
		Serial_int(f_num_palline_argento);
		Serial_println("");
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////ESECUZIONE MOVIMENTI SPECIFICI///////////////////////////////////////////
void es_mv(int f_tipo_movimento)
{
	if(IS_DEBUG)
	{
	Serial_int(f_id[0]);
	Serial_print(" ");
	Serial_int(f_id[1]);
	Serial_print(" ");
	Serial_int(f_id[2]);
	Serial_print(" ");
	Serial_int(f_id[3]);
	Serial_print("\t");
	}
		
	pos_salvato=pos_gl;
		
	switch(f_tipo_movimento)
	{
		case ROT_180:
		{
			if(IS_DEBUG)
			Serial_print("180");
			azione=4680;
			break;
		}
		case ROT_90_SX:
		{
			if(IS_DEBUG)
			Serial_print("90sx");
			azione=4410;
			break;
		}
		case ROT_90_DX:
		{
			if(IS_DEBUG)
			Serial_print("90dx");
			azione=4590;
			break;
		}
		case ROT_45_DX:
		{
			if(IS_DEBUG)
			Serial_print("45dx");
			azione=4545;
			break;
		}
		case ROT_45_SX:
		{
			if(IS_DEBUG)
			Serial_print("45sx");
			azione=4455;
			break;
		}
		case ALLINEAMENTO_FINECORSA:
		{
			if(IS_DEBUG)
			Serial_print("al_finecorsa");
			azione=2100;
			if(fin_dietro_dx() && fin_dietro_sx())
			{
				if(f_all_fin==0)
				{
					set_millis(0);
					f_all_fin=1;
				}
				else if(get_millis(0)>=100)
				{
					incrementa_step();
					f_all_fin=0;
				}
			}
			else if(fin_dietro_dx())
			{
				azione=3100;
				f_all_fin=0;
			}
			else if(fin_dietro_sx())
			{
				azione=3000;
				f_all_fin=0;
			}
			else
				f_all_fin=0;
			break;
		}
		case STOP:
		{
			if(IS_DEBUG)
			Serial_print("stop");
			azione=2000;
			break;
			
		}
		case STOP_INDIETRO:
		{
			if(IS_DEBUG)
			Serial_print("stop_ind");
			azione=2050;
			break;
		}
		case RILASCIO_MOTORI:
		{
			if(IS_DEBUG)
			Serial_print("rilascio");
			azione=2200;
			incrementa_step();
			break;
		}
		case POS_ENCODER:
		{
			if(IS_DEBUG)
			Serial_print("pos_enc");
			set_posizione(0);
			incrementa_step();
			break;
		}
		case VAR_RACCOLTA:
		{
			if(IS_DEBUG)
			Serial_print("var_raccolta");
			f_raccolta=1;
			azione=2200;
			incrementa_step();
			break;
		}
		case AZZERA_VAR_RACCOLTA:
		{
			if(IS_DEBUG)
			Serial_print("azzera_var_raccolta");
			f_raccolta=0;
			azione=2200;
			incrementa_step();
			break;
		}
		case VERIFICA_PRESA_SPOSTAMENTO:
		{
			uint8_t contNO=0, contSI=0;
			if(IS_DEBUG)
			Serial_print("presa_spostamento");
			while(contNO+contSI<5)
			{
				uint8_t range = readLASER();
				if(range<20)
				{
					contSI++;
				}
				else
				{
					contNO++;
				}
			}
			if(contNO>4)
			{
				f_presa_pallina=0;
			}
			else
			{
				f_presa_pallina=1;
			}
			incrementa_step();
			azione=2200;
			break;
		}
		case AZZERA_VAR_RASP_SPOSTAMENTO:
		{
			if(IS_DEBUG)
			Serial_print("az_var");
			distanza_rasp=VAL_INIZIALE;
			angolo_rasp=VAL_INIZIALE;
			incrementa_step();
			break;
		}
		case INIZIO_SPOSTAMENTO_PALLINE:
		{
			if(IS_DEBUG)
			Serial_print("start_spost_pall");
			f_spostamento_palline=1;
			incrementa_step();
			break;
		}
		case FINE_SPOSTAMENTO_PALLINE:
		{
			if(IS_DEBUG)
			Serial_print("end_spost_pall");
			f_spostamento_palline=0;
			incrementa_step();
			break;
		}
		case CONTROLLA_PALLINA_ARGENTO:
		{
			if(IS_DEBUG)
			Serial_print("cont_pal_arg\t");
			if((f_num_palline_pianale<4 && f_num_palline_argento>0) || f_spostamento_palline)
			{
				if(f_num_scandagliate==2 ||f_num_scandagliate==4)
					USART_Tx_3('b');//cerca palline fino a distanza 40
				else
					USART_Tx_3('a');//cerca palline fino a distanza 35
						
				set_millis(M_DELAY_PAL);
				while(angolo_rasp==VAL_INIZIALE )
				{
					receive_rasp(&angolo_rasp);
					if(getError()) 
					{
						USART_Tx_3('e');//spedisco eventuale codice d'errore
						flush_buffer();//pulisco il buffer da eventuali dati sbagliati
						set_val_error();//metto a zero il valore errore
					}
					if(get_millis(M_DELAY_PAL)>2500)//in caso ci siano problemi dopo 4s di tentativi si taglia la corda
						break;
				}
					
				USART_Tx_3('d');//dopo aver ricevuto con successo l'angolo chiedo la distanza
				set_millis(M_DELAY_PAL);
				while(distanza_rasp==VAL_INIZIALE)
				{
					receive_rasp(&distanza_rasp);
					if(getError()) 
					{
						USART_Tx_3('e');//spedisco eventuale codice d'errore
						flush_buffer();//pulisco il buffer da eventuali dati sbagliati
						set_val_error();//metto a zero il valore errore
					}
					if(get_millis(M_DELAY_PAL)>2500)//in caso ci siano problemi dopo 4s di tentativi si taglia la corda
					break;
				}
				USART_Tx_3('m');
				angolo_rasp = angolo_rasp<=-5 || angolo_rasp>=5 ? angolo_rasp==VAL_NOBALL ? VAL_INIZIALE : angolo_rasp : 0;
				distanza_rasp = distanza_rasp==VAL_NOBALL ? VAL_INIZIALE : distanza_rasp;
				if(((f_verso_gl==SX2DX && f_num_scandagliate==1) || (f_verso_gl==DX2SX && f_num_scandagliate==5)) && angolo_rasp<-8 && angolo_rasp!=254 && f_filtro_angolo==1) {angolo_rasp=-8;f_avanza_pid=1;}
				else if(((f_verso_gl==SX2DX && f_num_scandagliate==5) || (f_verso_gl==DX2SX && f_num_scandagliate==1)) && angolo_rasp>8 && angolo_rasp!=254 && f_filtro_angolo==1) {angolo_rasp=8;f_avanza_pid=1;}
				else f_avanza_pid=0;
			}
			azione=2200;
			incrementa_step();
			break;
		}
		case CONTROLLA_PALLINA_NERA:
		{
			if(IS_DEBUG)
			Serial_print("cont_pal_nera\t");
			if((f_num_palline_pianale<4 && f_num_palline_nere>0) || f_spostamento_palline)
			{
				if(f_num_scandagliate==2 ||f_num_scandagliate==4)
					USART_Tx_3('o');//cerca palline fino a distanza 40
				else
					USART_Tx_3('n');//cerca palline fino a distanza 35
						
				set_millis(M_DELAY_PAL);
				while(angolo_rasp==VAL_INIZIALE )
				{
					receive_rasp(&angolo_rasp);
					if(getError())
					{
						USART_Tx_3('e');//spedisco eventuale codice d'errore
						flush_buffer();//pulisco il buffer da eventuali dati sbagliati
						set_val_error();//metto a zero il valore errore
					}
					if(get_millis(M_DELAY_PAL)>2500)//in caso ci siano problemi dopo 4s di tentativi si taglia la corda
					break;
				}
					
				USART_Tx_3('d');//dopo aver ricevuto con successo l'angolo chiedo la distanza
				set_millis(M_DELAY_PAL);
				while(distanza_rasp==VAL_INIZIALE)
				{
					receive_rasp(&distanza_rasp);
					if(getError())
					{
						USART_Tx_3('e');//spedisco eventuale codice d'errore
						flush_buffer();//pulisco il buffer da eventuali dati sbagliati
						set_val_error();//metto a zero il valore errore
					}
					if(get_millis(M_DELAY_PAL)>2500)//in caso ci siano problemi dopo 4s di tentativi si taglia la corda
					break;
				}
				USART_Tx_3('m');
				angolo_rasp = angolo_rasp<=-5 || angolo_rasp>=5 ? angolo_rasp==VAL_NOBALL ? VAL_INIZIALE : angolo_rasp : 0;
				distanza_rasp = distanza_rasp==VAL_NOBALL ? VAL_INIZIALE : distanza_rasp;
				if(((f_verso_gl==SX2DX && f_num_scandagliate==1) || (f_verso_gl==DX2SX && f_num_scandagliate==5)) && angolo_rasp<-8 && angolo_rasp!=254 && f_filtro_angolo==1) {angolo_rasp=-8; f_avanza_pid=1;}
				else if(((f_verso_gl==SX2DX && f_num_scandagliate==5) || (f_verso_gl==DX2SX && f_num_scandagliate==1)) && angolo_rasp>8 && angolo_rasp!=254 && f_filtro_angolo==1) {angolo_rasp=8; f_avanza_pid=1;}
				else f_avanza_pid=0;
			}
			azione=2200;
			incrementa_step();
			break;
		}
		case LASER:
		{
			if(IS_DEBUG)
			Serial_print("laser");
			uint8_t range = readLASER();
			//Serial_print("\t\t\t\t\tlaser");
			//Serial_int(range);
			f_avanti_piano=1;
			if(range<130)//130 al posto di 150 per quando siamo vicini al muro
			{
				incrementa_step();
			}
			if(fin_pinza_dx()||fin_pinza_sx())
			{
				f_contatto_muro=1;
				incrementa_step();
			}
			if(f_avanza_pid)
				azione=3400;//avanti lentamente con PID per palline vicino al muro
			else
				azione=3300;//avanti più lentamente con solo pwm
			break;
		}
		case AZZERA_PRIMO_180:
		{
			if(IS_DEBUG)
			Serial_print("azzera_primo_180");
			f_primo_180=0;
			incrementa_step();
			break;
		}
		case RILEVA_MIS_ENC:
		{
			if(IS_DEBUG)
			Serial_print("ril_enc");
			distanza_percorsa=get_posizione(0)*10;
			incrementa_step();
			break;
		}
		case AZZERA_VAR_RASP:
		{
			if(IS_DEBUG)
			Serial_print("az_var");
			distanza_rasp=VAL_INIZIALE;
			angolo_rasp=VAL_INIZIALE;
			if(f_cerca_argento)
				f_num_palline_argento--;
			else if(f_cerca_nere)
				f_num_palline_nere--;
			f_num_palline_pianale++;
			incrementa_step();
			break;
		}
		case ALLINEAMENTO_FINECORSA_DAVANTI:
		{
			if(IS_DEBUG)
			Serial_print("al_fin_davanti");
			azione=0;
			if(fin_davanti_dx() && fin_davanti_sx())
			{
				if(f_all_fin==0)
				{
					set_millis(0);
					f_all_fin=1;
				}
				else if(get_millis(0)>=100)
				{
					incrementa_step();
					f_all_fin=0;
				}
			}
			else if(fin_davanti_dx())
			{
				azione=3000;
				f_all_fin=0;
			}
			else if(fin_davanti_sx())
			{
				azione=3100;
				f_all_fin=0;
			}
			else
				f_all_fin=0;
			break;
		}
		case DISATTIVA_CONTROLLO_ANGOLO:
		{
			if(IS_DEBUG)
			Serial_print("disattiva_cont_angolo");
			if(pos_raccolta!=0)f_controlla_angolo=0;
			incrementa_step();
			break;
		}
		case DISATTIVA_SCANDAGLIA:
		{
			if(IS_DEBUG)
			Serial_print("disattiva_scandaglia");
			if(f_num_palline_pianale>=4 || (f_num_palline_argento==0 && f_cerca_argento==1) || (f_num_palline_nere==0 && f_cerca_nere==1))
				f_scandaglia=0;
			incrementa_step();
			break;
		}
		case FINE_PRIMA_PARTE:
		{
			if(IS_DEBUG)
			Serial_print("fine_prima_parte");
			f_fine_prima_parte++;
			f_step[0]=0;
			f_step[1]=0;
			f_step[2]=0;
			f_step[3]=0;
			break;
		}
		case CONTATTO_FIN_DAVANTI:
		{
			if(IS_DEBUG)
			Serial_print("contatto_fin_davanti");
			azione=0;
			if(fin_davanti_dx() || fin_davanti_sx())
			{
				incrementa_step();
				azione=2200;
			}
			break;
		}
		case CONTATTO_FIN_DIETRO:
		{
			if(IS_DEBUG)
			Serial_print("contatto_fin_dietro");
			azione=2100;
			if(fin_dietro_dx() && fin_dietro_sx())
			{
				incrementa_step();
				azione=2200;
			}
			else if(fin_dietro_dx())
			{
				azione=3100;
			}
			else if(fin_dietro_sx())
			{
				azione=3000;
			}
			break;
		}
		case AZZERA_VAR_PALLINE:
		{
			if(IS_DEBUG)
			Serial_print("AZZ_VAR_PALLINE");
			f_num_palline_pianale=0;
			f_scaricato=1;
			f_scandaglia=1;
			f_muro_alle_spalle=0;
			incrementa_step();
			break;
		}
		case VERIFICA_PRESA:
		{
			uint8_t contNO=0, contSI=0;
			if(IS_DEBUG)
			Serial_print("presa");
			while(contNO+contSI<5)
			{
				uint8_t range = readLASER();
				if(range<20)
				{
					Serial_int(range);
					Serial_println(" SI");
					contSI++;
				}
				else
				{
					Serial_int(range);
					Serial_println(" NO");
					contNO++;
				}
			}
			if(contNO>4)
			{
				if(f_cerca_argento)
					f_num_palline_argento++;
				else if(f_cerca_nere)
					f_num_palline_nere++;
				f_num_palline_pianale--;
				f_presa_pallina=0;
			}
			else
			{
				f_presa_pallina=1;
			}
			incrementa_step();
			azione=2200;
			break;
		}
		case AZZERA_VAR_SCARICA:
		{
			if(IS_DEBUG)
			Serial_print("azzera_var_scarica");
			f_scaricato=0;
			azione=2200;
			incrementa_step();
			break;
		}
		case MURO_ALLE_SPALLE://serve per otimizzare alcuni movimenti per l aprocedura di scarico
		{
			if(IS_DEBUG)
			Serial_print("muro_alle_spalle");
			if(f_num_palline_pianale>=4 || (f_num_palline_argento==0 && f_cerca_argento==1) || (f_num_palline_nere==0 && f_cerca_nere==1)) f_muro_alle_spalle=1;
			azione=2200;
			incrementa_step();
			break;
		}
		case CONDUZIONE_ARGENTO:
		{
			azione=2200;
			if(IS_DEBUG)
			Serial_print("conduzione_argento");
			if(contatto_pallina())
				f_argento_vera=1;
			incrementa_step();
			break;
		}
		case CAMBIA_RICERCA:
		{
			if(IS_DEBUG)
			Serial_print("cambia_ricerca");
			if((f_num_palline_argento==0 && f_cerca_argento==1)||(f_avanzamenti>=NUM_AVANZAMENTI_MAX))
			{
				Serial_print("raccolte_tutte_le_argento");
				f_avanzamenti=0;
				f_cerca_argento=0;
				f_cerca_nere=1;
			}
			incrementa_step();
			break;
		}
		case TUTTE_NERE:
		{
			if(f_num_palline_nere==0 && f_cerca_nere==1)
			{
				Serial_print("raccolte_tutte_le_nere");
				while(1)
				{
						
				}
			}
			incrementa_step();
			break;
		}
		case S_AVANZAMENTO://stabilisce numero massimo di ricerche
		{
			if(IS_DEBUG)
			Serial_print("s_avanzamento");
			f_avanzamenti++;
			if(f_avanzamenti>=NUM_AVANZAMENTI_MAX && f_num_palline_pianale>0)
				f_num_palline_argento=0;
			incrementa_step();
			break;
		}
		case CONTA_SCANDAGLIATE:
		{
			if(IS_DEBUG)
			Serial_print("conta_scandagliate");
			f_num_scandagliate++;
			incrementa_step();
			break;
		}
		case AZZERA_SCANDAGLIATE:
		{
			if(IS_DEBUG)
			Serial_print("azzera_scandagliate");
			f_num_scandagliate=0;
			incrementa_step();
			break;
		}
		case AZZERA_CONTATTO_MURO:
		{
			if(IS_DEBUG)
			Serial_print("azzera_contatto_muro");
			f_contatto_muro=0;
			incrementa_step();
			break;
		}
		case IMP_FILTRO_ANGOLO:
		{
			if(IS_DEBUG)
			Serial_print("imp_filtro_angolo");
			f_filtro_angolo=1;
			incrementa_step();
			break;
		}
		case DIS_FILTRO_ANGOLO:
		{
			if(IS_DEBUG)
			Serial_print("disattiva_filtro_angolo");
			f_filtro_angolo=0;
			incrementa_step();
			break;
		}
		case SALVA_SU_EEPROM:
		{
			if(IS_DEBUG)
			Serial_print("salva_su_eeprom");
			write_EEPROM(IND_EE_POS_RACCOLTA,pos_raccolta);
			write_EEPROM(IND_EE_NUM_PAL_ARGENTO,f_num_palline_argento);
			write_EEPROM(IND_EE_NUM_PAL_NERE,f_num_palline_nere);
			write_EEPROM(IND_EE_NUMERO_AVANZAMENTI,f_avanzamenti);
			incrementa_step();
			break;
		}
		case VIBRA:
		{
			if(start_vibra)
			{
				set_millis(5);
				start_vibra=0;
				cont_vibra=0;
				azione=0;
			}
			else if(get_millis(5)>=80 && cont_vibra%2==0)
			{
				azione=2100;
				set_millis(5);
				cont_vibra++;
			}
			else if(fin_dietro_sx() && fin_dietro_dx() && cont_vibra%2==1)
			{
				azione=0;
				set_millis(5);
				cont_vibra++;
			}
			if(cont_vibra>=14)//dispari: l'ultima volta va indietro
			{
				start_vibra=1;
				incrementa_step();
			}
			break;
		}
		default:
		{
			if(f_tipo_movimento>=AVANTI_MISURA-1600 && f_tipo_movimento<=AVANTI_MISURA+1600)
			{
				if(IS_DEBUG)
				Serial_print("avanti_mis");
				if(f_tipo_movimento-AVANTI_MISURA==0)//aggiunto nel caso in cui la distanza da percorrere fosse zero
				{
					incrementa_step();
					f_avanti_misura0=1;
					Serial_print("av01");
					azione=2200;
				}
				else if(get_posizione(0)<f_tipo_movimento-AVANTI_MISURA)
				{
					if(f_scandaglia==0 && f_tipo_movimento-AVANTI_MISURA>250 && (f_fine_prima_parte==1 || f_fine_prima_parte==2))
						azione=3200;//velocità sostenuta per andare a scaricare più rapidamente
					else if(f_tipo_movimento-AVANTI_MISURA<70 && f_avanti_piano==1)
						azione=3300;//velocità bassa per brevi distanze
					else
						azione=0;//velocità ordinaria
					if(get_posizione(0)>(f_tipo_movimento-AVANTI_MISURA)/10.0)//33.5
					{
						incrementa_step();
						f_avanti_misura0=0;
					}
				}
				else
				{
					azione=2100;
					if(get_posizione(0)<(f_tipo_movimento-AVANTI_MISURA)/10.0)//33.5
					{
						incrementa_step();
						f_avanti_misura0=0;
					}
				}
					
			}
			else if(f_tipo_movimento>=ANGOLO_VARIABILE-30 && f_tipo_movimento<=ANGOLO_VARIABILE+30)
			{
				if(IS_DEBUG)
				Serial_print("ang_var");
				azione=f_tipo_movimento;
			}
			else if(f_tipo_movimento>=DELAY)
			{
				if(IS_DEBUG)
				Serial_print("delay");
				if(!f_mil)
				{
					set_millis(M_DELAY_STANZA);
					f_mil=1;
					azione=2200;
				}
				else if(get_millis(M_DELAY_STANZA)>=f_tipo_movimento-DELAY)
				{
					incrementa_step();
					f_mil=0;
				}
			}
			else if(f_tipo_movimento>=PINZA)
			{
				if(IS_DEBUG)
				{
					Serial_print("\tpinza ");
					Serial_print("\t");
					Serial_int(f_tipo_movimento-PINZA);
					Serial_println("");
				}
				if(f_tipo_movimento==PINZA+OFF)
				{
					f_argento_vera=0;
				}
				else if(f_tipo_movimento==PINZA+CHIUDI)
					f_avanti_piano=0;
				if(!pinza(f_tipo_movimento-PINZA))
					incrementa_step();
			}
			else if(f_tipo_movimento>=CONTROLLA_ANGOLO && f_tipo_movimento<=CONTROLLA_ANGOLO+4)
			{
				if(IS_DEBUG)
				Serial_print("cont_angolo");
				if(/*!f_dist_ang_compiuta*/f_dist_ang_compiuta==0)
				{
					if(get_posizione(0)<=15)
					{
						azione=0;
						if(fin_davanti_dx() || fin_davanti_sx())
						{
							pos_raccolta=f_tipo_movimento-CONTROLLA_ANGOLO;
							//f_dist_ang_compiuta=1;
							//f_dist_ang_compiuta=2;//c'erano questi
							//set_posizione(0);//c'erano questi
							incrementa_step();
						}
					}
					else
					{
						f_dist_ang_compiuta=1;
						azione=2200;
					}
				}
				//else
				else if(f_dist_ang_compiuta==1)
				{
					azione=2100;
					if(get_posizione(0)<=0)
					{
						azione=2200;
						f_dist_ang_compiuta=0;
						incrementa_step();
					}
				}
				/*else if(f_dist_ang_compiuta==2)
				{
					azione=2100;
					if(get_posizione(0)<=-10)
					{
						azione=2200;
						f_dist_ang_compiuta=0;
						incrementa_step();
					}
				}*/
			}
			else if(f_tipo_movimento>=ASSEGNA_ANGOLO && f_tipo_movimento<=ASSEGNA_ANGOLO+4)
			{
				if(IS_DEBUG)
				Serial_print("ass_ang");
				pos_raccolta=f_tipo_movimento-ASSEGNA_ANGOLO;
				incrementa_step();
			}
			else if(f_tipo_movimento>=LED && f_tipo_movimento<=LED+OFF_LED)
			{
				led(f_tipo_movimento-LED);
				incrementa_step();
			}
			break;
		}
	}
	if(IS_DEBUG)
	Serial_println("");
}
void esegui_movimento(int f_tipo_movimento)
{
	if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
		es_mv(f_tipo_movimento);
	f_id[pos_gl]++;
}
void esegui_movimento_if(int f_tipo_movimento,int num_parametri,...)
{
	if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
	{
		/*Serial_print("\t\tC=");
		Serial_int(f_tipo_movimento);
		Serial_println("");*/
		va_list parametri;
		int par1,par2,valido=1,num_confronti=num_parametri/2;
		/*Serial_int(num_confronti);
		Serial_println("");*/
		va_start(parametri,num_parametri);
		for(int i = 0; i < num_confronti && valido; i++) {
			par1 = va_arg(parametri, int);
			par2 = va_arg(parametri, int);
			/*Serial_int(par1);
			Serial_print("=");
			Serial_int(par2);
			Serial_println("");*/
			if(par1!=par2)
			valido=0;
		}
		/*Serial_print("\t\tV=");
		Serial_int(valido);
		Serial_println("");*/
		if(valido)
			es_mv(f_tipo_movimento);
		else
		{
			pos_salvato=pos_gl;
			incrementa_step();
		}
		va_end(parametri);
	}
	f_id[pos_gl]++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////IMPOSTAZIONI STEP ID//////////////////////////////////////////////////////
void esegui_movimento_imposta_id(uint16_t f_imp0,uint16_t f_imp1,uint16_t f_imp2,uint16_t f_imp3)
{
	f_id[0]=f_imp0;
	f_id[1]=f_imp1;
	f_id[2]=f_imp2;
	f_id[3]=f_imp3;
}

void es_mv_step(uint16_t f_imp0,uint16_t f_imp1,uint16_t f_imp2,uint16_t f_imp3)
{
	if(IS_DEBUG)
	{
		Serial_int(f_id[0]);
		Serial_print(" ");
		Serial_int(f_id[1]);
		Serial_print(" ");
		Serial_int(f_id[2]);
		Serial_print(" ");
		Serial_int(f_id[3]);
		Serial_print("\t");
		Serial_print("imp_step\t");
		Serial_int(f_imp0);
		Serial_print(" ");
		Serial_int(f_imp1);
		Serial_print(" ");
		Serial_int(f_imp2);
		Serial_print(" ");
		Serial_int(f_imp3);
		Serial_println("");
	}
	f_step[0]=f_imp0;
	f_step[1]=f_imp1;
	f_step[2]=f_imp2;
	f_step[3]=f_imp3;
}
void esegui_movimento_imposta_step(uint16_t f_imp0,uint16_t f_imp1,uint16_t f_imp2,uint16_t f_imp3)
{
	if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
		es_mv_step(f_imp0,f_imp1,f_imp2,f_imp3);
	f_id[pos_gl]++;
}
void esegui_movimento_imposta_step_if(uint16_t f_imp0,uint16_t f_imp1,uint16_t f_imp2,uint16_t f_imp3,int num_parametri,...)
{
	if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
	{
		/*Serial_print("\t\tC=");
		Serial_int(f_tipo_movimento);
		Serial_println("");*/
		va_list parametri;
		int par1,par2,valido=1,num_confronti=num_parametri/2;
		/*Serial_int(num_confronti);
		Serial_println("");*/
		va_start(parametri,num_parametri);
		for(int i = 0; i < num_confronti && valido; i++) {
			par1 = va_arg(parametri, int);
			par2 = va_arg(parametri, int);
			/*Serial_int(par1);
			Serial_print("=");
			Serial_int(par2);
			Serial_println("");*/
			if(par1!=par2)
			valido=0;
		}
		/*Serial_print("\t\tV=");
		Serial_int(valido);
		Serial_println("");*/
		if(valido)
			es_mv_step(f_imp0,f_imp1,f_imp2,f_imp3);
		else
			incrementa_step();
		va_end(parametri);
	}
	f_id[pos_gl]++;
}

void es_mv_ritorno()
{
	if(IS_DEBUG)
	{
		Serial_int(f_id[0]);
		Serial_print(" ");
		Serial_int(f_id[1]);
		Serial_print(" ");
		Serial_int(f_id[2]);
		Serial_print(" ");
		Serial_int(f_id[3]);
		Serial_print("\t");
	}
	if(pos_gl==1)
	{
		f_step[0]++;
		f_step[1]=0;
		f_step[2]=0;
		f_step[3]=0;
	}
	else if(pos_gl==2)
	{
		f_step[1]++;
		f_step[2]=0;
		f_step[3]=0;
	}
	else if(pos_gl==3)
	{
		f_step[2]++;
		f_step[3]=0;
	}
	if(IS_DEBUG)
	{
		Serial_print("imp_rit\t");
		Serial_int(f_step[0]);
		Serial_print(" ");
		Serial_int(f_step[1]);
		Serial_print(" ");
		Serial_int(f_step[2]);
		Serial_print(" ");
		Serial_int(f_step[3]);
		Serial_println("");
	}
}
void esegui_movimento_imposta_ritorno()
{
	if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
	{
		es_mv_ritorno();
	}
	f_id[pos_gl]++;
}
void esegui_movimento_imposta_ritorno_if(int num_parametri,...)
{
	if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
	{
		va_list parametri;
		int par1,par2,valido=1,num_confronti=num_parametri/2;
		/*Serial_int(num_confronti);
		Serial_println("");*/
		va_start(parametri,num_parametri);
		for(int i = 0; i < num_confronti && valido; i++) {
			par1 = va_arg(parametri, int);
			par2 = va_arg(parametri, int);
			/*Serial_int(par1);
			Serial_print("=");
			Serial_int(par2);
			Serial_println("");*/
			if(par1!=par2)
			valido=0;
		}
		/*Serial_print("\t\tV=");
		Serial_int(valido);
		Serial_println("");*/
		if(valido)
			es_mv_ritorno();
		else
			incrementa_step();
		va_end(parametri);
	}
	f_id[pos_gl]++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////


void esegui_movimento_if_if(int f_tipo_movimento,int flag1,int val1,int flag2,int val2)
{
	if(flag1==val1 && flag2==val2)
		esegui_movimento(f_tipo_movimento);
	else if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
	{
		pos_salvato=pos_gl;
		incrementa_step();
		f_id[pos_gl]++;
	}
	else
	f_id[pos_gl]++;
	
}//da eliminare
void esegui_movimento_imposta_step_if_if(uint16_t f_imp0,uint16_t f_imp1,uint16_t f_imp2,uint16_t f_imp3,uint8_t flag1,uint8_t val1,uint8_t flag2,uint8_t val2)
{
	if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3] && flag1==val1 && flag2==val2)
	{
		if(IS_DEBUG)
		{
			Serial_int(f_id[0]);
			Serial_print(" ");
			Serial_int(f_id[1]);
			Serial_print(" ");
			Serial_int(f_id[2]);
			Serial_print(" ");
			Serial_int(f_id[3]);
			Serial_print("\t");
			Serial_print("imp_step_if\t");
			Serial_int(f_imp0);
			Serial_print(" ");
			Serial_int(f_imp1);
			Serial_print(" ");
			Serial_int(f_imp2);
			Serial_print(" ");
			Serial_int(f_imp3);
			Serial_println("");
		}
		f_step[0]=f_imp0;
		f_step[1]=f_imp1;
		f_step[2]=f_imp2;
		f_step[3]=f_imp3;
	}
	else if(f_id[0]==f_step[0] && f_id[1]==f_step[1] && f_id[2]==f_step[2] && f_id[3]==f_step[3])
	{
		pos_salvato=pos_gl;
		incrementa_step();
	}
	f_id[pos_gl]++;
}//da eliminare

//////////////////////////////////FUNZIONI RIPETUTE MULTI_MOVIMENTO////////////////////////////////////////
void raccogli()
{
	pos_gl++;
	if(pos_gl==1 && f_step[0]==f_id[0])
	{
		if(angolo_rasp!=VAL_INIZIALE && angolo_rasp>=-17 && angolo_rasp<=17)//è stato aggiunto il controllo dell'angolo per evitare di prendere palline troppo decentrate
		{
			if(f_num_palline_pianale<4)
			{
				esegui_movimento(LED+G);
				esegui_movimento(ANGOLO_VARIABILE+angolo_rasp);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+(distanza_rasp*10-100));
				//il discorso degli stop è uguale a sotto ma al contrario perchè c'è un AVANTI_MISURA+
				esegui_movimento_if(STOP,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
				esegui_movimento_if(STOP_INDIETRO,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),-(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
				esegui_movimento(RILASCIO_MOTORI);
				esegui_movimento(PINZA+ON);
				esegui_movimento(PINZA+POS+3);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(LASER);
				esegui_movimento(STOP);
				esegui_movimento(RILEVA_MIS_ENC);
				esegui_movimento(POS_ENCODER);
				esegui_movimento_if(AVANTI_MISURA+10,4,f_contatto_muro,0,f_cerca_argento,1);//+20
				esegui_movimento_if(AVANTI_MISURA+10,4,f_contatto_muro,0,f_cerca_nere,1);
				esegui_movimento_if(STOP,2,f_contatto_muro,0);
				esegui_movimento_if(AVANTI_MISURA-20,2,f_contatto_muro,1);
				esegui_movimento_if(STOP_INDIETRO,2,f_contatto_muro,1);
				esegui_movimento(RILASCIO_MOTORI);
				esegui_movimento(PINZA+ON);
				esegui_movimento(PINZA+CHIUDI);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA-70);
				esegui_movimento(STOP_INDIETRO);
				esegui_movimento_if(CONDUZIONE_ARGENTO,2,f_cerca_argento,1);//su conduzione argento non diminuiamo il numero delle palline perchè lo fa gia verifica presa
				esegui_movimento_if(PINZA+APRI,4,f_cerca_argento,1,f_argento_vera,0);
				esegui_movimento(PINZA+POS+100);
				esegui_movimento(VERIFICA_PRESA);
				esegui_movimento(PINZA+APRI);
				esegui_movimento(POS_ENCODER);
				esegui_movimento_if(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-60),4,f_contatto_muro,0,f_cerca_argento,1);//-50
				esegui_movimento_if(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-60),4,f_contatto_muro,0,f_cerca_nere,1);//riga aggiunta per cambiare il ritorno in caso sia nera//-60
				esegui_movimento_if(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-90),2,f_contatto_muro,1);
				//il discorso degli stop è uguale a sopra ma al contrario perchè c'è un AVANTI_MISURA-
				esegui_movimento_if(STOP_INDIETRO,2,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),(distanza_rasp*10-100)*(distanza_rasp*10-100));
				esegui_movimento_if(STOP,2,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),-(distanza_rasp*10-100)*(distanza_rasp*10-100));
				esegui_movimento(ANGOLO_VARIABILE-angolo_rasp);//32
				esegui_movimento(PINZA+OFF);
				esegui_movimento(LED+OFF_LED);
				esegui_movimento(VAR_RACCOLTA);
				esegui_movimento(AZZERA_VAR_RASP);
				esegui_movimento_imposta_ritorno();
			}
			else esegui_movimento_imposta_ritorno();
		}
		else 
		{
			//esegui_movimento(AZZERA_VAR_RACCOLTA);
			esegui_movimento_imposta_ritorno();
		}
	}
	else if(pos_gl==2 && f_step[0]==f_id[0] && f_step[1]==f_id[1])
	{
		if(angolo_rasp!=VAL_INIZIALE)
		{
			esegui_movimento(LED+G);
			esegui_movimento(ANGOLO_VARIABILE+angolo_rasp);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+(distanza_rasp*10-100));
			//il discorso degli stop è uguale a sotto ma al contrario perchè c'è un AVANTI_MISURA+
			esegui_movimento_if(STOP,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
			esegui_movimento_if(STOP_INDIETRO,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),-(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
			esegui_movimento(RILASCIO_MOTORI);
			esegui_movimento(PINZA+ON);
			esegui_movimento(PINZA+POS+3);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(LASER);
			esegui_movimento(STOP);
			esegui_movimento(RILEVA_MIS_ENC);
			esegui_movimento(POS_ENCODER);
			esegui_movimento_if(AVANTI_MISURA+10,4,f_contatto_muro,0,f_cerca_argento,1);//+20
			esegui_movimento_if(AVANTI_MISURA+10,4,f_contatto_muro,0,f_cerca_nere,1);
			esegui_movimento_if(STOP,2,f_contatto_muro,0);
			esegui_movimento_if(AVANTI_MISURA-20,2,f_contatto_muro,1);
			esegui_movimento_if(STOP_INDIETRO,2,f_contatto_muro,1);
			esegui_movimento(RILASCIO_MOTORI);
			esegui_movimento(PINZA+ON);
			esegui_movimento(PINZA+CHIUDI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-70);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento_if(CONDUZIONE_ARGENTO,2,f_cerca_argento,1);//su conduzione argento non diminuiamo il numero delle palline perchè lo fa gia verifica presa
			esegui_movimento_if(PINZA+APRI,4,f_cerca_argento,1,f_argento_vera,0);
			esegui_movimento(PINZA+POS+100);
			esegui_movimento(VERIFICA_PRESA);
			esegui_movimento(PINZA+APRI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento_if(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-60),4,f_contatto_muro,0,f_cerca_argento,1);//-50
			esegui_movimento_if(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-60),4,f_contatto_muro,0,f_cerca_nere,1);//riga aggiunta per cambiare il ritorno in caso sia nera//-60
			esegui_movimento_if(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-90),2,f_contatto_muro,1);
			//il discorso degli stop è uguale a sopra ma al contrario perchè c'è un AVANTI_MISURA-
			esegui_movimento_if(STOP_INDIETRO,2,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),(distanza_rasp*10-100)*(distanza_rasp*10-100));
			esegui_movimento_if(STOP,2,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),-(distanza_rasp*10-100)*(distanza_rasp*10-100));
			esegui_movimento(ANGOLO_VARIABILE-angolo_rasp);//32
			esegui_movimento(LED+OFF_LED);
			esegui_movimento(PINZA+OFF);
			esegui_movimento(AZZERA_VAR_RASP);
			esegui_movimento_imposta_step_if(f_step[0],f_step[1]+1,0,0,6,f_contatto_muro,1,f_num_scandagliate,2,f_presa_pallina,0);
			esegui_movimento_imposta_step_if(f_step[0],f_step[1]+1,0,0,6,f_contatto_muro,1,f_num_scandagliate,4,f_presa_pallina,0);
			esegui_movimento_imposta_step_if(f_step[0],f_step[1]-2,0,0,2,f_cerca_argento,1);
			esegui_movimento_imposta_step_if(f_step[0],f_step[1]-1,0,0,2,f_cerca_nere,1);
		}
		else esegui_movimento_imposta_ritorno();
	}
	pos_gl--;
	f_id[pos_gl]++;
}

void sposta_dx()
{
	esegui_movimento(LED+R);
	esegui_movimento(ANGOLO_VARIABILE+angolo_rasp);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA+(distanza_rasp*10-100));
	esegui_movimento_if(STOP,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
	esegui_movimento_if(STOP_INDIETRO,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),-(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
	esegui_movimento(RILASCIO_MOTORI);
	esegui_movimento(PINZA+ON);
	esegui_movimento(PINZA+POS+3);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(LASER);
	esegui_movimento(STOP);
	esegui_movimento(RILEVA_MIS_ENC);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA+10);
	esegui_movimento(STOP);
	esegui_movimento(PINZA+ON);
	esegui_movimento(PINZA+CHIUDI);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA-70);
	esegui_movimento(STOP_INDIETRO);
	esegui_movimento(PINZA+POS+50);
	esegui_movimento(VERIFICA_PRESA_SPOSTAMENTO);//appena aggiunto da verificare per essere sicuri di aver preso la pallina
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-60));
	esegui_movimento(STOP_INDIETRO);
	esegui_movimento(ANGOLO_VARIABILE-angolo_rasp);
	esegui_movimento(DELAY+150);//aggiunto per evitare accumulo di errore
	esegui_movimento_if(ROT_90_DX,2,f_presa_pallina,1);
	esegui_movimento_if(POS_ENCODER,2,f_presa_pallina,1);
	esegui_movimento_if(AVANTI_MISURA+20,2,f_presa_pallina,1);
	esegui_movimento_if(STOP,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+POS+3,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+APRI,2,f_presa_pallina,1);
	esegui_movimento_if(POS_ENCODER,2,f_presa_pallina,1);
	esegui_movimento_if(AVANTI_MISURA-20,2,f_presa_pallina,1);
	esegui_movimento_if(STOP_INDIETRO,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+POS+100,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+OFF,2,f_presa_pallina,1);
	esegui_movimento_if(ROT_90_SX,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+APRI,2,f_presa_pallina,0);
	esegui_movimento_if(PINZA+POS+100,2,f_presa_pallina,0);
	esegui_movimento(LED+OFF_LED);
	esegui_movimento(VAR_RACCOLTA);
	esegui_movimento(AZZERA_VAR_RASP_SPOSTAMENTO);
	esegui_movimento_imposta_ritorno();
}
void sposta_sx()
{
	esegui_movimento(LED+R);
	esegui_movimento(ANGOLO_VARIABILE+angolo_rasp);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA+(distanza_rasp*10-100));
	esegui_movimento_if(STOP,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
	esegui_movimento_if(STOP_INDIETRO,4,(distanza_rasp*10-100)*abs(distanza_rasp*10-100),-(distanza_rasp*10-100)*(distanza_rasp*10-100),f_avanti_misura0,0);
	esegui_movimento(RILASCIO_MOTORI);
	esegui_movimento(PINZA+ON);
	esegui_movimento(PINZA+POS+3);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(LASER);
	esegui_movimento(STOP);
	esegui_movimento(RILEVA_MIS_ENC);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA+10);
	esegui_movimento(STOP);
	esegui_movimento(PINZA+ON);
	esegui_movimento(PINZA+CHIUDI);
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA-70);
	esegui_movimento(STOP_INDIETRO);
	esegui_movimento(PINZA+POS+50);
	esegui_movimento(VERIFICA_PRESA_SPOSTAMENTO);//appena aggiunto da verificare per essere sicuri di aver preso la pallina
	esegui_movimento(POS_ENCODER);
	esegui_movimento(AVANTI_MISURA-(distanza_rasp*10-100+distanza_percorsa-60));
	esegui_movimento(STOP_INDIETRO);
	esegui_movimento(ANGOLO_VARIABILE-angolo_rasp);
	esegui_movimento(DELAY+150);//aggiunto per evitare accumulo di errore
	esegui_movimento_if(ROT_90_SX,2,f_presa_pallina,1);
	esegui_movimento_if(POS_ENCODER,2,f_presa_pallina,1);
	esegui_movimento_if(AVANTI_MISURA+20,2,f_presa_pallina,1);
	esegui_movimento_if(STOP,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+POS+3,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+APRI,2,f_presa_pallina,1);
	esegui_movimento_if(POS_ENCODER,2,f_presa_pallina,1);
	esegui_movimento_if(AVANTI_MISURA-20,2,f_presa_pallina,1);
	esegui_movimento_if(STOP_INDIETRO,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+POS+100,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+OFF,2,f_presa_pallina,1);
	esegui_movimento_if(ROT_90_DX,2,f_presa_pallina,1);
	esegui_movimento_if(PINZA+APRI,2,f_presa_pallina,0);
	esegui_movimento_if(PINZA+POS+100,2,f_presa_pallina,0);
	esegui_movimento(LED+OFF_LED);
	esegui_movimento(VAR_RACCOLTA);
	esegui_movimento(AZZERA_VAR_RASP_SPOSTAMENTO);
	esegui_movimento_imposta_ritorno();
}
void sposta(uint8_t verso)
{
	pos_gl++;
	if(pos_gl==1 && f_step[0]==f_id[0])
	{
		if(angolo_rasp!=VAL_INIZIALE && angolo_rasp>=-17 && angolo_rasp<=17 && verso==SPOSTA_SX)//fatto il 16/03
		{
			sposta_sx();
		}
		else if(angolo_rasp!=VAL_INIZIALE && angolo_rasp>=-17 && angolo_rasp<=17 && verso==SPOSTA_DX)//fatto il 17/03
		{
			sposta_dx();
		}
		else
		{
			//esegui_movimento(AZZERA_VAR_RACCOLTA);
			distanza_rasp=VAL_INIZIALE;
			angolo_rasp=VAL_INIZIALE;
			esegui_movimento_imposta_ritorno();
		}
	}
	pos_gl--;
	f_id[pos_gl]++;
}
void sposta_if(uint8_t verso,uint8_t flag,uint8_t val)
{
	pos_gl++;
	if(pos_gl==1 && f_step[0]==f_id[0])
	{
		if(angolo_rasp!=VAL_INIZIALE && angolo_rasp>=-17 && angolo_rasp<=17 && verso==SPOSTA_SX)//fatto il 16/03
		{
			sposta_sx();
		}
		else if(angolo_rasp!=VAL_INIZIALE && angolo_rasp>=-17 && angolo_rasp<=17 && verso==SPOSTA_DX)//fatto il 17/03
		{
			sposta_dx();
		}
		else
		{
			//esegui_movimento(AZZERA_VAR_RACCOLTA);
			distanza_rasp=VAL_INIZIALE;
			angolo_rasp=VAL_INIZIALE;
			esegui_movimento_imposta_ritorno();
		}
	}
	else
	{
		esegui_movimento_imposta_ritorno();
		f_id[pos_gl]=0;
	}
	pos_gl--;
	f_id[pos_gl]++;
}

void analizza(uint8_t f_verso,uint8_t filtro_angolo)
{
	f_verso_gl=f_verso;
	esegui_movimento_if(IMP_FILTRO_ANGOLO,2,filtro_angolo,1);
	esegui_movimento_if(DIS_FILTRO_ANGOLO,2,filtro_angolo,0);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento_imposta_step(f_step[0],f_verso,0,0);
	
	//SX2DX
	esegui_movimento_imposta_id(f_id[0],10,0,0);
	esegui_movimento(ROT_45_DX);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento(ROT_45_DX);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento(ROT_45_DX);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento(ROT_45_DX);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	//esegui_movimento(ROT_90_SX);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno();
	
	//DX2SX
	esegui_movimento_imposta_id(f_id[0],100,0,0);
	esegui_movimento(ROT_45_SX);
	esegui_movimento(DELAY+200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento(ROT_45_SX);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento(ROT_45_SX);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento(ROT_45_SX);
	esegui_movimento(DELAY+	200);
	esegui_movimento(CONTA_SCANDAGLIATE);
	esegui_movimento_if(CONTROLLA_PALLINA_ARGENTO,2,f_cerca_argento,1);
	esegui_movimento_if(CONTROLLA_PALLINA_NERA,2,f_cerca_nere,1);
	raccogli();
	esegui_movimento(AZZERA_CONTATTO_MURO);
	esegui_movimento_imposta_ritorno_if(2,f_scandaglia,0);
	esegui_movimento(DISATTIVA_SCANDAGLIA);
	esegui_movimento_imposta_ritorno();
}
void scandaglia(uint8_t f_verso,uint8_t filtro_angolo)
{
	pos_gl++;
	if(pos_gl==1 && f_step[0]==f_id[0])
	{
		if(f_scandaglia)
		{
			analizza(f_verso,filtro_angolo);
		}
		else
			esegui_movimento_imposta_ritorno();
	}
	pos_gl--;
	f_id[pos_gl]++;
}
void scandaglia_if(uint8_t f_verso,uint8_t flag,uint8_t val,uint8_t filtro_angolo)
{
	pos_gl++;
	if(pos_gl==1 && f_step[0]==f_id[0] && flag==val)
	{
		if(f_scandaglia)
		{
			analizza(f_verso,filtro_angolo);
		}
		else
		esegui_movimento_imposta_ritorno();
	}
	else
	{
		esegui_movimento_imposta_ritorno();
		f_id[pos_gl]=0;//serve perchè altrimenti sfalso la macchina
	}
	pos_gl--;
	f_id[pos_gl]++;
}

void scarica(uint8_t zona)//da verificare funzionamento dello scarica per pos_raccolta=1-3-4
{ 
	pos_gl++;
	if(pos_gl==1 && f_step[0]==f_id[0])
	{
		if((((f_num_palline_pianale>=4 || f_num_palline_argento<=0)&& f_cerca_argento==1)||((f_num_palline_pianale>=4 || f_num_palline_nere<=0)&& f_cerca_nere==1)) && pos_raccolta==1 && zona==ZONA_BASSA)//gli if non si possono unificare perchè nonostante i movimenti siano uguali i due angoli sono in pos diverse
		{
			esegui_movimento(LED+B);
			esegui_movimento_if(ROT_90_SX,2,f_muro_alle_spalle,0);
			esegui_movimento_if(ROT_90_DX,2,f_muro_alle_spalle,1);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(STOP);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-10);
			esegui_movimento(ROT_45_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+150);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_APRI);
			esegui_movimento(DELAY+300);
			esegui_movimento(VIBRA);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_CHIUDI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+60);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-100);
			esegui_movimento(ROT_45_DX);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-140);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(RILASCIO_MOTORI);
			esegui_movimento(LED+OFF_LED);
			esegui_movimento(AZZERA_VAR_PALLINE);
			esegui_movimento(SALVA_SU_EEPROM);
			esegui_movimento_imposta_ritorno();
		}
		else if((((f_num_palline_pianale>=4 || f_num_palline_argento<=0)&& f_cerca_argento==1)||((f_num_palline_pianale>=4 || f_num_palline_nere<=0)&& f_cerca_nere==1)) && pos_raccolta==2 && zona==ZONA_ALTA)//2=4
		{
			esegui_movimento(LED+B);
			esegui_movimento_if(ROT_90_DX,2,f_muro_alle_spalle,0);
			esegui_movimento_if(ROT_90_SX,2,f_muro_alle_spalle,1);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(STOP);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-10);
			esegui_movimento(ROT_45_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+150);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_APRI);
			esegui_movimento(DELAY+300);
			esegui_movimento(VIBRA);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_CHIUDI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+60);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-100);
			esegui_movimento(ROT_45_SX);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-140);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(RILASCIO_MOTORI);
			esegui_movimento(LED+OFF_LED);
			esegui_movimento(AZZERA_VAR_PALLINE);
			esegui_movimento(SALVA_SU_EEPROM);
			esegui_movimento_imposta_ritorno();
		}
		else if((((f_num_palline_pianale>=4 || f_num_palline_argento<=0)&& f_cerca_argento==1)||((f_num_palline_pianale>=4 || f_num_palline_nere<=0)&& f_cerca_nere==1)) && pos_raccolta==3 && zona==ZONA_ALTA)//1==3
		{
			esegui_movimento(LED+B);
			esegui_movimento_if(ROT_90_SX,2,f_muro_alle_spalle,0);
			esegui_movimento_if(ROT_90_DX,2,f_muro_alle_spalle,1);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(STOP);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-10);
			esegui_movimento(ROT_45_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+150);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_APRI);
			esegui_movimento(DELAY+300);
			esegui_movimento(VIBRA);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_CHIUDI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+60);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-100);
			esegui_movimento(ROT_45_DX);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-140);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(RILASCIO_MOTORI);
			esegui_movimento(LED+OFF_LED);
			esegui_movimento(AZZERA_VAR_PALLINE);
			esegui_movimento(SALVA_SU_EEPROM);
			esegui_movimento_imposta_ritorno();
		}
		else if((((f_num_palline_pianale>=4 || f_num_palline_argento<=0)&& f_cerca_argento==1)||((f_num_palline_pianale>=4 || f_num_palline_nere<=0)&& f_cerca_nere==1)) && pos_raccolta==4 && zona==ZONA_BASSA)//4=2
		{
			esegui_movimento(LED+B);
			esegui_movimento_if(ROT_90_DX,2,f_muro_alle_spalle,0);
			esegui_movimento_if(ROT_90_SX,2,f_muro_alle_spalle,1);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(STOP);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-10);
			esegui_movimento(ROT_45_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+150);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_APRI);
			esegui_movimento(DELAY+300);
			esegui_movimento(VIBRA);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_CHIUDI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+60);
			esegui_movimento(STOP);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-100);
			esegui_movimento(ROT_45_SX);
			esegui_movimento(CONTATTO_FIN_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-140);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(RILASCIO_MOTORI);
			esegui_movimento(LED+OFF_LED);
			esegui_movimento(AZZERA_VAR_PALLINE);
			esegui_movimento(SALVA_SU_EEPROM);
			esegui_movimento_imposta_ritorno();
		}
		else
			esegui_movimento_imposta_ritorno();
	}
	pos_gl--;
	f_id[pos_gl]++;
}
void scarica_in_pulizia()
{
	pos_gl++;
	if(pos_gl==2 && f_step[0]==f_id[0] && f_step[1]==f_id[1])
	{
		if( ((f_num_palline_pianale>=4 || f_num_palline_argento<=0) && f_cerca_argento==1) || ((f_num_palline_pianale>=4 || f_num_palline_nere<=0) && f_cerca_nere==1) )
		{
			esegui_movimento(LED+B);
			esegui_movimento(PINZA+PIANALE_APRI);
			esegui_movimento(DELAY+300);
			esegui_movimento(VIBRA);
			esegui_movimento(CONTATTO_FIN_DIETRO);
			esegui_movimento(PINZA+PIANALE_CHIUDI);
			esegui_movimento(LED+OFF_LED);
			esegui_movimento(AZZERA_VAR_PALLINE);
			esegui_movimento(SALVA_SU_EEPROM);
			esegui_movimento_imposta_ritorno();
		}
		else
			esegui_movimento_imposta_ritorno();
	}
	pos_gl--;
	f_id[pos_gl]++;
}

void pulisci_angolo(uint8_t direzione,uint8_t val_pos_raccolta)
{
	pos_gl++;
	if(pos_gl==1 && f_step[0]==f_id[0])
	{
		if(pos_raccolta == val_pos_raccolta)
		{
			if(direzione==DX)
			{
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+15);
				esegui_movimento(STOP);
				esegui_movimento(ROT_45_DX);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+250);
				esegui_movimento(STOP);
				esegui_movimento(ANGOLO_VARIABILE+30);
				esegui_movimento(ANGOLO_VARIABILE-30);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA-130);
				esegui_movimento(STOP_INDIETRO);
				esegui_movimento(ROT_90_DX);
				esegui_movimento(ALLINEAMENTO_FINECORSA);
				esegui_movimento(STOP);
				scarica_in_pulizia();
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+35);
				esegui_movimento(STOP);
				esegui_movimento(ROT_90_DX);
				esegui_movimento(CONTATTO_FIN_DAVANTI);
				esegui_movimento(ROT_45_SX);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+80);
				esegui_movimento(STOP);
				esegui_movimento_imposta_ritorno();
			}
			else if(direzione==SX)
			{
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+15);
				esegui_movimento(STOP);
				esegui_movimento(ROT_45_SX);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+250);
				esegui_movimento(STOP);
				esegui_movimento(ANGOLO_VARIABILE-30);
				esegui_movimento(ANGOLO_VARIABILE+30);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA-130);
				esegui_movimento(STOP_INDIETRO);
				esegui_movimento(ROT_90_SX);
				esegui_movimento(ALLINEAMENTO_FINECORSA);
				esegui_movimento(STOP);
				scarica_in_pulizia();
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+35);
				esegui_movimento(STOP);
				esegui_movimento(ROT_90_SX);
				esegui_movimento(CONTATTO_FIN_DAVANTI);
				esegui_movimento(ROT_45_DX);
				esegui_movimento(POS_ENCODER);
				esegui_movimento(AVANTI_MISURA+80);
				esegui_movimento(STOP);
				esegui_movimento_imposta_ritorno();
			}
		}
		else
			esegui_movimento_imposta_ritorno();
	}
	pos_gl--;
	f_id[pos_gl]++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////STANZE///////////////////////////////////////////////////////
void programma_movimento_e_corto_sx()
{
	f_id[0]=0;
	f_id[1]=0;
	f_id[2]=0;
	f_id[3]=0;
	if(f_fine_prima_parte==0)
	{
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+220);
		esegui_movimento(ROT_90_DX);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(RILASCIO_MOTORI);
		esegui_movimento(DELAY+5);//fatto per lasciare un ciclo macchina a rilascio motori
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+30);
		esegui_movimento(STOP);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(20,0,0,0);
		esegui_movimento_imposta_id(20,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(RILASCIO_MOTORI);
		esegui_movimento(DELAY+5);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+370);

		esegui_movimento(POS_ENCODER);
		esegui_movimento(CONTROLLA_ANGOLO+1);
		esegui_movimento_if(STOP_INDIETRO,2,pos_raccolta,0);
		esegui_movimento_if(STOP,2,pos_raccolta,1);
		
		pulisci_angolo(SX,1);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,1);
		
		esegui_movimento(DISATTIVA_CONTROLLO_ANGOLO);

		scandaglia_if(DX2SX,pos_raccolta,0,1);
		scandaglia_if(SX2DX,pos_raccolta,1,1);//aggiunto questo per puli angolo
		
		esegui_movimento(DELAY+150);
		
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,5,pos_raccolta,1);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,4,pos_raccolta,1);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,2,pos_raccolta,1);
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,1,pos_raccolta,1);
		
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,5,pos_raccolta,0);//aggiunto questo per puli angolo
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,4,pos_raccolta,0);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,2,pos_raccolta,0);
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,1,pos_raccolta,0);//aggiunto questo per puli angolo
		
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		//////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(100,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(100,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(AVANTI_MISURA+200);//
		esegui_movimento(STOP);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(200,0,0,0);
		esegui_movimento_imposta_id(200,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(200,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(200,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(300,0,0,0);
		esegui_movimento_imposta_id(300,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+150);
		esegui_movimento(DISATTIVA_SCANDAGLIA);
		esegui_movimento_if(STOP,2,f_scandaglia,1);
		esegui_movimento_if(DELAY+300,2,f_scandaglia,1);
		esegui_movimento_if(ROT_90_SX,2,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		///////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(400,0,0,0);
		esegui_movimento_imposta_id(400,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-30);
		esegui_movimento_if(ROT_180,2,f_controlla_angolo,0);
		esegui_movimento_if(ROT_90_DX,2,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,2,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+2,2,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,2,f_controlla_angolo,1);
		
		pulisci_angolo(DX,2);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,2);
		esegui_movimento_if(ROT_180,2,pos_raccolta,0);
		esegui_movimento_if(CONTATTO_FIN_DAVANTI,2,pos_raccolta,0);
		esegui_movimento_if(ASSEGNA_ANGOLO+3,4,f_controlla_angolo,1,pos_raccolta,0);
		
		pulisci_angolo(SX,3);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,3);
		//esegui_movimento_if(ROT_90_SX,4,f_controlla_angolo,1,pos_raccolta,2);
		//esegui_movimento_if(ROT_90_DX,4,f_controlla_angolo,1,pos_raccolta,3);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,2,f_controlla_angolo,1);
		
		esegui_movimento(FINE_PRIMA_PARTE);
	}
	else if(f_fine_prima_parte==1)
	{
		esegui_movimento_if(MURO_ALLE_SPALLE,4,f_primo_180,1,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,4,f_primo_180,1,pos_raccolta,3);
		esegui_movimento_if(S_AVANZAMENTO,2,f_cerca_argento,1);
		scarica(ZONA_ALTA);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,2);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,3);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,2);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,3);
		esegui_movimento_if(ROT_180,4,f_scaricato,0,f_primo_180,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,2,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,4,f_scandaglia,1,f_primo_180,0);
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,1,f_primo_180,0);
		esegui_movimento_if(AVANTI_MISURA+50,4,f_scandaglia,1,f_primo_180,0);
		esegui_movimento_if(ROT_90_DX,6,f_scandaglia,1,pos_raccolta,2,f_primo_180,0);
		esegui_movimento_if(ROT_90_DX,4,f_scandaglia,1,pos_raccolta,1);
		esegui_movimento_if(ROT_90_SX,6,f_scandaglia,1,pos_raccolta,3,f_primo_180,0);
		esegui_movimento(AZZERA_PRIMO_180);
		
		scandaglia_if(DX2SX,pos_raccolta,2,1);
		scandaglia_if(DX2SX,pos_raccolta,1,1);
		scandaglia_if(SX2DX,pos_raccolta,3,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,5,pos_raccolta,2);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,4,pos_raccolta,2);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,2,pos_raccolta,2);
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,1,pos_raccolta,2);
		
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,5,pos_raccolta,1);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,4,pos_raccolta,1);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,2,pos_raccolta,1);
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,1,pos_raccolta,1);
		
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,5,pos_raccolta,3);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,4,pos_raccolta,3);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,2,pos_raccolta,3);
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,1,pos_raccolta,3);
		
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,3);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_imposta_step_if(0,0,0,0,4,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_imposta_step_if(0,0,0,0,4,f_scandaglia,0,pos_raccolta,3);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,2,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,2,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,2,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		////////////////////////////////////////
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento_if(S_AVANZAMENTO,2,f_cerca_argento,1);
		scarica(ZONA_BASSA);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,1);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,4);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,1);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,4);
		esegui_movimento_if(ROT_180,2,f_scaricato,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,2,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,2,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,2,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,2,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,2,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,1);
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,4);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_imposta_step_if(100,0,0,0,4,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_imposta_step_if(100,0,0,0,4,f_scandaglia,0,pos_raccolta,4);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,2,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,2,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,2,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		esegui_movimento_imposta_step(0,0,0,0);//lo faccio ripartire da capo creando un while nella macchina a stati
	}
	else if(f_fine_prima_parte==2)
	{
		if(pos_raccolta==1 || pos_raccolta==4)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+60);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-50);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+230);//terza casella dopo la stanza
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
		else if(pos_raccolta==2 || pos_raccolta==3)
		{
			f_scandaglia=0;
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+650);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-60);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-50);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+230);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
	}
}
void programma_movimento_e_corto_dx()
{
	f_id[0]=0;
	f_id[1]=0;
	f_id[2]=0;
	f_id[3]=0;
	if(f_fine_prima_parte==0)
	{
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+220);//era 250
		esegui_movimento(ROT_90_SX);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(RILASCIO_MOTORI);
		esegui_movimento(DELAY+5);//fatto per lasciare un ciclo macchina a rilascio motori
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+30);
		esegui_movimento(STOP);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(20,0,0,0);
		esegui_movimento_imposta_id(20,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(20,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(20,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(RILASCIO_MOTORI);
		esegui_movimento(DELAY+5);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+370);

		esegui_movimento(POS_ENCODER);
		esegui_movimento(CONTROLLA_ANGOLO+4);//più 4 perchè questo è l'angolo 4 e non l'1
		esegui_movimento_if(STOP_INDIETRO,2,pos_raccolta,0);
		esegui_movimento_if(STOP,2,pos_raccolta,4);
		
		pulisci_angolo(DX,4);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,4);
		
		esegui_movimento(DISATTIVA_CONTROLLO_ANGOLO);

		scandaglia_if(DX2SX,pos_raccolta,4,1);
		scandaglia_if(SX2DX,pos_raccolta,0,1);//aggiunto questo per puli angolo
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,5,pos_raccolta,4);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,4,pos_raccolta,4);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,2,pos_raccolta,4);
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,1,pos_raccolta,4);
		
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,5,pos_raccolta,0);//aggiunto questo per puli angolo
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,4,pos_raccolta,0);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,2,pos_raccolta,0);
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,1,pos_raccolta,0);//aggiunto questo per puli angolo
		
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(100,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(100,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(AVANTI_MISURA+200);//
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(200,0,0,0);
		esegui_movimento_imposta_id(200,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(200,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(200,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(300,0,0,0);
		esegui_movimento_imposta_id(300,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(300,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(300,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+150);
		esegui_movimento(DISATTIVA_SCANDAGLIA);
		esegui_movimento_if(STOP,2,f_scandaglia,1);
		esegui_movimento_if(DELAY+300,2,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,2,f_scandaglia,1);
		scandaglia(DX2SX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(400,0,0,0);
		esegui_movimento_imposta_id(400,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(400,0,0,0,2,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(400,0,0,0,2,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-30);//era 60
		esegui_movimento_if(ROT_180,2,f_controlla_angolo,0);
		esegui_movimento_if(ROT_90_DX,2,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,2,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+2,2,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,2,f_controlla_angolo,1);
		
		pulisci_angolo(DX,2);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,2);
		esegui_movimento_if(ROT_180,2,pos_raccolta,0);
		esegui_movimento_if(CONTATTO_FIN_DAVANTI,2,pos_raccolta,0);
		esegui_movimento_if(ASSEGNA_ANGOLO+3,4,f_controlla_angolo,1,pos_raccolta,0);
		
		pulisci_angolo(SX,3);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,3);
		//esegui_movimento_if(ROT_90_SX,4,f_controlla_angolo,1,pos_raccolta,2);
		//esegui_movimento_if(ROT_90_DX,4,f_controlla_angolo,1,pos_raccolta,3);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,2,f_controlla_angolo,1);
		
		esegui_movimento(FINE_PRIMA_PARTE);
	}
	else if(f_fine_prima_parte==1)
	{
		esegui_movimento_if(MURO_ALLE_SPALLE,4,f_primo_180,1,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,4,f_primo_180,1,pos_raccolta,3);
		esegui_movimento_if(S_AVANZAMENTO,2,f_cerca_argento,1);
		scarica(ZONA_ALTA);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,2);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,3);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,2);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,3);
		esegui_movimento_if(ROT_180,4,f_scaricato,0,f_primo_180,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,2,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,4,f_scandaglia,1,f_primo_180,0);
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,1,f_primo_180,0);
		esegui_movimento_if(AVANTI_MISURA+50,4,f_scandaglia,1,f_primo_180,0);
		esegui_movimento_if(ROT_90_DX,6,f_scandaglia,1,pos_raccolta,2,f_primo_180,0);
		esegui_movimento_if(ROT_90_DX,4,f_scandaglia,1,pos_raccolta,4);
		esegui_movimento_if(ROT_90_SX,6,f_scandaglia,1,pos_raccolta,3,f_primo_180,0);
		esegui_movimento(AZZERA_PRIMO_180);
		
		scandaglia_if(DX2SX,pos_raccolta,2,1);
		scandaglia_if(DX2SX,pos_raccolta,4,1);
		scandaglia_if(SX2DX,pos_raccolta,3,1);
		esegui_movimento(DELAY+150);
		
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,5,pos_raccolta,2);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,4,pos_raccolta,2);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,2,pos_raccolta,2);
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,1,pos_raccolta,2);
		
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,5,pos_raccolta,4);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,4,pos_raccolta,4);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,2,pos_raccolta,4);
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,1,pos_raccolta,4);
		
		esegui_movimento_if(ROT_90_SX,4,f_num_scandagliate,5,pos_raccolta,3);
		esegui_movimento_if(ROT_45_SX,4,f_num_scandagliate,4,pos_raccolta,3);
		esegui_movimento_if(ROT_45_DX,4,f_num_scandagliate,2,pos_raccolta,3);
		esegui_movimento_if(ROT_90_DX,4,f_num_scandagliate,1,pos_raccolta,3);
		
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,3);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_imposta_step_if(0,0,0,0,4,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_imposta_step_if(0,0,0,0,4,f_scandaglia,0,pos_raccolta,3);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,2,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,2,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,2,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		////////////////////////////////////////
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento_if(S_AVANZAMENTO,2,f_cerca_argento,1);
		scarica(ZONA_BASSA);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,1);
		esegui_movimento_if(FINE_PRIMA_PARTE,8,f_num_palline_argento,0,f_num_palline_nere,0,f_num_palline_pianale,0,pos_raccolta,4);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,1);
		esegui_movimento_if(CAMBIA_RICERCA,2,pos_raccolta,4);
		esegui_movimento_if(ROT_180,2,f_scaricato,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,2,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,2,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,2,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,2,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,2,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,1);
		esegui_movimento_if(MURO_ALLE_SPALLE,2,pos_raccolta,4);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_imposta_step_if(100,0,0,0,4,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if(POS_ENCODER,4,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if(AVANTI_MISURA+60,4,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_imposta_step_if(100,0,0,0,4,f_scandaglia,0,pos_raccolta,4);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,2,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,2,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,2,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,2,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,2,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,2,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,2,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		esegui_movimento_imposta_step(0,0,0,0);
	}
	else if(f_fine_prima_parte==2)
	{
		if(pos_raccolta==1 || pos_raccolta==4)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+60);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-50);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+230);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
		else
		{
			f_scandaglia=0;
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+650);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-50);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-60);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+230);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
	}
}
void programma_movimento_e_lungo_sx()
{
	f_id[0]=0;
	f_id[1]=0;
	f_id[2]=0;
	f_id[3]=0;
	if(f_fine_prima_parte==0)
	{
		esegui_movimento(POS_ENCODER);//parte iniziale una volta sentito argento avanzo di 25cm
		esegui_movimento(AVANTI_MISURA+250);
		esegui_movimento(STOP);
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(20,0,0,0);
		esegui_movimento_imposta_id(20,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+330);//da modificare una volta che è stata fatta la modifica dei contatti striscianti

		esegui_movimento(POS_ENCODER);
		esegui_movimento(CONTROLLA_ANGOLO+4);//più 4 perchè questo è l'angolo 4 e non l'1
		esegui_movimento(STOP_INDIETRO);
		esegui_movimento(DISATTIVA_CONTROLLO_ANGOLO);

		scandaglia(SX2DX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(AVANTI_MISURA+200);//
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(200,0,0,0);
		esegui_movimento_imposta_id(200,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(300,0,0,0);
		esegui_movimento_imposta_id(300,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+150);
		esegui_movimento(DISATTIVA_SCANDAGLIA);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+300,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(400,0,0,0);
		esegui_movimento_imposta_id(400,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_DX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-60);
		esegui_movimento_if(ROT_180,f_controlla_angolo,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+2,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if_if(ASSEGNA_ANGOLO+3,f_controlla_angolo,1,pos_raccolta,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		esegui_movimento(FINE_PRIMA_PARTE);
	}
	else if(f_fine_prima_parte==1)
	{
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,2);
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,3);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_ALTA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,2);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,3);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,2);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,3);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,2);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,3);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,2);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,3);
		esegui_movimento_if_if(ROT_180,f_scaricato,0,f_primo_180,0);
		esegui_movimento(AZZERA_PRIMO_180);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,3);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,3);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		////////////////////////////////////////
		esegui_movimento_imposta_step(50,0,0,0);
		esegui_movimento_imposta_id(50,0,0,0);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_BASSA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,1);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,4);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,1);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,4);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,1);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,4);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,1);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,4);
		esegui_movimento_if(ROT_180,f_scaricato,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,1);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,4);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,4);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		esegui_movimento_imposta_step(0,0,0,0);
	}
	else if(f_fine_prima_parte==2)
	{
		if(pos_raccolta==1 || pos_raccolta==4)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+50);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
		else
		{
			f_scandaglia=0;
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+650);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-50);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
	}
}
void programma_movimento_e_lungo_dx()
{
	f_id[0]=0;
	f_id[1]=0;
	f_id[2]=0;
	f_id[3]=0;
	if(f_fine_prima_parte==0)
	{
		esegui_movimento(POS_ENCODER);//parte iniziale una volta sentito argento avanzo di 25cm
		esegui_movimento(AVANTI_MISURA+250);
		esegui_movimento(STOP);
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(20,0,0,0);
		esegui_movimento_imposta_id(20,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+330);

		esegui_movimento(POS_ENCODER);
		esegui_movimento(CONTROLLA_ANGOLO+1);
		esegui_movimento(STOP_INDIETRO);
		esegui_movimento(DISATTIVA_CONTROLLO_ANGOLO);

		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		//////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(AVANTI_MISURA+200);//
		esegui_movimento(STOP);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(200,0,0,0);
		esegui_movimento_imposta_id(200,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(300,0,0,0);
		esegui_movimento_imposta_id(300,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+150);
		esegui_movimento(DISATTIVA_SCANDAGLIA);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+300,f_scandaglia,1);
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		///////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(400,0,0,0);
		esegui_movimento_imposta_id(400,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-60);
		esegui_movimento_if(ROT_180,f_controlla_angolo,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+2,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if_if(ASSEGNA_ANGOLO+3,f_controlla_angolo,1,pos_raccolta,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		esegui_movimento(FINE_PRIMA_PARTE);
	}
	else if(f_fine_prima_parte==1)
	{
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,2);
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,3);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_ALTA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,2);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,3);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,2);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,3);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,2);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,3);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,2);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,3);
		esegui_movimento_if_if(ROT_180,f_scaricato,0,f_primo_180,0);
		esegui_movimento(AZZERA_PRIMO_180);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,3);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,3);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		////////////////////////////////////////
		esegui_movimento_imposta_step(50,0,0,0);
		esegui_movimento_imposta_id(50,0,0,0);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_BASSA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,1);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,4);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,1);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,4);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,1);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,4);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,1);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,4);
		esegui_movimento_if(ROT_180,f_scaricato,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,1);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,4);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,4);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		esegui_movimento_imposta_step(0,0,0,0);//lo faccio ripartire da capo creando un while nella macchina a stati
	}
	else if(f_fine_prima_parte==2)
	{
		if(pos_raccolta==1 || pos_raccolta==4)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+50);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
		else
		{
			f_scandaglia=0;
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+650);
			esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA-50);
			esegui_movimento(STOP_INDIETRO);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
	}
}
void programma_movimento_e_lungo_cx_verso_dx()
{
	f_id[0]=0;
	f_id[1]=0;
	f_id[2]=0;
	f_id[3]=0;
	if(f_fine_prima_parte==0)
	{
		esegui_movimento(POS_ENCODER);//parte iniziale una volta sentito argento avanzo di 25cm
		esegui_movimento(AVANTI_MISURA+250);
		esegui_movimento(STOP);
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(20,0,0,0);
		esegui_movimento_imposta_id(20,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+300);
		esegui_movimento(STOP);
		esegui_movimento(ROT_90_DX);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(50,0,0,0);
		esegui_movimento_imposta_id(50,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(50,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(50,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-60);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+2,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		scandaglia_if(SX2DX,f_controlla_angolo,0,1);
		esegui_movimento_if(DELAY+150,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_90_SX,f_num_scandagliate,5,f_controlla_angolo,0);//andrebbe doppio if ma non serve perchè f_num_scandagliate è inizializzato a 0
		esegui_movimento_if_if(ROT_45_SX,f_num_scandagliate,4,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_45_DX,f_num_scandagliate,2,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_90_DX,f_num_scandagliate,1,f_controlla_angolo,0);
		esegui_movimento_if(AZZERA_SCANDAGLIATE,f_controlla_angolo,0);
		
		esegui_movimento_if(ROT_180,f_controlla_angolo,1);
		
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+3,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		
		scandaglia_if(DX2SX,f_controlla_angolo,1,1);
		
		esegui_movimento_if(DELAY+150,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_90_DX,f_num_scandagliate,5,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_45_DX,f_num_scandagliate,4,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_45_SX,f_num_scandagliate,2,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_90_SX,f_num_scandagliate,1,f_controlla_angolo,1);
		esegui_movimento_if(AZZERA_SCANDAGLIATE,f_controlla_angolo,1);
		
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		//////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(200,0,0,0);
		esegui_movimento_imposta_id(200,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(300,0,0,0);
		esegui_movimento_imposta_id(300,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+150);
		esegui_movimento(DISATTIVA_SCANDAGLIA);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+300,f_scandaglia,1);
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		///////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(400,0,0,0);
		esegui_movimento_imposta_id(400,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-60);
		esegui_movimento_if(ROT_180,f_controlla_angolo,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+4,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if_if(ASSEGNA_ANGOLO+1,f_controlla_angolo,1,pos_raccolta,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		esegui_movimento(FINE_PRIMA_PARTE);
	}
	else if(f_fine_prima_parte==1)
	{
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,1);
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,4);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_BASSA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,2);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,3);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,1);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,4);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,1);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,4);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,1);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,4);
		esegui_movimento_if_if(ROT_180,f_scaricato,0,f_primo_180,0);
		esegui_movimento(AZZERA_PRIMO_180);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,1);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,4);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,4);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		////////////////////////////////////////
		esegui_movimento_imposta_step(50,0,0,0);
		esegui_movimento_imposta_id(50,0,0,0);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_ALTA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,1);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,4);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,2);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,3);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,2);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,3);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,2);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,3);
		esegui_movimento_if(ROT_180,f_scaricato,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,3);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,3);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		esegui_movimento_imposta_step(0,0,0,0);//lo faccio ripartire da capo creando un while nella macchina a stati
	}
	else if(f_fine_prima_parte==2)
	{
		if(pos_raccolta==1 || pos_raccolta==4)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+650);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
		else if(pos_raccolta==2 || pos_raccolta==3)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+350);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
	}
}
void programma_movimento_e_lungo_cx_verso_sx()
{
	f_id[0]=0;
	f_id[1]=0;
	f_id[2]=0;
	f_id[3]=0;
	if(f_fine_prima_parte==0)
	{
		esegui_movimento(POS_ENCODER);//parte iniziale una volta sentito argento avanzo di 25cm
		esegui_movimento(AVANTI_MISURA+250);
		esegui_movimento(STOP);
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(20,0,0,0);
		esegui_movimento_imposta_id(20,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(20,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+300);
		esegui_movimento(STOP);
		esegui_movimento(ROT_90_SX);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(50,0,0,0);
		esegui_movimento_imposta_id(50,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(50,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(50,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-60);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+2,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		scandaglia_if(SX2DX,f_controlla_angolo,0,1);
		esegui_movimento_if(DELAY+150,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_90_SX,f_num_scandagliate,5,f_controlla_angolo,0);//andrebbe doppio if ma non serve perchè f_num_scandagliate è inizializzato a 0
		esegui_movimento_if_if(ROT_45_SX,f_num_scandagliate,4,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_45_DX,f_num_scandagliate,2,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_90_DX,f_num_scandagliate,1,f_controlla_angolo,0);
		esegui_movimento_if(AZZERA_SCANDAGLIATE,f_controlla_angolo,0);
		
		esegui_movimento_if(ROT_180,f_controlla_angolo,1);
		
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+3,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		
		scandaglia_if(DX2SX,f_controlla_angolo,1,1);
		
		esegui_movimento_if(DELAY+150,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_90_DX,f_num_scandagliate,5,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_45_DX,f_num_scandagliate,4,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_45_SX,f_num_scandagliate,2,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_90_SX,f_num_scandagliate,1,f_controlla_angolo,1);
		esegui_movimento_if(AZZERA_SCANDAGLIATE,f_controlla_angolo,1);
		
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		//////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(200,0,0,0);
		esegui_movimento_imposta_id(200,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(300,0,0,0);
		esegui_movimento_imposta_id(300,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+150);
		esegui_movimento(DISATTIVA_SCANDAGLIA);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+300,f_scandaglia,1);
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		///////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(400,0,0,0);
		esegui_movimento_imposta_id(400,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-60);
		esegui_movimento_if(ROT_180,f_controlla_angolo,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+4,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if_if(ASSEGNA_ANGOLO+1,f_controlla_angolo,1,pos_raccolta,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		esegui_movimento(FINE_PRIMA_PARTE);
	}
	else if(f_fine_prima_parte==1)
	{
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,1);
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,4);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_BASSA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,2);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,3);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,1);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,4);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,1);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,4);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,1);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,4);
		esegui_movimento_if_if(ROT_180,f_scaricato,0,f_primo_180,0);
		esegui_movimento(AZZERA_PRIMO_180);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,1);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,4);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,4);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		////////////////////////////////////////
		esegui_movimento_imposta_step(50,0,0,0);
		esegui_movimento_imposta_id(50,0,0,0);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_ALTA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,1);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,4);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,2);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,3);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,2);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,3);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,2);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,3);
		esegui_movimento_if(ROT_180,f_scaricato,0);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,3);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_imposta_step_if_if(50,0,0,0,f_scandaglia,0,pos_raccolta,3);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		esegui_movimento_imposta_step(0,0,0,0);
	}
	else if(f_fine_prima_parte==2)
	{
		if(pos_raccolta==1 || pos_raccolta==4)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+650);
			esegui_movimento(ROT_90_SX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
		else if(pos_raccolta==2 || pos_raccolta==3)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+350);
			esegui_movimento(ROT_90_DX);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+530);
			esegui_movimento(STOP);
			esegui_movimento(RILASCIO_MOTORI);
		}
	}
}
void programma_movimento_e_corto_cx()
{
	f_id[0]=0;
	f_id[1]=0;
	f_id[2]=0;
	f_id[3]=0;
	if(f_fine_prima_parte==0)
	{
		esegui_movimento(POS_ENCODER);//parte iniziale una volta sentito argento avanzo di 25cm
		esegui_movimento(AVANTI_MISURA+250);
		esegui_movimento(STOP);
		
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+1,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		scandaglia_if(DX2SX,f_controlla_angolo,0,1);
		esegui_movimento_if(DELAY+150,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_90_DX,f_num_scandagliate,5,f_controlla_angolo,0);//andrebbe doppio if ma non serve perchè f_num_scandagliate è inizializzato a 0
		esegui_movimento_if_if(ROT_45_DX,f_num_scandagliate,4,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_45_SX,f_num_scandagliate,2,f_controlla_angolo,0);
		esegui_movimento_if_if(ROT_90_SX,f_num_scandagliate,1,f_controlla_angolo,0);
		esegui_movimento_if(AZZERA_SCANDAGLIATE,f_controlla_angolo,0);
		esegui_movimento_if(DELAY+150,f_controlla_angolo,0);
		
		esegui_movimento_if(ROT_90_SX,f_controlla_angolo,1);
		esegui_movimento_if(ROT_90_SX,f_controlla_angolo,1);
		
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+4,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		
		scandaglia_if(SX2DX,f_controlla_angolo,1,1);
		
		esegui_movimento_if(DELAY+150,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_90_SX,f_num_scandagliate,5,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_45_SX,f_num_scandagliate,4,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_45_DX,f_num_scandagliate,2,f_controlla_angolo,1);
		esegui_movimento_if_if(ROT_90_DX,f_num_scandagliate,1,f_controlla_angolo,1);
		esegui_movimento_if(AZZERA_SCANDAGLIATE,f_controlla_angolo,1);
		esegui_movimento_if(DELAY+150,f_controlla_angolo,1);
		
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		//////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(100,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(200,0,0,0);
		esegui_movimento_imposta_id(200,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta_if(SPOSTA_DX,pos_raccolta,2);
		sposta_if(SPOSTA_SX,pos_raccolta,3);
		sposta_if(SPOSTA_DX,pos_raccolta,0);
		esegui_movimento_imposta_step_if(200,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+200);
		esegui_movimento(STOP);
		
		/////////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(300,0,0,0);
		esegui_movimento_imposta_id(300,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(300,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA+150);
		esegui_movimento(DISATTIVA_SCANDAGLIA);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+300,f_scandaglia,1);
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		esegui_movimento(DELAY+150);
		
		///////////////Palline in mezzo
		esegui_movimento(INIZIO_SPOSTAMENTO_PALLINE);
		esegui_movimento_imposta_step(400,0,0,0);
		esegui_movimento_imposta_id(400,0,0,0);
		esegui_movimento(AZZERA_VAR_RACCOLTA);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_NERA);
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(DELAY+200);
		esegui_movimento(CONTROLLA_PALLINA_ARGENTO);
		raccogli();
		sposta(SPOSTA_SX);
		esegui_movimento_imposta_step_if(400,0,0,0,f_raccolta,1);
		esegui_movimento(FINE_SPOSTAMENTO_PALLINE);
		/////////////////
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-60);
		esegui_movimento_if(ROT_180,f_controlla_angolo,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(POS_ENCODER,f_controlla_angolo,1);
		esegui_movimento_if(CONTROLLA_ANGOLO+2,f_controlla_angolo,1);
		esegui_movimento_if(STOP_INDIETRO,f_controlla_angolo,1);
		esegui_movimento_if_if(ASSEGNA_ANGOLO+3,f_controlla_angolo,1,pos_raccolta,0);
		esegui_movimento_if(ROT_90_DX,f_controlla_angolo,1);
		esegui_movimento_if(DISATTIVA_CONTROLLO_ANGOLO,f_controlla_angolo,1);
		
		esegui_movimento(FINE_PRIMA_PARTE);
	}
	else if(f_fine_prima_parte==1)
	{
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,2);
		esegui_movimento_if_if(MURO_ALLE_SPALLE,f_primo_180,1,pos_raccolta,3);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_ALTA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,2);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,3);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,1);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,4);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,2);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,3);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,2);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,3);
		esegui_movimento_if_if(ROT_180,f_scaricato,0,f_primo_180,0);
		esegui_movimento(AZZERA_PRIMO_180);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		esegui_movimento_if(ALLINEAMENTO_FINECORSA,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+50,f_scandaglia,1);
		esegui_movimento_if(ROT_90_DX,f_scandaglia,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		esegui_movimento(DELAY+150);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,2);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,3);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(ALLINEAMENTO_FINECORSA);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_imposta_step_if_if(0,0,0,0,f_scandaglia,0,pos_raccolta,3);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento(STOP);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		esegui_movimento(DELAY+150);
		
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(AVANTI_MISURA+250,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(STOP,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if_if(AVANTI_MISURA+250,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if_if(STOP,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_if(POS_ENCODER,f_scandaglia,1);
		esegui_movimento_if(AVANTI_MISURA+310,f_scandaglia,1);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if_if(ROT_90_DX,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(ROT_90_DX,f_scandaglia,0,pos_raccolta,2);
		esegui_movimento_if_if(ROT_90_DX,f_scandaglia,0,pos_raccolta,3);
		esegui_movimento_if_if(ROT_90_DX,f_scandaglia,0,pos_raccolta,3);
		////////////////////////////////////////
		esegui_movimento_imposta_step(100,0,0,0);
		esegui_movimento_imposta_id(100,0,0,0);
		esegui_movimento_if(S_AVANZAMENTO,f_cerca_argento,1);
		scarica(ZONA_BASSA);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,1);
		//esegui_movimento_if(AZZERA_SCANDAGLIATE,pos_raccolta,4);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,2);
		//esegui_movimento_if(TUTTE_NERE,pos_raccolta,3);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,1);
		esegui_movimento_if_if(FINE_PRIMA_PARTE,(f_num_palline_argento+f_num_palline_nere+f_num_palline_pianale),0,pos_raccolta,4);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,1);
		esegui_movimento_if(CAMBIA_RICERCA,pos_raccolta,4);
		esegui_movimento_if_if(ROT_90_SX,f_scaricato,0,f_scandaglia,1);
		esegui_movimento_if(POS_ENCODER,f_scaricato,1);
		esegui_movimento_if(AVANTI_MISURA-30,f_scaricato,1);
		esegui_movimento_if(STOP_INDIETRO,f_scaricato,1);
		esegui_movimento_if(ROT_90_DX,f_scaricato,1);
		esegui_movimento_if(AZZERA_VAR_SCARICA,f_scaricato,1);
		scandaglia(DX2SX,1);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		esegui_movimento(DELAY+150);
		
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,1);
		esegui_movimento_if(MURO_ALLE_SPALLE,pos_raccolta,4);
		esegui_movimento(POS_ENCODER);
		
// 		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,1);
// 		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,1);
		esegui_movimento_imposta_step_if_if(100,0,0,0,f_scandaglia,0,pos_raccolta,1);
// 		esegui_movimento_if_if(POS_ENCODER,f_scandaglia,0,pos_raccolta,4);
// 		esegui_movimento_if_if(AVANTI_MISURA+60,f_scandaglia,0,pos_raccolta,4);
		esegui_movimento_imposta_step_if_if(100,0,0,0,f_scandaglia,0,pos_raccolta,4);
		
		esegui_movimento(AVANTI_MISURA+550);
		esegui_movimento_if(STOP,f_scandaglia,1);
		esegui_movimento_if(DELAY+100,f_scandaglia,1);
		
		esegui_movimento_if(ROT_90_SX,f_scandaglia,1);
		scandaglia(SX2DX,0);
		
		esegui_movimento(DELAY+150);
		esegui_movimento_if(ROT_90_SX,f_num_scandagliate,5);
		esegui_movimento_if(ROT_45_SX,f_num_scandagliate,4);
		esegui_movimento_if(ROT_45_DX,f_num_scandagliate,2);
		esegui_movimento_if(ROT_90_DX,f_num_scandagliate,1);
		esegui_movimento(AZZERA_SCANDAGLIATE);
		esegui_movimento(DELAY+150);
		
		esegui_movimento(ALLINEAMENTO_FINECORSA_DAVANTI);
		esegui_movimento(POS_ENCODER);
		esegui_movimento(AVANTI_MISURA-40);
		esegui_movimento_imposta_step(0,0,0,0);
	}
	else if(f_fine_prima_parte==2)
	{
		if(pos_raccolta==1 || pos_raccolta==4)
		{
			esegui_movimento(ROT_180);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+200);
			esegui_movimento(RILASCIO_MOTORI);
		}
		else if(pos_raccolta==2 || pos_raccolta==3)
		{
			esegui_movimento(ALLINEAMENTO_FINECORSA);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+1000);
			esegui_movimento(POS_ENCODER);
			esegui_movimento(AVANTI_MISURA+250);
			esegui_movimento(RILASCIO_MOTORI);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
