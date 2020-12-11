#include "board.h"


#ifdef GROVE
#include "../Drivers/MT3620_Grove_Shield_Library/Grove.h"
#ifdef BME280
#include "../Drivers/MT3620_Grove_Shield_Library/Sensors/GroveTempHumiBaroBME280.h"
static void* bme280;
#endif
#endif




/// <summary>
///     Reads telemetry and returns the length of JSON data.
/// </summary>
bool lp_readTelemetry(LP_ENVIRONMENT* environment)
{
	int rnd=0;

#ifdef GROVE
#ifdef BME280
	//Grove Shield and BME280 Sensor;
	GroveTempHumiBaroBME280_ReadTemperature(bme280);
	GroveTempHumiBaroBME280_ReadTemperature(bme280);
	GroveTempHumiBaroBME280_ReadPressure(bme280);
	GroveTempHumiBaroBME280_ReadHumidity(bme280);

	environment->temperature = GroveTempHumiBaroBME280_GetTemperature(bme280);
	environment->humidity = GroveTempHumiBaroBME280_GetHumidity(bme280);
	environment->pressure = GroveTempHumiBaroBME280_GetPressure(bme280);



	//Log_Debug("\nTemperature: %.1fC\n", environment->temperature);
	//Log_Debug("Humidity: %.1f\%c\n", environment->humidity); // , 0x25);
	//Log_Debug("Pressure: %.1fhPa\n", environment->pressure);
#else
	rnd = (rand() % 10) - 5;
	environment->temperature = (float)(25.0 + rnd);
	environment->humidity = (float)(50.0 + rnd);

	rnd = (rand() % 50) - 25;
	environment->pressure = (float)(1000.0 + rnd);
#endif
#else
	rnd = (rand() % 10) - 5;
	environment->temperature = (float)(25.0 + rnd);
	environment->humidity = (float)(50.0 + rnd);

	rnd = (rand() % 50) - 25;
	environment->pressure = (float)(1000.0 + rnd);
#endif
	environment->light = 0;

	return true;
}

bool lp_initializeDevKit(void) {

	srand((unsigned int)time(NULL)); // seed the random number generator for fake telemetry

	return true;
}

bool lp_initializeSensor(void) {
#ifdef GROVE
	int i2cFd;
	GroveShield_Initialize(&i2cFd, 115200);
	//Log_Debug("Looks like the Grove Shield started OK\n");
#ifdef BME280
	bme280 = GroveTempHumiBaroBME280_Open(i2cFd);
	//Log_Debug("Looks like the Grove BME280 Sensor started OK\n");
#endif
#endif
	return true;
}

bool lp_closeDevKit(void) {

	return true;
}