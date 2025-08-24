/**
 * @file app.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief For application specific includes and definitions
 *        Will be included from main.h
 * @version 0.3
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef APP_H
#define APP_H

//**********************************************/
//** Set the application firmware version here */
//**********************************************/
// ; major version increase on API change / not backwards compatible
#ifndef SW_VERSION_1
#define SW_VERSION_1 1
#endif
// ; minor version increase on API change / backward compatible
#ifndef SW_VERSION_2
#define SW_VERSION_2 1
#endif
// ; patch version increase on bugfix, no affect on API
#ifndef SW_VERSION_3
#define SW_VERSION_3 2
#endif

#include <Arduino.h>
/** Add you required includes after Arduino.h */
#include <Wire.h>

/** Include the SX126x-API */
#include <WisBlock-API-V2.h> // Click to install library: http://librarymanager/All#WisBlock-API-V2

// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 0
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)                     \
	do                                      \
	{                                       \
		if (tag)                            \
			PRINTF("[%s] ", tag);           \
		PRINTF(__VA_ARGS__);                \
		PRINTF("\n");                       \
		if (g_ble_uart_is_connected)        \
		{                                   \
			g_ble_uart.printf(__VA_ARGS__); \
			g_ble_uart.printf("\n");        \
		}                                   \
	} while (0)
#else
#define MYLOG(...)
#endif

/** Application function definitions */
void setup_app(void);
bool init_app(void);
void app_event_handler(void);
void ble_data_handler(void) __attribute__((weak));
void lora_data_handler(void);

/** Application stuff */
/** Examples for application events */
#define ACC_TRIGGER 0b1000000000000000
#define N_ACC_TRIGGER 0b0111111111111111
#define GNSS_FIN 0b0100000000000000
#define N_GNSS_FIN 0b1011111111111111

/** Accelerometer stuff */
#include <SparkFunLIS3DH.h>
#define INT1_PIN WB_IO1 // Slot A or WB_IO3 // Slot C or WB_IO5 // Slot D or WB_IO3 // Slot C or
bool init_acc(void);
void clear_acc_int(void);
void read_acc(void);
extern bool g_submit_acc;
extern bool acc_ok;

// GNSS functions
#define NO_GNSS_INIT 0
#define RAK1910_GNSS 1
#define RAK12500_GNSS 2
#include "TinyGPS++.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
bool init_gnss(void);
bool poll_gnss(void);
void gnss_task(void *pvParameters);
extern SemaphoreHandle_t g_gnss_sem;
extern TaskHandle_t gnss_task_handle;
extern volatile bool last_read_ok;
extern uint8_t gnss_option;
extern bool gnss_ok;
extern bool g_loc_high_prec;

/** Temperature + Humidity stuff */
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
bool init_bme(void);
bool read_bme(void);
void start_bme(void);
extern bool has_env_sensor;

// LoRaWan functions
#include <wisblock_cayenne.h>
extern WisCayenne g_data_packet;
// #define LPP_CHANNEL_GPS 10
// #define LPP_CHANNEL_BATT 1
// #define LPP_CHANNEL_HUMID 6
// #define LPP_CHANNEL_TEMP 7
// #define LPP_CHANNEL_PRESS 8
// #define LPP_CHANNEL_GAS 9
#define LPP_ACC 64

extern uint8_t g_last_fport;

extern bool g_gps_prec_6;
extern bool g_is_helium;

void read_gps_settings(void);
void save_gps_settings(void);
void read_batt_settings(void);
void save_batt_settings(bool check_batt_enables);

void init_user_at(void);

extern bool battery_check_enabled;

/** Battery level uinion */
union batt_s
{
	uint16_t batt16 = 0;
	uint8_t batt8[2];
};
/** Latitude/Longitude value union */
union latLong_s
{
	uint32_t val32;
	uint8_t val8[4];
};

#endif
