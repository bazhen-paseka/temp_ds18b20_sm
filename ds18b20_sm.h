
#ifndef	TEMP_DS18B20_SM_H_INCLUDED
#define	TEMP_DS18B20_SM_H_INCLUDED

	#include "main.h"
	#include <string.h>
	#include <stdio.h>
/***************************************************************/

/***************************************************************/
void DS18b20_Read_scratchpad(char * _scratchpad, char * _serial_numb);
void DS18b20_Get_serial_number(char * _serial_numb);
void DS18b20_Send_byte (uint8_t _byte);
void DS18b20_Send_bit (uint8_t _bit);
uint8_t DS18b20_Read_byte(void);
uint8_t DS18b20_Start_strob(void);
void DS18b20_Delay(unsigned int t);

/***************************************************************/

/***************************************************************/

/***************************************************************/


#endif	//	TEMP_DS18B20_SM_H_INCLUDED
