/**
 * @file wisblock_cayenne.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Extend CayenneLPP class with custom channels
 * @version 0.1
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef WISBLOCK_CAYENNE_H
#define WISBLOCK_CAYENNE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <CayenneLPP.h>

#define LPP_GPS4 136 // 3 byte lon/lat 0.0001 °, 3 bytes alt 0.01 meter (Cayenne LPP default)
#define LPP_GPS6 137 // 4 byte lon/lat 0.000001 °, 3 bytes alt 0.01 meter (Customized Cayenne LPP)

// Only Data Size
#define LPP_GPS4_SIZE 9
#define LPP_GPS6_SIZE 11
#define LPP_GPSH_SIZE 14

class WisCayenne : public CayenneLPP
{
public:
	WisCayenne(uint8_t size) : CayenneLPP(size) {}

	uint8_t addGNSS_4(uint8_t channel, uint32_t latitude, uint32_t longitude, uint32_t altitude);
	uint8_t addGNSS_6(uint8_t channel, uint32_t latitude, uint32_t longitude, uint32_t altitude);
	uint8_t addGNSS_H(uint32_t latitude, uint32_t longitude, uint16_t altitude, uint16_t accuracy, uint16_t battery);

private:
};
#endif