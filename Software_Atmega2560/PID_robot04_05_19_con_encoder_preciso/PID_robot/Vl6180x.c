/*
 * Vl6180x.c
 *
 * Created: 27/04/2018 21:14:47
 *  Author: Matteo
 */ 
#include "VL6180x.h"
#include "millis.h"
#define F_CPU 16000000UL
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <util/twi.h>

// RANGE_SCALER values for 1x, 2x, 3x scaling - see STSW-IMG003 core/src/vl6180x_api.c (ScalerLookUP[])
static uint16_t const ScalerValues[] = {0, 253, 127, 84};

uint8_t slave=ADDRESS_DEFAULT, address=ADDRESS_DEFAULT;
// Constructors ////////////////////////////////////////////////////////////////


// Public Methods //////////////////////////////////////////////////////////////
void init_laser()
{
	//init_I2C();
	Serial_print("I2Cset");
	initVL6180();
	Serial_print("\t inizializzazione");
	configureDefault();
	Serial_print("\t configureDefault");
	// Reduce range max convergence time and ALS integration
	// time to 30 ms and 50 ms, respectively, to allow 10 Hz
	// operation (as suggested by Table 6 ("Interleaved mode
	// limits (10 Hz operation)") in the datasheet).
	writeReg(SYSRANGE__MAX_CONVERGENCE_TIME, 30);
	writeReg16Bit(SYSALS__INTEGRATION_PERIOD, 50);

	setTimeout(500);//500
	// stop continuous mode if already active
	stopContinuous();
	// in case stopContinuous() triggered a single-shot
	// measurement, wait for it to complete
	_delay_ms(300);
	// start interleaved continuous mode with period of 100 ms
	startRangeContinuous(100);
	Serial_print("\t startInterleavedContinuous \n");
}
void setAddress(uint8_t new_addr)
{
	writeReg(I2C_SLAVE__DEVICE_ADDRESS, new_addr & 0x7F);
}

void setSensori()
{
	DDRA=0b11111110;//sensore Avanti Alto				AD
	//PORTA=0x01;
	_delay_ms(100);
	initVL6180();
	Serial_print("\t inizializzazione");
	configureDefault();
	Serial_print("\t configureDefault");
	setTimeout(500);
	Serial_print("\t setTimeout");
	writeReg(SYSRANGE__MAX_CONVERGENCE_TIME, 30);
	writeReg16Bit(SYSALS__INTEGRATION_PERIOD, 50);
	_delay_ms(100);
	setAddress(addrAD);//0x54
	Serial_print("\t setAddress");
/*
	DDRA=0b11111100;
	//PORTA=0b00000011;
	_delay_ms(100);
	initVL6180();
	Serial_print("\t inizializzazione2");
	configureDefault();
	Serial_print("\t configureDefault2");
	setTimeout(500);
	Serial_print("\t setTimeout2");
	writeReg(SYSRANGE__MAX_CONVERGENCE_TIME, 30);
	writeReg16Bit(SYSALS__INTEGRATION_PERIOD, 50);
	_delay_ms(100);
	setAddress(addrAS);//0x54
	Serial_print("\t setAddress2");
*/
	/*DDRA=0b11111000;
	PORTA=0b00000111;
	_delay_ms(100);
	initVL6180();
	Serial_print("\t inizializzazione3");
	configureDefault();
	Serial_print("\t configureDefault3");
	setTimeout(500);
	Serial_print("\t setTimeout3");
	writeReg(SYSRANGE__MAX_CONVERGENCE_TIME, 30);
	writeReg16Bit(SYSALS__INTEGRATION_PERIOD, 50);
	_delay_ms(100);
	setAddress(addrDD);//0x54
	Serial_print("\t setAddress3");
	
	DDRA=0b11110000;
	PORTA=0b00001111;
	_delay_ms(100);
	initVL6180();
	Serial_print("\t inizializzazione4");
	configureDefault();
	Serial_print("\t configureDefault4");
	setTimeout(500);
	Serial_print("\t setTimeout4");
	writeReg(SYSRANGE__MAX_CONVERGENCE_TIME, 30);
	writeReg16Bit(SYSALS__INTEGRATION_PERIOD, 50);
	_delay_ms(100);
	setAddress(addrDS);//0x54
	Serial_print("\t setAddress4");*/
}

//per leggere dai vari sensori
uint16_t readLASER(/*int which*/)//richiamare la funzione tra apici il nome del sensore
{
	/*
	switch(which)
	{
		case AD:						//Avanti Alto			'AD'
		slave=(addrAD<<1);				//0x54
		break;
		case AS:						//Avanti Basso			'AS'
		slave=(addrAS<<1);
		break;
		case DD:						//Laterale Sinistra		'DD'
		slave=(addrDD<<1);
		break;
		case DS:						//Laterale Destra		'DS'
		slave=(addrDS<<1);
		break;
	}
	uint16_t val=readRangeContinuousMillimeters();
	slave=ADDRESS_DEFAULT;
	return val;
	*/
	return RangeStatusError(readRangeStatus(), readRangeContinuousMillimeters());
}

float readLux(uint8_t gain)
{
	uint8_t reg;

	reg = readReg(VL6180X_REG_SYSTEM_INTERRUPT_CONFIG);
	reg &= ~0x38;
	reg |= (0x4 << 3); // IRQ on ALS ready
	writeReg(VL6180X_REG_SYSTEM_INTERRUPT_CONFIG, reg);
	
	// 100 ms integration period
	writeReg(VL6180X_REG_SYSALS_INTEGRATION_PERIOD_HI, 0);
	writeReg(VL6180X_REG_SYSALS_INTEGRATION_PERIOD_LO, 100);

	// analog gain
	if (gain > VL6180X_ALS_GAIN_40) {
		gain = VL6180X_ALS_GAIN_40;
	}
	writeReg(VL6180X_REG_SYSALS_ANALOGUE_GAIN, 0x40 | gain);

	// start ALS
	writeReg(VL6180X_REG_SYSALS_START, 0x1);

	// Poll until "New Sample Ready threshold event" is set
	while (4 != ((readReg(VL6180X_REG_RESULT_INTERRUPT_STATUS_GPIO) >> 3) & 0x7));

	// read lux!
	float lux = readReg16Bit(VL6180X_REG_RESULT_ALS_VAL);

	// clear interrupt
	writeReg(VL6180X_REG_SYSTEM_INTERRUPT_CLEAR, 0x07);

	lux *= 0.32; // calibrated count/lux
	switch(gain) {
		case VL6180X_ALS_GAIN_1:
		break;
		case VL6180X_ALS_GAIN_1_25:
		lux /= 1.25;
		break;
		case VL6180X_ALS_GAIN_1_67:
		lux /= 1.76;
		break;
		case VL6180X_ALS_GAIN_2_5:
		lux /= 2.5;
		break;
		case VL6180X_ALS_GAIN_5:
		lux /= 5;
		break;
		case VL6180X_ALS_GAIN_10:
		lux /= 10;
		break;
		case VL6180X_ALS_GAIN_20:
		lux /= 20;
		break;
		case VL6180X_ALS_GAIN_40:
		lux /= 20;
		break;
	}
	lux *= 100;
	lux /= 100; // integration time in ms


	return lux;
}

float readLuxAll(uint8_t gain, char which)
{
	/*
	switch(which)
	{
		case AD:						//Avanti Alto			'AD'
		slave=(addrAD<<1);				//0x54
		break;
		case AS:						//Avanti Basso			'AS'
		slave=(addrAS<<1);
		break;
		case DD:						//Laterale Sinistra		'DD'
		slave=(addrDD<<1);
		break;
		case DS:						//Laterale Destra		'DS'
		slave=(addrDS<<1);
		break;
	}*/
	uint16_t lux=readLux(gain);
	slave=ADDRESS_DEFAULT;
	return lux;
}

uint8_t readRangeStatus(/*char which*/)
{
	/*switch(which)
	{
		case AD:						//Avanti Alto			'AD'
		slave=(addrAD<<1);				//0x54
		break;
		case AS:						//Avanti Basso			'AS'
		slave=(addrAS<<1);
		break;
		case DD:						//Laterale Sinistra		'DD'
		slave=(addrDD<<1);
		break;
		case DS:						//Laterale Destra		'DS'
		slave=(addrDS<<1);
		break;
	}*/
	return (readReg(VL6180X_REG_RESULT_RANGE_STATUS) >> 4);
}

char RangeStatusError(uint8_t status, uint8_t range)
{
	// Some error occurred, print it out!
	if (range==255)
	{
		return range;
	}
	if  ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
		Serial_println("System error");
		return 0;
	}
	else if (status == VL6180X_ERROR_ECEFAIL) {
		Serial_println("ECE failure");
		return 0;
	}
	else if (status == VL6180X_ERROR_NOCONVERGE) {
		Serial_println("No convergence");
		return 0;
	}
	else if (status == VL6180X_ERROR_RANGEIGNORE) {
		Serial_println("Ignoring range");
		return 0;
	}
	else if (status == VL6180X_ERROR_SNR) {
		Serial_println("Signal/Noise error");
		return 0;
	}
	else if (status == VL6180X_ERROR_RAWUFLOW) {
		Serial_println("Raw reading underflow");
		return 0;
	}
	else if (status == VL6180X_ERROR_RAWOFLOW) {
		Serial_println("Raw reading overflow");
		return 0;
	}
	else if (status == VL6180X_ERROR_RANGEUFLOW) {
		Serial_println("Range reading underflow");
		return 0;
	}
	else if (status == VL6180X_ERROR_RANGEOFLOW) {
		Serial_println("Range reading overflow");
		return 0;
	}
	else return range;
}

void writeReg(uint16_t reg, uint8_t value)
{
	if (startI2C()) {stop();Serial_print("start");}									//mi restituisce 1 se lo start NON � avvenuto correttamente
	if (sendI2C(slave|TW_WRITE)) {stop();Serial_print("slave");}				////mi restituisce 1 se la trasmissione NON � avvenuta correttamente
	if (sendI2C(reg >> 8)) {stop();Serial_print("regHB");}
	if (sendI2C(reg & 0xFF)) {stop();Serial_print("regLB");}
	if (sendI2C(value)) {stop();Serial_print("sendData");}
	stop();
	_delay_ms(5);
}

// Write a 16-bit register
void writeReg16Bit(uint16_t reg, uint16_t value)
{
	if (startI2C()) {stop();}
	if (sendI2C(slave|TW_WRITE)) {stop();}
	if (sendI2C(reg >> 8)) {stop();}
	if (sendI2C(reg & 0xFF)) {stop();}
	if (sendI2C((value>>8) & 0xFF)) {stop();}
	if (sendI2C((value) & 0xFF)) {stop();}
	stop();
	_delay_ms(5);
}

// Write a 32-bit register
void writeReg32Bit(uint16_t reg, uint32_t value)
{
	if (startI2C()) {stop();}
	if (sendI2C(slave|TW_WRITE)) {stop();}
	if (sendI2C(reg >> 8)) {stop();}
	if (sendI2C(reg & 0xFF)) {stop();}
	if (sendI2C((value>>24) & 0xFF)) {stop();}
	if (sendI2C((value>>16) & 0xFF)) {stop();}
	if (sendI2C((value>>8) & 0xFF)) {stop();}
	if (sendI2C((value) & 0xFF)) {stop();}
	stop();
	_delay_ms(5);
}

// Read an 8-bit register
uint8_t readReg(uint16_t reg)
{
	if (startI2C()) {stop();}
	if (sendI2C(slave|TW_WRITE)) {stop();}
	if (sendI2C(reg >> 8)) {stop();}
	if (sendI2C(reg & 0xFF)) {stop();}
	stop();
	startI2C();
	if (sendI2C(slave|TW_READ)) {stop();}
	uint8_t value=readNACK();
	stop();
	_delay_ms(5);
	if (value>>8) {stop();}
	return value;
}

// Read a 16-bit register
uint16_t readReg16Bit(uint16_t reg)
{
	uint16_t val;
	if (startI2C()) {stop();}
	if (sendI2C(slave|TW_WRITE)) {stop();}
	if (sendI2C(reg >> 8)) {stop();}
	if (sendI2C(reg & 0xFF)) {stop();}
	stop();
	startI2C();
	if (sendI2C(slave|TW_READ)) {stop();}
	uint16_t value=readACK();
	if(value>>8){stop(); return 0;}
	uint16_t value1=readNACK();
	stop();
	if(value1>>8){stop(); return 0;}
	val=((value<<8)|value1);
	_delay_ms(5);
	return val;
}

// Read a 32-bit register
uint32_t readReg32Bit(uint16_t reg)
{
	uint32_t val;
	if (startI2C()) {stop();}
	if (sendI2C(slave|TW_WRITE)) {stop();}
	if (sendI2C(reg >> 8)) {stop();}
	if (sendI2C(reg & 0xFF)) {stop();}
	stop();
	startI2C();
	if (sendI2C(slave|TW_READ)) {stop();}
	uint32_t value=readACK();
	if(value>>8){stop(); return 0;}
	uint32_t value1=readACK();
	if(value1>>8){stop(); return 0;}
	uint32_t value2=readACK();
	if (value2>>8){stop(); return 0;}
	uint32_t value3=readNACK();
	if (value3>>8){stop(); return 0;}
	stop();
	val=((value3<<24)|(value2<<16)|(value1<<8)|value);
	_delay_ms(5);
	return val;
}

// Initialize sensor with settings from ST application note AN4545, section 9 -
// "Mandatory : private registers"
void initVL6180()
{
	// Store part-to-part range offset so it can be adjusted if scaling is changed
	ptp_offset = readReg(SYSRANGE__PART_TO_PART_RANGE_OFFSET);
	//Serial_print("\t 1");
	if (readReg(SYSTEM__FRESH_OUT_OF_RESET) == 1)
	{
		scaling = 1;

		writeReg(0x207, 0x01);
		writeReg(0x208, 0x01);
		writeReg(0x096, 0x00);
		writeReg(0x097, 0xFD); // RANGE_SCALER = 253
		writeReg(0x0E3, 0x00);
		writeReg(0x0E4, 0x04);
		writeReg(0x0E5, 0x02);
		writeReg(0x0E6, 0x01);
		writeReg(0x0E7, 0x03);
		writeReg(0x0F5, 0x02);
		writeReg(0x0D9, 0x05);
		writeReg(0x0DB, 0xCE);
		writeReg(0x0DC, 0x03);
		writeReg(0x0DD, 0xF8);
		writeReg(0x09F, 0x00);
		writeReg(0x0A3, 0x3C);
		writeReg(0x0B7, 0x00);
		writeReg(0x0BB, 0x3C);
		writeReg(0x0B2, 0x09);
		writeReg(0x0CA, 0x09);
		writeReg(0x198, 0x01);
		writeReg(0x1B0, 0x17);
		writeReg(0x1AD, 0x00);
		writeReg(0x0FF, 0x05);
		writeReg(0x100, 0x05);
		writeReg(0x199, 0x05);
		writeReg(0x1A6, 0x1B);
		writeReg(0x1AC, 0x3E);
		writeReg(0x1A7, 0x1F);
		writeReg(0x030, 0x00);

		writeReg(SYSTEM__FRESH_OUT_OF_RESET, 0);
	}
	else
	{
		// Sensor has already been initialized, so try to get scaling settings by
		// reading registers.

		uint16_t s = readReg16Bit(RANGE_SCALER);

		if      (s == ScalerValues[3]) { scaling = 3; }
		else if (s == ScalerValues[2]) { scaling = 2; }
		else                           { scaling = 1; }

		// Adjust the part-to-part range offset value read earlier to account for
		// existing scaling. If the sensor was already in 2x or 3x scaling mode,
		// precision will be lost calculating the original (1x) offset, but this can
		// be resolved by resetting the sensor and Arduino again.
		ptp_offset *= scaling;
	}
}

// Configure some settings for the sensor's default behavior from AN4545 -
// "Recommended : Public registers" and "Optional: Public registers"
//
// Note that this function does not set up GPIO1 as an interrupt output as
// suggested, though you can do so by calling:
// writeReg(SYSTEM__MODE_GPIO1, 0x10);
void configureDefault(void)
{
	// "Recommended : Public registers"

	// readout__averaging_sample_period = 48
	writeReg(READOUT__AVERAGING_SAMPLE_PERIOD, 0x30);

	// sysals__analogue_gain_light = 6 (ALS gain = 1 nominal, actually 1.01 according to Table 14 in datasheet)
	writeReg(SYSALS__ANALOGUE_GAIN, 0x46);

	// sysrange__vhv_repeat_rate = 255 (auto Very High Voltage temperature recalibration after every 255 range measurements)
	writeReg(SYSRANGE__VHV_REPEAT_RATE, 0xFF);

	// sysals__integration_period = 99 (100 ms)
	// AN4545 incorrectly recommends writing to register 0x040; 0x63 should go in the lower byte, which is register 0x041.
	writeReg16Bit(SYSALS__INTEGRATION_PERIOD, 0x0063);

	// sysrange__vhv_recalibrate = 1 (manually trigger a VHV recalibration)
	writeReg(SYSRANGE__VHV_RECALIBRATE, 0x01);


	// "Optional: Public registers"

	// sysrange__intermeasurement_period = 9 (100 ms)
	writeReg(SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09);

	// sysals__intermeasurement_period = 49 (500 ms)
	writeReg(SYSALS__INTERMEASUREMENT_PERIOD, 0x31);

	// als_int_mode = 4 (ALS new sample ready interrupt); range_int_mode = 4 (range new sample ready interrupt)
	writeReg(SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24);


	// Reset other settings to power-on defaults

	// sysrange__max_convergence_time = 49 (49 ms)
	writeReg(SYSRANGE__MAX_CONVERGENCE_TIME, 0x31);

	// disable interleaved mode
	writeReg(INTERLEAVED_MODE__ENABLE, 0);

	// reset range scaling factor to 1x
	setScaling(1);
}

// Set range scaling factor. The sensor uses 1x scaling by default, giving range
// measurements in units of mm. Increasing the scaling to 2x or 3x makes it give
// raw values in units of 2 mm or 3 mm instead. In other words, a bigger scaling
// factor increases the sensor's potential maximum range but reduces its
// resolution.

// Implemented using ST's VL6180X API as a reference (STSW-IMG003); see
// VL6180x_UpscaleSetScaling() in vl6180x_api.c.
void setScaling(uint8_t new_scaling)
{
	uint8_t const DefaultCrosstalkValidHeight = 20; // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

	// do nothing if scaling value is invalid
	if (new_scaling < 1 || new_scaling > 3) { return; }

	scaling = new_scaling;
	writeReg16Bit(RANGE_SCALER, ScalerValues[scaling]);

	// apply scaling on part-to-part offset
	writeReg(SYSRANGE__PART_TO_PART_RANGE_OFFSET, ptp_offset / scaling);

	// apply scaling on CrossTalkValidHeight
	writeReg(SYSRANGE__CROSSTALK_VALID_HEIGHT, DefaultCrosstalkValidHeight / scaling);

	// This function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.

	// enable early convergence estimate only at 1x scaling
	uint8_t rce = readReg(SYSRANGE__RANGE_CHECK_ENABLES);
	writeReg(SYSRANGE__RANGE_CHECK_ENABLES, (rce & 0xFE) | (scaling == 1));
}

// Performs a single-shot ranging measurement
uint8_t readRangeSingle()
{
	writeReg(SYSRANGE__START, 0x01);
	return readRangeContinuous();
}

// Performs a single-shot ambient light measurement
uint16_t readAmbientSingle()
{
	writeReg(SYSALS__START, 0x01);
	return readAmbientContinuous();
}

uint16_t constrain(int x, int a, int b)
{
	if (x < a)	{return a;}
	else if (x > b)	{return b;}
	else {return x;}
}
// Starts continuous ranging measurements with the given period in ms
// (10 ms resolution; defaults to 100 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void startRangeContinuous(uint16_t period)//periodo inversamente proporzionale alla veloci�
{
	period = constrain(period, 10/*50*/, 100);//per variare velocit� di lettura	-> rischio di blocco maggiore se si aumenta la velocit�
	int16_t period_reg = (int16_t)(period / 10) - 1;
	period_reg = constrain(period_reg, 0, 254);

	writeReg(SYSRANGE__INTERMEASUREMENT_PERIOD, period_reg);
	writeReg(SYSRANGE__START, 0x03);
}

void startRangeContinuousAll(uint16_t period)
{
	for (char k=0;k<3;k++)
	{
		
		if (k==0)	slave=(addrAD<<1);
			else if (k==1)   /*slave=(addrAS<<1)*/break;
				else if (k==2) /*slave=(addrDD<<1)*/break;
					else /*slave=(addrDS<<1)*/break;
		
		startRangeContinuous(period);
		_delay_ms(200);
	}
	slave=ADDRESS_DEFAULT;
	
}
// Starts continuous ambient light measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified).
//
// The period must be greater than the time it takes to perform a
// measurement. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void startAmbientContinuous(uint16_t period)
{
	period=500;
	int16_t period_reg = (int16_t)(period / 10) - 1;
	period_reg = constrain(period_reg, 0, 254);

	writeReg(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
	writeReg(SYSALS__START, 0x03);
}

// Starts continuous interleaved measurements with the given period in ms
// (10 ms resolution; defaults to 500 ms if not specified). In this mode, each
// ambient light measurement is immediately followed by a range measurement.
//
// The datasheet recommends using this mode instead of running "range and ALS
// continuous modes simultaneously (i.e. asynchronously)".
//
// The period must be greater than the time it takes to perform both
// measurements. See section 2.4.4 ("Continuous mode limits") in the datasheet
// for details.
void startInterleavedContinuous(uint16_t period)
{
	period=500;
	int16_t period_reg = (int16_t)(period / 10) - 1;
	period_reg = constrain(period_reg, 0, 254);

	writeReg(INTERLEAVED_MODE__ENABLE, 1);
	writeReg(SYSALS__INTERMEASUREMENT_PERIOD, period_reg);
	writeReg(SYSALS__START, 0x03);
}

// Stops continuous mode. This will actually start a single measurement of range
// and/or ambient light if continuous mode is not active, so it's a good idea to
// wait a few hundred ms after calling this function to let that complete
// before starting continuous mode again or taking a reading.
void stopContinuous()
{

	writeReg(SYSRANGE__START, 0x01);
	writeReg(SYSALS__START, 0x01);

	writeReg(INTERLEAVED_MODE__ENABLE, 0);
}

// Returns a range reading when continuous mode is activated
// (readRangeSingle() also calls this function after starting a single-shot
// range measurement)
uint8_t readRangeContinuous()///////////////////////////////////////////////////////////////////millis///////////////////////////////////////////////////////////////////
{
	uint32_t millis_start = millis();//uint16_t
	while ((readReg(RESULT__INTERRUPT_STATUS_GPIO) & 0x04) == 0)
	{
		if (io_timeout > 0 && ((uint32_t)millis() - millis_start) > io_timeout)//uint16_t
		{
			did_timeout = true;
			Serial_print("timeout");
			Serial_int(io_timeout);
			Serial_print("millis_start");
			Serial_int(millis_start);
			Serial_print("millis");
			Serial_int(millis());
			return 255;
		}
	}

	uint8_t range = readReg(RESULT__RANGE_VAL);
	writeReg(SYSTEM__INTERRUPT_CLEAR, 0x01);

	return range;
}

// Returns an ambient light reading when continuous mode is activated
// (readAmbientSingle() also calls this function after starting a single-shot
// ambient light measurement)
uint16_t readAmbientContinuous()///////////////////////////////////////////////////////////////////millis///////////////////////////////////////////////////////////////////
{
	uint16_t millis_start = millis();
	while ((readReg(RESULT__INTERRUPT_STATUS_GPIO) & 0x20) == 0)
	{
 		if (io_timeout > 0 && ((uint16_t)millis() - millis_start) > io_timeout)
 		{
 			did_timeout = true;
 			return 0;
 		}
	}

	uint16_t ambient = readReg16Bit(RESULT__ALS_VAL);
	writeReg(SYSTEM__INTERRUPT_CLEAR, 0x02);

	return ambient;
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool timeoutOccurred()
{
	bool tmp = did_timeout;
	did_timeout = false;
	return tmp;
}
