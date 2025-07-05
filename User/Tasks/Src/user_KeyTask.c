/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
#include "ui_HomePage.h"
#include "main.h"
#include "key.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  Key press check task
  * @param  argument: Not used
  * @retval None
  */
  //按键发生即发出信号量
  
void KeyTask(void *argument)
{
	uint8_t keystr=0;
	uint8_t Stopstr=0;
	uint8_t IdleBreakstr=0;
	while(1)
	{
		switch(KeyScan(0))
		{
			case 1:  
				keystr = 1;
				osMessageQueuePut(Key_MessageQueue, &keystr, 0, 1);//告诉其他任务按键已按下
				osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);//告知系统空闲状态被打断
				break;

			case 2:
				if(Page_Get_NowPage()->page_obj == &ui_HomePage)//页面是否为主页
				{
					osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
				}
				else
				{
					keystr = 2;
					osMessageQueuePut(Key_MessageQueue, &keystr, 0, 1);
					osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
				}
				break;
		}
		osDelay(1);
	}
}
