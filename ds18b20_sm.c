#include "ds18b20_sm.h"

/***************************************************************/

/***************************************************************/

	char DataChar[100];

/***************************************************************/

#define CHECK_BIT(var, pos) (((var) & (1UL << (pos))) != 0)
//#define SET_BIT(var, pos) ((var) |= (1UL << (pos)))
#define CLR_BIT(var, pos) (var &= ~(1UL << (pos)))

/***************************************************************/
void DS18b20_Delay(unsigned int t) {
	for (; t > 0; t--) {
		__asm("nop");
	}
}
/***************************************************************/

uint8_t DS18b20_Read_byte(void) {
	uint8_t ds_res = 0xFF;
	for (int i = 0; i< 8; i++) {
		HAL_GPIO_WritePin(DQ2_GPIO_Port, DQ2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
		DS18b20_Delay(20);
		HAL_GPIO_WritePin(DQ2_GPIO_Port, DQ2_Pin, GPIO_PIN_SET);

		DS18b20_Delay(80);
		GPIO_PinState res = HAL_GPIO_ReadPin(DQ1_GPIO_Port, DQ1_Pin);
		HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
		if (res == GPIO_PIN_RESET){
			CLR_BIT(ds_res,i);
		}
		DS18b20_Delay(200);
	}
	return ds_res;
}
/***************************************************************/

void DS18b20_Send_byte (uint8_t _byte)
{
	for(register uint8_t i = 0; i < 8; i++)        {
		DS18b20_Send_bit(CHECK_BIT(_byte, i));
	}
}
/***************************************************************/

void DS18b20_Send_bit (uint8_t _bit) {
	HAL_GPIO_WritePin(DQ2_GPIO_Port, DQ2_Pin, GPIO_PIN_RESET);
	DS18b20_Delay(50 + _bit*300);
	HAL_GPIO_WritePin(DQ2_GPIO_Port, DQ2_Pin, GPIO_PIN_SET);
	DS18b20_Delay(700 - _bit*300);
}
/***************************************************************/
uint8_t DS18b20_Start_strob(void){
	HAL_GPIO_WritePin(DQ2_GPIO_Port, DQ2_Pin, GPIO_PIN_RESET);
	DS18b20_Delay(5000);
	HAL_GPIO_WritePin(DQ2_GPIO_Port, DQ2_Pin, GPIO_PIN_SET);
	HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
	DS18b20_Delay(600);
	GPIO_PinState res = HAL_GPIO_ReadPin(DQ1_GPIO_Port, DQ1_Pin);
	HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
	if (res == GPIO_PIN_RESET){
		return 1;
	}
	return 0;
}
/***************************************************************/
/***************************************************************/
/***************************************************************/
/***************************************************************/


