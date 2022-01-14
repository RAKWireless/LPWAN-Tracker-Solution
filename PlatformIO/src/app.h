/**
 * @file app.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief For application specific includes and definitions
 *        Will be included from main.h
 * @version 0.2
 * @date 2021-12-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef APP_H
#define APP_H

#include <Arduino.h>
/** Add you required includes after Arduino.h */
#include <Wire.h>

/** Include the SX126x-API */
#include <WisBlock-API.h> // Click to install library: http://librarymanager/All#WisBlock-API

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

/** Examples for application events */
#define ACC_TRIGGER 0b1000000000000000
#define N_ACC_TRIGGER 0b0111111111111111
#define GNSS_FIN 0b0100000000000000
#define N_GNSS_FIN 0b1011111111111111

/** Application stuff */
extern BaseType_t g_higher_priority_task_woken;

/** Accelerometer stuff */
#include <SparkFunLIS3DH.h>
#define INT1_PIN WB_IO3
bool init_acc(void);
void clear_acc_int(void);
void read_acc(void);

// GNSS functions
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
bool init_gnss(void);
bool poll_gnss(void);
void gnss_task(void *pvParameters);
extern SemaphoreHandle_t g_gnss_sem;
extern TaskHandle_t gnss_task_handle;
extern volatile bool last_read_ok;

/** Temperature + Humidity stuff */
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
bool init_bme(void);
bool read_bme(void);
void start_bme(void);
extern bool has_env_sensor;

// LoRaWan functions
/** Packet definition for low precision */
struct tracker_data_short_s
{
	uint8_t data_flag1 = 0x01; // 1 Cayenne LPP channel
	uint8_t data_flag2 = 0x88; // 2 Cayenne LPP location
	uint8_t lat_1 = 0;		   // 3
	uint8_t lat_2 = 0;		   // 4
	uint8_t lat_3 = 0;		   // 5
	uint8_t long_1 = 0;		   // 6
	uint8_t long_2 = 0;		   // 7
	uint8_t long_3 = 0;		   // 8
	uint8_t alt_1 = 0;		   // 9
	uint8_t alt_2 = 0;		   // 10
	uint8_t alt_3 = 0;		   // 11
};
extern tracker_data_short_s g_tracker_data_s;
#define TRACKER_DATA_SHORT_LEN 11 // sizeof(g_tracker_data)

/** Packet definition for high precision */
struct tracker_data_prec_s
{
	uint8_t data_flag1 = 0x01; // 1 Cayenne LPP channel
	uint8_t data_flag2 = 0x89; // 2 Cayenne LPP location
	uint8_t lat_1 = 0;		   // 3
	uint8_t lat_2 = 0;		   // 4
	uint8_t lat_3 = 0;		   // 5
	uint8_t lat_4 = 0;		   // 6
	uint8_t long_1 = 0;		   // 7
	uint8_t long_2 = 0;		   // 8
	uint8_t long_3 = 0;		   // 9
	uint8_t long_4 = 0;		   // 10
	uint8_t alt_1 = 0;		   // 11
	uint8_t alt_2 = 0;		   // 12
	uint8_t alt_3 = 0;		   // 13
};
extern tracker_data_prec_s g_tracker_data_l;
#define TRACKER_DATA_PREC_LEN 13 // sizeof(g_tracker_data)

/** Packet definition for low precision */
struct env_data_s
{
	uint8_t data_flag1 = 0x02;	// 1  Cayenne LPP channel
	uint8_t data_flag2 = 0x02;	// 2  Cayenne LPP analog value battery
	uint8_t batt_1 = 0;			// 3
	uint8_t batt_2 = 0;			// 4
	uint8_t data_flag3 = 0x03;	// 5  Cayenne LPP channel
	uint8_t data_flag4 = 0x68;	// 6  Cayenne LPP humidity
	uint8_t humid_1 = 0;		// 7
	uint8_t data_flag5 = 0x04;	// 8  Cayenne LPP channel
	uint8_t data_flag6 = 0x67;	// 9  Cayenne LPP temperature
	uint8_t temp_1 = 0;			// 10
	uint8_t temp_2 = 0;			// 11
	uint8_t data_flag7 = 0x05;	// 12 Cayenne LPP channel
	uint8_t data_flag8 = 0x73;	// 13 Cayenne LPP barometric pressure
	uint8_t press_1 = 0;		// 14
	uint8_t press_2 = 0;		// 15
	uint8_t data_flag9 = 0x06;	// 16 Cayenne LPP channel
	uint8_t data_flag10 = 0x02; // 17 Cayenne LPP analog value gas resistence
	uint8_t gas_1 = 0;			// 18
	uint8_t gas_2 = 0;			// 19
};
extern env_data_s g_env_data;
#define ENVIRONMENT_DATA_LEN 19 // sizeof(g_tracker_data)

extern uint8_t g_last_fport;
extern volatile bool lora_busy;

extern bool g_gps_prec_6;

void read_gps_settings(void);
void save_gps_settings(void);

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