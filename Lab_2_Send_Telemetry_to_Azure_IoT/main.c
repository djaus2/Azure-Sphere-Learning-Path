/*
 *   Please read the disclaimer and the developer board selection section below
 *
 *
 * 
 *   DISCLAIMER
 *
 *   The learning_path_libs functions provided in the learning_path_libs folder:
 *
 *	   1. are NOT supported Azure Sphere APIs.
 *	   2. are prefixed with lp_, typedefs are prefixed with LP_
 *	   3. are built from the Azure Sphere SDK Samples at https://github.com/Azure/azure-sphere-samples
 *	   4. are not intended as a substitute for understanding the Azure Sphere SDK Samples.
 *	   5. aim to follow best practices as demonstrated by the Azure Sphere SDK Samples.
 *	   6. are provided as is and as a convenience to aid the Azure Sphere Developer Learning experience.
 *
 *
 *   DEVELOPER BOARD SELECTION
 *
 *   The following developer boards are supported.
 *
 *	   1. AVNET Azure Sphere Starter Kit.
 *     2. AVNET Azure Sphere Starter Kit Revision 2.
 *	   3. Seeed Studio Azure Sphere MT3620 Development Kit aka Reference Design Board or rdb.
 *	   4. Seeed Studio Seeed Studio MT3620 Mini Dev Board.
 *
 *   ENABLE YOUR DEVELOPER BOARD
 *
 *   Each Azure Sphere developer board manufacturer maps pins differently. You need to select the configuration that matches your board.
 *
 *   Follow these steps:
 *
 *	   1. Open CMakeLists.txt.
 *	   2. Uncomment the set command that matches your developer board.
 *	   3. Click File, then Save to save the CMakeLists.txt file which will auto generate the CMake Cache.
 */
#define GROVE
#define BME280


 // Hardware definition
#include "hw/azure_sphere_learning_path.h"

// Learning Path Libraries
#include "azure_iot.h"
#include "config.h"
#include "exit_codes.h"
#include "peripheral_gpio.h"
#include "terminate.h"
#include "timer.h"

// System Libraries
#include "applibs_versions.h"
#include <applibs/gpio.h>
#include <applibs/log.h>
#include <applibs/powermanagement.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// Hardware specific
#ifdef OEM_AVNET
#include "board.h"
#include "imu_temp_pressure.h"
#include "light_sensor.h"
#endif // OEM_AVNET

// Hardware specific
#ifdef OEM_SEEED_STUDIO
#include "board.h"
#ifdef GROVE
#include "../Drivers/MT3620_Grove_Shield_Library/Grove.h"
#ifdef BME280
#include "../Drivers/MT3620_Grove_Shield_Library/Sensors//GroveTempHumiBaroBME280.h"
#endif
#endif
#endif // SEEED_STUDIO

#define LP_LOGGING_ENABLED FALSE
#define JSON_MESSAGE_BYTES 256  // Number of bytes to allocate for the JSON telemetry message for IoT Central
#define IOT_PLUG_AND_PLAY_MODEL_ID "dtmi:com:example:azuresphere:labmonitor;1"	// https://docs.microsoft.com/en-us/azure/iot-pnp/overview-iot-plug-and-play

// Forward signatures
static void MeasureSensorHandler(EventLoopTimer* eventLoopTimer);
static void AzureIoTConnectionStatusHandler(EventLoopTimer* eventLoopTimer);

LP_USER_CONFIG lp_config;

static char msgBuffer[JSON_MESSAGE_BYTES] = { 0 };

// GPIO Input Peripherals
static LP_GPIO azureIotConnectedLed = {
	.pin = NETWORK_CONNECTED_LED,
	.direction = LP_OUTPUT,
	.initialState = GPIO_Value_Low,
	.invertPin = true,
	.name = "azureIotConnectedLed" };

// Timers
static LP_TIMER azureIotConnectionStatusTimer = {
	.period = { 5, 0 },
	.name = "azureIotConnectionStatusTimer",
	.handler = AzureIoTConnectionStatusHandler };

static LP_TIMER measureSensorTimer = {
	.period = { 60, 0 },
	.name = "measureSensorTimer",
	.handler = MeasureSensorHandler };

// Initialize Sets
LP_GPIO* peripheralGpioSet[] = { &azureIotConnectedLed };
LP_TIMER* timerSet[] = { &azureIotConnectionStatusTimer, &measureSensorTimer };

// Message templates and property sets

static const char* msgTemplate = "{ \"Temperature\":%3.2f, \"Humidity\":%3.1f, \"Pressure\":%3.1f, \"MsgId\":%d }";

static LP_MESSAGE_PROPERTY* telemetryMessageProperties[] = {
	&(LP_MESSAGE_PROPERTY) { .key = "appid", .value = "hvac" },
	&(LP_MESSAGE_PROPERTY) {.key = "format", .value = "json" },
	&(LP_MESSAGE_PROPERTY) {.key = "type", .value = "telemetry" },
	&(LP_MESSAGE_PROPERTY) {.key = "version", .value = "1" }
};


/// <summary>
/// Check status of connection to Azure IoT
/// </summary>
static void AzureIoTConnectionStatusHandler(EventLoopTimer* eventLoopTimer)
{
	static bool toggleConnectionStatusLed = true;

	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0) {
		lp_terminate(ExitCode_ConsumeEventLoopTimeEvent);
	}
	else {
		if (lp_azureConnect()) {
			lp_gpioStateSet(&azureIotConnectedLed, toggleConnectionStatusLed);
			toggleConnectionStatusLed = !toggleConnectionStatusLed;
		}
		else {
			lp_gpioStateSet(&azureIotConnectedLed, false);
		}
	}
}

#ifdef GROVE
#ifdef BME280
void* bme280;
#endif
#endif

/// <summary>
/// Read sensor and send to Azure IoT
/// </summary>
static void MeasureSensorHandler(EventLoopTimer* eventLoopTimer)
{
	static int msgId = 0;

#ifdef GROVE
#ifdef BME280
#else
	static LP_ENVIRONMENT environment;
#endif
#else
	static LP_ENVIRONMENT environment;
#endif
	

	if (ConsumeEventLoopTimerEvent(eventLoopTimer) != 0)
	{
		lp_terminate(ExitCode_ConsumeEventLoopTimeEvent);
	}
	else {
#ifdef GROVE
#ifdef BME280
		//Grove Shield and BME280 Sensor
		GroveTempHumiBaroBME280_ReadTemperature(bme280);
		GroveTempHumiBaroBME280_ReadTemperature(bme280);
		GroveTempHumiBaroBME280_ReadPressure(bme280);
		GroveTempHumiBaroBME280_ReadHumidity(bme280);
		float temperature = GroveTempHumiBaroBME280_GetTemperature(bme280);
		float humidity = GroveTempHumiBaroBME280_GetHumidity(bme280);
		//Log_Debug("\nTemperature: %.1fC\n", temperature);
		//Log_Debug("Humidity: %.1f\%c\n", humidity, 0x25);
		float pressure = GroveTempHumiBaroBME280_GetPressure(bme280);
		//Log_Debug("Pressure: %.1fhPa\n", pressure);
		
		if (snprintf(msgBuffer, JSON_MESSAGE_BYTES, msgTemplate,
				/*environment.*/temperature, /*environment.*/humidity, /*environment.*/pressure, msgId++) > 0)
		{
			Log_Debug("%s\n", msgBuffer);
			lp_azureMsgSendWithProperties(msgBuffer, telemetryMessageProperties, NELEMS(telemetryMessageProperties));
		}
#else
		//If Grove Shield but no Sensor
		if (lp_readTelemetry(&environment) &&
			snprintf(msgBuffer, JSON_MESSAGE_BYTES, msgTemplate,
				environment.temperature, environment.humidity, environment.pressure, msgId++) > 0)
		{
			Log_Debug("%s\n", msgBuffer);
			lp_azureMsgSendWithProperties(msgBuffer, telemetryMessageProperties, NELEMS(telemetryMessageProperties));
		}
#endif
#else
		//If not Grove Shield
		if (lp_readTelemetry(&environment) &&
			snprintf(msgBuffer, JSON_MESSAGE_BYTES, msgTemplate,
				environment.temperature, environment.humidity, environment.pressure, msgId++) > 0)
		{
			Log_Debug("%s\n", msgBuffer);
			lp_azureMsgSendWithProperties(msgBuffer, telemetryMessageProperties, NELEMS(telemetryMessageProperties));
		}
#endif
	}
}


/// <summary>
///  Initialize peripherals, device twins, direct methods, timers.
/// </summary>
static void InitPeripheralsAndHandlers(void)
{
	lp_azureInitialize(lp_config.scopeId, IOT_PLUG_AND_PLAY_MODEL_ID);

	lp_initializeDevKit();

	lp_gpioSetOpen(peripheralGpioSet, NELEMS(peripheralGpioSet));

#ifdef GROVE
	int i2cFd;
	GroveShield_Initialize(&i2cFd, 115200);
	Log_Debug("Looks like the Grove Shield started OK\n");
#ifdef BME280
	bme280 = GroveTempHumiBaroBME280_Open(i2cFd);
	Log_Debug("Looks like the Grove BME280 Sensor started OK\n");
#endif
#endif

	lp_timerSetStart(timerSet, NELEMS(timerSet));
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
	Log_Debug("Closing file descriptors\n");

	lp_timerSetStop(timerSet, NELEMS(timerSet));
	lp_azureToDeviceStop();

	lp_gpioSetClose(peripheralGpioSet, NELEMS(peripheralGpioSet));

	lp_closeDevKit();

	lp_timerEventLoopStop();
}


int main(int argc, char* argv[])
{
	lp_registerTerminationHandler();

	lp_configParseCmdLineArguments(argc, argv, &lp_config);
	if (!lp_configValidate(&lp_config)) {
		return lp_getTerminationExitCode();
	}

	InitPeripheralsAndHandlers();


	// Main loop
	while (!lp_isTerminationRequired())
	{
		int result = EventLoop_Run(lp_timerGetEventLoop(), -1, true);
		// Continue if interrupted by signal, e.g. due to breakpoint being set.
		if (result == -1 && errno != EINTR)
		{
			lp_terminate(ExitCode_Main_EventLoopFail);
		}
	}

	ClosePeripheralsAndHandlers();

	Log_Debug("Application exiting.\n");
	return lp_getTerminationExitCode();
}