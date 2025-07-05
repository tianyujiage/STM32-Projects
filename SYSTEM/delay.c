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

//��ʱnms
//nms:Ҫ��ʱ��ms��
void delay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}
*/

void delay_init(void)
{
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);   //����ʱ��Դ
	/* Configure the SysTick to have interrupt in 1ms time basis*/
	HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));  //���ö�ʱ��
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
			if(tnow<told)tcnt+=told-tnow;//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.	
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
	u32 ticks;  //�洢Ŀ��ĵδ���
	u32 told,tnow,tcnt=0;   
	//tole��¼��һ��SysTick��ʱ��ֵ��tnow��¼��ǰֵ��tcnt�ۼƵδ�������ʼ��Ϊ0
	u32 reload=SysTick->LOAD;		
    //��ȡSysTick����ֵ��������ʱ����������	
	ticks=nus*SYS_CLK; 						
	told=SysTick->VAL;   //��¼��ʼSysTick��ʱ��ֵ			
	while(1)
	{
		tnow=SysTick->VAL;	//��ȡ��ǰSysTick��ʱ��ֵ
		if(tnow!=told)  //���ȲŻ��еδ���
		{	    
			if(tnow<told)tcnt+=told-tnow;	//��tnowС����һ��told��˵����ʱ����������
			else tcnt+=reload-tnow+told;	  //��told��tnowȫ���ĵδ���  
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
