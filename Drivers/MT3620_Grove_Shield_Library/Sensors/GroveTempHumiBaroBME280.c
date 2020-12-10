#include "GroveTempHumiBaroBME280.h"
#include <stdlib.h>
#include <math.h>
#include "../HAL/GroveI2C.h"

#define  BME280_S32_t int32_t
#define BME280_U32_t  uint32_t
#define BME280_S64_t uint64_t
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T);
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P);
BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H);

//Ref: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf  Page 32 (At bottom)
#define BME280_ADDRESS			(0x76 << 1)  //	(0xee)  
//(0x77) SD0 set to hi . If set to low use 0x76 <<1


//Ref: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf  Page 24

#define BME280_REG_DIG_T1			(0x88)
#define BME280_REG_DIG_T2			(0x8A)
#define BME280_REG_DIG_T3			(0x8C)

#define BME280_REG_DIG_P1			(0x8E)
#define BME280_REG_DIG_P2			(0x90)
#define BME280_REG_DIG_P3			(0x92)
#define BME280_REG_DIG_P4			(0x94)
#define BME280_REG_DIG_P5			(0x96)
#define BME280_REG_DIG_P6			(0x98)
#define BME280_REG_DIG_P7			(0x9A)
#define BME280_REG_DIG_P8			(0x9C)
#define BME280_REG_DIG_P9			(0x9E)


#define BME280_REG_DIG_H1			(0xA1)
#define BME280_REG_DIG_H2			(0xE1)
#define BME280_REG_DIG_H3			(0xE3)
#define BME280_REG_DIG_E4			(0xE4)
#define BME280_REG_DIG_E5			(0xE5)
#define BME280_REG_DIG_E6			(0xE6)
#define BME280_REG_DIG_H6			(0xE7)


#define BME280_REG_CHIPID			(0xD0)
#define BME280_REG_CONTROLHUMID		(0xF2)
#define BME280_REG_CONTROL			(0xF4)

#define BME280_REG_TEMPDATA			(0xFA)
#define BME280_REG_PRESSDATA		(0xF7)
#define BME280_REG_HUMIDDATA		(0xFE)

typedef struct
{
	int I2cFd;
	float Temperature;
	float Pressure;
	float Humidity;
}
GroveTempHumiBaroBME280Instance;

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;

uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

uint8_t dig_H1;
int16_t dig_H2;
uint8_t dig_H3;
int16_t dig_H4;
int16_t dig_H5;
int8_t dig_H6;







void* GroveTempHumiBaroBME280_Open(int i2cFd)
{
	GroveTempHumiBaroBME280Instance* this = (GroveTempHumiBaroBME280Instance*)malloc(sizeof(GroveTempHumiBaroBME280Instance));

	this->I2cFd = i2cFd;
	this->Temperature = NAN;
	

	uint8_t val8;

	if (!GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_CHIPID, &val8)) return NULL;
	
	if (val8 != 0x60) return NULL;

	GroveI2C_WriteReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_CONTROLHUMID, 0x05);
	GroveI2C_WriteReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_CONTROL, 0xb7);

	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_T1, &dig_T1)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_T2, (uint16_t*)&dig_T2)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_T3, (uint16_t*)&dig_T3)) return NULL;

	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P1, &dig_P1)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P2, (uint16_t*)&dig_P2)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P3, (uint16_t*)&dig_P3)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P4, (uint16_t*)&dig_P4)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P5, (uint16_t*)&dig_P5)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P6, (uint16_t*)&dig_P6)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P7, (uint16_t*)&dig_P7)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P8, (uint16_t*)&dig_P8)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_P9, (uint16_t*)&dig_P9)) return NULL;


	if (!GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_H1, (uint8_t*) &dig_H1)) return NULL;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_H2, (uint16_t*)&dig_H2)) return NULL;
	if (!GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_H3, (uint8_t*) &dig_H3)) return NULL;

	//Ref: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf  Page 24 (At bottom):
	// 0xE4/0xE5[3:0]   dig_H4[11:4]/[3:0]   signed short
	// 0xE5[7:4]/0xE6   dig_H5[3:0]/[11:4]   signed short
	// Get individual bytes and then get appropiate bits:

	uint8_t e4, e5, e6;
	if (!GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_E4, (uint8_t*)&e4)) return NULL;
	if (!GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_E5, (uint8_t*)&e5)) return NULL;
	if (!GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_E6, (uint8_t*)&e6)) return NULL;

	int16_t dig_h4_msb = (int16_t)(int8_t)e4 * 16;
	int16_t dig_h4_lsb = (int16_t)(e5 & 0x0F);
	dig_H4 = dig_h4_msb | dig_h4_lsb;
	int16_t dig_h5_msb = (int16_t)(int8_t)e6 * 16;
	int16_t dig_h5_lsb = (int16_t)(e5 >> 4);
	dig_H5 = dig_h5_msb | dig_h5_lsb;

	if (!GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, BME280_REG_DIG_H6, (int8_t*)&dig_H6)) return NULL;
	
	



	return this;
}

void GroveTempHumiBaroBME280_ReadTemperature(void* inst)
{
	GroveTempHumiBaroBME280Instance* this = (GroveTempHumiBaroBME280Instance*)inst;

	this->Temperature = NAN;   

	int32_t adc_T;
	if (!GroveI2C_ReadReg24BE(this->I2cFd, BME280_ADDRESS, BME280_REG_TEMPDATA, &adc_T)) return;

	BME280_S32_t altTemp=  BME280_compensate_T_int32(adc_T);
	this->Temperature = ((float)altTemp) / 100.0;

	//adc_T >>= 4;
	//int32_t var1 = (((adc_T >> 3) - ((int32_t)(dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
	//int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;

	//int32_t t_fine = var1 + var2;
	//this->Temperature = (float)((t_fine * 5 + 128) >> 8) / 100;
}

void GroveTempHumiBaroBME280_ReadPressure(void* inst)
{
	GroveTempHumiBaroBME280Instance* this = (GroveTempHumiBaroBME280Instance*)inst;

	this->Pressure = NAN;

	int32_t adc_P;
	if (!GroveI2C_ReadReg24BE(this->I2cFd, BME280_ADDRESS, BME280_REG_PRESSDATA, &adc_P)) return;

	BME280_S32_t altPress = BME280_compensate_P_int64(adc_P);
	this->Pressure = ((float)altPress) / (100.0* 256);
}

void GroveTempHumiBaroBME280_ReadHumidity(void* inst)
{
	GroveTempHumiBaroBME280Instance* this = (GroveTempHumiBaroBME280Instance*)inst;

	this->Humidity = NAN;

	int32_t adc_H;
	if (!GroveI2C_ReadReg16(this->I2cFd, BME280_ADDRESS, BME280_REG_HUMIDDATA, &adc_H)) return;

	unsigned char aa;
	unsigned char bb;
	GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, 0xFE, &aa);
	GroveI2C_ReadReg8(this->I2cFd, BME280_ADDRESS, 0xFD, &bb);
	adc_H = bb * 256 + aa;

	BME280_S32_t altHumid = bme280_compensate_H_int32(adc_H);
	this->Humidity =  ((float)altHumid) / 1024;
}

float GroveTempHumiBaroBME280_GetTemperature(void* inst)
{
	GroveTempHumiBaroBME280Instance* this = (GroveTempHumiBaroBME280Instance*)inst;

	return this->Temperature;
}

float GroveTempHumiBaroBME280_GetPressure(void* inst)
{
	GroveTempHumiBaroBME280Instance* this = (GroveTempHumiBaroBME280Instance*)inst;

	return this->Pressure;
}

float GroveTempHumiBaroBME280_GetHumidity(void* inst)
{
	GroveTempHumiBaroBME280Instance* this = (GroveTempHumiBaroBME280Instance*)inst;

	return this->Humidity;
}


//Ref: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf  Page 25
// And data registers Page 27

BME280_S32_t  t_fine;

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value BME280_S32_t t_fine;
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T)
{	
	adc_T >>= 4;
	BME280_S32_t var1, var2, T;
	var1 = ((((adc_T >> 3) -((BME280_S32_t)dig_T1 << 1)))* ((BME280_S32_t)dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) -((BME280_S32_t)dig_T1))* ((adc_T >> 4) -((BME280_S32_t)dig_T1))) >> 12)*
		((BME280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P)
{
	adc_P >>= 4;
	BME280_S64_t var1, var2, p;
	var1 = ((BME280_S64_t)t_fine) - 128000;
	var2 = var1 * var1 * (BME280_S64_t)dig_P6;
	var2 = var2 + ((var1 * (BME280_S64_t)dig_P5) << 17); var2 = var2 + (((BME280_S64_t)dig_P4) << 35);
	var1 = ((var1 * var1 * (BME280_S64_t)dig_P3) >> 8) + ((var1 * (BME280_S64_t)dig_P2) << 12); var1 = (((((BME280_S64_t)1) << 47) + var1)) * ((BME280_S64_t)dig_P1) >> 33;
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((BME280_S64_t)dig_P9) * (p >> 13)* (p >> 13)) >> 25;
	var2 = (((BME280_S64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)dig_P7) << 4);
	return (BME280_U32_t)p;
}

// Ref: https://github.com/BoschSensortec/BME280_driver/
static double compensate_humidity(double uncomp_data_humidity)
{
	double humidity;
	double humidity_min = 0.0;
	double humidity_max = 100.0;
	double var1;
	double var2;
	double var3;
	double var4;
	double var5;
	double var6;

	var1 = ((double)t_fine) - 76800.0;
	var2 = (((double)dig_H4) * 64.0 + (((double)dig_H5) / 16384.0) * var1);
	var3 = uncomp_data_humidity - var2;
	var4 = ((double)dig_H2) / 65536.0;
	var5 = (1.0 + (((double)dig_H3) / 67108864.0) * var1);
	var6 = 1.0 + (((double)dig_H6) / 67108864.0) * var1 * var5;
	var6 = var3 * var4 * (var5 * var6);
	humidity = var6 * (1.0 - ((double)dig_H1) * var6 / 524288.0);

	if (humidity > humidity_max)
	{
		humidity = humidity_max;
	}
	else if (humidity < humidity_min)
	{
		humidity = humidity_min;
	}

	return humidity;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H)
{
	//double hum = compensate_humidity(adc_H);
	
	//return hum;

	BME280_S32_t v_x1_u32r;

	v_x1_u32r = (t_fine -((BME280_S32_t)76800));
	
	v_x1_u32r = (((((adc_H << 14) -(((BME280_S32_t)dig_H4) << 20) -(((BME280_S32_t)dig_H5) * v_x1_u32r)) + ((BME280_S32_t)16384)) >> 15)* (((((((v_x1_u32r *
		((BME280_S32_t)dig_H6)) >> 10)* (((v_x1_u32r * ((BME280_S32_t)dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152))* ((BME280_S32_t)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r -(((((v_x1_u32r >> 15)* (v_x1_u32r >> 15)) >> 7)* ((BME280_S32_t)dig_H1)) >> 4));
	
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (BME280_U32_t)(v_x1_u32r >> 12);
}






