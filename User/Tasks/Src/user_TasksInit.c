/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
//sys
#include "sys.h"
#include "stdio.h"
#include "lcd.h"
#include "WDOG.h"
//gui
#include "lvgl.h"
#include "ui_TimerPage.h"
//tasks
#include "user_HardwareInitTask.h"
#include "user_RunModeTasks.h"
#include "user_KeyTask.h"
#include "user_ScrRenewTask.h"
#include "user_SensUpdateTask.h"
#include "user_ChargCheckTask.h"
#include "user_MessageSendTask.h"
#include "user_DataSaveTask.h"
//ע��������񣬷���ռ䣬ע��һЩ�ź���������Ļ��ܿ��Կ�����ļ���
//ͬʱҲ������һ�������ʱ�������ڼ�¼����ʱ�䣬���û�û�в��������ͻᷢ��idle�źţ�
//idle�������ͻᷢ��STOP�źţ�����˯�ߡ�
//LVGL��ʱ���ṩҲ��������ļ��У�ΪLvHandlerTask�����Ź���ι��TaskҲ��������ļ��С�
/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Timers --------------------------------------------------------------------*/
osTimerId_t IdleTimerHandle;


/* Tasks ---------------------------------------------------------------------*/
// Hardwares initialization�洢Ӳ����ʼ��
osThreadId_t HardwareInitTaskHandle;
const osThreadAttr_t HardwareInitTask_attributes = {
  .name = "HardwareInitTask",
  .stack_size = 128 * 10,  //����ջ��С
  .priority = (osPriority_t) osPriorityHigh3, //�������ȼ�
};

//LVGL Handler task lvg��������
osThreadId_t LvHandlerTaskHandle;
const osThreadAttr_t LvHandlerTask_attributes = {
  .name = "LvHandlerTask",
  .stack_size = 128 * 24,
  .priority = (osPriority_t) osPriorityLow,
};

//WDOG Feed task���Ź�ι����
osThreadId_t WDOGFeedTaskHandle;
const osThreadAttr_t WDOGFeedTask_attributes = {
  .name = "WDOGFeedTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityHigh2,
};

//Idle Enter Task���н�������
osThreadId_t IdleEnterTaskHandle;
const osThreadAttr_t IdleEnterTask_attributes = {
  .name = "IdleEnterTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityHigh,
};

//Stop Enter Taskֹͣ��������
osThreadId_t StopEnterTaskHandle;
const osThreadAttr_t StopEnterTask_attributes = {
  .name = "StopEnterTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityHigh1,
};

//Key task��������
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityNormal,
};

//ScrRenew task��Ļˢ������
osThreadId_t ScrRenewTaskHandle;
const osThreadAttr_t ScrRenewTask_attributes = {
  .name = "ScrRenewTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};

//SensorDataRenew task���������ݸ���
osThreadId_t SensorDataTaskHandle;
const osThreadAttr_t SensorDataTask_attributes = {
  .name = "SensorDataTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

//HRDataRenew task
osThreadId_t HRDataTaskHandle;
const osThreadAttr_t HRDataTask_attributes = {
  .name = "HRDataTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

//ChargPageEnterTask���ҳ���������
osThreadId_t ChargPageEnterTaskHandle;
const osThreadAttr_t ChargPageEnterTask_attributes = {
  .name = "ChargPageEnterTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};

//messagesendtask��Ϣ����
osThreadId_t MessageSendTaskHandle;
const osThreadAttr_t MessageSendTask_attributes = {
  .name = "MessageSendTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow1,
};

//MPUCheckTask
osThreadId_t MPUCheckTaskHandle;
const osThreadAttr_t MPUCheckTask_attributes = {
  .name = "MPUCheckTask",
  .stack_size = 128 * 3,
  .priority = (osPriority_t) osPriorityLow2,
};

//DataSaveTask
osThreadId_t DataSaveTaskHandle;
const osThreadAttr_t DataSaveTask_attributes = {
  .name = "DataSaveTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityLow2,
};


/* Message queues���� ------------------------------------------------------------*/
//Key message
osMessageQueueId_t Key_MessageQueue;
osMessageQueueId_t Idle_MessageQueue;
osMessageQueueId_t Stop_MessageQueue;
osMessageQueueId_t IdleBreak_MessageQueue;
osMessageQueueId_t HomeUpdata_MessageQueue;
osMessageQueueId_t DataSave_MessageQueue;

/* Private function prototypes -----------------------------------------------*/
void LvHandlerTask(void *argument);  //LVGL��ʱ��
void WDOGFeedTask(void *argument);   //���Ź���ι��Task

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void User_Tasks_Init(void)
{
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

	IdleTimerHandle = osTimerNew(IdleTimerCallback, osTimerPeriodic, NULL, NULL);
	osTimerStart(IdleTimerHandle,100);//100ms

  /* add queues, ... */
	Key_MessageQueue  = osMessageQueueNew(1, 1, NULL);
	Idle_MessageQueue = osMessageQueueNew(1, 1, NULL);
	Stop_MessageQueue = osMessageQueueNew(1, 1, NULL);
	IdleBreak_MessageQueue = osMessageQueueNew(1, 1, NULL);
	HomeUpdata_MessageQueue = osMessageQueueNew(1, 1, NULL);
	DataSave_MessageQueue = osMessageQueueNew(2, 1, NULL);

	/* add threads,�����߳� ... */
  HardwareInitTaskHandle  = osThreadNew(HardwareInitTask, NULL, &HardwareInitTask_attributes);
  LvHandlerTaskHandle  = osThreadNew(LvHandlerTask, NULL, &LvHandlerTask_attributes);
  WDOGFeedTaskHandle   = osThreadNew(WDOGFeedTask, NULL, &WDOGFeedTask_attributes);
	IdleEnterTaskHandle  = osThreadNew(IdleEnterTask, NULL, &IdleEnterTask_attributes);
	StopEnterTaskHandle  = osThreadNew(StopEnterTask, NULL, &StopEnterTask_attributes);
	KeyTaskHandle 			 = osThreadNew(KeyTask, NULL, &KeyTask_attributes);
	ScrRenewTaskHandle   = osThreadNew(ScrRenewTask, NULL, &ScrRenewTask_attributes);
	SensorDataTaskHandle = osThreadNew(SensorDataUpdateTask, NULL, &SensorDataTask_attributes);
	HRDataTaskHandle		 = osThreadNew(HRDataUpdateTask, NULL, &HRDataTask_attributes);
	ChargPageEnterTaskHandle = osThreadNew(ChargPageEnterTask, NULL, &ChargPageEnterTask_attributes);
  MessageSendTaskHandle = osThreadNew(MessageSendTask, NULL, &MessageSendTask_attributes);
	MPUCheckTaskHandle		= osThreadNew(MPUCheckTask, NULL, &MPUCheckTask_attributes);
	DataSaveTaskHandle		= osThreadNew(DataSaveTask, NULL, &DataSaveTask_attributes);

  /* add events, ... */


	/* add  others ... */
	uint8_t HomeUpdataStr;
	osMessageQueuePut(HomeUpdata_MessageQueue, &HomeUpdataStr, 0, 1);

}


/**
  * @brief  FreeRTOS Tick Hook, to increase the LVGL tick
  * @param  None
  * @retval None
  */
void TaskTickHook(void)
{
	//to increase the LVGL tick
	lv_tick_inc(1);
	//to increase the timerpage's timer(put in here is to ensure the Real Time)
	if(ui_TimerPageFlag)  //��ʱ��ҳ��ά��
	{
			ui_TimerPage_ms+=1;
			if(ui_TimerPage_ms>=10)
			{
				ui_TimerPage_ms=0;
				ui_TimerPage_10ms+=1;
			}
			if(ui_TimerPage_10ms>=100)
			{
					ui_TimerPage_10ms=0;
					ui_TimerPage_sec+=1;
					uint8_t IdleBreakstr = 0;
					osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 0);
			}
			if(ui_TimerPage_sec>=60)
			{
					ui_TimerPage_sec=0;
					ui_TimerPage_min+=1;
			}
			if(ui_TimerPage_min>=60)
			{
					ui_TimerPage_min=0;
			}
	}
	user_HR_timecount+=1;
}


/**
  * @brief  LVGL Handler task, to run the lvgl
  * @param  argument: Not used
  * @retval None
  */
void LvHandlerTask(void *argument)  //lvglʱ��
{
	uint8_t IdleBreakstr=0;
  while(1)
  {
		if(lv_disp_get_inactive_time(NULL)<1000)
		{
			//Idle time break, set to 0
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 0);
		}
		lv_task_handler();
    osDelay(1);
	}
}


/**
  * @brief  Watch Dog Feed task
  * @param  argument: Not used
  * @retval None
  */
void WDOGFeedTask(void *argument)
{
	//owdg
	WDOG_Port_Init();
  while(1)
  {
		WDOG_Feed();
		WDOG_Enable();
    osDelay(100);
  }
}


