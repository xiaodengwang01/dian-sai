//-----------------------------------------------------------------
//	TIMx定时器头文件
// 头文件名: TIMx.h
// 作    者: 凌智电子
// 编写日期: 2014-01-31
// 修改日期:
//-----------------------------------------------------------------
#ifndef _TIM_TimeBase_H
#define _TIM_TimeBase_H
#include <stm32f10x.h>
//-----------------------------------------------------------------
// 宏定义
//-----------------------------------------------------------------
#define Draw_Number  			280    			// 绘制点数
#define Sampling_Number  	290    			// 采样点数
//-----------------------------------------------------------------
// 外部函数声明
//-----------------------------------------------------------------
extern void TIM3_Mode_Init(uint16_t arr,uint16_t psc);
static void NVIC_Configuration(void);
extern void TIM3_Init(uint16_t arr,uint16_t psc);
#endif
//-----------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------
