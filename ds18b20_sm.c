#include "ds18b20_sm.h"

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
#define SERIALNUMB_SIZE		8

/***************************************************************/

// extern UART_HandleTypeDef huart1;

uint8_t DS18B20_CRC8(uint8_t *addr, uint8_t len);
void DS18b20_ReadScratchpad_MatchROM(char * _scratchpad, char * _serial_numb);
void DS18b20_ReadScratchpad_SkipROM (char * _scratchpad);
void DS18b20_Get_serial_number(char * _serial_numb);

void Send_serial( char * _serial_numb);
void Read_serial(char * _serial_numb);

void Read_Scratchpad(char * _scratchpad);

void Send_byte (uint8_t _byte);
void Send_bit (uint8_t _bit);
uint8_t Read_byte(void);
uint8_t Start_strob(void);

void local_delay_DS18b20(uint32_t _delay_u32);

/***************************************************************/

#define CHECK_BIT(var, pos) (((var) & (1UL << (pos))) != 0)
//#define SET_BIT(var, pos) ((var) |= (1UL << (pos)))
#define CLR_BIT(var, pos) (var &= ~(1UL << (pos)))
/***************************************************************/

uint8_t DS18B20_CRC8(uint8_t *addr, uint8_t len) {
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
}
/***************************************************************/

int DS18b20_Get_temp_MatchROM(char * _serial_numb) {
	char scratchpad[9];
	DS18b20_ReadScratchpad_MatchROM(scratchpad, _serial_numb);
	return (100 * ((scratchpad[1]<<8) | scratchpad[0]))/16;
}
/***************************************************************/

int DS18b20_Get_Temp_SkipROM(void) {
	char scratchpad[9];
	DS18b20_ReadScratchpad_SkipROM(scratchpad);
//	uint8_t crc1 = scratchpad[8];
//	uint8_t crc2 = DS18B20_CRC8( (uint8_t*)scratchpad ,8);
	int temp_int = (scratchpad[1]<<8) | scratchpad[0];
	if (CHECK_BIT(temp_int, 15)) {
		temp_int = temp_int - 1 - 0xFFFF;
	}
	return (temp_int * 100)/16;
}
/***************************************************************/

void DS18b20_Print_serial_number(UART_HandleTypeDef * uart) {
	char DataChar[100];
	char serial_number[8];
	memset(serial_number, 0xFF, SERIALNUMB_SIZE);

	sprintf(DataChar,"ds18b20 ROM:");
	HAL_UART_Transmit(uart, (uint8_t *)DataChar, strlen(DataChar), 100);

	DS18b20_Get_serial_number(serial_number);
	for (int i=0; i<8; i++) {
		sprintf(DataChar," %02X", serial_number[i]);
		HAL_UART_Transmit(uart, (uint8_t *)DataChar, strlen(DataChar), 100);
	}
	sprintf(DataChar,"\r\n");
	HAL_UART_Transmit(uart, (uint8_t *)DataChar, strlen(DataChar), 100);
}
/***************************************************************/
void DS18b20_Get_serial_number(char * _serial_numb){
	memset(_serial_numb, 0xFF, SERIALNUMB_SIZE);
	uint8_t present = Start_strob();
	if (present){
		Send_byte(READ_ROM);	//	READ ROM (read serial number)
		Read_serial(_serial_numb);
	}
}
/***************************************************************/

void DS18b20_ReadScratchpad_MatchROM(char * _scratchpad, char * _serial_numb){
	memset(_scratchpad, 0xFF, SCRATCHPAD_SIZE);
	uint8_t present = Start_strob();
	if (present){
		Send_byte(MATCH_ROM);	//	MATCH ROM (compare with serial number)
		Send_serial(_serial_numb);
		Send_byte(READ_SCRATCHPAD);	//	Read Scratchpad
		Read_Scratchpad(_scratchpad);
	}
}
/***************************************************************/

void DS18b20_ReadScratchpad_SkipROM(char * _scratchpad){
	memset(_scratchpad, 0xFF, SCRATCHPAD_SIZE);
	uint8_t present = Start_strob();
	if (present){
		Send_byte(SKIP_ROM);	//	SKIP ROM (skip serial number)
		Send_byte(READ_SCRATCHPAD);	//	Read Scratchpad
		Read_Scratchpad(_scratchpad);
	}
}
/***************************************************************/

void DS18b20_ConvertTemp_MatchROM( char * _serial_numb){
	uint8_t present = Start_strob();
	if (present){
		Send_byte(MATCH_ROM);
		Send_serial(_serial_numb);
		Send_byte(CONVERT_TEMP);	//	Read Scratchpad
	}
}
/***************************************************************/

void DS18b20_ConvertTemp_SkipROM (void) {
	uint8_t present = Start_strob();
	if (present){
		Send_byte(SKIP_ROM);	//	SKIP ROM (skip serial number)
		Send_byte(CONVERT_TEMP);	//	Read Scratchpad
	}
}
/***************************************************************/

uint8_t Read_byte(void) {
	uint8_t read_byte_u8 = 0xFF;
	for (int i = 0; i < BIT_IN_BYTE; i++) {

		HAL_GPIO_WritePin(DQ_WRITE_GPIO_Port, DQ_WRITE_Pin, GPIO_PIN_RESET);
		local_delay_DS18b20(20);
		HAL_GPIO_WritePin(DQ_WRITE_GPIO_Port, DQ_WRITE_Pin, GPIO_PIN_SET);

		local_delay_DS18b20(80);

		GPIO_PinState res = HAL_GPIO_ReadPin(DQ_READ_GPIO_Port, DQ_READ_Pin);

		if (res == GPIO_PIN_RESET){
			CLR_BIT(read_byte_u8,i);
		}

		local_delay_DS18b20(200);
	}
	return read_byte_u8;
}
/***************************************************************/

void Send_byte (uint8_t _byte)
{
	for(register uint8_t i = 0; i < BIT_IN_BYTE; i++) {
		Send_bit(CHECK_BIT(_byte, i));
	}
}
/***************************************************************/

void Send_bit (uint8_t _bit) {
	HAL_GPIO_WritePin(DQ_WRITE_GPIO_Port, DQ_WRITE_Pin, GPIO_PIN_RESET);
	local_delay_DS18b20(350 - _bit * 300);
	HAL_GPIO_WritePin(DQ_WRITE_GPIO_Port, DQ_WRITE_Pin, GPIO_PIN_SET);
	local_delay_DS18b20(400 + _bit * 300);
}
/***************************************************************/
uint8_t Start_strob(void){
	HAL_GPIO_WritePin(DQ_WRITE_GPIO_Port, DQ_WRITE_Pin, GPIO_PIN_RESET);
	local_delay_DS18b20(5000);
	HAL_GPIO_WritePin(DQ_WRITE_GPIO_Port, DQ_WRITE_Pin, GPIO_PIN_SET);

	local_delay_DS18b20(600);
	GPIO_PinState res = HAL_GPIO_ReadPin(DQ_READ_GPIO_Port, DQ_READ_Pin);

	if (res == GPIO_PIN_RESET){
		local_delay_DS18b20(1555);
		return 1;
	}
	return 0;
}
/***************************************************************/

void Read_serial(char * _serial_numb){
	for (int i = 0; i < SERIALNUMB_SIZE; i++) {
		_serial_numb[i] = Read_byte();
	}
}
/***************************************************************/

void Read_Scratchpad(char * _scratchpad){
	for (int i = 0; i < SCRATCHPAD_SIZE; i++) {
		_scratchpad[i] = Read_byte();
	}
}
/***************************************************************/

void Send_serial( char * _serial_numb){
	for (int i = 0; i < SERIALNUMB_SIZE; i++) {
		Send_byte(_serial_numb[i]);
	}
}
/***************************************************************/

void local_delay_DS18b20(uint32_t _delay_u32) {
	_delay_u32 = (_delay_u32 * 300) / COEFFICIENT;
	for (; _delay_u32 > 0; _delay_u32--) {
		__asm("nop");
	}
}
/***************************************************************/
