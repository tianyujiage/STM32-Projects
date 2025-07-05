#include "delay.h"
#include "sys.h"
 
/* 
//roughly delay
void delay_us(u32 nus)
{		
	u32 i;
	u16 temp;
	temp = nus*SYS_CLK/15;
	for(i=0;i<temp;i++);
}

//延时nms
//nms:要延时的ms数
void delay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}
*/

void delay_init(void)
{
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);   //配置时钟源
	/* Configure the SysTick to have interrupt in 1ms time basis*/
	HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));  //配置定时器
}

#if OS_SUPPORT 						    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				   	 
	ticks=nus*SYS_CLK; 						 
	delay_osschedlock();					
	told=SysTick->VAL;        				
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;//这里注意一下SYSTICK是一个递减的计数器就可以了.	
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	};
	delay_osschedunlock();															    
}  

void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting==0)    
	{		 
		if(nms>=fac_ms)						
		{ 
   			delay_ostimedly(nms/fac_ms);	
		}
		nms%=fac_ms;						 
	}
	delay_us((u32)(nms*1000));			
}
#else  
	 
void delay_us(u32 nus)
{		
	u32 ticks;  //存储目标的滴答数
	u32 told,tnow,tcnt=0;   
	//tole记录上一次SysTick定时器值，tnow记录当前值，tcnt累计滴答数并初始化为0
	u32 reload=SysTick->LOAD;		
    //获取SysTick重载值，决定定时器计数周期	
	ticks=nus*SYS_CLK; 						
	told=SysTick->VAL;   //记录初始SysTick定时器值			
	while(1)
	{
		tnow=SysTick->VAL;	//读取当前SysTick定时器值
		if(tnow!=told)  //不等才会有滴答数
		{	    
			if(tnow<told)tcnt+=told-tnow;	//若tnow小于上一次told，说明定时器发生下溢
			else tcnt+=reload-tnow+told;	  //从told到tnow全部的滴答数  
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	}
}

void delay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}
#endif
