#if 0

#include "cmsis_os.h"
#include "lcd.h"
#include "main.h"
#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_sram.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __has_builtin
#if !__has_builtin(__builtin_memcpy)
#include <string.h>
#endif
#else
#include <string.h>
#endif

LCD_settings_t LCD_settings;

/**
 * @brief Helper functions set/read the register/ram
 *        They are not intended to be exported, but also not have to be static
 */
#define LCD_REG_ADDR ((uint32_t)(0x6C000000))
#define LCD_RAM_ADDR ((uint32_t)(0x6C000080))
/**
 * @brief Send the value as command.
 * @param value : The value to be sent
 */
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
    // 2 redundent instructions for delay
    // I don't know why, please figure out which command needs the redundency.
    // (I may guess that read commands needs. that is, in LCD_read)
    __NOP(); __NOP();
}
/**
 * @brief Send the value as data.
 * @param value : the value to be sent
 */
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
    __NOP(); __NOP();
}
/**
 * @brief Receive the value and return.
 * @return uint16_t : the value received
 */
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
/**
 * @brief Send the command first, then its data.
 * @param reg : the command
 * @param value : its data
 */
void LCD_write(uint16_t reg, uint16_t value) {
    LCD_set_register(reg);
    LCD_set_ram(value);
}
/**
 * @brief Send the command first, then get its returned data.
 * @param reg : the command
 * @return uint16_t : received data
 */
uint16_t LCD_read(uint16_t reg) {
    LCD_set_register(reg);
    return LCD_read_ram();
}

// There're 2 weak definitions of this function. To avoid conflicts, define a strong symbol. 
void HAL_SRAM_MspInit(SRAM_HandleTypeDef* sramHandle) {}
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

    // query the screen type, assert to be (NT3)5510.
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
        LCD_write(0x3a00, 0x55); 
        //16-bit/pixel
        LCD_set_register(0x1100);
        LCD_set_register(0x2900);
    }

    // reset ADDSET && DATASET
    FSMC_Bank1E->BWTR[6] &= ~0xff0fu;
    // set ADDSET = 3, set DATASET = 2 * HCLK
    FSMC_Bank1E->BWTR[6] |= 0x0203u;
    // these two steps are aimed to speed up the FSMC

    LCD_settings.direction = true;
    LCD_display_dir(false);

    // open PB15, the background light
    GPIOB->BSRR = 1u << 15;
}

/**
 * @brief Change the direction, according to which the LCD scans
 *        You're not intended to call it, because it has no effect on what it shows
 * @param direction : the scanning direction
 */
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
    // Send the commands 8bits by 8bits.
    // Is it possible to send it 16bits at a time?
    // Define the scan region, that is the whole screen 480 * 800
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
    uint16_t x_max, y_max;
    if (LCD_settings.direction == false) {
        x_max = 480;
        y_max = 800;
    }
    else {
        x_max = 800;
        y_max = 480;
    }
    // width is how many bits the char holds in the font library
    // size is the 
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
            if (y < y_max) {
                if (temp & 0x80) LCD_draw_point(x, y, 0x0000);
                else if (transparent == false) LCD_draw_point(x, y, 0xffff);
            }
            temp <<= 1;
            ++y;
            if (y - y0 == size) {
                y = y0;
                ++x;
                if (x >= x_max) return;
            }
        }
    }
}
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    uint16_t x_max, y_max;
    if (LCD_settings.direction == false) {
        x_max = 480;
        y_max = 800;
    }
    else {
        x_max = 800;
        y_max = 480;
    }
    num = num - ' ';
    for (t = 0; t < csize; t++) {
        if (size == 12) temp = asc2_1206[num][t];
        else if (size == 16) temp = asc2_1608[num][t];
        else if (size == 24) temp = asc2_2412[num][t];
        else if (size == 32) temp = asc2_3216[num][t];
        else return;
        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80) LCD_draw_point(x, y, 0x0000);
            else if (mode == 0) LCD_draw_point(x, y, 0xffff);
            temp <<= 1;
            y++;
            if (y >= y_max) return;
            if ((y - y0) == size) {
                y = y0;
                x++;
                if (x >= x_max) return;
                break;
            }
        }
    }
}
int LCD_print_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, font_size_t font_size, uint8_t *p)
{
    int retval = 0;
    uint8_t x0 = x;
    uint8_t size;
    switch (font_size) {
    case FONT_SMALL: size = 12; break;
    case FONT_MEDIUM: size = 16; break;
    case FONT_LARGE: size = 24; break;
    case FONT_VERY_LARGE: size = 32; break;
#ifdef __has_builtin
#if __has_builtin(__builtin_unreachable)
    default: __builtin_unreachable();
#endif
#endif
    }
    width += x;
    height += y;
    for(; *p != 0; ++p, ++retval) {
        if ((*p < ' ') || (*p > '~')) break;
        if (x >= width) {
            x = x0;
            y += size;
        }
        if (y >= height) break;
        LCD_display_char(x, y, *p, font_size, false);
        x += size / 2;
    }
    return retval;
}
#include <stdio.h>
#include <stdarg.h>
__attribute__((format(printf, 6, 7)))
int LCD_print_format_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, font_size_t font_size, char const* format, ...) {
    va_list ptr;
    va_start(ptr, format);
    char buffer[100];
    sprintf(buffer, format, ptr);
    int retval = LCD_print_string(x, y, width, height, font_size, (uint8_t*)buffer);
    va_end(ptr);
    return retval;
}

#else

#include "lcd.h"
#include "font.h"

//////////////////////////////////////////////////////////////////////////////////
// ?????????????????????????????????????????????????????????????????????????????????
// ALIENTEK STM32?????????
// 2.8???/3.5???/4.3???/7??? TFT????????????
// ????????????IC????????????:ILI9341/NT35310/NT35510/SSD1963???
// ????????????@ALIENTEK
// ????????????:www.openedv.com
// ????????????:2017/4/8
// ?????????V4.2
// ??????????????????????????????
// Copyright(C) ??????????????????????????????????????? 2014-2024
// All rights reserved
//*******************************************************************************
// V4.0 20211111
// 1?????????????????????????????????IC?????????
// 2????????????ST7789??????IC?????????
// 3???????????????????????????????????????
// V4.1 20211208
// ??????NT5510
// ID????????????,?????????????????????,????????????C500???C501,?????????????????????ID(0X5510) V4.2
// 20211222 1?????????FSMC???????????????7789??????OV2640?????? 2????????????NT5510
// ID???????????????C501???????????????SSD1963???????????????????????????????????????ID????????????????????????
//////////////////////////////////////////////////////////////////////////////////

SRAM_HandleTypeDef TFTSRAM_Handler; // SRAM??????(????????????LCD)

// LCD???????????????????????????
uint32_t POINT_COLOR = 0xFF000000; // ????????????
uint32_t BACK_COLOR = 0xFFFFFFFF;  // ?????????

// ??????LCD????????????
// ???????????????
_lcd_dev lcddev;

// ??????????????????
// regval:????????????
void LCD_WR_REG(uint16_t volatile regval) {
    regval = regval;       // ??????-O2???????????????,?????????????????????
    LCD->LCD_REG = regval; // ??????????????????????????????
}

// ???LCD??????
// data:???????????????
void LCD_WR_DATA(uint16_t volatile data) {
    data = data; // ??????-O2???????????????,?????????????????????
    LCD->LCD_RAM = data;
}

// ???LCD??????
// ?????????:????????????
uint16_t LCD_RD_DATA(void) {
    uint16_t volatile ram; // ???????????????
    ram = LCD->LCD_RAM;
    return ram;
}

// ????????????
// LCD_Reg:???????????????
// LCD_RegValue:??????????????????
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue) {
    LCD->LCD_REG = LCD_Reg;      // ??????????????????????????????
    LCD->LCD_RAM = LCD_RegValue; // ????????????
}

// ????????????
// LCD_Reg:???????????????
// ?????????:???????????????
uint16_t LCD_ReadReg(uint16_t LCD_Reg) {
    LCD_WR_REG(LCD_Reg);  // ??????????????????????????????
    return LCD_RD_DATA(); // ??????????????????
}

// ?????????GRAM
void LCD_WriteRAM_Prepare(void) { LCD->LCD_REG = lcddev.wramcmd; }

// LCD???GRAM
// RGB_Code:?????????
void LCD_WriteRAM(uint16_t RGB_Code) {
    LCD->LCD_RAM = RGB_Code; // ????????????GRAM
}

// ???ILI93xx??????????????????GBR????????????????????????????????????RGB?????????
// ?????????????????????
// c:GBR??????????????????
// ????????????RGB??????????????????
uint16_t LCD_BGR2RGB(uint16_t c) {
    uint16_t r, g, b, rgb;
    b = (c >> 0) & 0x1f;
    g = (c >> 5) & 0x3f;
    r = (c >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return (rgb);
}

// ???mdk -O1???????????????????????????
// ??????i
void opt_delay(uint8_t i) {
    while (i--)
        ;
}

// ???????????????????????????
// x,y:??????
// ?????????:???????????????
uint32_t LCD_ReadPoint(uint16_t x, uint16_t y) {
    uint16_t r = 0, g = 0, b = 0;
    if (x >= lcddev.width || y >= lcddev.height)
        return 0; // ???????????????,????????????
    LCD_SetCursor(x, y);
    if (lcddev.id == 0X5510) // 5510 ?????????GRAM??????
    {
        LCD_WR_REG(0X2E00);
    } else // ??????IC(9341/5310/1963/7789)?????????GRAM??????
    {
        LCD_WR_REG(0X2E);
    }
    r = LCD_RD_DATA();       // dummy Read
    if (lcddev.id == 0X1963) // ???1963??????,?????????
    {
        return r; // 1963??????????????????
    }
    opt_delay(2);
    r = LCD_RD_DATA(); // ??????????????????

    // 9341/5310/5510/7789 ??????2?????????
    opt_delay(2);
    b = LCD_RD_DATA();
    g = r & 0XFF; // ?????? 9341/5310/5510/7789,
                                // ?????????????????????RG??????,R??????,G??????,??????8???
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) |
                    (b >> 11)); // 9341/5310/5510/7789 ????????????????????????
}

// LCD????????????
void LCD_DisplayOn(void) {
    if (lcddev.id == 0X5510) // 5510??????????????????
    {
        LCD_WR_REG(0X2900); // ????????????
    } else                // 9341/5310/1963/7789 ???????????????????????????
    {
        LCD_WR_REG(0X29); // ????????????
    }
}

// LCD????????????
void LCD_DisplayOff(void) {
    if (lcddev.id == 0X5510) // 5510??????????????????
    {
        LCD_WR_REG(0X2800); // ????????????
    } else                // 9341/5310/1963/7789 ???????????????????????????
    {
        LCD_WR_REG(0X28); // ????????????
    }
}

// ??????????????????
// Xpos:?????????
// Ypos:?????????
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos) {
    if (lcddev.id == 0X1963) {
        if (lcddev.dir == 0) // x??????????????????
        {
            Xpos = lcddev.width - 1 - Xpos;
            LCD_WR_REG(lcddev.setxcmd);
            LCD_WR_DATA(0);
            LCD_WR_DATA(0);
            LCD_WR_DATA(Xpos >> 8);
            LCD_WR_DATA(Xpos & 0XFF);
        } else {
            LCD_WR_REG(lcddev.setxcmd);
            LCD_WR_DATA(Xpos >> 8);
            LCD_WR_DATA(Xpos & 0XFF);
            LCD_WR_DATA((lcddev.width - 1) >> 8);
            LCD_WR_DATA((lcddev.width - 1) & 0XFF);
        }

        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_DATA(Ypos & 0XFF);
        LCD_WR_DATA((lcddev.height - 1) >> 8);
        LCD_WR_DATA((lcddev.height - 1) & 0XFF);

    } else if (lcddev.id == 0X5510) {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(Ypos & 0XFF);
    } else // 9341/5310/7789???????????????
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_DATA(Ypos & 0XFF);
    }
}

// ??????LCD?????????????????????
// dir:0~7,??????8?????????(???????????????lcd.h)
// 9341/5310/5510/1963/7789???IC??????????????????
// ??????:???????????????????????????????????????????????????(?????????9341),
// ??????,???????????????L2R_U2D??????,?????????????????????????????????,???????????????????????????.
void LCD_Scan_Dir(uint8_t dir) {
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;
    // ???????????????1963?????????????????????, ??????IC??????????????????????????????1963????????????,
    // ??????IC?????????????????????
    if ((lcddev.dir == 1 && lcddev.id != 0X1963) ||
            (lcddev.dir == 0 && lcddev.id == 0X1963)) {
        switch (dir) // ????????????
        {
        case 0:
            dir = 6;
            break;

        case 1:
            dir = 7;
            break;

        case 2:
            dir = 4;
            break;

        case 3:
            dir = 5;
            break;

        case 4:
            dir = 1;
            break;

        case 5:
            dir = 0;
            break;

        case 6:
            dir = 3;
            break;

        case 7:
            dir = 2;
            break;
        }
    }

    switch (dir) {
    case L2R_U2D: // ????????????,????????????
        regval |= (0 << 7) | (0 << 6) | (0 << 5);
        break;

    case L2R_D2U: // ????????????,????????????
        regval |= (1 << 7) | (0 << 6) | (0 << 5);
        break;

    case R2L_U2D: // ????????????,????????????
        regval |= (0 << 7) | (1 << 6) | (0 << 5);
        break;

    case R2L_D2U: // ????????????,????????????
        regval |= (1 << 7) | (1 << 6) | (0 << 5);
        break;

    case U2D_L2R: // ????????????,????????????
        regval |= (0 << 7) | (0 << 6) | (1 << 5);
        break;

    case U2D_R2L: // ????????????,????????????
        regval |= (0 << 7) | (1 << 6) | (1 << 5);
        break;

    case D2U_L2R: // ????????????,????????????
        regval |= (1 << 7) | (0 << 6) | (1 << 5);
        break;

    case D2U_R2L: // ????????????,????????????
        regval |= (1 << 7) | (1 << 6) | (1 << 5);
        break;
    }

    if (lcddev.id == 0X5510)
        dirreg = 0X3600;
    else
        dirreg = 0X36;

    if (lcddev.id == 0X9341 || lcddev.id == 0X7789) // 9341 & 7789 ?????????BGR???
    {
        regval |= 0X08;
    }

    LCD_WriteReg(dirreg, regval);

    if (lcddev.id != 0X1963) // 1963??????????????????
    {
        if (regval & 0X20) {
            if (lcddev.width < lcddev.height) // ??????X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        } else {
            if (lcddev.width > lcddev.height) // ??????X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    // ??????????????????(??????)??????
    if (lcddev.id == 0X5510) {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setxcmd + 2);
        LCD_WR_DATA((lcddev.width - 1) >> 8);
        LCD_WR_REG(lcddev.setxcmd + 3);
        LCD_WR_DATA((lcddev.width - 1) & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(0);
        LCD_WR_REG(lcddev.setycmd + 2);
        LCD_WR_DATA((lcddev.height - 1) >> 8);
        LCD_WR_REG(lcddev.setycmd + 3);
        LCD_WR_DATA((lcddev.height - 1) & 0XFF);
    } else {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(0);
        LCD_WR_DATA(0);
        LCD_WR_DATA((lcddev.width - 1) >> 8);
        LCD_WR_DATA((lcddev.width - 1) & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(0);
        LCD_WR_DATA(0);
        LCD_WR_DATA((lcddev.height - 1) >> 8);
        LCD_WR_DATA((lcddev.height - 1) & 0XFF);
    }
}

// ??????
// x,y:??????
// POINT_COLOR:???????????????
void LCD_DrawPoint(uint16_t x, uint16_t y) {
    LCD_SetCursor(x, y);    // ??????????????????
    LCD_WriteRAM_Prepare(); // ????????????GRAM
    LCD->LCD_RAM = POINT_COLOR;
}

// ????????????
// x,y:??????
// color:??????
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint32_t color) {
    if (lcddev.id == 0X5510) {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(x >> 8);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(y >> 8);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(y & 0XFF);
    } else if (lcddev.id == 0X1963) {
        if (lcddev.dir == 0)
            x = lcddev.width - 1 - x;

        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(x >> 8);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_DATA(x >> 8);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(y >> 8);
        LCD_WR_DATA(y & 0XFF);
        LCD_WR_DATA(y >> 8);
        LCD_WR_DATA(y & 0XFF);
    } else // 9341/5310/7789???????????????
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(x >> 8);
        LCD_WR_DATA(x & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(y >> 8);
        LCD_WR_DATA(y & 0XFF);
    }

    LCD->LCD_REG = lcddev.wramcmd;
    LCD->LCD_RAM = color;
}

// SSD1963 ????????????
// pwm:????????????,0~100.????????????.
void LCD_SSD_BackLightSet(uint8_t pwm) {
    LCD_WR_REG(0xBE);        // ??????PWM??????
    LCD_WR_DATA(0x05);       // 1??????PWM??????
    LCD_WR_DATA(pwm * 2.55); // 2??????PWM?????????
    LCD_WR_DATA(0x01);       // 3??????C
    LCD_WR_DATA(0xFF);       // 4??????D
    LCD_WR_DATA(0x00);       // 5??????E
    LCD_WR_DATA(0x00);       // 6??????F
}

// ??????LCD????????????
// dir:0,?????????1,??????
void LCD_Display_Dir(uint8_t dir) {
    lcddev.dir = dir; // ??????/??????

    if (dir == 0) // ??????
    {
        lcddev.width = 240;
        lcddev.height = 320;

        if (lcddev.id == 0x5510) {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        } else if (lcddev.id == 0X1963) {
            lcddev.wramcmd = 0X2C; // ????????????GRAM?????????
            lcddev.setxcmd = 0X2B; // ?????????X????????????
            lcddev.setycmd = 0X2A; // ?????????Y????????????
            lcddev.width = 480;    // ????????????480
            lcddev.height = 800;   // ????????????800
        } else                   // ??????IC, ??????: 9341 / 5310 / 7789???IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310) // ?????????5310 ???????????? 320*480?????????
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }
    } else // ??????
    {
        lcddev.width = 320;
        lcddev.height = 240;

        if (lcddev.id == 0x5510) {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        } else if (lcddev.id == 0X1963) {
            lcddev.wramcmd = 0X2C; // ????????????GRAM?????????
            lcddev.setxcmd = 0X2A; // ?????????X????????????
            lcddev.setycmd = 0X2B; // ?????????Y????????????
            lcddev.width = 800;    // ????????????800
            lcddev.height = 480;   // ????????????480
        } else                   // ??????IC, ??????: 9341 / 5310 / 7789???IC
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }

        if (lcddev.id == 0X5310) // ?????????5310 ???????????? 320*480?????????
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    LCD_Scan_Dir(DFT_SCAN_DIR); // ??????????????????
}

// ????????????,?????????????????????????????????????????????(sx,sy).
// sx,sy:??????????????????(?????????)
// width,height:?????????????????????,????????????0!!
// ????????????:width*height.
void LCD_Set_Window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height) {
    uint16_t twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0X1963 && lcddev.dir != 1) // 1963??????????????????
    {
        sx = lcddev.width - width - sx;
        height = sy + height - 1;
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(sx >> 8);
        LCD_WR_DATA(sx & 0XFF);
        LCD_WR_DATA((sx + width - 1) >> 8);
        LCD_WR_DATA((sx + width - 1) & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(sy >> 8);
        LCD_WR_DATA(sy & 0XFF);
        LCD_WR_DATA(height >> 8);
        LCD_WR_DATA(height & 0XFF);
    } else if (lcddev.id == 0X5510) {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(sx >> 8);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(sx & 0XFF);
        LCD_WR_REG(lcddev.setxcmd + 2);
        LCD_WR_DATA(twidth >> 8);
        LCD_WR_REG(lcddev.setxcmd + 3);
        LCD_WR_DATA(twidth & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(sy >> 8);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(sy & 0XFF);
        LCD_WR_REG(lcddev.setycmd + 2);
        LCD_WR_DATA(theight >> 8);
        LCD_WR_REG(lcddev.setycmd + 3);
        LCD_WR_DATA(theight & 0XFF);
    } else // 9341/5310/7789/1963?????? ??? ????????????
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(sx >> 8);
        LCD_WR_DATA(sx & 0XFF);
        LCD_WR_DATA(twidth >> 8);
        LCD_WR_DATA(twidth & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(sy >> 8);
        LCD_WR_DATA(sy & 0XFF);
        LCD_WR_DATA(theight >> 8);
        LCD_WR_DATA(theight & 0XFF);
    }
}

// SRAM??????????????????????????????????????????
// ???????????????HAL_SRAM_Init()??????
// hsram:SRAM??????
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram) {
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_FSMC_CLK_ENABLE();  // ??????FSMC??????
    __HAL_RCC_GPIOD_CLK_ENABLE(); // ??????GPIOD??????
    __HAL_RCC_GPIOE_CLK_ENABLE(); // ??????GPIOE??????
    __HAL_RCC_GPIOF_CLK_ENABLE(); // ??????GPIOF??????
    __HAL_RCC_GPIOG_CLK_ENABLE(); // ??????GPIOG??????

    // ?????????PD0,1,4,5,8,9,10,14,15
    GPIO_Initure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
                                         GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
                                         GPIO_PIN_15;
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;     // ????????????
    GPIO_Initure.Pull = GPIO_PULLUP;         // ??????
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;    // ??????
    GPIO_Initure.Alternate = GPIO_AF12_FSMC; // ?????????FSMC
    HAL_GPIO_Init(GPIOD, &GPIO_Initure);     // ?????????

    // ?????????PE7,8,9,10,11,12,13,14,15
    GPIO_Initure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                                         GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                                         GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_Initure);

    // ?????????PF12
    GPIO_Initure.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOF, &GPIO_Initure);

    // ?????????PG12
    GPIO_Initure.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOG, &GPIO_Initure);
}

// ?????????lcd
// ???????????????????????????????????????ILI93XX??????,???????????????????????????ILI9320???!!!
// ?????????????????????????????????????????????!
void LCD_Init(void) {
    GPIO_InitTypeDef GPIO_Initure;
    FSMC_NORSRAM_TimingTypeDef FSMC_ReadWriteTim;
    FSMC_NORSRAM_TimingTypeDef FSMC_WriteTim;

    __HAL_RCC_GPIOB_CLK_ENABLE();            // ??????GPIOB??????
    GPIO_Initure.Pin = GPIO_PIN_15;          // PB15,????????????
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP; // ????????????
    GPIO_Initure.Pull = GPIO_PULLUP;         // ??????
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;    // ??????
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);

    TFTSRAM_Handler.Instance = FSMC_NORSRAM_DEVICE;
    TFTSRAM_Handler.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

    TFTSRAM_Handler.Init.NSBank = FSMC_NORSRAM_BANK4; // ??????NE4
    TFTSRAM_Handler.Init.DataAddressMux =
            FSMC_DATA_ADDRESS_MUX_DISABLE; // ??????/??????????????????
    TFTSRAM_Handler.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM; // SRAM
    TFTSRAM_Handler.Init.MemoryDataWidth =
            FSMC_NORSRAM_MEM_BUS_WIDTH_16; // 16???????????????
    TFTSRAM_Handler.Init.BurstAccessMode =
            FSMC_BURST_ACCESS_MODE_DISABLE; // ????????????????????????,?????????????????????????????????,???????????????
    TFTSRAM_Handler.Init.WaitSignalPolarity =
            FSMC_WAIT_SIGNAL_POLARITY_LOW; // ?????????????????????,?????????????????????????????????
    TFTSRAM_Handler.Init.WaitSignalActive =
            FSMC_WAIT_TIMING_BEFORE_WS; // ????????????????????????????????????????????????????????????????????????????????????NWAIT
    TFTSRAM_Handler.Init.WriteOperation =
            FSMC_WRITE_OPERATION_ENABLE; // ??????????????????
    TFTSRAM_Handler.Init.WaitSignal =
            FSMC_WAIT_SIGNAL_DISABLE; // ???????????????,???????????????
    TFTSRAM_Handler.Init.ExtendedMode =
            FSMC_EXTENDED_MODE_ENABLE; // ???????????????????????????
    TFTSRAM_Handler.Init.AsynchronousWait =
            FSMC_ASYNCHRONOUS_WAIT_DISABLE; // ????????????????????????????????????????????????,???????????????
    TFTSRAM_Handler.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE; // ???????????????
    TFTSRAM_Handler.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ASYNC;

    // FMC????????????????????????
    FSMC_ReadWriteTim.AddressSetupTime =
            0x0F; // ?????????????????????ADDSET??????16???HCLK 1/168M=6ns*16=96ns
    FSMC_ReadWriteTim.AddressHoldTime = 0;
    FSMC_ReadWriteTim.DataSetupTime =
            60; // ?????????????????????60???HCLK	=6*60=360ns
    FSMC_ReadWriteTim.AccessMode = FSMC_ACCESS_MODE_A; // ??????A
    // FMC????????????????????????
    FSMC_WriteTim.BusTurnAroundDuration =
            0; // ?????????????????????????????????0????????????????????????????????????????????????????????????4???????????????????????????
    FSMC_WriteTim.AddressSetupTime = 9; // ?????????????????????ADDSET??????9???HCLK =54ns
    FSMC_WriteTim.AddressHoldTime = 0;
    FSMC_WriteTim.DataSetupTime = 8; // ?????????????????????6ns*9???HCLK=54n
    FSMC_WriteTim.AccessMode = FSMC_ACCESS_MODE_A; // ??????A
    HAL_SRAM_Init(&TFTSRAM_Handler, &FSMC_ReadWriteTim, &FSMC_WriteTim);

    HAL_Delay(50); // delay 50 ms

    // ??????9341 ID?????????
    LCD_WR_REG(0XD3);
    lcddev.id = LCD_RD_DATA(); // dummy read
    lcddev.id = LCD_RD_DATA(); // ??????0X00
    lcddev.id = LCD_RD_DATA(); // ??????93
    lcddev.id <<= 8;
    lcddev.id |= LCD_RD_DATA(); // ??????41
    if (lcddev.id != 0X9341)    // ???9341,???????????????7789
    {
        LCD_WR_REG(0X04);
        lcddev.id = LCD_RD_DATA(); // dummy read
        lcddev.id = LCD_RD_DATA(); // ??????0X85
        lcddev.id = LCD_RD_DATA(); // ??????0X85
        lcddev.id <<= 8;
        lcddev.id |= LCD_RD_DATA(); // ??????0X52

        if (lcddev.id == 0X8552) // ???8552???ID?????????7789
        {
            lcddev.id = 0x7789;
        }

        if (lcddev.id != 0x7789) // ?????????ST7789, ??????????????? 7789
        {
            LCD_WR_REG(0XD4);
            lcddev.id = LCD_RD_DATA(); // dummy read
            lcddev.id = LCD_RD_DATA(); // ??????0X01
            lcddev.id = LCD_RD_DATA(); // ??????0X53
            lcddev.id <<= 8;
            lcddev.id |= LCD_RD_DATA(); // ????????????0X10
            if (lcddev.id != 0X5310) // ?????????NT35310,?????????????????????NT35510
            {
                // ???????????????????????????,???????????????
                LCD_WriteReg(0xF000, 0x0055);
                LCD_WriteReg(0xF001, 0x00AA);
                LCD_WriteReg(0xF002, 0x0052);
                LCD_WriteReg(0xF003, 0x0008);
                LCD_WriteReg(0xF004, 0x0001);

                LCD_WR_REG(0xC500);        // ??????ID???8???
                lcddev.id = LCD_RD_DATA(); // ??????0X55
                lcddev.id <<= 8;

                LCD_WR_REG(0xC501);         // ??????ID???8???
                lcddev.id |= LCD_RD_DATA(); // ??????0X10
                HAL_Delay(5);
                if (lcddev.id == 0x8000)
                    lcddev.id =
                            0x5510; // NT35510?????????ID???8000H,???????????????,?????????????????????5510
                if (lcddev.id != 0X5510) // ?????????NT5510,?????????????????????SSD1963
                {
                    LCD_WR_REG(0XA1);
                    lcddev.id = LCD_RD_DATA();
                    lcddev.id = LCD_RD_DATA(); // ??????0X57
                    lcddev.id <<= 8;
                    lcddev.id |= LCD_RD_DATA(); // ??????0X61
                    if (lcddev.id == 0X5761)
                        lcddev.id =
                                0X1963; // SSD1963?????????ID???5761H,???????????????,?????????????????????1963
                }
            }
        }
    }
    if (lcddev.id == 0X9341) // 9341?????????
    {
        LCD_WR_REG(0xCF);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC1);
        LCD_WR_DATA(0X30);
        LCD_WR_REG(0xED);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0X12);
        LCD_WR_DATA(0X81);
        LCD_WR_REG(0xE8);
        LCD_WR_DATA(0x85);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x7A);
        LCD_WR_REG(0xCB);
        LCD_WR_DATA(0x39);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x34);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xF7);
        LCD_WR_DATA(0x20);
        LCD_WR_REG(0xEA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC0);  // Power control
        LCD_WR_DATA(0x1B); // VRH[5:0]
        LCD_WR_REG(0xC1);  // Power control
        LCD_WR_DATA(0x01); // SAP[2:0];BT[3:0]
        LCD_WR_REG(0xC5);  // VCM control
        LCD_WR_DATA(0x30); // 3F
        LCD_WR_DATA(0x30); // 3C
        LCD_WR_REG(0xC7);  // VCM control2
        LCD_WR_DATA(0XB7);
        LCD_WR_REG(0x36); // Memory Access Control
        LCD_WR_DATA(0x48);
        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0xB1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1A);
        LCD_WR_REG(0xB6); // Display Function Control
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0xA2);
        LCD_WR_REG(0xF2); // 3Gamma Function Disable
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x26); // Gamma curve selected
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xE0); // Set Gamma
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x2A);
        LCD_WR_DATA(0x28);
        LCD_WR_DATA(0x08);
        LCD_WR_DATA(0x0E);
        LCD_WR_DATA(0x08);
        LCD_WR_DATA(0x54);
        LCD_WR_DATA(0XA9);
        LCD_WR_DATA(0x43);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0XE1); // Set Gamma
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x15);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x07);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x2B);
        LCD_WR_DATA(0x56);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x0F);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x3f);
        LCD_WR_REG(0x2A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xef);
        LCD_WR_REG(0x11); // Exit Sleep
        HAL_Delay(120);
        LCD_WR_REG(0x29);             // display on
    } else if (lcddev.id == 0x7789) // 7789?????????
    {
        LCD_WR_REG(0x11);

        HAL_Delay(120);

        LCD_WR_REG(0x36);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0X05);

        LCD_WR_REG(0xB2);
        LCD_WR_DATA(0x0C);
        LCD_WR_DATA(0x0C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x33);
        LCD_WR_DATA(0x33);

        LCD_WR_REG(0xB7);
        LCD_WR_DATA(0x35);

        LCD_WR_REG(0xBB);  // vcom
        LCD_WR_DATA(0x32); // 30

        LCD_WR_REG(0xC0);
        LCD_WR_DATA(0x0C);

        LCD_WR_REG(0xC2);
        LCD_WR_DATA(0x01);

        LCD_WR_REG(0xC3);  // vrh
        LCD_WR_DATA(0x10); // 17 0D

        LCD_WR_REG(0xC4);  // vdv
        LCD_WR_DATA(0x20); // 20

        LCD_WR_REG(0xC6);
        LCD_WR_DATA(0x0f);

        LCD_WR_REG(0xD0);
        LCD_WR_DATA(0xA4);
        LCD_WR_DATA(0xA1);

        LCD_WR_REG(0xE0); // Set Gamma
        LCD_WR_DATA(0xd0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x07);
        LCD_WR_DATA(0x0a);
        LCD_WR_DATA(0x28);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0X44);
        LCD_WR_DATA(0x42);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x0e);
        LCD_WR_DATA(0x12);
        LCD_WR_DATA(0x14);
        LCD_WR_DATA(0x17);

        LCD_WR_REG(0XE1); // Set Gamma
        LCD_WR_DATA(0xd0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x07);
        LCD_WR_DATA(0x0a);
        LCD_WR_DATA(0x28);
        LCD_WR_DATA(0x31);
        LCD_WR_DATA(0x54);
        LCD_WR_DATA(0x47);
        LCD_WR_DATA(0x0e);
        LCD_WR_DATA(0x1c);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x1b);
        LCD_WR_DATA(0x1e);

        LCD_WR_REG(0x2A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xef);

        LCD_WR_REG(0x2B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x3f);

        LCD_WR_REG(0x29); // display on
    } else if (lcddev.id == 0x5310) {
        LCD_WR_REG(0xED);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0xFE);

        LCD_WR_REG(0xEE);
        LCD_WR_DATA(0xDE);
        LCD_WR_DATA(0x21);

        LCD_WR_REG(0xF1);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xDF);
        LCD_WR_DATA(0x10);

        // VCOMvoltage//
        LCD_WR_REG(0xC4);
        LCD_WR_DATA(0x8F); // 5f

        LCD_WR_REG(0xC6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE2);
        LCD_WR_DATA(0xE2);
        LCD_WR_DATA(0xE2);
        LCD_WR_REG(0xBF);
        LCD_WR_DATA(0xAA);

        LCD_WR_REG(0xB0);
        LCD_WR_DATA(0x0D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x19);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x21);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB1);
        LCD_WR_DATA(0x80);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x96);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB4);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x96);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA1);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB5);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5E);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xAC);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDC);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x70);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x90);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xEB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDC);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xB8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xBA);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC1);
        LCD_WR_DATA(0x20);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x54);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xFF);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC2);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC3);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x39);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x37);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x26);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x23);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x26);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x23);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC4);
        LCD_WR_DATA(0x62);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x84);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x18);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x18);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x50);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x95);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE6);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC5);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x65);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x76);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC6);
        LCD_WR_DATA(0x20);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xC9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0x16);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x21);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x46);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x52);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x7A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE1);
        LCD_WR_DATA(0x16);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x22);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x45);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x52);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x7A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE2);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x34);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x4F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x61);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x79);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x97);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE3);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x33);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x50);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x62);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x78);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x97);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD5);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE4);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x4B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x74);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x84);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x93);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBE);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE5);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x4B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x74);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x84);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x93);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBE);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDC);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE6);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x34);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x56);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x76);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x77);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x45);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x43);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE7);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x76);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x67);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x67);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x87);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x77);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x56);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x23);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x33);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x45);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x87);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x77);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xAA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xE9);
        LCD_WR_DATA(0xAA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0x00);
        LCD_WR_DATA(0xAA);

        LCD_WR_REG(0xCF);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xF0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x50);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xF3);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xF9);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x55); // 66

        LCD_WR_REG(0x11);
        HAL_Delay(100);
        LCD_WR_REG(0x29);
        LCD_WR_REG(0x35);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0x51);
        LCD_WR_DATA(0xFF);
        LCD_WR_REG(0x53);
        LCD_WR_DATA(0x2C);
        LCD_WR_REG(0x55);
        LCD_WR_DATA(0x82);
        LCD_WR_REG(0x2c);
    } else if (lcddev.id == 0x5510) {
        LCD_WriteReg(0xF000, 0x55);
        LCD_WriteReg(0xF001, 0xAA);
        LCD_WriteReg(0xF002, 0x52);
        LCD_WriteReg(0xF003, 0x08);
        LCD_WriteReg(0xF004, 0x01);
        // AVDD Set AVDD 5.2V
        LCD_WriteReg(0xB000, 0x0D);
        LCD_WriteReg(0xB001, 0x0D);
        LCD_WriteReg(0xB002, 0x0D);
        // AVDD ratio
        LCD_WriteReg(0xB600, 0x34);
        LCD_WriteReg(0xB601, 0x34);
        LCD_WriteReg(0xB602, 0x34);
        // AVEE -5.2V
        LCD_WriteReg(0xB100, 0x0D);
        LCD_WriteReg(0xB101, 0x0D);
        LCD_WriteReg(0xB102, 0x0D);
        // AVEE ratio
        LCD_WriteReg(0xB700, 0x34);
        LCD_WriteReg(0xB701, 0x34);
        LCD_WriteReg(0xB702, 0x34);
        // VCL -2.5V
        LCD_WriteReg(0xB200, 0x00);
        LCD_WriteReg(0xB201, 0x00);
        LCD_WriteReg(0xB202, 0x00);
        // VCL ratio
        LCD_WriteReg(0xB800, 0x24);
        LCD_WriteReg(0xB801, 0x24);
        LCD_WriteReg(0xB802, 0x24);
        // VGH 15V (Free pump)
        LCD_WriteReg(0xBF00, 0x01);
        LCD_WriteReg(0xB300, 0x0F);
        LCD_WriteReg(0xB301, 0x0F);
        LCD_WriteReg(0xB302, 0x0F);
        // VGH ratio
        LCD_WriteReg(0xB900, 0x34);
        LCD_WriteReg(0xB901, 0x34);
        LCD_WriteReg(0xB902, 0x34);
        // VGL_REG -10V
        LCD_WriteReg(0xB500, 0x08);
        LCD_WriteReg(0xB501, 0x08);
        LCD_WriteReg(0xB502, 0x08);
        LCD_WriteReg(0xC200, 0x03);
        // VGLX ratio
        LCD_WriteReg(0xBA00, 0x24);
        LCD_WriteReg(0xBA01, 0x24);
        LCD_WriteReg(0xBA02, 0x24);
        // VGMP/VGSP 4.5V/0V
        LCD_WriteReg(0xBC00, 0x00);
        LCD_WriteReg(0xBC01, 0x78);
        LCD_WriteReg(0xBC02, 0x00);
        // VGMN/VGSN -4.5V/0V
        LCD_WriteReg(0xBD00, 0x00);
        LCD_WriteReg(0xBD01, 0x78);
        LCD_WriteReg(0xBD02, 0x00);
        // VCOM
        LCD_WriteReg(0xBE00, 0x00);
        LCD_WriteReg(0xBE01, 0x64);
        // Gamma Setting
        LCD_WriteReg(0xD100, 0x00);
        LCD_WriteReg(0xD101, 0x33);
        LCD_WriteReg(0xD102, 0x00);
        LCD_WriteReg(0xD103, 0x34);
        LCD_WriteReg(0xD104, 0x00);
        LCD_WriteReg(0xD105, 0x3A);
        LCD_WriteReg(0xD106, 0x00);
        LCD_WriteReg(0xD107, 0x4A);
        LCD_WriteReg(0xD108, 0x00);
        LCD_WriteReg(0xD109, 0x5C);
        LCD_WriteReg(0xD10A, 0x00);
        LCD_WriteReg(0xD10B, 0x81);
        LCD_WriteReg(0xD10C, 0x00);
        LCD_WriteReg(0xD10D, 0xA6);
        LCD_WriteReg(0xD10E, 0x00);
        LCD_WriteReg(0xD10F, 0xE5);
        LCD_WriteReg(0xD110, 0x01);
        LCD_WriteReg(0xD111, 0x13);
        LCD_WriteReg(0xD112, 0x01);
        LCD_WriteReg(0xD113, 0x54);
        LCD_WriteReg(0xD114, 0x01);
        LCD_WriteReg(0xD115, 0x82);
        LCD_WriteReg(0xD116, 0x01);
        LCD_WriteReg(0xD117, 0xCA);
        LCD_WriteReg(0xD118, 0x02);
        LCD_WriteReg(0xD119, 0x00);
        LCD_WriteReg(0xD11A, 0x02);
        LCD_WriteReg(0xD11B, 0x01);
        LCD_WriteReg(0xD11C, 0x02);
        LCD_WriteReg(0xD11D, 0x34);
        LCD_WriteReg(0xD11E, 0x02);
        LCD_WriteReg(0xD11F, 0x67);
        LCD_WriteReg(0xD120, 0x02);
        LCD_WriteReg(0xD121, 0x84);
        LCD_WriteReg(0xD122, 0x02);
        LCD_WriteReg(0xD123, 0xA4);
        LCD_WriteReg(0xD124, 0x02);
        LCD_WriteReg(0xD125, 0xB7);
        LCD_WriteReg(0xD126, 0x02);
        LCD_WriteReg(0xD127, 0xCF);
        LCD_WriteReg(0xD128, 0x02);
        LCD_WriteReg(0xD129, 0xDE);
        LCD_WriteReg(0xD12A, 0x02);
        LCD_WriteReg(0xD12B, 0xF2);
        LCD_WriteReg(0xD12C, 0x02);
        LCD_WriteReg(0xD12D, 0xFE);
        LCD_WriteReg(0xD12E, 0x03);
        LCD_WriteReg(0xD12F, 0x10);
        LCD_WriteReg(0xD130, 0x03);
        LCD_WriteReg(0xD131, 0x33);
        LCD_WriteReg(0xD132, 0x03);
        LCD_WriteReg(0xD133, 0x6D);
        LCD_WriteReg(0xD200, 0x00);
        LCD_WriteReg(0xD201, 0x33);
        LCD_WriteReg(0xD202, 0x00);
        LCD_WriteReg(0xD203, 0x34);
        LCD_WriteReg(0xD204, 0x00);
        LCD_WriteReg(0xD205, 0x3A);
        LCD_WriteReg(0xD206, 0x00);
        LCD_WriteReg(0xD207, 0x4A);
        LCD_WriteReg(0xD208, 0x00);
        LCD_WriteReg(0xD209, 0x5C);
        LCD_WriteReg(0xD20A, 0x00);

        LCD_WriteReg(0xD20B, 0x81);
        LCD_WriteReg(0xD20C, 0x00);
        LCD_WriteReg(0xD20D, 0xA6);
        LCD_WriteReg(0xD20E, 0x00);
        LCD_WriteReg(0xD20F, 0xE5);
        LCD_WriteReg(0xD210, 0x01);
        LCD_WriteReg(0xD211, 0x13);
        LCD_WriteReg(0xD212, 0x01);
        LCD_WriteReg(0xD213, 0x54);
        LCD_WriteReg(0xD214, 0x01);
        LCD_WriteReg(0xD215, 0x82);
        LCD_WriteReg(0xD216, 0x01);
        LCD_WriteReg(0xD217, 0xCA);
        LCD_WriteReg(0xD218, 0x02);
        LCD_WriteReg(0xD219, 0x00);
        LCD_WriteReg(0xD21A, 0x02);
        LCD_WriteReg(0xD21B, 0x01);
        LCD_WriteReg(0xD21C, 0x02);
        LCD_WriteReg(0xD21D, 0x34);
        LCD_WriteReg(0xD21E, 0x02);
        LCD_WriteReg(0xD21F, 0x67);
        LCD_WriteReg(0xD220, 0x02);
        LCD_WriteReg(0xD221, 0x84);
        LCD_WriteReg(0xD222, 0x02);
        LCD_WriteReg(0xD223, 0xA4);
        LCD_WriteReg(0xD224, 0x02);
        LCD_WriteReg(0xD225, 0xB7);
        LCD_WriteReg(0xD226, 0x02);
        LCD_WriteReg(0xD227, 0xCF);
        LCD_WriteReg(0xD228, 0x02);
        LCD_WriteReg(0xD229, 0xDE);
        LCD_WriteReg(0xD22A, 0x02);
        LCD_WriteReg(0xD22B, 0xF2);
        LCD_WriteReg(0xD22C, 0x02);
        LCD_WriteReg(0xD22D, 0xFE);
        LCD_WriteReg(0xD22E, 0x03);
        LCD_WriteReg(0xD22F, 0x10);
        LCD_WriteReg(0xD230, 0x03);
        LCD_WriteReg(0xD231, 0x33);
        LCD_WriteReg(0xD232, 0x03);
        LCD_WriteReg(0xD233, 0x6D);
        LCD_WriteReg(0xD300, 0x00);
        LCD_WriteReg(0xD301, 0x33);
        LCD_WriteReg(0xD302, 0x00);
        LCD_WriteReg(0xD303, 0x34);
        LCD_WriteReg(0xD304, 0x00);
        LCD_WriteReg(0xD305, 0x3A);
        LCD_WriteReg(0xD306, 0x00);
        LCD_WriteReg(0xD307, 0x4A);
        LCD_WriteReg(0xD308, 0x00);
        LCD_WriteReg(0xD309, 0x5C);
        LCD_WriteReg(0xD30A, 0x00);

        LCD_WriteReg(0xD30B, 0x81);
        LCD_WriteReg(0xD30C, 0x00);
        LCD_WriteReg(0xD30D, 0xA6);
        LCD_WriteReg(0xD30E, 0x00);
        LCD_WriteReg(0xD30F, 0xE5);
        LCD_WriteReg(0xD310, 0x01);
        LCD_WriteReg(0xD311, 0x13);
        LCD_WriteReg(0xD312, 0x01);
        LCD_WriteReg(0xD313, 0x54);
        LCD_WriteReg(0xD314, 0x01);
        LCD_WriteReg(0xD315, 0x82);
        LCD_WriteReg(0xD316, 0x01);
        LCD_WriteReg(0xD317, 0xCA);
        LCD_WriteReg(0xD318, 0x02);
        LCD_WriteReg(0xD319, 0x00);
        LCD_WriteReg(0xD31A, 0x02);
        LCD_WriteReg(0xD31B, 0x01);
        LCD_WriteReg(0xD31C, 0x02);
        LCD_WriteReg(0xD31D, 0x34);
        LCD_WriteReg(0xD31E, 0x02);
        LCD_WriteReg(0xD31F, 0x67);
        LCD_WriteReg(0xD320, 0x02);
        LCD_WriteReg(0xD321, 0x84);
        LCD_WriteReg(0xD322, 0x02);
        LCD_WriteReg(0xD323, 0xA4);
        LCD_WriteReg(0xD324, 0x02);
        LCD_WriteReg(0xD325, 0xB7);
        LCD_WriteReg(0xD326, 0x02);
        LCD_WriteReg(0xD327, 0xCF);
        LCD_WriteReg(0xD328, 0x02);
        LCD_WriteReg(0xD329, 0xDE);
        LCD_WriteReg(0xD32A, 0x02);
        LCD_WriteReg(0xD32B, 0xF2);
        LCD_WriteReg(0xD32C, 0x02);
        LCD_WriteReg(0xD32D, 0xFE);
        LCD_WriteReg(0xD32E, 0x03);
        LCD_WriteReg(0xD32F, 0x10);
        LCD_WriteReg(0xD330, 0x03);
        LCD_WriteReg(0xD331, 0x33);
        LCD_WriteReg(0xD332, 0x03);
        LCD_WriteReg(0xD333, 0x6D);
        LCD_WriteReg(0xD400, 0x00);
        LCD_WriteReg(0xD401, 0x33);
        LCD_WriteReg(0xD402, 0x00);
        LCD_WriteReg(0xD403, 0x34);
        LCD_WriteReg(0xD404, 0x00);
        LCD_WriteReg(0xD405, 0x3A);
        LCD_WriteReg(0xD406, 0x00);
        LCD_WriteReg(0xD407, 0x4A);
        LCD_WriteReg(0xD408, 0x00);
        LCD_WriteReg(0xD409, 0x5C);
        LCD_WriteReg(0xD40A, 0x00);
        LCD_WriteReg(0xD40B, 0x81);

        LCD_WriteReg(0xD40C, 0x00);
        LCD_WriteReg(0xD40D, 0xA6);
        LCD_WriteReg(0xD40E, 0x00);
        LCD_WriteReg(0xD40F, 0xE5);
        LCD_WriteReg(0xD410, 0x01);
        LCD_WriteReg(0xD411, 0x13);
        LCD_WriteReg(0xD412, 0x01);
        LCD_WriteReg(0xD413, 0x54);
        LCD_WriteReg(0xD414, 0x01);
        LCD_WriteReg(0xD415, 0x82);
        LCD_WriteReg(0xD416, 0x01);
        LCD_WriteReg(0xD417, 0xCA);
        LCD_WriteReg(0xD418, 0x02);
        LCD_WriteReg(0xD419, 0x00);
        LCD_WriteReg(0xD41A, 0x02);
        LCD_WriteReg(0xD41B, 0x01);
        LCD_WriteReg(0xD41C, 0x02);
        LCD_WriteReg(0xD41D, 0x34);
        LCD_WriteReg(0xD41E, 0x02);
        LCD_WriteReg(0xD41F, 0x67);
        LCD_WriteReg(0xD420, 0x02);
        LCD_WriteReg(0xD421, 0x84);
        LCD_WriteReg(0xD422, 0x02);
        LCD_WriteReg(0xD423, 0xA4);
        LCD_WriteReg(0xD424, 0x02);
        LCD_WriteReg(0xD425, 0xB7);
        LCD_WriteReg(0xD426, 0x02);
        LCD_WriteReg(0xD427, 0xCF);
        LCD_WriteReg(0xD428, 0x02);
        LCD_WriteReg(0xD429, 0xDE);
        LCD_WriteReg(0xD42A, 0x02);
        LCD_WriteReg(0xD42B, 0xF2);
        LCD_WriteReg(0xD42C, 0x02);
        LCD_WriteReg(0xD42D, 0xFE);
        LCD_WriteReg(0xD42E, 0x03);
        LCD_WriteReg(0xD42F, 0x10);
        LCD_WriteReg(0xD430, 0x03);
        LCD_WriteReg(0xD431, 0x33);
        LCD_WriteReg(0xD432, 0x03);
        LCD_WriteReg(0xD433, 0x6D);
        LCD_WriteReg(0xD500, 0x00);
        LCD_WriteReg(0xD501, 0x33);
        LCD_WriteReg(0xD502, 0x00);
        LCD_WriteReg(0xD503, 0x34);
        LCD_WriteReg(0xD504, 0x00);
        LCD_WriteReg(0xD505, 0x3A);
        LCD_WriteReg(0xD506, 0x00);
        LCD_WriteReg(0xD507, 0x4A);
        LCD_WriteReg(0xD508, 0x00);
        LCD_WriteReg(0xD509, 0x5C);
        LCD_WriteReg(0xD50A, 0x00);
        LCD_WriteReg(0xD50B, 0x81);

        LCD_WriteReg(0xD50C, 0x00);
        LCD_WriteReg(0xD50D, 0xA6);
        LCD_WriteReg(0xD50E, 0x00);
        LCD_WriteReg(0xD50F, 0xE5);
        LCD_WriteReg(0xD510, 0x01);
        LCD_WriteReg(0xD511, 0x13);
        LCD_WriteReg(0xD512, 0x01);
        LCD_WriteReg(0xD513, 0x54);
        LCD_WriteReg(0xD514, 0x01);
        LCD_WriteReg(0xD515, 0x82);
        LCD_WriteReg(0xD516, 0x01);
        LCD_WriteReg(0xD517, 0xCA);
        LCD_WriteReg(0xD518, 0x02);
        LCD_WriteReg(0xD519, 0x00);
        LCD_WriteReg(0xD51A, 0x02);
        LCD_WriteReg(0xD51B, 0x01);
        LCD_WriteReg(0xD51C, 0x02);
        LCD_WriteReg(0xD51D, 0x34);
        LCD_WriteReg(0xD51E, 0x02);
        LCD_WriteReg(0xD51F, 0x67);
        LCD_WriteReg(0xD520, 0x02);
        LCD_WriteReg(0xD521, 0x84);
        LCD_WriteReg(0xD522, 0x02);
        LCD_WriteReg(0xD523, 0xA4);
        LCD_WriteReg(0xD524, 0x02);
        LCD_WriteReg(0xD525, 0xB7);
        LCD_WriteReg(0xD526, 0x02);
        LCD_WriteReg(0xD527, 0xCF);
        LCD_WriteReg(0xD528, 0x02);
        LCD_WriteReg(0xD529, 0xDE);
        LCD_WriteReg(0xD52A, 0x02);
        LCD_WriteReg(0xD52B, 0xF2);
        LCD_WriteReg(0xD52C, 0x02);
        LCD_WriteReg(0xD52D, 0xFE);
        LCD_WriteReg(0xD52E, 0x03);
        LCD_WriteReg(0xD52F, 0x10);
        LCD_WriteReg(0xD530, 0x03);
        LCD_WriteReg(0xD531, 0x33);
        LCD_WriteReg(0xD532, 0x03);
        LCD_WriteReg(0xD533, 0x6D);
        LCD_WriteReg(0xD600, 0x00);
        LCD_WriteReg(0xD601, 0x33);
        LCD_WriteReg(0xD602, 0x00);
        LCD_WriteReg(0xD603, 0x34);
        LCD_WriteReg(0xD604, 0x00);
        LCD_WriteReg(0xD605, 0x3A);
        LCD_WriteReg(0xD606, 0x00);
        LCD_WriteReg(0xD607, 0x4A);
        LCD_WriteReg(0xD608, 0x00);
        LCD_WriteReg(0xD609, 0x5C);
        LCD_WriteReg(0xD60A, 0x00);
        LCD_WriteReg(0xD60B, 0x81);

        LCD_WriteReg(0xD60C, 0x00);
        LCD_WriteReg(0xD60D, 0xA6);
        LCD_WriteReg(0xD60E, 0x00);
        LCD_WriteReg(0xD60F, 0xE5);
        LCD_WriteReg(0xD610, 0x01);
        LCD_WriteReg(0xD611, 0x13);
        LCD_WriteReg(0xD612, 0x01);
        LCD_WriteReg(0xD613, 0x54);
        LCD_WriteReg(0xD614, 0x01);
        LCD_WriteReg(0xD615, 0x82);
        LCD_WriteReg(0xD616, 0x01);
        LCD_WriteReg(0xD617, 0xCA);
        LCD_WriteReg(0xD618, 0x02);
        LCD_WriteReg(0xD619, 0x00);
        LCD_WriteReg(0xD61A, 0x02);
        LCD_WriteReg(0xD61B, 0x01);
        LCD_WriteReg(0xD61C, 0x02);
        LCD_WriteReg(0xD61D, 0x34);
        LCD_WriteReg(0xD61E, 0x02);
        LCD_WriteReg(0xD61F, 0x67);
        LCD_WriteReg(0xD620, 0x02);
        LCD_WriteReg(0xD621, 0x84);
        LCD_WriteReg(0xD622, 0x02);
        LCD_WriteReg(0xD623, 0xA4);
        LCD_WriteReg(0xD624, 0x02);
        LCD_WriteReg(0xD625, 0xB7);
        LCD_WriteReg(0xD626, 0x02);
        LCD_WriteReg(0xD627, 0xCF);
        LCD_WriteReg(0xD628, 0x02);
        LCD_WriteReg(0xD629, 0xDE);
        LCD_WriteReg(0xD62A, 0x02);
        LCD_WriteReg(0xD62B, 0xF2);
        LCD_WriteReg(0xD62C, 0x02);
        LCD_WriteReg(0xD62D, 0xFE);
        LCD_WriteReg(0xD62E, 0x03);
        LCD_WriteReg(0xD62F, 0x10);
        LCD_WriteReg(0xD630, 0x03);
        LCD_WriteReg(0xD631, 0x33);
        LCD_WriteReg(0xD632, 0x03);
        LCD_WriteReg(0xD633, 0x6D);
        // LV2 Page 0 enable
        LCD_WriteReg(0xF000, 0x55);
        LCD_WriteReg(0xF001, 0xAA);
        LCD_WriteReg(0xF002, 0x52);
        LCD_WriteReg(0xF003, 0x08);
        LCD_WriteReg(0xF004, 0x00);
        // Display control
        LCD_WriteReg(0xB100, 0xCC);
        LCD_WriteReg(0xB101, 0x00);
        // Source hold time
        LCD_WriteReg(0xB600, 0x05);
        // Gate EQ control
        LCD_WriteReg(0xB700, 0x70);
        LCD_WriteReg(0xB701, 0x70);
        // Source EQ control (Mode 2)
        LCD_WriteReg(0xB800, 0x01);
        LCD_WriteReg(0xB801, 0x03);
        LCD_WriteReg(0xB802, 0x03);
        LCD_WriteReg(0xB803, 0x03);
        // Inversion mode (2-dot)
        LCD_WriteReg(0xBC00, 0x02);
        LCD_WriteReg(0xBC01, 0x00);
        LCD_WriteReg(0xBC02, 0x00);
        // Timing control 4H w/ 4-delay
        LCD_WriteReg(0xC900, 0xD0);
        LCD_WriteReg(0xC901, 0x02);
        LCD_WriteReg(0xC902, 0x50);
        LCD_WriteReg(0xC903, 0x50);
        LCD_WriteReg(0xC904, 0x50);
        LCD_WriteReg(0x3500, 0x00);
        LCD_WriteReg(0x3A00, 0x55); // 16-bit/pixel
        LCD_WR_REG(0x1100);
        HAL_Delay(1);
        LCD_WR_REG(0x2900);
    } else if (lcddev.id == 0X1963) {
        LCD_WR_REG(0xE2); // Set PLL with OSC = 10MHz (hardware),	Multiplier N =
                                            // 35, 250MHz < VCO < 800MHz = OSC*(N+1), VCO = 300MHz
        LCD_WR_DATA(0x1D); // ??????1
        LCD_WR_DATA(0x02); // ??????2 Divider M = 2, PLL = 300/(M+1) = 100MHz
        LCD_WR_DATA(0x04); // ??????3 Validate M and N values
        HAL_Delay(1);
        LCD_WR_REG(0xE0);  // Start PLL command
        LCD_WR_DATA(0x01); // enable PLL
        HAL_Delay(10);
        LCD_WR_REG(0xE0);  // Start PLL command again
        LCD_WR_DATA(0x03); // now, use PLL output as system clock
        HAL_Delay(12);
        LCD_WR_REG(0x01); // ?????????
        HAL_Delay(10);

        LCD_WR_REG(0xE6); // ??????????????????,33Mhz
        LCD_WR_DATA(0x2F);
        LCD_WR_DATA(0xFF);
        LCD_WR_DATA(0xFF);

        LCD_WR_REG(0xB0);  // ??????LCD??????
        LCD_WR_DATA(0x20); // 24?????????
        LCD_WR_DATA(0x00); // TFT ??????

        LCD_WR_DATA((SSD_HOR_RESOLUTION - 1) >> 8); // ??????LCD????????????
        LCD_WR_DATA(SSD_HOR_RESOLUTION - 1);
        LCD_WR_DATA((SSD_VER_RESOLUTION - 1) >> 8); // ??????LCD????????????
        LCD_WR_DATA(SSD_VER_RESOLUTION - 1);
        LCD_WR_DATA(0x00); // RGB??????

        LCD_WR_REG(0xB4); // Set horizontal period
        LCD_WR_DATA((SSD_HT - 1) >> 8);
        LCD_WR_DATA(SSD_HT - 1);
        LCD_WR_DATA(SSD_HPS >> 8);
        LCD_WR_DATA(SSD_HPS);
        LCD_WR_DATA(SSD_HOR_PULSE_WIDTH - 1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB6); // Set vertical period
        LCD_WR_DATA((SSD_VT - 1) >> 8);
        LCD_WR_DATA(SSD_VT - 1);
        LCD_WR_DATA(SSD_VPS >> 8);
        LCD_WR_DATA(SSD_VPS);
        LCD_WR_DATA(SSD_VER_FRONT_PORCH - 1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);

        LCD_WR_REG(0xF0);  // ??????SSD1963???CPU?????????16bit
        LCD_WR_DATA(0x03); // 16-bit(565 format) data for 16bpp

        LCD_WR_REG(0x29); // ????????????
        // ??????PWM??????  ???????????????????????????
        LCD_WR_REG(0xD0);  // ?????????????????????DBC
        LCD_WR_DATA(0x00); // disable

        LCD_WR_REG(0xBE);  // ??????PWM??????
        LCD_WR_DATA(0x05); // 1??????PWM??????
        LCD_WR_DATA(0xFE); // 2??????PWM?????????
        LCD_WR_DATA(0x01); // 3??????C
        LCD_WR_DATA(0x00); // 4??????D
        LCD_WR_DATA(0x00); // 5??????E
        LCD_WR_DATA(0x00); // 6??????F

        LCD_WR_REG(0xB8);  // ??????GPIO??????
        LCD_WR_DATA(0x03); // 2???IO??????????????????
        LCD_WR_DATA(0x01); // GPIO???????????????IO??????
        LCD_WR_REG(0xBA);
        LCD_WR_DATA(0X01); // GPIO[1:0]=01,??????LCD??????

        LCD_SSD_BackLightSet(100); // ?????????????????????
    }
    // ?????????????????????,??????
    if (lcddev.id == 0X9341 || lcddev.id == 0X7789 || lcddev.id == 0X5310 ||
            lcddev.id == 0X5510 ||
            lcddev.id == 0X1963) // ??????????????????IC,?????????WR???????????????
    {
        // ?????????????????????????????????????????????
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 0); // ??????????????????(ADDSET)??????
        FSMC_Bank1E->BWTR[6] &= ~(0XF << 8); // ????????????????????????
        FSMC_Bank1E->BWTR[6] |= 3 << 0; // ??????????????????(ADDSET)???4???HCLK =24ns
        if (lcddev.id == 0X7789) // 7789????????????,????????????????????????????????????
        {
            FSMC_Bank1E->BWTR[6] |= 3 << 8; // ??????????????????(DATAST)???6ns*4???HCLK=24ns
        } else {
            FSMC_Bank1E->BWTR[6] |= 2 << 8; // ??????????????????(DATAST)???6ns*3???HCLK=18ns
        }
    }
    LCD_Display_Dir(0);     // ???????????????
    GPIOB->BSRR = 1u << 15; // ????????????
    LCD_Clear(WHITE);
}
// ????????????
// color:?????????????????????
void LCD_Clear(uint32_t color) {
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height; // ???????????????
    LCD_SetCursor(0x00, 0x0000); // ??????????????????
    LCD_WriteRAM_Prepare();      // ????????????GRAM
    for (index = 0; index < totalpoint; index++) {
        LCD->LCD_RAM = color;
    }
}

// ????????????????????????????????????
//(sx,sy),(ex,ey):????????????????????????,???????????????:(ex-sx+1)*(ey-sy+1)
// color:??????????????????
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey,
                            uint32_t color) {
    uint16_t i, j;
    uint16_t xlen = 0;
    xlen = ex - sx + 1;
    for (i = sy; i <= ey; i++) {
        LCD_SetCursor(sx, i);   // ??????????????????
        LCD_WriteRAM_Prepare(); // ????????????GRAM
        for (j = 0; j < xlen; j++) {
            LCD->LCD_RAM = color; // ??????????????????
        }
    }
}

// ???????????????????????????????????????
//(sx,sy),(ex,ey):????????????????????????,???????????????:(ex-sx+1)*(ey-sy+1)
// color:??????????????????
void LCD_Color_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey,
                                        uint16_t *color) {
    uint16_t height, width;
    uint16_t i, j;
    width = ex - sx + 1;  // ?????????????????????
    height = ey - sy + 1; // ??????
    for (i = 0; i < height; i++) {
        LCD_SetCursor(sx, sy + i); // ??????????????????
        LCD_WriteRAM_Prepare();    // ????????????GRAM
        for (j = 0; j < width; j++) {
            LCD->LCD_RAM = color[i * width + j]; // ????????????
        }
    }
}

// ??????
// x1,y1:????????????
// x2,y2:????????????
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; // ??????????????????
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)
        incx = 1; // ??????????????????
    else if (delta_x == 0)
        incx = 0; // ?????????
    else {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0; // ?????????
    else {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
        distance = delta_x; // ???????????????????????????
    else
        distance = delta_y;

    for (t = 0; t <= distance + 1; t++) // ????????????
    {
        LCD_DrawPoint(uRow, uCol); // ??????
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

// ?????????
//(x1,y1),(x2,y2):?????????????????????
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

// ??????????????????????????????????????????
//(x,y):?????????
// r    :??????
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r) {
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1); // ??????????????????????????????

    while (a <= b) {
        LCD_DrawPoint(x0 + a, y0 - b); // 5
        LCD_DrawPoint(x0 + b, y0 - a); // 0
        LCD_DrawPoint(x0 + b, y0 + a); // 4
        LCD_DrawPoint(x0 + a, y0 + b); // 6
        LCD_DrawPoint(x0 - a, y0 + b); // 1
        LCD_DrawPoint(x0 - b, y0 + a);
        LCD_DrawPoint(x0 - a, y0 - b); // 2
        LCD_DrawPoint(x0 - b, y0 - a); // 7
        a++;

        // ??????Bresenham????????????
        if (di < 0)
            di += 4 * a + 6;
        else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

// ?????????????????????????????????
// x,y:????????????
// num:??????????????????:" "--->"~"
// size:???????????? 12/16/24/32
// mode:????????????(1)?????????????????????(0)
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size,
                                    uint8_t mode) {
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) *
                                    (size / 2); // ?????????????????????????????????????????????????????????
    num = num - ' '; // ????????????????????????ASCII???????????????????????????????????????-'
                                     // '??????????????????????????????
    for (t = 0; t < csize; t++) {
        if (size == 12)
            temp = asc2_1206[num][t]; // ??????1206??????
        else if (size == 16)
            temp = asc2_1608[num][t]; // ??????1608??????
        else if (size == 24)
            temp = asc2_2412[num][t]; // ??????2412??????
        else if (size == 32)
            temp = asc2_3216[num][t]; // ??????3216??????
        else
            return; // ???????????????
        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80)
                LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)
                LCD_Fast_DrawPoint(x, y, BACK_COLOR);
            temp <<= 1;
            y++;
            if (y >= lcddev.height)
                return; // ????????????
            if ((y - y0) == size) {
                y = y0;
                x++;
                if (x >= lcddev.width)
                    return; // ????????????
                break;
            }
        }
    }
}

// m^n??????
// ?????????:m^n??????.
uint32_t LCD_Pow(uint8_t m, uint8_t n) {
    uint32_t result = 1;
    while (n--)
        result *= m;
    return result;
}

// ????????????,?????????0,????????????
// x,y :????????????
// len :???????????????
// size:????????????
// color:??????
// num:??????(0~4294967295);
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,
                                 uint8_t size) {
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++) {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                LCD_ShowChar(x + (size / 2) * t, y, ' ', size, 0);
                continue;
            } else
                enshow = 1;
        }

        LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, 0);
    }
}

// ????????????,?????????0,????????????
// x,y:????????????
// num:??????(0~999999999);
// len:??????(?????????????????????)
// size:????????????
// mode:
//[7]:0,?????????;1,??????0.
//[6:1]:??????
//[0]:0,???????????????;1,????????????.
void LCD_ShowxNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,
                                    uint8_t size, uint8_t mode) {
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++) {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                if (mode & 0X80)
                    LCD_ShowChar(x + (size / 2) * t, y, '0', size, mode & 0X01);
                else
                    LCD_ShowChar(x + (size / 2) * t, y, ' ', size, mode & 0X01);
                continue;
            } else
                enshow = 1;
        }
        LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, mode & 0X01);
    }
}

// ???????????????
// x,y:????????????
// width,height:????????????
// size:????????????
//*p:?????????????????????
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                        uint8_t size, uint8_t *p) {
    uint8_t x0 = x;
    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' ')) // ???????????????????????????!
    {
        if (x >= width) {
            x = x0;
            y += size;
        }

        if (y >= height)
            break; // ??????

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

#endif
