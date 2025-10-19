#include "ds18b20_sm.h"

/***************************************************************/

/***************************************************************/

	uint8_t Ds18b20_CRC8				(uint8_t *addr, uint8_t len);
	void 	Read_Scratchpad_Match_ROM	(char * _scratchpad, char * _serial_numb);
	void 	Read_Scratchpad_Skip_ROM	(char * _scratchpad);
	void 	Send_serial					(char * _serial_numb);
	void 	Read_serial					(char * _serial_numb);
	void 	Read_Scratchpad				(char * _scratchpad);
	void 	Send_byte 					(uint8_t _byte);
	void 	Send_bit					(uint8_t _bit);
	uint8_t Read_byte					(void);
	uint8_t Start_strob					(void);
	void 	Dwt_Delay					(uint32_t _delay_u32);
	void 	Set_DQ_Pin_Write			(void);
	void 	Set_DQ_Pin_Read				(void);

/***************************************************************/

	#define 	CHECK_BIT(var, pos) 	(((var) & (1UL << (pos))) != 0)
	//#define 	SET_BIT(var, pos) 		((var) |= (1UL << (pos)))
	#define 	CLR_BIT(var, pos) 		(var &= ~(1UL << (pos)))

/***************************************************************/

uint8_t Ds18b20_CRC8(uint8_t *addr, uint8_t len) {
	uint8_t crc = 0;
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
} /***************************************************************/

int Ds18b20_Get_temp_MatchROM(char * _serial_numb) {
	char scratchpad[9];
	Read_Scratchpad_Match_ROM(scratchpad, _serial_numb);
	return (100 * ((scratchpad[1]<<8) | scratchpad[0]))/16;
} /***************************************************************/

int Ds18b20_Get_Temp_SkipROM(void) {
	char scratchpad[9];
	Read_Scratchpad_Skip_ROM(scratchpad);
//	uint8_t crc1 = scratchpad[8];
//	uint8_t crc2 = DS18B20_CRC8( (uint8_t*)scratchpad ,8);
	int temp_int = (scratchpad[1]<<8) | scratchpad[0];
	if (CHECK_BIT(temp_int, 15)) {
		temp_int = temp_int - 1 - 0xFFFF;
	}
	return (temp_int * 100)/16;
} /***************************************************************/

void Ds18b20_Print_serial_number(void) {
	char serial_number[SERIAL_NUMB_SIZE] = {0};
	DBG2("ds18b20.serial.number: { ");
	Ds18b20_Get_serial_number(serial_number);
	for (int i=0; i<8; i++) {
		DBG2("0x%02X, ", serial_number[i]);
	}
	DBG2("}\r\n");
} /***************************************************************/

void Ds18b20_Get_serial_number(char * _serial_numb) {
	memset(_serial_numb, 0xFF, SERIAL_NUMB_SIZE);
	uint8_t present = Start_strob();
	if (present){
		Send_byte(READ_ROM);	//	READ ROM (read serial number)
		Read_serial(_serial_numb);
	}
} /***************************************************************/

void Read_Scratchpad_Match_ROM(char * _scratchpad, char * _serial_numb){
	memset(_scratchpad, 0xFF, SCRATCHPAD_SIZE);
	uint8_t present = Start_strob();
	if (present){
		Send_byte(MATCH_ROM);		//	MATCH ROM (compare with serial number)
		Send_serial(_serial_numb);
		Send_byte(READ_SCRATCHPAD);	//	Read Scratchpad
		Read_Scratchpad(_scratchpad);
	}
} /***************************************************************/

void Read_Scratchpad_Skip_ROM(char * _scratchpad) {
	memset(_scratchpad, 0xFF, SCRATCHPAD_SIZE);
	uint8_t present = Start_strob();
	if (present){
		Send_byte(SKIP_ROM);		//	SKIP ROM (skip serial number)
		Send_byte(READ_SCRATCHPAD);	//	Read Scratchpad
		Read_Scratchpad(_scratchpad);
	}
}
/***************************************************************/

void Ds18b20_ConvertTemp_MatchROM( char * _serial_numb) {
	uint8_t present = Start_strob();
	if (present) {
		Send_byte(MATCH_ROM);
		Send_serial(_serial_numb);
		Send_byte(CONVERT_TEMP);	//	Read Scratchpad
	}
}
/***************************************************************/

void Ds18b20_ConvertTemp_SkipROM (void) {
	uint8_t present = Start_strob();
	if (present){
		Send_byte(SKIP_ROM);		//	SKIP ROM (skip serial number)
		Send_byte(CONVERT_TEMP);	//	Read Scratchpad
	}
}
/***************************************************************/

uint8_t Read_byte(void) {
	uint8_t read_byte_u8 = 0xFF;
	for (int i = 0; i < BIT_IN_BYTE; i++) {
		Set_DQ_Pin_Write();
		HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_RESET);
		Dwt_Delay(2);
		HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_SET);
		Set_DQ_Pin_Read();
		Dwt_Delay(6);
		GPIO_PinState res = HAL_GPIO_ReadPin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin);
		if (res == GPIO_PIN_RESET){
			CLR_BIT(read_byte_u8,i);
		}
		Dwt_Delay(51);
	}
	return read_byte_u8;
} /***************************************************************/

void Send_byte (uint8_t _byte) {
	for(register uint8_t i = 0; i < BIT_IN_BYTE; i++) {
		Send_bit(CHECK_BIT(_byte, i));
	}
} /***************************************************************/

void Send_bit (uint8_t _bit) {
	Set_DQ_Pin_Write();
	HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_RESET);
	Dwt_Delay(58 - _bit * 50);
	HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_SET);
	Set_DQ_Pin_Read();
	Dwt_Delay(67 + _bit * 50);
}/***************************************************************/

uint8_t Start_strob(void){
	Set_DQ_Pin_Write();
	HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_RESET);
	Dwt_Delay(720);
	HAL_GPIO_WritePin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin, GPIO_PIN_SET);
	Set_DQ_Pin_Read();
	Dwt_Delay(100);
	GPIO_PinState res = HAL_GPIO_ReadPin(DS18B20_DQ_GPIO_Port, DS18B20_DQ_Pin);
	if (res == GPIO_PIN_RESET){
		Dwt_Delay(259);
		return 1; // present
	}
	return 0; // no answer
} /***************************************************************/

void Read_serial(char * _serial_numb) {
	for (int i = 0; i < SERIAL_NUMB_SIZE; i++) {
		_serial_numb[i] = Read_byte();
	}
} /***************************************************************/

void Read_Scratchpad(char * _scratchpad) {
	for (int i = 0; i < SCRATCHPAD_SIZE; i++) {
		_scratchpad[i] = Read_byte();
	}
} /***************************************************************/

void Send_serial( char * _serial_numb) {
	for (int i = 0; i < SERIAL_NUMB_SIZE; i++) {
		Send_byte(_serial_numb[i]);
	}
} /***************************************************************/

void Ds18b20_Init_DWT_Delay (void) {
	DWT_Delay_Init();
} /***************************************************************/

void Dwt_Delay(uint32_t _delay_u32) {
	DWT_Delay_us(_delay_u32);
} /***************************************************************/

void Set_DQ_Pin_Write() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  	= DS18B20_DQ_Pin;
    GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull 	= GPIO_PULLUP;
    GPIO_InitStruct.Speed	= GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_DQ_GPIO_Port, &GPIO_InitStruct);
} /***************************************************************/

void Set_DQ_Pin_Read() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   	= DS18B20_DQ_Pin;
    GPIO_InitStruct.Mode  	= GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull	= GPIO_PULLUP;
    HAL_GPIO_Init(DS18B20_DQ_GPIO_Port, &GPIO_InitStruct);
} /***************************************************************/

/***************************************************************/
/***************************************************************/
