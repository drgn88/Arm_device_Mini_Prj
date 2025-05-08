#include "device_driver.h"

#define TIM2_TICK         	(20) 				// usec
#define TIM2_FREQ 	  		(1000000/TIM2_TICK)	// Hz
#define TIME2_PLS_OF_1ms  	(1000/TIM2_TICK)
#define TIM2_MAX	  		(0xffffu)
#define TIM2_1sec	  		(0xC350u)

#define TIM4_TICK	  		(20) 				// usec
#define TIM4_FREQ 	  		(1000000/TIM4_TICK) // Hz
#define TIME4_PLS_OF_1ms  	(1000/TIM4_TICK)
#define TIM4_MAX	  		(0xffffu)

#define TIM3_FREQ 	  		(8000000) 	      	// Hz
#define TIM3_TICK	  		(1000000/TIM3_FREQ)	// usec
#define TIME3_PLS_OF_1ms  	(1000/TIM3_TICK)

extern volatile int cnt_time;
extern volatile int TIM2_expired;

void TIM2_Repeat_Interrupt_Enable(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);
	TIM2->CR1 = (1<<4)|(1<<0);
	TIM2->PSC = (unsigned int)(TIMXCLK/50000.0 + 0.5)-1;
	TIM2->ARR = TIM2_MAX;
	Macro_Set_Bit(TIM2->EGR,0);

	// TIM2->SR 레지스터에서 Timer Pending Clear
	Macro_Clear_Bit(TIM2->SR, 0);
	// NVIC에서 28번 인터럽트 Pending Clear => NVIC용 Macro 사용
	NVIC_ClearPendingIRQ(28);
	// TIM4->DIER 레지스터에서 Timer 인터럽트 허용
	Macro_Set_Bit(TIM2->DIER, 0);
	// NVIC에서 30번 인터럽트를 허용으로 설정 => NVIC용 Macro 사용
	NVIC_EnableIRQ(28);
	Macro_Set_Bit(TIM2->CR1, 0);
}

void TIM2_Repeat_Interrupt_Enable_1sec(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);
	TIM2->CR1 = (1<<4)|(1<<0);
	TIM2->PSC = (unsigned int)(TIMXCLK/(double)TIM2_FREQ + 0.5)-1;
	TIM2->ARR = TIM2_1sec;
	Macro_Set_Bit(TIM2->EGR,0);

	// TIM2->SR 레지스터에서 Timer Pending Clear
	Macro_Clear_Bit(TIM2->SR, 0);
	// NVIC에서 28번 인터럽트 Pending Clear => NVIC용 Macro 사용
	NVIC_ClearPendingIRQ(28);
	// TIM4->DIER 레지스터에서 Timer 인터럽트 허용
	Macro_Set_Bit(TIM2->DIER, 0);
	// NVIC에서 30번 인터럽트를 허용으로 설정 => NVIC용 Macro 사용
	NVIC_EnableIRQ(28);
	Macro_Set_Bit(TIM2->CR1, 0);
}

#if 1
void TIM2_Repeat_Interrupt_Enable_time(int en, int time)
{
	if(en)
	{
		Macro_Set_Bit(RCC->APB1ENR, 0);

		TIM2->CR1 = (1<<4)|(0<<3);
		TIM2->PSC = (unsigned int)(TIMXCLK/(double)TIM2_FREQ + 0.5)-1;
		TIM2->ARR = TIME2_PLS_OF_1ms * time;
		Macro_Set_Bit(TIM2->EGR,0);

		// TIM2->SR 레지스터에서 Timer Pending Clear
		Macro_Clear_Bit(TIM2->SR, 0);
		// NVIC에서 28번 인터럽트 Pending Clear => NVIC용 Macro 사용
		NVIC_ClearPendingIRQ(28);
		// TIM2->DIER 레지스터에서 Timer 인터럽트 허용
		Macro_Set_Bit(TIM2->DIER, 0);
		// NVIC에서 28번 인터럽트를 허용으로 설정 => NVIC용 Macro 사용
		NVIC_EnableIRQ(28);
		// TIM2 Start
		TIM2->CR1 |= (1<<0);
	}

	else
	{
		NVIC_DisableIRQ(28);
		Macro_Clear_Bit(TIM2->CR1, 0);
		Macro_Clear_Bit(TIM2->DIER, 0);
		Macro_Clear_Bit(TIM2->SR, 0);
	}
}
#endif

#if 1
void TIM2_Stop(void)
{
	Macro_Clear_Bit(TIM2->CR1, 0);       // TIM2 Stop
    Macro_Clear_Bit(TIM2->DIER, 0);       // Interrupt Disable
	Macro_Clear_Bit(TIM2->SR, 0);       // 상태 플래그 클리어
    Macro_Clear_Bit(RCC->APB1ENR, 0);     // Clock Disable
    NVIC_DisableIRQ(28);                 // NVIC Interrupt Disable
    cnt_time = 0;
    TIM2_expired = 0;
}
#endif

#if 0
void TIM2_Stopwatch_Start(void)
{
	Macro_Set_Bit(TIM2->EGR,0);
	
}
#endif

unsigned int TIM2_Stopwatch_Stop(void)
{
	unsigned int time;

	Macro_Clear_Bit(TIM2->CR1, 0);
	time = (TIM2_MAX - TIM2->CNT) * TIM2_TICK;
	return time;
}

/* Delay Time Max = 65536 * 20use = 1.3sec */

#if 0

void TIM2_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	TIM2->CR1 = (1<<4)|(1<<3);
	TIM2->PSC = (unsigned int)(TIMXCLK/(double)TIM2_FREQ + 0.5)-1;
	TIM2->ARR = TIME2_PLS_OF_1ms * time;

	Macro_Set_Bit(TIM2->EGR,0);
	Macro_Clear_Bit(TIM2->SR, 0);
	Macro_Set_Bit(TIM2->DIER, 0);
	Macro_Set_Bit(TIM2->CR1, 0);

	while(Macro_Check_Bit_Clear(TIM2->SR, 0));

	Macro_Clear_Bit(TIM2->CR1, 0);
	Macro_Clear_Bit(TIM2->DIER, 0);
}

#else

/* Delay Time Extended */

void TIM2_Delay(int time)
{
	int i;
	unsigned int t = TIME2_PLS_OF_1ms * time;

	Macro_Set_Bit(RCC->APB1ENR, 0);

	TIM2->PSC = (unsigned int)(TIMXCLK/(double)TIM2_FREQ + 0.5)-1;
	TIM2->CR1 = (1<<4)|(1<<3);
	TIM2->ARR = 0xffff;
	Macro_Set_Bit(TIM2->EGR,0);
	Macro_Set_Bit(TIM2->DIER, 0);

	for(i=0; i<(t/0xffffu); i++)
	{
		Macro_Set_Bit(TIM2->EGR,0);
		Macro_Clear_Bit(TIM2->SR, 0);
		Macro_Set_Bit(TIM2->CR1, 0);
		while(Macro_Check_Bit_Clear(TIM2->SR, 0));
	}

	TIM2->ARR = t % 0xffffu;
	Macro_Set_Bit(TIM2->EGR,0);
	Macro_Clear_Bit(TIM2->SR, 0);
	Macro_Set_Bit(TIM2->CR1, 0);
	while (Macro_Check_Bit_Clear(TIM2->SR, 0));

	Macro_Clear_Bit(TIM2->CR1, 0);
	Macro_Clear_Bit(TIM2->DIER, 0);
}

#endif

void TIM4_Repeat(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 2);

	TIM4->CR1 = (1<<4)|(0<<3);
	TIM4->PSC = (unsigned int)(TIMXCLK/(double)TIM4_FREQ + 0.5)-1;
	TIM4->ARR = TIME4_PLS_OF_1ms * time - 1;

	Macro_Set_Bit(TIM4->EGR,0);
	Macro_Clear_Bit(TIM4->SR, 0);
	Macro_Set_Bit(TIM4->DIER, 0);
	Macro_Set_Bit(TIM4->CR1, 0);
}

int TIM4_Check_Timeout(void)
{
	if(Macro_Check_Bit_Set(TIM4->SR, 0))
	{
		Macro_Clear_Bit(TIM4->SR, 0);
		return 1;
	}
	else
	{
		return 0;
	}
}

void TIM4_Stop(void)
{
	Macro_Clear_Bit(TIM4->CR1, 0);
	Macro_Clear_Bit(TIM4->DIER, 0);
}

void TIM4_Change_Value(int time)
{
	TIM4->ARR = TIME4_PLS_OF_1ms * time;
}

void TIM3_Out_Init(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 1);
	Macro_Set_Bit(RCC->APB2ENR, 3);
	Macro_Write_Block(GPIOB->CRL,0xf,0xb,0);
	Macro_Write_Block(TIM3->CCMR2,0x7,0x6,4);
	TIM3->CCER = (0<<9)|(1<<8);
}

void TIM3_Out_Freq_Generation(unsigned short freq)
{
	TIM3->PSC = (unsigned int)(TIMXCLK/(double)TIM3_FREQ + 0.5)-1;
	TIM3->ARR = (double)TIM3_FREQ/freq-1;
	TIM3->CCR3 = TIM3->ARR/2;

	Macro_Set_Bit(TIM3->EGR,0);
	TIM3->CR1 = (1<<4)|(0<<3)|(0<<1)|(1<<0);
}

void TIM3_Out_Stop(void)
{
	Macro_Clear_Bit(TIM3->CR1, 0);
	Macro_Clear_Bit(TIM3->DIER, 0);
}

void TIM4_Repeat_Interrupt_Enable(int en, int time)
{
	if(en)
	{
		Macro_Set_Bit(RCC->APB1ENR, 2);

		TIM4->CR1 = (1<<4)|(0<<3);
		TIM4->PSC = (unsigned int)(TIMXCLK/(double)TIM4_FREQ + 0.5)-1;
		TIM4->ARR = TIME4_PLS_OF_1ms * time;
		Macro_Set_Bit(TIM4->EGR,0);

		// TIM4->SR 레지스터에서 Timer Pending Clear
		Macro_Clear_Bit(TIM4->SR, 0);
		// NVIC에서 30번 인터럽트 Pending Clear => NVIC용 Macro 사용
		NVIC_ClearPendingIRQ(30);
		// TIM4->DIER 레지스터에서 Timer 인터럽트 허용
		Macro_Set_Bit(TIM4->DIER, 0);
		// NVIC에서 30번 인터럽트를 허용으로 설정 => NVIC용 Macro 사용
		NVIC_EnableIRQ(30);
		// TIM4 Start
		TIM4->CR1 |= (1<<0);
	}

	else
	{
		NVIC_DisableIRQ(30);
		Macro_Clear_Bit(TIM4->CR1, 0);
		Macro_Clear_Bit(TIM4->DIER, 0);
		Macro_Clear_Bit(RCC->APB1ENR, 2);
	}
}

#if 0
void TIM4_Repeat_Interrupt_Enable(int en, int time)
{
  if(en)
  {
    Macro_Set_Bit(RCC->APB1ENR, 2);

    TIM4->CR1 = (1<<4)+(0<<3)+(0<<0);
    TIM4->PSC = (unsigned int)(TIMXCLK/(double)TIM4_FREQ + 0.5)-1;
    TIM4->ARR = TIME4_PLS_OF_1ms * time;

    Macro_Set_Bit(TIM4->EGR,0);
    Macro_Set_Bit(TIM4->SR, 0);
    NVIC_ClearPendingIRQ((IRQn_Type)30);
    Macro_Set_Bit(TIM4->DIER, 0);
    NVIC_EnableIRQ((IRQn_Type)30);
    Macro_Set_Bit(TIM4->CR1, 0);
  }

  else
  {
    NVIC_DisableIRQ((IRQn_Type)30);
    Macro_Clear_Bit(TIM4->CR1, 0);
    Macro_Clear_Bit(TIM4->DIER, 0);
    Macro_Clear_Bit(RCC->APB1ENR, 2);
  }
}
#endif

#if 0
// 음악재생을 위한 타이머
void TIM5_Oneshot_Interrupt_Enable(int en)
{
	if(en)
	{
		Macro_Set_Bit(RCC->APB1ENR, 3);

		//Down counter, One-shot
		TIM5->CR1 = (1<<4)|(1<<3);
		TIM5->PSC = (unsigned int)(TIMXCLK/(double)TIM2_FREQ + 0.5)-1;
		TIM5->ARR = TIME2_PLS_OF_1ms;
		Macro_Set_Bit(TIM5->EGR,0);

		// TIM5->SR 레지스터에서 Timer Pending Clear
		Macro_Clear_Bit(TIM5->SR, 0);
		// NVIC에서 50번 인터럽트 Pending Clear => NVIC용 Macro 사용
		NVIC_ClearPendingIRQ(50);
		// TIM5->DIER 레지스터에서 Timer 인터럽트 허용
		Macro_Set_Bit(TIM5->DIER, 0);
		// NVIC에서 50번 인터럽트를 허용으로 설정 => NVIC용 Macro 사용
		NVIC_EnableIRQ(50);
		// TIM5 Start
		TIM5->CR1 |= (1<<0);
	}

	else
	{
		NVIC_DisableIRQ(50);
		Macro_Clear_Bit(TIM5->CR1, 0);
		Macro_Clear_Bit(TIM5->DIER, 0);
	}
}

#endif