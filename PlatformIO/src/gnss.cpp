/**
 * @file gnss.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief GNSS functions and task
 * @version 0.1
 * @date 2020-07-24
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "app.h"

// The GNSS object
SFE_UBLOX_GNSS my_gnss;

/** LoRa task handle */
TaskHandle_t gnss_task_handle;
/** GPS reading task */
void gnss_task(void *pvParameters);

/** Semaphore for GNSS aquisition task */
SemaphoreHandle_t g_gnss_sem;

/** GNSS polling function */
bool poll_gnss(void);

/** Location data as byte array */
tracker_data_s g_tracker_data;

/** Latitude/Longitude value converter */
latLong_s pos_union;

/** Flag if location was found */
volatile bool last_read_ok = false;

/** Flag if GNSS is serial or I2C */
bool i2c_gnss = false;

/** Switch between GNSS on/off (1) and GNSS power save mode (0)*/
#define GNSS_OFF 1

/**
 * @brief Initialize the GNSS
 * 
 */
bool init_gnss(void)
{
	bool gnss_found = false;
	// // Give the module some time to power up
	// delay(2000);

	// Power on the GNSS module
	// pinMode(WB_IO2, OUTPUT);
	// delay(100);
	digitalWrite(WB_IO2, HIGH);

	// Give the module some time to power up
	delay(500);

	if (!my_gnss.begin())
	{
		MYLOG("GNSS", "UBLOX did not answer on I2C, retry on Serial1");
		i2c_gnss = false;
	}
	else
	{
		MYLOG("GNSS", "UBLOX found on I2C");
		i2c_gnss = true;
		gnss_found = true;
		my_gnss.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
	}

	if (!i2c_gnss)
	{
		//Assume that the U-Blox GNSS is running at 9600 baud (the default) or at 38400 baud.
		//Loop until we're in sync and then ensure it's at 38400 baud.
		do
		{
			MYLOG("GNSS", "GNSS: trying 38400 baud");
			Serial1.begin(38400);
			while (!Serial1)
				;
			if (my_gnss.begin(Serial1) == true)
			{
				MYLOG("GNSS", "UBLOX found on Serial1 with 38400");
				my_gnss.setUART1Output(COM_TYPE_UBX); //Set the UART port to output UBX only
				gnss_found = true;

				break;
			}
			delay(100);
			MYLOG("GNSS", "GNSS: trying 9600 baud");
			Serial1.begin(9600);
			while (!Serial1)
				;
			if (my_gnss.begin(Serial1) == true)
			{
				MYLOG("GNSS", "GNSS: connected at 9600 baud, switching to 38400");
				my_gnss.setSerialRate(38400);
				delay(100);
			}
			else
			{
				my_gnss.factoryReset();
				delay(2000); //Wait a bit before trying again to limit the Serial output
			}
		} while (1);
	}

	// my_gnss.setI2COutput(COM_TYPE_UBX);	  //Set the I2C port to output UBX only (turn off NMEA noise)
	// my_gnss.setUART1Output(COM_TYPE_UBX); //Set the UART port to output UBX only
	my_gnss.saveConfiguration(); //Save the current settings to flash and BBR

	my_gnss.setMeasurementRate(500);

	// my_gnss.powerSaveMode(true);

	return gnss_found;
}

/**
 * @brief Check GNSS module for position
 * 
 * @return true Valid position found
 * @return false No valid position
 */
bool poll_gnss(void)
{
	MYLOG("GNSS", "poll_gnss");

#if GNSS_OFF == 1
	// Start connection
	init_gnss();
#endif

	time_t time_out = millis();
	bool has_pos = false;
	int64_t latitude = 0;
	int64_t longitude = 0;
	int32_t altitude = 0;
	int32_t accuracy = 0;

	time_t check_limit = 15000;

	// time_t check_limit = 90000;

	// if (g_lorawan_settings.send_repeat_time == 0)
	// {
	// 	check_limit = 90000;
	// }
	// else if (g_lorawan_settings.send_repeat_time <= 90000)
	// {
	// 	check_limit = g_lorawan_settings.send_repeat_time / 2;
	// }
	// else
	// {
	// 	check_limit = 90000;
	// }

	MYLOG("GNSS", "GNSS timeout %ld", (long int)check_limit);

	while ((millis() - time_out) < check_limit)
	{
		byte fix_type = my_gnss.getFixType(); // Get the fix type
		char fix_type_str[32] = {0};
		if (fix_type == 0)
			sprintf(fix_type_str, "No Fix");
		else if (fix_type == 1)
			sprintf(fix_type_str, "Dead reckoning");
		else if (fix_type == 2)
			sprintf(fix_type_str, "Fix type 2D");
		else if (fix_type == 3)
			sprintf(fix_type_str, "Fix type 3D");
		else if (fix_type == 4)
			sprintf(fix_type_str, "GNSS fix");
		else if (fix_type == 5)
			sprintf(fix_type_str, "Time fix");

		// if (my_gnss.getGnssFixOk()) /** Don't care about accuracy */
		// if ((fix_type >= 3) && (my_gnss.getSIV() >= 5)) /** Fix type 3D and at least 5 satellites */
		if (fix_type >= 3) /** Fix type 3D */
		{
			// digitalWrite(LED_CONN, HIGH);
			has_pos = true;
			last_read_ok = true;
			latitude = my_gnss.getLatitude();
			longitude = my_gnss.getLongitude();
			altitude = my_gnss.getAltitude();
			accuracy = my_gnss.getHorizontalDOP();

			MYLOG("GNSS", "Fixtype: %d %s", my_gnss.getFixType(), fix_type_str);
			MYLOG("GNSS", "Lat: %.4f Lon: %.4f", latitude / 10000000.0, longitude / 10000000.0);
			MYLOG("GNSS", "Alt: %.2f", altitude / 1000.0);
			MYLOG("GNSS", "Acy: %.2f ", accuracy / 100.0);

			pos_union.val32 = latitude / 1000; // Cayenne LPP 0.0001 ° Signed MSB
			g_tracker_data.lat_1 = pos_union.val8[2];
			g_tracker_data.lat_2 = pos_union.val8[1];
			g_tracker_data.lat_3 = pos_union.val8[0];

			pos_union.val32 = longitude / 1000; // Cayenne LPP 0.0001 ° Signed MSB
			g_tracker_data.long_1 = pos_union.val8[2];
			g_tracker_data.long_2 = pos_union.val8[1];
			g_tracker_data.long_3 = pos_union.val8[0];

			pos_union.val32 = altitude / 10; // Cayenne LPP 0.01 meter Signed MSB
			g_tracker_data.alt_1 = pos_union.val8[2];
			g_tracker_data.alt_2 = pos_union.val8[1];
			g_tracker_data.alt_3 = pos_union.val8[0];

			// pos_union.val32 = accuracy;
			// g_tracker_data.acy_1 = pos_union.val8[0];
			// g_tracker_data.acy_2 = pos_union.val8[1];

			// Break the while()
			break;
		}
		else
		{
			delay(1000);
		}
	}

#if GNSS_OFF == 1
	// Power down the module
	digitalWrite(WB_IO2, LOW);
	delay(100);
#endif

	if (has_pos)
	{
#if GNSS_OFF == 0
		my_gnss.setMeasurementRate(10000);
		my_gnss.setNavigationFrequency(1, 10000);
		my_gnss.powerSaveMode(true, 10000);
#endif
		return true;
	}
	else
	{
		// No location found, set the data to 0
		g_tracker_data.lat_1 = 0;
		g_tracker_data.lat_2 = 0;
		g_tracker_data.lat_3 = 0;

		g_tracker_data.long_1 = 0;
		g_tracker_data.long_2 = 0;
		g_tracker_data.long_3 = 0;

		g_tracker_data.alt_1 = 0;
		g_tracker_data.alt_2 = 0;
		g_tracker_data.alt_3 = 0;

		/// \todo Enable below to get a fake GPS position if no location fix could be obtained
		// 	Serial.println("Faking GPS");
		// 	/// \todo  For testing, an address in Recife, Brazil, which has both latitude and longitude negative
		// 	// lat -8.0487740
		// 	// long -34.9021580
		// 	// alt 156.024
		// 	// 14.4213730, 121.0069140, 35.000
		// 	latitude = 144213730;
		// 	longitude = 1210069140;
		// 	altitude = 35000;

		// 	pos_union.val32 = latitude / 1000;
		// 	g_tracker_data.lat_1 = pos_union.val8[2];
		// 	g_tracker_data.lat_2 = pos_union.val8[1];
		// 	g_tracker_data.lat_3 = pos_union.val8[0];

		// 	pos_union.val32 = longitude / 1000;
		// 	g_tracker_data.long_1 = pos_union.val8[2];
		// 	g_tracker_data.long_2 = pos_union.val8[1];
		// 	g_tracker_data.long_3 = pos_union.val8[0];

		// pos_union.val32 = altitude / 10;
		// g_tracker_data.alt_1 = pos_union.val8[2];
		// g_tracker_data.alt_2 = pos_union.val8[1];
		// g_tracker_data.alt_3 = pos_union.val8[0];
	}

	MYLOG("GNSS", "No valid location found");
	last_read_ok = false;

	// digitalWrite(LED_CONN, LOW);

#if GNSS_OFF == 0
	my_gnss.setMeasurementRate(1000);
#endif
	return false;
}

void gnss_task(void *pvParameters)
{
	MYLOG("GNSS", "GNSS Task started");

#if GNSS_OFF == 1
	// Power down the module
	digitalWrite(WB_IO2, LOW);
	delay(100);
#endif

	uint8_t busy_cnt = 0;
	while (1)
	{
		if (xSemaphoreTake(g_gnss_sem, portMAX_DELAY) == pdTRUE)
		{
			MYLOG("GNSS", "GNSS Task wake up");
			if (!lora_busy)
			{
				AT_PRINTF("+EVT:START_LOCATION\n");
				// Get location
				bool got_location = poll_gnss();
				AT_PRINTF("+EVT:LOCATION %s\n", got_location ? "FIX" : "NOFIX");

				// if ((g_task_sem != NULL) && got_location)
				if (g_task_sem != NULL)
				{
					g_task_event_type |= GNSS_FIN;
					xSemaphoreGiveFromISR(g_task_sem, &g_higher_priority_task_woken);
				}
			}
			else
			{
				busy_cnt++;
				if (busy_cnt == 2)
				{
					busy_cnt = 0;
					lora_busy = false;
				}
				AT_PRINTF("+EVT:LOCATION_SKIP\n");
			}
			MYLOG("GNSS", "GNSS Task finished");
		}
	}
}
