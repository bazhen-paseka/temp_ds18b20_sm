
#ifndef	TEMP_DS18B20_SM_H_INCLUDED
#define	TEMP_DS18B20_SM_H_INCLUDED

	#include "main.h"
	#include <string.h>
	#include <stdio.h>
/***************************************************************/

int DS18b20_Get_temp_MatchROM(char * _serial_numb);
int DS18b20_Get_Temp_SkipROM (void);

uint8_t DS18B20_CRC8(uint8_t *addr, uint8_t len);

void DS18b20_ConvertTemp_MatchROM( char * _serial_numb);
void DS18b20_ConvertTemp_SkipROM (void);
void DS18b20_ReadScratchpad_MatchROM(char * _scratchpad, char * _serial_numb);
void DS18b20_ReadScratchpad_SkipROM (char * _scratchpad);

void DS18b20_Get_serial_number(char * _serial_numb);
void DS18b20_Print_serial_number(UART_HandleTypeDef * uart);

/***************************************************************/
/***************************************************************/

//	DQ_WRITE	GPIO_Output;	Pull_up;	Open Drain;		level:High;		speed:High;		PA1
//	DQ_READ		GPIO_Input;		Pull_up;													PA0

//		char serial_number[8] = {0x28, 0xFF, 0x55, 0x64, 0x4C, 0x04, 0x00, 0x20};
//		char serial_number[8];
//		DS18b20_Print_serial_number(&huart1);

#endif	//	TEMP_DS18B20_SM_H_INCLUDED
