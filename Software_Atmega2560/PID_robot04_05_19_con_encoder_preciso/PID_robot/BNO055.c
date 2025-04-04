/*
 * BNO055.c
 *
 * Created: 06/04/2018 12:58:46
 *  Author: Studente
 */ 
#include "BNO055.h"
#include <avr/io.h>
#include <util/twi.h>

uint8_t indirizzo=(BNO055_ADDRESS_A<<1);//0x50
int8_t x=0,y=0;
float z=0;
double Acc_z=0,Acc_x=0,Acc_y=0;
float riferimentoZ=0;
int8_t riferimentoY=0;
double Acc_y_I=0;
double max[10], min[10];
double maxN=0, minN=0;
int cont=0;
uint8_t bumper=1;

void writeReg_BNO(uint8_t reg, uint8_t value)
{
	if (startI2C()) {stop();}									//mi restituisce 1 se lo start NON è avvenuto correttamente
	if (sendI2C(indirizzo|TW_WRITE)) {stop();}				////mi restituisce 1 se la trasmissione NON è avvenuta correttamente
	if (sendI2C(reg)) {stop();}
	if (sendI2C(value)) {stop();}
	stop();
	_delay_ms(5);
}

uint8_t readReg_BNO(uint8_t reg)
{
	if (startI2C()) {stop();}
	if (sendI2C(indirizzo|TW_WRITE)) {stop();}
	if (sendI2C(reg)) {stop();}
	startI2C();
	if (sendI2C(indirizzo|TW_READ)) {stop();}
	uint16_t value=readNACK();
	stop();
	_delay_ms(5);
	if (value>>8) {stop();}
	return value;
}
void multipleRead_BNO(uint8_t reg,uint8_t numb_data)
{
	int xyz[numb_data];
	if(startI2C()){stop();}
	if (sendI2C(indirizzo|TW_WRITE)) {stop();}
	if (sendI2C(reg)) {stop();}
	
	startI2C();
	if (sendI2C(indirizzo|TW_READ)) {stop();}
	for(int i=0;i<numb_data-1;i++)
	{
		xyz[i]=readACK();
		if(xyz[i]>>8){stop();}
	}
	xyz[numb_data-1]=readNACK();
	if(xyz[numb_data-1]>>8){stop();}
	
	z=(float)( ((int16_t)xyz[0]) | ((int16_t)xyz[1]<<8) )/16.0;
	x=(((int16_t)xyz[2])|((int16_t)xyz[3]<<8))/16;
	y=(((int16_t)xyz[4])|((int16_t)xyz[5]<<8))/16;
	
}

void multipleRead_Acc (uint8_t reg)
{
	int xyz[6];
	if(startI2C()){stop();}
	if (sendI2C(indirizzo|TW_WRITE)) {stop();}
	if (sendI2C(reg)) {stop();}
	
	startI2C();
	if (sendI2C(indirizzo|TW_READ)) {stop();}
	for(int i=0;i<5;i++)
	{
		xyz[i]=readACK();
		if(xyz[i]>>8){stop();}
	}
	xyz[5]=readNACK();
	if(xyz[5]>>8){stop();}
		
	Acc_y=( (int16_t)xyz[2] | (int16_t)xyz[3]<<8 )/100.00;
}
void setMode (uint8_t mode)
{
	writeReg_BNO(BNO055_OPR_MODE_ADDR, mode);//0x3D
	_delay_ms(30);
}
int BNO_begin()
{
	uint8_t id = readReg_BNO(BNO055_CHIP_ID_ADDR);
	if(id != BNO055_ID)
	{
		_delay_ms(1000);
		id = readReg_BNO(BNO055_CHIP_ID_ADDR);
		if(id != BNO055_ID)
		{
			return 0;
		}
	}
	
	/* Switch to config mode (just in case since this is the default) */
	setMode(OPERATION_MODE_CONFIG);
	
	writeReg_BNO(BNO055_SYS_TRIGGER_ADDR,0x01);
	_delay_ms(1000);
	uint8_t risul_self_test;
	risul_self_test=readReg_BNO(BNO055_SELFTEST_RESULT_ADDR);
	if(risul_self_test!=15)
	{
		return 0;
	}
	
	writeReg_BNO(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
	_delay_ms(10);
	
	 writeReg_BNO(BNO055_PAGE_ID_ADDR, 0);
	 uint8_t unitsel =	 (0 << 7) | // Orientation = Android
						 (0 << 4) | // Temperature = Celsius
						 (0 << 2) | // Euler = Degrees
						 (1 << 1) | // Gyro = Rads
						 (0 << 0);  // Accelerometer = m/s^2
	writeReg_BNO(BNO055_UNIT_SEL_ADDR, unitsel);
	
	setMode(OPERATION_MODE_IMUPLUS);//OPERATION_MODE_IMUPLUS
	_delay_ms(20);
	
	return 1;
}
void getTemp(void)
{
	int8_t temp = (int8_t)(readReg_BNO(BNO055_TEMP_ADDR));
	if(temp>=0)
	{
		Serial_println("");
		Serial_print("temperatura:");
		Serial_int(temp);
	}
	else
	{
		Serial_println("");
		Serial_print("temperatura:-");
		temp=temp*(-1);
		Serial_int(temp);
	}
}
uint8_t BNO_get_calibration(void)
{
	uint8_t calibrazione=readReg_BNO(BNO055_CALIB_STAT_ADDR);
	return calibrazione;
}
uint8_t Init_gyro()
{
	init_I2C();
	if(!(BNO_begin()))
		return 1;
	_delay_ms(50);//dopo il delay legge correttamente
	return 0;
}
void calibraAngolo()
{
	multipleRead_BNO(BNO055_EULER_H_LSB_ADDR,6);
	riferimentoZ=z;
}
void calibraPendenza()
{
	multipleRead_BNO(BNO055_EULER_H_LSB_ADDR,6);
	riferimentoY=y;
}
float angolo(uint8_t direzione)
{
	multipleRead_BNO(BNO055_EULER_H_LSB_ADDR,6);
	
	/*Serial_print("z=");
	Serial_int(z,'s',10);
	Serial_print(" rif=");
	Serial_int(riferimentoZ,'s',10);
	Serial_println("");*/
	
	if(direzione==DX && z>=riferimentoZ)
		return z-riferimentoZ;
	if(direzione==DX && z<riferimentoZ)
		return 360-riferimentoZ+z;
	if(direzione==SX && z<=riferimentoZ)
		return riferimentoZ-z;
	if(direzione==SX && z>riferimentoZ)
		return riferimentoZ+360-z;
	return 0;
}
int8_t pendenza()
{
	multipleRead_BNO(BNO055_EULER_H_LSB_ADDR,6);
	return y-riferimentoY;
}
uint8_t get_bumper()
{
	multipleRead_Acc(BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR);
	if(Acc_y>0 && Acc_y>maxN)
		maxN=Acc_y;
	else if(Acc_y<0 && Acc_y<minN)
		minN=Acc_y;
		
	if(Acc_y*Acc_y_I<=0)
	{
		if(minN!=0)
		min[cont]=minN;
		if(maxN!=0)
		max[cont]=maxN;
		minN=maxN=0;
		cont++;
		if(cont==10)
		cont=0;
		bumper=1;
		for(int i=0;i<10;i++)
			if(max[i]-min[i]<2)
				bumper=0;
	}
	Acc_y_I=Acc_y;
	return bumper;
}


