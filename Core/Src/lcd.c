#include "lcd.h"
#include "main.h"
#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_sram.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __has_builtin
#if !__has_builtin(__builtin_memcpy)
#include <string.h>
#endif
#else
#include <string.h>
#endif

LCD_settings_t LCD_settings;

#define LCD_REG_ADDR ((uint32_t)(0x6C000000))
#define LCD_RAM_ADDR ((uint32_t)(0x6C000080))
void LCD_set_register(uint16_t value) {
#ifdef __has_builtin
#if __has_builtin(__builtin_memcpy)
	__builtin_memcpy
#else
	memcpy
#endif
#else
	memcpy
#endif
		((void*)LCD_REG_ADDR, &value, sizeof(uint16_t));
	__ASM volatile ("nop"); // 2 redundent instructions for delay
	__ASM volatile ("nop");
}
void LCD_set_ram(uint16_t value) {
#ifdef __has_builtin
#if __has_builtin(__builtin_memcpy)
	__builtin_memcpy
#else
	memcpy
#endif
#else
	memcpy
#endif
		((void*)LCD_RAM_ADDR, &value, sizeof(uint16_t));
	__ASM volatile ("nop");
	__ASM volatile ("nop");
}
uint16_t LCD_read_register(void) {
	uint16_t retval;
#ifdef __has_builtin
#if __has_builtin(__builtin_memcpy)
	__builtin_memcpy
#else
	memcpy
#endif
#else
	memcpy
#endif
		(&retval, (void*)LCD_REG_ADDR, sizeof(uint16_t));
	return retval;
}
uint16_t LCD_read_ram(void) {
	uint16_t retval;
#ifdef __has_builtin
#if __has_builtin(__builtin_memcpy)
	__builtin_memcpy
#else
	memcpy
#endif
#else
	memcpy
#endif
		(&retval, (void*)LCD_RAM_ADDR, sizeof(uint16_t));
	return retval;
}
void LCD_write(uint16_t reg, uint16_t value) {
	LCD_set_register(reg);
	LCD_set_ram(value);
}
uint16_t LCD_read(uint16_t reg) {
	LCD_set_register(reg);
	return LCD_read_ram();
}

// It doesn't work well with cubemx
void LCD_init(void) {
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_FSMC_CLK_ENABLE();
	// PB15 controls the back light
	GPIO_InitTypeDef GPIOB_InitStruct;
	GPIOB_InitStruct.Pin = GPIO_PIN_15;
	GPIOB_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIOB_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIOB_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIOB_InitStruct);
	// PD0, 1, 4, 5, 8, 9, 10, 14, 15 AF out
	GPIO_InitTypeDef GPIO_FSMCPin_InitStruct;
	GPIO_FSMCPin_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |
								GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 |
								GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_FSMCPin_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_FSMCPin_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_FSMCPin_InitStruct.Pull = GPIO_PULLUP;
	GPIO_FSMCPin_InitStruct.Alternate = GPIO_AF12_FSMC;
	HAL_GPIO_Init(GPIOD, &GPIO_FSMCPin_InitStruct);
	// PE7, 8, 9, 10, 11, 12, 13, 14, 15
	GPIO_FSMCPin_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
								GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
								GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &GPIO_FSMCPin_InitStruct);
	// PF12
	GPIO_FSMCPin_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOF, &GPIO_FSMCPin_InitStruct);
	// PG12
	GPIO_FSMCPin_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOG, &GPIO_FSMCPin_InitStruct);
	// FSMC region1, bank4, first initialization
	FSMC_NORSRAM_TimingTypeDef FSMC_ReadTimingStruct, FSMC_WriteTimingStruct;
	SRAM_HandleTypeDef TFTSRAM_Handler;
	TFTSRAM_Handler.Instance = FSMC_NORSRAM_DEVICE;
	TFTSRAM_Handler.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
	TFTSRAM_Handler.Init.NSBank = FSMC_NORSRAM_BANK4;
	TFTSRAM_Handler.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
	TFTSRAM_Handler.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
	TFTSRAM_Handler.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
	TFTSRAM_Handler.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
	TFTSRAM_Handler.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
	TFTSRAM_Handler.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
	TFTSRAM_Handler.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
	TFTSRAM_Handler.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
	TFTSRAM_Handler.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
	TFTSRAM_Handler.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
	TFTSRAM_Handler.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
	TFTSRAM_Handler.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;

	FSMC_ReadTimingStruct.AddressSetupTime = 15;
	FSMC_ReadTimingStruct.AddressHoldTime = 0;
	FSMC_ReadTimingStruct.DataSetupTime = 60;
	FSMC_ReadTimingStruct.AccessMode = FSMC_ACCESS_MODE_A;

	FSMC_WriteTimingStruct.BusTurnAroundDuration = 0;
	FSMC_WriteTimingStruct.AddressSetupTime = 9;
	FSMC_WriteTimingStruct.AddressHoldTime = 0;
	FSMC_WriteTimingStruct.DataSetupTime = 8;
	FSMC_WriteTimingStruct.AccessMode = FSMC_ACCESS_MODE_A;

	if (HAL_SRAM_Init(&TFTSRAM_Handler, &FSMC_ReadTimingStruct,
					&FSMC_WriteTimingStruct) != HAL_OK)
		Error_Handler();

	HAL_Delay(20);

	LCD_write(0xf000, 0x0055);
	LCD_write(0xf001, 0x00aa);
	LCD_write(0xf002, 0x0052);
	LCD_write(0xf003, 0x0008);
	LCD_write(0xf004, 0x0001);
	uint8_t id_high = LCD_read(0xc500);
	uint8_t id_low = LCD_read(0xc501);
	if (id_high != 0x55 || id_low != 0x10)
		Error_Handler();

	// magic numbers
	{
		LCD_write(0xf000, 0x55);
		LCD_write(0xf001, 0xaa);
		LCD_write(0xf002, 0x52);
		LCD_write(0xf003, 0x08);
		LCD_write(0xf004, 0x01);
		//AVDD Set AVDD 5.2V
		LCD_write(0xb000, 0x0d);
		LCD_write(0xb001, 0x0d);
		LCD_write(0xb002, 0x0d);
		//AVDD ratio
		LCD_write(0xb600, 0x34);
		LCD_write(0xb601, 0x34);
		LCD_write(0xb602, 0x34);
		//AVEE -5.2V
		LCD_write(0xb100, 0x0d);
		LCD_write(0xb101, 0x0d);
		LCD_write(0xb102, 0x0d);
		//AVEE ratio
		LCD_write(0xb700, 0x34);
		LCD_write(0xb701, 0x34);
		LCD_write(0xb702, 0x34);
		//VCL -2.5V
		LCD_write(0xb200, 0x00);
		LCD_write(0xb201, 0x00);
		LCD_write(0xb202, 0x00);
		//VCL ratio
		LCD_write(0xb800, 0x24);
		LCD_write(0xb801, 0x24);
		LCD_write(0xb802, 0x24);
		//VGH 15V (Free pump)
		LCD_write(0xbf00, 0x01);
		LCD_write(0xb300, 0x0f);
		LCD_write(0xb301, 0x0f);
		LCD_write(0xb302, 0x0f);
		//VGH ratio
		LCD_write(0xb900, 0x34);
		LCD_write(0xb901, 0x34);
		LCD_write(0xb902, 0x34);
		//VGL_REG -10V
		LCD_write(0xb500, 0x08);
		LCD_write(0xb501, 0x08);
		LCD_write(0xb502, 0x08);
		LCD_write(0xc200, 0x03);
		//VGLX ratio
		LCD_write(0xba00, 0x24);
		LCD_write(0xba01, 0x24);
		LCD_write(0xba02, 0x24);
		//VGMP/VGSP 4.5V/0V
		LCD_write(0xbc00, 0x00);
		LCD_write(0xbc01, 0x78);
		LCD_write(0xbc02, 0x00);
		//VGMN/VGSN -4.5V/0V
		LCD_write(0xbd00, 0x00);
		LCD_write(0xbd01, 0x78);
		LCD_write(0xbd02, 0x00);
		//VCOM
		LCD_write(0xbe00, 0x00);
		LCD_write(0xbe01, 0x64);
		//Gamma Setting
		LCD_write(0xd100, 0x00);
		LCD_write(0xd101, 0x33);
		LCD_write(0xd102, 0x00);
		LCD_write(0xd103, 0x34);
		LCD_write(0xd104, 0x00);
		LCD_write(0xd105, 0x3a);
		LCD_write(0xd106, 0x00);
		LCD_write(0xd107, 0x4a);
		LCD_write(0xd108, 0x00);
		LCD_write(0xd109, 0x5c);
		LCD_write(0xd10a, 0x00);
		LCD_write(0xd10b, 0x81);
		LCD_write(0xd10c, 0x00);
		LCD_write(0xd10d, 0xa6);
		LCD_write(0xd10e, 0x00);
		LCD_write(0xd10f, 0xe5);
		LCD_write(0xd110, 0x01);
		LCD_write(0xd111, 0x13);
		LCD_write(0xd112, 0x01);
		LCD_write(0xd113, 0x54);
		LCD_write(0xd114, 0x01);
		LCD_write(0xd115, 0x82);
		LCD_write(0xd116, 0x01);
		LCD_write(0xd117, 0xca);
		LCD_write(0xd118, 0x02);
		LCD_write(0xd119, 0x00);
		LCD_write(0xd11a, 0x02);
		LCD_write(0xd11b, 0x01);
		LCD_write(0xd11c, 0x02);
		LCD_write(0xd11d, 0x34);
		LCD_write(0xd11e, 0x02);
		LCD_write(0xd11f, 0x67);
		LCD_write(0xd120, 0x02);
		LCD_write(0xd121, 0x84);
		LCD_write(0xd122, 0x02);
		LCD_write(0xd123, 0xa4);
		LCD_write(0xd124, 0x02);
		LCD_write(0xd125, 0xb7);
		LCD_write(0xd126, 0x02);
		LCD_write(0xd127, 0xcf);
		LCD_write(0xd128, 0x02);
		LCD_write(0xd129, 0xde);
		LCD_write(0xd12a, 0x02);
		LCD_write(0xd12b, 0xf2);
		LCD_write(0xd12c, 0x02);
		LCD_write(0xd12d, 0xfe);
		LCD_write(0xd12e, 0x03);
		LCD_write(0xd12f, 0x10);
		LCD_write(0xd130, 0x03);
		LCD_write(0xd131, 0x33);
		LCD_write(0xd132, 0x03);
		LCD_write(0xd133, 0x6d);
		LCD_write(0xd200, 0x00);
		LCD_write(0xd201, 0x33);
		LCD_write(0xd202, 0x00);
		LCD_write(0xd203, 0x34);
		LCD_write(0xd204, 0x00);
		LCD_write(0xd205, 0x3a);
		LCD_write(0xd206, 0x00);
		LCD_write(0xd207, 0x4a);
		LCD_write(0xd208, 0x00);
		LCD_write(0xd209, 0x5c);
		LCD_write(0xd20a, 0x00);
	
		LCD_write(0xd20b, 0x81);
		LCD_write(0xd20c, 0x00);
		LCD_write(0xd20d, 0xa6);
		LCD_write(0xd20e, 0x00);
		LCD_write(0xd20f, 0xe5);
		LCD_write(0xd210, 0x01);
		LCD_write(0xd211, 0x13);
		LCD_write(0xd212, 0x01);
		LCD_write(0xd213, 0x54);
		LCD_write(0xd214, 0x01);
		LCD_write(0xd215, 0x82);
		LCD_write(0xd216, 0x01);
		LCD_write(0xd217, 0xca);
		LCD_write(0xd218, 0x02);
		LCD_write(0xd219, 0x00);
		LCD_write(0xd21a, 0x02);
		LCD_write(0xd21b, 0x01);
		LCD_write(0xd21c, 0x02);
		LCD_write(0xd21d, 0x34);
		LCD_write(0xd21e, 0x02);
		LCD_write(0xd21f, 0x67);
		LCD_write(0xd220, 0x02);
		LCD_write(0xd221, 0x84);
		LCD_write(0xd222, 0x02);
		LCD_write(0xd223, 0xa4);
		LCD_write(0xd224, 0x02);
		LCD_write(0xd225, 0xb7);
		LCD_write(0xd226, 0x02);
		LCD_write(0xd227, 0xcf);
		LCD_write(0xd228, 0x02);
		LCD_write(0xd229, 0xde);
		LCD_write(0xd22a, 0x02);
		LCD_write(0xd22b, 0xf2);
		LCD_write(0xd22c, 0x02);
		LCD_write(0xd22d, 0xfe);
		LCD_write(0xd22e, 0x03);
		LCD_write(0xd22f, 0x10);
		LCD_write(0xd230, 0x03);
		LCD_write(0xd231, 0x33);
		LCD_write(0xd232, 0x03);
		LCD_write(0xd233, 0x6d);
		LCD_write(0xd300, 0x00);
		LCD_write(0xd301, 0x33);
		LCD_write(0xd302, 0x00);
		LCD_write(0xd303, 0x34);
		LCD_write(0xd304, 0x00);
		LCD_write(0xd305, 0x3a);
		LCD_write(0xd306, 0x00);
		LCD_write(0xd307, 0x4a);
		LCD_write(0xd308, 0x00);
		LCD_write(0xd309, 0x5c);
		LCD_write(0xd30a, 0x00);
	
		LCD_write(0xd30b, 0x81);
		LCD_write(0xd30c, 0x00);
		LCD_write(0xd30d, 0xa6);
		LCD_write(0xd30e, 0x00);
		LCD_write(0xd30f, 0xe5);
		LCD_write(0xd310, 0x01);
		LCD_write(0xd311, 0x13);
		LCD_write(0xd312, 0x01);
		LCD_write(0xd313, 0x54);
		LCD_write(0xd314, 0x01);
		LCD_write(0xd315, 0x82);
		LCD_write(0xd316, 0x01);
		LCD_write(0xd317, 0xca);
		LCD_write(0xd318, 0x02);
		LCD_write(0xd319, 0x00);
		LCD_write(0xd31a, 0x02);
		LCD_write(0xd31b, 0x01);
		LCD_write(0xd31c, 0x02);
		LCD_write(0xd31d, 0x34);
		LCD_write(0xd31e, 0x02);
		LCD_write(0xd31f, 0x67);
		LCD_write(0xd320, 0x02);
		LCD_write(0xd321, 0x84);
		LCD_write(0xd322, 0x02);
		LCD_write(0xd323, 0xa4);
		LCD_write(0xd324, 0x02);
		LCD_write(0xd325, 0xb7);
		LCD_write(0xd326, 0x02);
		LCD_write(0xd327, 0xcf);
		LCD_write(0xd328, 0x02);
		LCD_write(0xd329, 0xde);
		LCD_write(0xd32a, 0x02);
		LCD_write(0xd32b, 0xf2);
		LCD_write(0xd32c, 0x02);
		LCD_write(0xd32d, 0xfe);
		LCD_write(0xd32e, 0x03);
		LCD_write(0xd32f, 0x10);
		LCD_write(0xd330, 0x03);
		LCD_write(0xd331, 0x33);
		LCD_write(0xd332, 0x03);
		LCD_write(0xd333, 0x6d);
		LCD_write(0xd400, 0x00);
		LCD_write(0xd401, 0x33);
		LCD_write(0xd402, 0x00);
		LCD_write(0xd403, 0x34);
		LCD_write(0xd404, 0x00);
		LCD_write(0xd405, 0x3a);
		LCD_write(0xd406, 0x00);
		LCD_write(0xd407, 0x4a);
		LCD_write(0xd408, 0x00);
		LCD_write(0xd409, 0x5c);
		LCD_write(0xd40a, 0x00);
		LCD_write(0xd40b, 0x81);
	
		LCD_write(0xd40c, 0x00);
		LCD_write(0xd40d, 0xa6);
		LCD_write(0xd40e, 0x00);
		LCD_write(0xd40f, 0xe5);
		LCD_write(0xd410, 0x01);
		LCD_write(0xd411, 0x13);
		LCD_write(0xd412, 0x01);
		LCD_write(0xd413, 0x54);
		LCD_write(0xd414, 0x01);
		LCD_write(0xd415, 0x82);
		LCD_write(0xd416, 0x01);
		LCD_write(0xd417, 0xca);
		LCD_write(0xd418, 0x02);
		LCD_write(0xd419, 0x00);
		LCD_write(0xd41a, 0x02);
		LCD_write(0xd41b, 0x01);
		LCD_write(0xd41c, 0x02);
		LCD_write(0xd41d, 0x34);
		LCD_write(0xd41e, 0x02);
		LCD_write(0xd41f, 0x67);
		LCD_write(0xd420, 0x02);
		LCD_write(0xd421, 0x84);
		LCD_write(0xd422, 0x02);
		LCD_write(0xd423, 0xa4);
		LCD_write(0xd424, 0x02);
		LCD_write(0xd425, 0xb7);
		LCD_write(0xd426, 0x02);
		LCD_write(0xd427, 0xcf);
		LCD_write(0xd428, 0x02);
		LCD_write(0xd429, 0xde);
		LCD_write(0xd42a, 0x02);
		LCD_write(0xd42b, 0xf2);
		LCD_write(0xd42c, 0x02);
		LCD_write(0xd42d, 0xfe);
		LCD_write(0xd42e, 0x03);
		LCD_write(0xd42f, 0x10);
		LCD_write(0xd430, 0x03);
		LCD_write(0xd431, 0x33);
		LCD_write(0xd432, 0x03);
		LCD_write(0xd433, 0x6d);
		LCD_write(0xd500, 0x00);
		LCD_write(0xd501, 0x33);
		LCD_write(0xd502, 0x00);
		LCD_write(0xd503, 0x34);
		LCD_write(0xd504, 0x00);
		LCD_write(0xd505, 0x3a);
		LCD_write(0xd506, 0x00);
		LCD_write(0xd507, 0x4a);
		LCD_write(0xd508, 0x00);
		LCD_write(0xd509, 0x5c);
		LCD_write(0xd50a, 0x00);
		LCD_write(0xd50b, 0x81);
	
		LCD_write(0xd50c, 0x00);
		LCD_write(0xd50d, 0xa6);
		LCD_write(0xd50e, 0x00);
		LCD_write(0xd50f, 0xe5);
		LCD_write(0xd510, 0x01);
		LCD_write(0xd511, 0x13);
		LCD_write(0xd512, 0x01);
		LCD_write(0xd513, 0x54);
		LCD_write(0xd514, 0x01);
		LCD_write(0xd515, 0x82);
		LCD_write(0xd516, 0x01);
		LCD_write(0xd517, 0xca);
		LCD_write(0xd518, 0x02);
		LCD_write(0xd519, 0x00);
		LCD_write(0xd51a, 0x02);
		LCD_write(0xd51b, 0x01);
		LCD_write(0xd51c, 0x02);
		LCD_write(0xd51d, 0x34);
		LCD_write(0xd51e, 0x02);
		LCD_write(0xd51f, 0x67);
		LCD_write(0xd520, 0x02);
		LCD_write(0xd521, 0x84);
		LCD_write(0xd522, 0x02);
		LCD_write(0xd523, 0xa4);
		LCD_write(0xd524, 0x02);
		LCD_write(0xd525, 0xb7);
		LCD_write(0xd526, 0x02);
		LCD_write(0xd527, 0xcf);
		LCD_write(0xd528, 0x02);
		LCD_write(0xd529, 0xde);
		LCD_write(0xd52a, 0x02);
		LCD_write(0xd52b, 0xf2);
		LCD_write(0xd52c, 0x02);
		LCD_write(0xd52d, 0xfe);
		LCD_write(0xd52e, 0x03);
		LCD_write(0xd52f, 0x10);
		LCD_write(0xd530, 0x03);
		LCD_write(0xd531, 0x33);
		LCD_write(0xd532, 0x03);
		LCD_write(0xd533, 0x6d);
		LCD_write(0xd600, 0x00);
		LCD_write(0xd601, 0x33);
		LCD_write(0xd602, 0x00);
		LCD_write(0xd603, 0x34);
		LCD_write(0xd604, 0x00);
		LCD_write(0xd605, 0x3a);
		LCD_write(0xd606, 0x00);
		LCD_write(0xd607, 0x4a);
		LCD_write(0xd608, 0x00);
		LCD_write(0xd609, 0x5c);
		LCD_write(0xd60a, 0x00);
		LCD_write(0xd60b, 0x81);
	
		LCD_write(0xd60c, 0x00);
		LCD_write(0xd60d, 0xa6);
		LCD_write(0xd60e, 0x00);
		LCD_write(0xd60f, 0xe5);
		LCD_write(0xd610, 0x01);
		LCD_write(0xd611, 0x13);
		LCD_write(0xd612, 0x01);
		LCD_write(0xd613, 0x54);
		LCD_write(0xd614, 0x01);
		LCD_write(0xd615, 0x82);
		LCD_write(0xd616, 0x01);
		LCD_write(0xd617, 0xca);
		LCD_write(0xd618, 0x02);
		LCD_write(0xd619, 0x00);
		LCD_write(0xd61a, 0x02);
		LCD_write(0xd61b, 0x01);
		LCD_write(0xd61c, 0x02);
		LCD_write(0xd61d, 0x34);
		LCD_write(0xd61e, 0x02);
		LCD_write(0xd61f, 0x67);
		LCD_write(0xd620, 0x02);
		LCD_write(0xd621, 0x84);
		LCD_write(0xd622, 0x02);
		LCD_write(0xd623, 0xa4);
		LCD_write(0xd624, 0x02);
		LCD_write(0xd625, 0xb7);
		LCD_write(0xd626, 0x02);
		LCD_write(0xd627, 0xcf);
		LCD_write(0xd628, 0x02);
		LCD_write(0xd629, 0xde);
		LCD_write(0xd62a, 0x02);
		LCD_write(0xd62b, 0xf2);
		LCD_write(0xd62c, 0x02);
		LCD_write(0xd62d, 0xfe);
		LCD_write(0xd62e, 0x03);
		LCD_write(0xd62f, 0x10);
		LCD_write(0xd630, 0x03);
		LCD_write(0xd631, 0x33);
		LCD_write(0xd632, 0x03);
		LCD_write(0xd633, 0x6d);
		//LV2 Page 0 enable
		LCD_write(0xf000, 0x55);
		LCD_write(0xf001, 0xaa);
		LCD_write(0xf002, 0x52);
		LCD_write(0xf003, 0x08);
		LCD_write(0xf004, 0x00);
		//Display control
		LCD_write(0xb100, 0xcc);
		LCD_write(0xb101, 0x00);
		//Source hold time
		LCD_write(0xb600, 0x05);
		//Gate EQ control
		LCD_write(0xb700, 0x70);
		LCD_write(0xb701, 0x70);
		//Source EQ control (Mode 2)
		LCD_write(0xb800, 0x01);
		LCD_write(0xb801, 0x03);
		LCD_write(0xb802, 0x03);
		LCD_write(0xb803, 0x03);
		//Inversion mode (2-dot)
		LCD_write(0xbc00, 0x02);
		LCD_write(0xbc01, 0x00);
		LCD_write(0xbc02, 0x00);
		//Timing control 4H w/ 4-delay
		LCD_write(0xc900, 0xd0);
		LCD_write(0xc901, 0x02);
		LCD_write(0xc902, 0x50);
		LCD_write(0xc903, 0x50);
		LCD_write(0xc904, 0x50);
		LCD_write(0x3500, 0x00);
		LCD_write(0x3a00, 0x55);  //16-bit/pixel
		LCD_set_register(0x1100);
		LCD_set_register(0x2900);
	}

	// reset ADDSET && DATASET
	FSMC_Bank1E->BWTR[6] &= ~0xff0fu;
	// set ADDSET = 3, set DATASET = 2 * HCLK
	FSMC_Bank1E->BWTR[6] |= 0x0203u;

	LCD_settings.direction = true;
	LCD_display_dir(false);

	// open PB15, the background light
	GPIOB->BSRR = 1u << 15;
}

void LCD_scan_dir(direction_t direction) {
	if (LCD_settings.direction == true) {
		switch (direction) {
		case L2R_U2D: direction = D2U_L2R; break;
		case L2R_D2U: direction = D2U_R2L; break;
		case R2L_U2D: direction = U2D_L2R; break;
		case R2L_D2U: direction = U2D_R2L; break;
		case U2D_L2R: direction = L2R_D2U; break;
		case U2D_R2L: direction = L2R_U2D; break;
		case D2U_L2R: direction = R2L_D2U; break;
		case D2U_R2L: direction = R2L_U2D; break;
#ifdef __has_builtin
#if __has_builtin(__builtin_unreachable)
		default: __builtin_unreachable();
#endif
#endif
		}
	}
	uint8_t regval = direction << 5;
	LCD_write(0x3600, regval);
	LCD_write(0x2a00, 0);
	LCD_write(0x2a01, 0);
	LCD_write(0x2a02, (480 - 1) >> 8);
	LCD_write(0x2a03, (480 - 1) & 0xff);
	LCD_write(0x2b00, 0);
	LCD_write(0x2b01, 0);
	LCD_write(0x2b02, (800 - 1) >> 8);
	LCD_write(0x2b03, (800 - 1) & 0xff);
}
void LCD_display_dir(bool direction) {
	if (direction == LCD_settings.direction)
		return;
	LCD_settings.direction = direction;
	LCD_scan_dir(L2R_U2D);
	LCD_clear(COLOR_WHITE);
}
void LCD_set_cursor(uint16_t x, uint16_t y) {
	LCD_write(0x2a00, x >> 8);
	LCD_write(0x2a01, x & 0x7f);
	LCD_write(0x2b00, y >> 8);
	LCD_write(0x2b01, y & 0x7f);
}
void LCD_clear(uint16_t color) {
	LCD_set_cursor(0, 0);
	LCD_set_register(0x2c00);
	for (uint32_t i = 0; i < 480 * 800; ++i) {
		LCD_set_ram(color);
	}
}
void LCD_draw_point(uint16_t x, uint16_t y, uint16_t color) {
	LCD_set_cursor(x, y);
	LCD_write(0x2c00, color);
}
#include "font.h"
void LCD_display_char(uint16_t x, uint16_t y, uint8_t character, font_size_t font_size, bool transparent) {
	character -= ' ';
	uint8_t width;
	uint8_t size;
	switch (font_size) {
	case FONT_SMALL: width = 12; size = 12; break;
	case FONT_MEDIUM: width = 16; size = 16; break;
	case FONT_LARGE: width = 36; size = 24; break;
	case FONT_VERY_LARGE: width = 128; size = 32; break;
#ifdef __has_builtin
#if __has_builtin(__builtin_unreachable)
	default: __builtin_unreachable();
#endif
#endif
	}
	for (int i = 0; i < width; ++i) {
		uint8_t temp;
		switch (font_size) {
		case FONT_SMALL: temp = asc2_1206[character][i]; break;
		case FONT_MEDIUM: temp = asc2_1608[character][i]; break;
		case FONT_LARGE: temp = asc2_2412[character][i]; break;
		case FONT_VERY_LARGE: temp = asc2_3216[character][i]; break;
#ifdef __has_builtin
#if __has_builtin(__builtin_unreachable)
		default: __builtin_unreachable();
#endif
#endif
		}
		uint16_t y0 = y;
		for (int j = 0; j < 8; ++j) {
			if (temp & 0x80) LCD_draw_point(x, y, 0x0000);
			else if (transparent == false) LCD_draw_point(x, y, 0xffff);
			temp <<= 1;
			++y;
			if (y - y0 == size) {
				y = y0;
				++x;
			}
		}
	}
}
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{
	uint8_t temp, t1, t;
	uint16_t y0 = y;
	uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // 得到字体一个字符对应点阵集所占的字节数
	num = num - ' '; // 得到偏移后的值
	for (t = 0; t < csize; t++) {
		if (size == 12) temp = asc2_1206[num][t];      // 调用1206字体
		else if (size == 16) temp = asc2_1608[num][t]; // 调用1608字体
		else if (size == 24) temp = asc2_2412[num][t]; // 调用2412字体
		else if (size == 32) temp = asc2_3216[num][t]; // 调用3216字体
		else return;                                   // 没有的字库
		for (t1 = 0; t1 < 8; t1++) {
			if (temp & 0x80) LCD_draw_point(x, y, 0x0000);
			else if (mode == 0) LCD_draw_point(x, y, 0xffff);
			temp <<= 1;
			y++;
			if (y >= 800) return; // 超区域了
			if ((y - y0) == size) {
				y = y0;
				x++;
				if (x >= 480) return; // 超区域了
				break;
			}
		}
	}
}
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint8_t *p)
{
    uint8_t x0 = x;
    width += x;
    height += y;
    while ((*p <= '~') && (*p >= ' ')) { // 判断非法字符
        if (x >= width) {
            x = x0;
            y += size;
        }
        if (y >= height) break; // 超区域了
        LCD_ShowChar(x, y, *p, size, 0);
        x += size / 2;
        p++;
    }  
}
#include <stdio.h>
#include <stdarg.h>
__attribute__((format(printf, 6, 7)))
void LCD_PrintString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char const* format, ...) {
	va_list ptr;
	va_start(ptr, format);
	char buffer[100];
	sprintf(buffer, format, ptr);
	LCD_ShowString(x, y, width, height, size, (uint8_t*)buffer);
	va_end(ptr);
}
