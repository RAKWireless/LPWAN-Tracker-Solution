/**
 * @file user_at.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Handle user defined AT commands
 * @version 0.1
 * @date 2021-12-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "app.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

/** Filename to save GPS precision setting */
static const char gps_name[] = "GPS";

/** File to save GPS precision setting */
File gps_file(InternalFS);

#define AT_PRINTF(...)                  \
	Serial.printf(__VA_ARGS__);         \
	if (g_ble_uart_is_connected)        \
	{                                   \
		g_ble_uart.printf(__VA_ARGS__); \
	}

bool user_at_handler(char *user_cmd, uint8_t cmd_size)
{
	MYLOG("APP", "Received User AT commmand >>%s<< len %d", user_cmd, cmd_size);

	// Get the command itself
	char *param;

	param = strtok(user_cmd, ":");

	// Check if the command is supported
	if (strcmp(param, (const char *)"+GPS?") == 0)
	{
		// Location precision query
		AT_PRINTF("Get/Set the GPS precision 0 = 4 digit, 1 = 6 digit");
		return true;
	}
	if (strcmp(param, (const char *)"+GPS=?") == 0)
	{
		// Location precision query
		AT_PRINTF("GPS precision: %d", g_gps_prec_6 ? 1 : 0);
		return true;
	}
	if (strcmp(param, (const char *)"+GPS=0") == 0)
	{
		// Low location precision requested
		g_gps_prec_6 = false;
		save_gps_settings();
		AT_PRINTF("GPS precision: %d\nOK", g_gps_prec_6 ? 1 : 0);
		return true;
	}
	if (strcmp(param, (const char *)"+GPS=1") == 0)
	{
		// High location precision requested
		g_gps_prec_6 = true;
		save_gps_settings();
		AT_PRINTF("GPS precision: %d\nOK", g_gps_prec_6 ? 1 : 0);
		return true;
	}

	return false;
}

void read_gps_settings(void)
{
	if (InternalFS.exists(gps_name))
	{
		g_gps_prec_6 = true;
		MYLOG("USR_AT", "File found, set precision to high");
	}
	else
	{
		g_gps_prec_6 = false;
		MYLOG("USR_AT", "File not found, set precision to low");
	}
}

void save_gps_settings(void)
{
	if (g_gps_prec_6)
	{
		gps_file.open(gps_name, FILE_O_WRITE);
		gps_file.write("1");
		gps_file.close();
		MYLOG("USR_AT", "Created File for high precision");
	}
	else
	{
		InternalFS.remove(gps_name);
		MYLOG("USR_AT", "Remove File for high precision");
	}
}
