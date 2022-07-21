/**
 * @file user_at.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Handle user defined AT commands
 * @version 0.3
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "app.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

/** Filename to save GPS precision setting */
static const char gnss_name[] = "GNSS";

/** Filename to save data format setting */
static const char helium_format[] = "HELIUM";

/** File to save GPS precision setting */
File gps_file(InternalFS);

#define AT_PRINTF(...)                  \
	Serial.printf(__VA_ARGS__);         \
	if (g_ble_uart_is_connected)        \
	{                                   \
		g_ble_uart.printf(__VA_ARGS__); \
	}

/**
 * @brief Returns in g_at_query_buf the current settings for the GNSS precision
 * 
 * @return int always 0
 */
static int at_query_gnss()
{
	if (g_is_helium)
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "GPS precision: 2");
	}
	else
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "GPS precision: %d", g_gps_prec_6 ? 1 : 0);
	}
	return 0;
}

/**
 * @brief Command to set the GNSS precision
 * 
 * @param str Either '0' or '1'
 *  '0' sets the precission to 4 digits
 *  '1' sets the precission to 6 digits
 *  '2' sets the dataformat to Helium Mapper
 * @return int 0 if the command was succesfull, 5 if the parameter was wrong
 */
static int at_exec_gnss(char *str)
{
	if (str[0] == '0')
	{
		g_is_helium = false;
		g_gps_prec_6 = false;
		save_gps_settings();
	}
	else if (str[0] == '1')
	{
		g_is_helium = false;
		g_gps_prec_6 = true;
		save_gps_settings();
	}
	else if (str[0] == '2')
	{
		g_is_helium = true;
		save_gps_settings();
	}
	else
	{
		return AT_ERRNO_PARA_VAL;
	}
	return 0;
}

/**
 * @brief Read saved setting for precision and packet format
 * 
 */
void read_gps_settings(void)
{
	if (InternalFS.exists(gnss_name))
	{
		g_gps_prec_6 = true;
		MYLOG("USR_AT", "File found, set precision to high");
	}
	else
	{
		g_gps_prec_6 = false;
		MYLOG("USR_AT", "File not found, set precision to low");
	}
	if (InternalFS.exists(helium_format))
	{
		g_is_helium = true;
		MYLOG("USR_AT", "File found, set Helium Mapper format");
	}
	else
	{
		g_is_helium = false;
		MYLOG("USR_AT", "File not found, set Cayenne LPP format");
	}
}

/**
 * @brief Save the GPS settings
 * 
 */
void save_gps_settings(void)
{
	if (g_gps_prec_6)
	{
		gps_file.open(gnss_name, FILE_O_WRITE);
		gps_file.write("1");
		gps_file.close();
		MYLOG("USR_AT", "Created File for high precision");
	}
	else
	{
		InternalFS.remove(gnss_name);
		MYLOG("USR_AT", "Remove File for high precision");
	}
	if (g_is_helium)
	{
		gps_file.open(helium_format, FILE_O_WRITE);
		gps_file.write("1");
		gps_file.close();
		MYLOG("USR_AT", "Created File for Helium Mapper format");
	}
	else
	{
		InternalFS.remove(helium_format);
		MYLOG("USR_AT", "Remove File for Helium Mapper format");
	}
}

/**
 * @brief List of all available commands with short help and pointer to functions
 * 
 */
atcmd_t g_user_at_cmd_list_gps[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// GNSS commands
	{"+GNSS", "Get/Set the GNSS precision and format 0 = 4 digit, 1 = 6 digit, 2 = Helium Mapper", at_query_gnss, at_exec_gnss, NULL},
};

/** Number of user defined AT commands */
uint8_t g_user_at_cmd_num = 0;

/** Pointer to the combined user AT command structure */
atcmd_t *g_user_at_cmd_list;

/**
 * @brief Initialize the user defined AT command list
 *
 */
void init_user_at(void)
{
	uint16_t index_next_cmds = 0;
	uint16_t required_structure_size = sizeof(g_user_at_cmd_list_gps);
	MYLOG("USR_AT", "Structure size %d GNSS", required_structure_size);

	// Reserve memory for the structure
	g_user_at_cmd_list = (atcmd_t *)malloc(required_structure_size);

	// Add AT commands to structure
	MYLOG("USR_AT", "Adding GNSS user AT commands");
	g_user_at_cmd_num += sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t);
	memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_gps, sizeof(g_user_at_cmd_list_gps));
	index_next_cmds += sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t);
	MYLOG("USR_AT", "Index after adding GNSS %d", index_next_cmds);
}

// /** Number of user defined AT commands */
// uint8_t g_user_at_cmd_num = sizeof(g_user_at_cmd_list) / sizeof(atcmd_t);