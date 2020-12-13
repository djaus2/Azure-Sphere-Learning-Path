#include "board.h"





/// <summary>
///     Reads telemetry and returns the length of JSON data.
/// </summary>
bool lp_readTelemetry(LP_ENVIRONMENT* environment)
{
	bool result = true;
	environment->light = 0;
#ifdef GROVE
	result = ReadGroveSensors(environment);
#else
	int rnd = 0;
	rnd = (rand() % 10) - 5;
	environment->temperature = (float)(25.0 + rnd);
	environment->humidity = (float)(50.0 + rnd);

	rnd = (rand() % 50) - 25;
	environment->pressure = (float)(1000.0 + rnd);
#endif
	return result;
}

bool lp_initializeDevKit(void) {

	srand((unsigned int)time(NULL)); // seed the random number generator for fake telemetry

	return true;
}

bool lp_initializeSensors(void) {
	bool result = true;
#ifdef GROVE
	result = lp_initializeGroveSensors();
#endif
	return result;
}

bool lp_closeDevKit(void) {

	return true;
}