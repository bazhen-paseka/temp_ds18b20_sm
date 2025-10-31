
#ifndef	TEMP_DS18B20_SM_H_INCLUDED
#define	TEMP_DS18B20_SM_H_INCLUDED

	#include <string.h>
	#include <stdio.h>
	#include "main.h"
	#include "debug_gx.h"
	#include "local_config.h"

/***************************************************************/

	#define	READ_ROM			0x33
	#define	MATCH_ROM			0x55
	#define	SKIP_ROM			0xCC
	#define	ALARM_SEARCH		0xEC
	#define	SEARCH_ROM			0xF0
	#define	CONVERT_TEMP		0x44
	#define WRITE_SCRATCHPAD	0x4E
	#define READ_SCRATCHPAD		0xBE
	#define COPY_SCRATCHPAD		0x48
	#define	RECALL_E2			0xB8
	#define READ_POWER_SUPPLY	0xB4

	#define BIT_IN_BYTE			8
	#define SCRATCHPAD_SIZE		9
	#define SERIAL_NUMB_SIZE	8

/***************************************************************/

	void	Ds18b20_Init_DWT_Delay			(void);	// see debug_gx.h
	void 	Ds18b20_Get_serial_number		(uint8_t* _serial_numb);
	void 	Ds18b20_Print_serial_number		(void);
	void	Ds18b20_ConvertTemp_SkipROM 	(void);
	int 	Ds18b20_Get_Temp_SkipROM 		(void);
	void 	Ds18b20_ConvertTemp_MatchROM	(uint8_t* _serial_numb);
	int 	Ds18b20_Get_temp_MatchROM		(uint8_t* _serial_numb);

/***************************************************************/
/***************************************************************/

// 		!!!		Якщо не буде працювати (видавати 0xFF), то треба перевірити таймінг in Read_byte() 	!!!

//		Power= 3V3 or 5V
//		DS18B20_DQ		PB13	GPIO_Output;	Open Drain;
//		DS18B20_3V3		PB12	GPIO_Input;		Pull_up; Допоміжне, якщо немаж піддтяжки до 3V3

//		EXAMPLE:
//		uint8_t serial_number[8] = {0x28, 0xFF, 0x55, 0x64, 0x4C, 0x04, 0x00, 0x20};
//		Ds18b20_Init_DWT_Delay();
//		Ds18b20_Print_serial_number();
//		Ds18b20_Get_serial_number(serial_number);

#endif	//	TEMP_DS18B20_SM_H_INCLUDED
