/*
 * millis.h
 *
 * Created: 02/11/2017 16:31:03
 *  Author: Riccardo e Matteo
 */ 


#ifndef MILLIS_H_
#define MILLIS_H_
void init_millis();
uint32_t millis();
void set_millis(int pos);
uint32_t /*int*/ get_millis(int pos);
void del_millis(int pos);
#endif /* MILLIS_H_ */