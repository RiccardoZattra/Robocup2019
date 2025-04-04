/*
 * PID_linea.h
 *
 * Created: 23/08/2018 15:32:36
 *  Author: Riki
 */ 


#ifndef PID_LINEA_H_
#define PID_LINEA_H_
#define F_CPU 16000000UL
#define M_ACUTA 9
#define M_STOP 8
#define M_BUMP 7
#define k_avanti 0.00078
#define k1_indietro 0.005
//#define k_avanti_acc 0.00322 //y=0.00322*dif+0.7
//#define k1_indietro_acc 0.009 //y=0.009*dif+0.7
#define k2_indietro 0.0028
#define k_avanti_discesa 0.00077
#define k1_indietro_discesa 0.0038
#define k2_indietro_discesa 0.0028
#define SX 1
#define DX 0
#define AVANTI_MISURA 7000
#define ANGOLO_VARIABILE 4500
#define RESET 0
#define POS_ENCODER 5

void PID_linea(int16_t differenza, uint8_t pend);
double get_PIDsx();
double get_PIDdx();
double PID_gira_sx();
double PID_gira_dx();
void PID_gira(float angolo_voluto,double ng_iniz);
void PID_stanza(int16_t differenza/*, uint8_t pend*/);
void PID_ostacolo(uint8_t direzione);
uint8_t GetSetOstacolo(uint8_t az);

#endif /* PID_LINEA_H_ */