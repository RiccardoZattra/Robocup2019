/*
 * Stanza.h
 *
 * Created: 28/12/2018 13:16:05
 *  Author: Riki
 */ 


#ifndef STANZA_H_
#define STANZA_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdarg.h>
#include "Seriale.h"
#include "millis.h"
#include "Posizione.h"
#include "Accessori.h"
#include "PID_linea.h"
#include "VL6180x.h"
#include "EEPROM.h"


#define M_DELAY_STANZA 5
#define M_DELAY_PAL 4

#define DX2SX 100
#define SX2DX 10


#define ROT_90_DX 0
#define ROT_90_SX 1
#define RILASCIO_MOTORI 2
#define CONTROLLA_ANGOLO 4000
#define ASSEGNA_ANGOLO 4005
#define ANGOLO_VARIABILE 4500
#define AVANTI_MISURA 7000	//max +-1 metro in millimetri (9000/11000)
#define PINZA 11001			//occupato fino a 11107
#define DELAY 12000			//max 60 sec (72000)
#define LED 100
#define ALLINEAMENTO_FINECORSA 3
#define STOP 4
#define POS_ENCODER 5
#define ROT_45_DX 6
#define ROT_45_SX 7
#define ALLINEAMENTO_FINECORSA_DAVANTI 8
#define CONTROLLA_PALLINA_ARGENTO 9//da spezzare in due per argento e nera
#define CONTROLLA_PALLINA_NERA 10
#define LASER 11
#define RILEVA_MIS_ENC 12
#define AZZERA_VAR_RASP 13
#define STOP_INDIETRO 14
#define DISATTIVA_CONTROLLO_ANGOLO 15
#define DISATTIVA_SCANDAGLIA 16
#define FINE_PRIMA_PARTE 17
#define CONTATTO_FIN_DAVANTI 18
#define CONTATTO_FIN_DIETRO 19
#define ROT_180 20
#define AZZERA_VAR_PALLINE 21
#define VERIFICA_PRESA 22
#define AZZERA_VAR_SCARICA 23
#define MURO_ALLE_SPALLE 24
#define CONDUZIONE_ARGENTO 25
#define CAMBIA_RICERCA 26
#define TUTTE_NERE 27
#define S_AVANZAMENTO 28
#define CONTA_SCANDAGLIATE 29
#define AZZERA_SCANDAGLIATE 30
#define AZZERA_CONTATTO_MURO 31
#define AZZERA_PRIMO_180 32
#define VAR_RACCOLTA 33
#define AZZERA_VAR_RACCOLTA 34
#define VERIFICA_PRESA_SPOSTAMENTO 35
#define AZZERA_VAR_RASP_SPOSTAMENTO 36
#define INIZIO_SPOSTAMENTO_PALLINE 37
#define FINE_SPOSTAMENTO_PALLINE 38
#define IMP_FILTRO_ANGOLO 39
#define DIS_FILTRO_ANGOLO 40
#define SALVA_SU_EEPROM 41
#define VIBRA 42

#define ZONA_ALTA 1
#define ZONA_BASSA 0
#define OFF 0
#define ON 1
#define APRI 2
#define CHIUDI 3
#define PIANALE_APRI 4
#define PIANALE_CHIUDI 5
#define POS 6
#define M_PINZA 6

#define VAL_INIZIALE 254
#define VAL_NOBALL 255

#define SPOSTA_DX 0
#define SPOSTA_SX 1

#define IND_EE_POS_RACCOLTA 5
#define IND_EE_NUM_PAL_ARGENTO 6
#define IND_EE_NUM_PAL_NERE 7
#define IND_EE_NUMERO_AVANZAMENTI 9

void inizializza_valori_eeprom();
void dichiara_variabili_da_eeprom();
void incrementa_step();
int16_t get_azione();
void programma_movimento_e_corto_dx();
void programma_movimento_e_corto_sx();
void programma_movimento_e_lungo_sx();
void programma_movimento_e_lungo_dx();
void programma_movimento_e_lungo_cx_verso_dx();
void programma_movimento_e_lungo_cx_verso_sx();
void programma_movimento_e_corto_cx();

///////////////////////////ESEMPIO PROGRAMMA PINZA ///////////////////////////////
/*	esegui_movimento(STOP);
	esegui_movimento(RILASCIO_MOTORI);
	esegui_movimento(PINZA+ON);
	esegui_movimento(PINZA+POS+0);
	esegui_movimento(PINZA+CHIUDI);
	esegui_movimento(PINZA+POS+100);
	esegui_movimento(PINZA+APRI);*/
//////////////////////////////////////////////////////////////////////////////////

#endif /* STANZA_H_ */