#if 0

#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// false for vertical
// true for horitonal
typedef struct {
    bool direction;
} LCD_settings_t;
extern LCD_settings_t LCD_settings;

/**
 * @brief Scanning direction.
 * @warning Don't change the number as it's the exact command to the screen.
 */
typedef enum {
    L2R_U2D = 0, // left-to-right, upward-to-downward
    L2R_D2U = 4,
    R2L_U2D = 2,
    R2L_D2U = 6,
    U2D_L2R = 1,
    U2D_R2L = 3,
    D2U_L2R = 5,
    D2U_R2L = 7,
} direction_t;

// color constants
enum {
    COLOR_WHITE         = 0xffff,
    COLOR_BLACK         = 0x0000,
    COLOR_BLUE          = 0x001f,
    COLOR_BRED          = 0xf81f,
    COLOR_GRED          = 0xffe0,
    COLOR_GBLUE         = 0x07ff,
    COLOR_RED           = 0xf800,
    COLOR_GREEN         = 0x07e0,
    COLOR_YELLOW        = 0xffe0,
    COLOR_LIGHTGREEN    = 0x841f,
};

// font size, there're only 4 alternatives in the font library
typedef enum {
    FONT_SMALL,
    FONT_MEDIUM,
    FONT_LARGE,
    FONT_VERY_LARGE,
} font_size_t;

/**
 * @brief Init the LCD screen
 * @details Call it manually in main function after MX_GPIO_Init and MX_FSMC_Init
 *          because it doesn't work well with CubeMX. 
 *          Pins to use: PB15, PD0, 1, 4, 5, 8, 9, 10, 14, 15,
 *                       PE7, 8, 9, 10, 11, 12, 13, 14, 15, PF12, PG12
 *          Works to do: 1. Init the pins
 *                       2. Init FSMC
 *                       3. Send query command and assert the screen type is 5510
 *                       4. Send init commands to screen
 *                       5. Set FSMC to higher speed
 *                       6. Init global variable LCD_settings, and set the scan direction to default
 *                       7. Lighten the background light
 */
void LCD_init(void);

/**
 * @brief Change the display direction and clear the screen if it does change. 
 * @warning Never use it to fresh. 
 * @param direction : the new direction, false for vertical, true for horizonal
 */
void LCD_display_dir(bool direction);
/**
 * @brief Set current cursor position. 
 * @param x
 * @param y
 */
void LCD_set_cursor(uint16_t x, uint16_t y);
/**
 * @brief Clear the screen with the given color. 
 *        Most common usage: LCD_clear(COLOR_WHITE);
 * @param color : the color to fill the screen
 */
void LCD_clear(uint16_t color);

/**
 * @brief Draw a point at the given position with the given color.
 * @param x
 * @param y
 * @param color
 */
void LCD_draw_point(uint16_t x, uint16_t y, uint16_t color);
/**
 * @brief Display a single char at the given position.
 * @param x : position x
 * @param y : position y
 * @param character : the char to be displayed
 * @param font_size : the font size
 * @param transparent : whether the blank space of a char is white or transparent
 * @warning Bug exists.
 */
void LCD_display_char(uint16_t x, uint16_t y, uint8_t character, font_size_t font_size, bool transparent);
/**
 * @brief Copied from other's code. It's correct one, but calling arguments are different.
 * @param x : position x
 * @param y : position y
 * @param num : the char to be displayed
 * @param size : one of the font size, others will lead to undefined behaviour
 * @param mode : transparent mode
 * @deprecated please fix the method above
 */
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);
/**
 * @brief Print the given string in the given area.
 * @details Imagin that there is a rectangular textbox,
 *          the first 4 params define the box size and position.
 *          Then specify the font size and input the string.
 * @warning not tested
 * @param x : starting position x
 * @param y : starting position y
 * @param width : the specific area width
 * @param height : the specific area height
 * @param font_size : font size
 * @param p : a pointer to string. may need reinterpret cast from char*
 * @return int : the number of letters printed
 */
int LCD_print_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, font_size_t font_size, uint8_t* p);
/**
 * @brief Print the string using printf
 * @warning not tested
 */
int LCD_print_format_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, font_size_t font_size, char const* format, ...)
__attribute__((format(printf, 6, 7)));

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H */

#else

#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_sram.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//2.8寸/3.5寸/4.3寸/7寸 TFT液晶驱动	  
//支持驱动IC型号包括:ILI9341/NT35310/NT35510/SSD1963等		    
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2017/4/8
//版本：V4.2
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved	
//******************************************************************************* 
//修改说明
//V4.0 20211111
//1，去掉大部分不常用驱动IC的支持
//2，新增对ST7789驱动IC的支持
//3，优化代码结构（精简源码）
//V4.1 20211208
//修改NT5510 ID读取方式,改为先发送秘钥,然后读取C500和C501,从而获取正确的ID(0X5510)
//V4.2 20211222
//1，修改FSMC速度，兼容7789显示OV2640数据
//2，解决因NT5510 ID读取（发送C501指令）导致SSD1963误触发软件复位进而读取不到ID问题，加延时解决
//////////////////////////////////////////////////////////////////////////////////	 

extern SRAM_HandleTypeDef TFTSRAM_Handler;    //SRAM句柄(用于控制LCD)

//LCD重要参数集
typedef struct  
{		 	 
	uint16_t width;			//LCD 宽度
	uint16_t height;			//LCD 高度
	uint16_t id;				//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	wramcmd;		//开始写gram指令
	uint16_t setxcmd;		//设置x坐标指令
	uint16_t setycmd;		//设置y坐标指令 
}_lcd_dev; 	  

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
//LCD的画笔颜色和背景色	   
extern uint32_t  POINT_COLOR;//默认红色    
extern uint32_t  BACK_COLOR; //背景颜色.默认为白色


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------MCU屏 LCD端口定义---------------- 
#define	LCD_LED PBout(15) 	//LCD背光	PB15	    
//LCD地址结构体
typedef struct
{
	uint16_t volatile LCD_REG;
	uint16_t volatile LCD_RAM;
} LCD_TypeDef;
//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A6作为数据命令区分线 
//注意设置时STM32内部会右移一位对其! 111 1110=0X7E			    
#define LCD_BASE        ((uint32_t)(0x6C000000 | 0x0000007E))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
	 
//扫描方向定义
#define L2R_U2D  0 		//从左到右,从上到下
#define L2R_D2U  1 		//从左到右,从下到上
#define R2L_U2D  2 		//从右到左,从上到下
#define R2L_D2U  3 		//从右到左,从下到上

#define U2D_L2R  4 		//从上到下,从左到右
#define U2D_R2L  5 		//从上到下,从右到左
#define D2U_L2R  6 		//从下到上,从左到右
#define D2U_R2L  7		//从下到上,从右到左	 

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)
	    															  
void LCD_Init(void);													   	//初始化
void LCD_DisplayOn(void);													//开显示
void LCD_DisplayOff(void);													//关显示
void LCD_Clear(uint32_t Color);	 												//清屏
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);										//设置光标
void LCD_DrawPoint(uint16_t x,uint16_t y);											//画点
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color);								//快速画点
uint32_t  LCD_ReadPoint(uint16_t x,uint16_t y); 											//读点 
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);						 			//画圆
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);							//画线
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   				//画矩形
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);		   				//填充单色
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);				//填充指定颜色
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);						//显示一个字符
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);  						//显示一个数字
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);				//显示 数字
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p);		//显示一个字符串,12/16字体

void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_SSD_BackLightSet(uint8_t pwm);							//SSD1963 背光控制
void LCD_Scan_Dir(uint8_t dir);									//设置屏扫描方向
void LCD_Display_Dir(uint8_t dir);								//设置屏幕显示方向
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);	//设置窗口					   						   																			 
//LCD分辨率设置
#define SSD_HOR_RESOLUTION		800		//LCD水平分辨率
#define SSD_VER_RESOLUTION		480		//LCD垂直分辨率
//LCD驱动参数设置
#define SSD_HOR_PULSE_WIDTH		1		//水平脉宽
#define SSD_HOR_BACK_PORCH		46		//水平前廊
#define SSD_HOR_FRONT_PORCH		210		//水平后廊

#define SSD_VER_PULSE_WIDTH		1		//垂直脉宽
#define SSD_VER_BACK_PORCH		23		//垂直前廊
#define SSD_VER_FRONT_PORCH		22		//垂直前廊
//如下几个参数，自动计算
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif

void LCD_PrintString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char const* format, ...) __attribute__((format(printf, 6, 7)));

#endif
