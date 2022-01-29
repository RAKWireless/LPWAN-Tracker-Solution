/**
 * @file environment.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialization and reading of BME680 sensor
 * @version 0.3
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "app.h"

/** Instance of the BME680 class */
Adafruit_BME680 bme;

/**
 * @brief Initialize the BME680 sensor
 * 
 * @return true if sensor was found
 * @return false if sensor was not found
 */
bool init_bme(void)
{
	if (!bme.begin(0x76, false))
	{
		MYLOG("BME", "Could not find a valid BME680 sensor, check wiring!");
		return false;
	}
	// Set up oversampling and filter initialization
	bme.setTemperatureOversampling(BME680_OS_8X);
	bme.setHumidityOversampling(BME680_OS_2X);
	bme.setPressureOversampling(BME680_OS_4X);
	bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
	bme.setGasHeater(320, 150); // 320*C for 150 ms

	return true;
}

/**
 * @brief Start sensing on the BME6860
 * 
 */
void start_bme(void)
{
	MYLOG("BME", "Start BME reading");
	bme.beginReading();
}

/**
 * @brief Read environment data from BME680
 * 
 * @return true if reading was successful
 * @return false if reading failed
 */
bool read_bme(void)
{
	time_t wait_start = millis();
	bool read_success = false;
	while ((millis() - wait_start) < 5000)
	{
		if (bme.endReading())
		{
			read_success = true;
			break;
		}
	}

	if (!read_success)
	{
		return false;
	}

#if MY_DEBUG > 0
	int16_t temp_int = (int16_t)(bme.temperature * 10.0);
	uint16_t humid_int = (uint16_t)(bme.humidity * 2);
	uint16_t press_int = (uint16_t)(bme.pressure / 10);
	uint16_t gasres_int = (uint16_t)(bme.gas_resistance / 10);
#endif

	g_data_packet.addRelativeHumidity(LPP_CHANNEL_HUMID, bme.humidity);
	g_data_packet.addTemperature(LPP_CHANNEL_TEMP, bme.temperature);
	g_data_packet.addBarometricPressure(LPP_CHANNEL_PRESS, bme.pressure / 100);
	g_data_packet.addAnalogInput(LPP_CHANNEL_GAS, (float)(bme.gas_resistance) / 1000.0);

#if MY_DEBUG > 0
	MYLOG("BME", "RH= %.2f T= %.2f", (float)(humid_int / 2.0), (float)(temp_int / 10.0));
	MYLOG("BME", "P= %d R= %d", press_int * 10, gasres_int * 10);
	MYLOG("BME", "RH= %.2f T= %.2f", bme.humidity, bme.temperature);
	MYLOG("BME", "P= %ld R= %ld", bme.pressure, bme.gas_resistance);
#endif
	return true;
}