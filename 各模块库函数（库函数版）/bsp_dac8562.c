/*
*********************************************************************************************************
*
*	模块名称 : DAC8562/8563 驱动模块(单通道带16位DAC)
*	文件名称 : bsp_dac8562.c
*	版    本 : V1.0
*	说    明 : DAC8562/8563模块和CPU之间采用SPI接口。本驱动程序支持硬件SPI接口和软件SPI接口。
*			  通过宏切换。
*
*	修改记录 :
*		版本号  日期         作者     说明
*		V1.0    2014-01-17  armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

#define SOFT_SPI		/* 定义此行表示使用GPIO模拟SPI接口 */
//#define HARD_SPI		/* 定义此行表示使用CPU的硬件SPI接口 */

/*
	DAC8501模块可以直接插到STM32-V5开发板CN19排母(2*4P 2.54mm)接口上

   AD9833/ DAC8563模块    STM32F407开发板
	  GND   ------  GND    
	  VCC   ------  3.3V
	  
			FSYNC/SYNC  ------  PF7/NRF24L01_CSN
      	  
      SCLK/SCLK  ------  PB3/SPI3_SCK
      MOSI/DIN   ------  PB5/SPI3_MOSI

			------  PB4/SPI3_MISO
 	  CLR   ------  PH7/NRF24L01_IRQ
*/

/*
	AD9833基本特性:
	1、供电2.3 - 5.5V;  【本例使用3.3V】
	4、参考电压2.5V，使用内部参考

	对SPI的时钟速度要求: 高达40MHz， 速度很快.
	SCLK下降沿读取数据, 每次传送24bit数据， 高位先传?�/???
*/

#if !defined(SOFT_SPI) && !defined(HARD_SPI)
 	#error "Please define SPI Interface mode : SOFT_SPI or HARD_SPI"
#endif

#ifdef SOFT_SPI		/* 软件SPI */
	/* 定义GPIO端口 */
	#define RCC_SCLK 	RCC_AHB1Periph_GPIOB
	#define PORT_SCLK	GPIOB
	#define PIN_SCLK	GPIO_Pin_3
	
	#define RCC_DIN 	RCC_AHB1Periph_GPIOB
	#define PORT_DIN	GPIOB
	#define PIN_DIN	GPIO_Pin_5
	
	/* 片选 */
	#define RCC_SYNC 	RCC_AHB1Periph_GPIOF
	#define PORT_SYNC	GPIOF
	#define PIN_SYNC	GPIO_Pin_7

	/* 定义口线置0和置1的宏 */
	#define SYNC_0()	PORT_SYNC->BSRRH = PIN_SYNC
	#define SYNC_1()	PORT_SYNC->BSRRL = PIN_SYNC

	#define SCLK_0()	PORT_SCLK->BSRRH = PIN_SCLK
	#define SCLK_1()	PORT_SCLK->BSRRL = PIN_SCLK

	#define DIN_0()		PORT_DIN->BSRRH = PIN_DIN
	#define DIN_1()		PORT_DIN->BSRRL = PIN_DIN

#endif

#ifdef HARD_SPI		/* 硬件SPI (未做) */
	;
#endif

/*
*********************************************************************************************************
*	函 数 名: bsp_InitAD9833
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 AD9833
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDAC8562(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef SOFT_SPI
	SYNC_1();	/* SYNC = 1 */

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_SCLK | RCC_DIN | RCC_SYNC, ENABLE);

	/* 配置几个推完输出IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = PIN_SCLK;
	GPIO_Init(PORT_SCLK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_DIN;
	GPIO_Init(PORT_DIN, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_SYNC;
	GPIO_Init(PORT_SYNC, &GPIO_InitStructure);
#endif

	/* Power up DAC-A and DAC-B */
	DAC8562_WriteCmd((4 << 19) | (0 << 16) | (3 << 0));
	
	/* LDAC pin inactive for DAC-B and DAC-A  不使用LDAC引脚更新数据 */
	DAC8562_WriteCmd((6 << 19) | (0 << 16) | (3 << 0));

	/* 复位2个DAC到中间值, 输出2.5V */
	DAC8562_SetData(0, 32767);
	DAC8562_SetData(1, 32767);

	/* 选择内部参考并复位2个DAC的增益=2 （复位时，内部参考是禁止的) */
	DAC8562_WriteCmd((7 << 19) | (0 << 16) | (1 << 0));
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_WriteCmd
*	功能说明: 向SPI总线发送16个bit数据
*	形    参: _cmd : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8562_WriteCmd(uint32_t _cmd)
{
	uint8_t i;
	
	FSYNC_0();
	
	/*　DAC8562 SCLK时钟高达50M，因此可以不延迟 */
	for(i = 0; i < 16; i++)
	{
		if (_cmd & 0x800000)
		{
			DIN_1();
		}
		else
		{
			DIN_0();
		}
		SCLK_1();
		_cmd <<= 1;
		SCLK_0();
	}
	
	FSYNC_1();
}

/*
*********************************************************************************************************
*函数名：AD9833_Select_Wave
*功能说明：
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	函 数 名: DAC8562_SetData
*	功能说明: 设置DAC输出，并立即更新。
*	形    参: _ch, 通道, 0 , 1
*		     _data : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8562_SetData(uint8_t _ch, uint16_t _dac)
{
	if (_ch == 0)
	{
		/* Write to DAC-A input register and update DAC-A; */
		DAC8562_WriteCmd((3 << 19) | (0 << 16) | (_dac << 0));
	}
	else if (_ch == 1)
	{
		/* Write to DAC-B input register and update DAC-A; */
		DAC8562_WriteCmd((3 << 19) | (1 << 16) | (_dac << 0));		
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
