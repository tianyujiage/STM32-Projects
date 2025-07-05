/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
#include "user_ScrRenewTask.h"
#include "user_SensUpdateTask.h"
#include "ui_HomePage.h"
#include "ui_MenuPage.h"
#include "ui_SetPage.h"
#include "ui_HRPage.h"
#include "ui_SPO2Page.h"
#include "ui_ENVPage.h"
#include "ui_CompassPage.h"
#include "main.h"

#include "AHT21.h"
#include "LSM303.h"
#include "SPL06_001.h"
#include "em70x8.h"
#include "HrAlgorythm.h"

#include "HWDataAccess.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint32_t user_HR_timecount=0;

/* Private function prototypes -----------------------------------------------*/
// 这是EM7028官方lib的库函数, 没有lib用不了
extern uint8_t GET_BP_MAX (void);
extern uint8_t GET_BP_MIN (void);
extern void Blood_Process (void);
extern void Blood_50ms_process (void);
extern void Blood_500ms_process(void);
extern int em70xx_bpm_dynamic(int RECEIVED_BYTE, int g_sensor_x, int g_sensor_y, int g_sensor_z);
extern int em70xx_reset(int ref);


/**
  * @brief  MPU6050 Check the state
  * @param  argument: Not used
  * @retval None
  */
  //若手腕状态不水平，进入睡眠模式
void MPUCheckTask(void *argument)
{
	while(1)
	{
		if(HWInterface.IMU.wrist_is_enabled)
		{
			if(MPU_isHorizontal()) //判断MPU6050是否是水平状态
			{
				HWInterface.IMU.wrist_state = WRIST_UP;
			}
			else
			{
				if(WRIST_UP == HWInterface.IMU.wrist_state)
				{
					HWInterface.IMU.wrist_state = WRIST_DOWN;
					if( Page_Get_NowPage()->page_obj == &ui_HomePage ||
						Page_Get_NowPage()->page_obj == &ui_MenuPage ||
						Page_Get_NowPage()->page_obj == &ui_SetPage )
					{
						uint8_t Stopstr;
						osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);//sleep
					}
				}
				HWInterface.IMU.wrist_state = WRIST_DOWN;
			}
		}
		osDelay(300);
	}
}

/**
  * @brief  HR data renew task
  * @param  argument: Not used
  * @retval None
  */
//更新心率数据
void HRDataUpdateTask(void *argument)
{
	uint8_t IdleBreakstr=0;
	uint16_t dat=0;
	uint8_t hr_temp=0;
	while(1)
	{
		if(Page_Get_NowPage()->page_obj == &ui_HRPage)//心率页面
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
			//传感器唤醒
			EM7028_hrs_Enable();
			//接收传感器唤醒消息，传感器唤醒
			if(!HWInterface.HR_meter.ConnectionError)
			{
				//Hr messure
				vTaskSuspendAll();
				hr_temp = HR_Calculate(EM7028_Get_HRS1(),user_HR_timecount);
				xTaskResumeAll();
				if(HWInterface.HR_meter.HrRate != hr_temp && hr_temp>50 && hr_temp<120)
				{
					HWInterface.HR_meter.HrRate = hr_temp;
				}
			}
		}
		osDelay(50);
	}
}


/**
  * @brief  Sensor data update task
  * @param  argument: Not used
  * @retval None
  */
//根据当前页面更新不同传感器的数据，并且在主页面更新时触发数据保存操作
void SensorDataUpdateTask(void *argument)
{
	uint8_t value_strbuf[6];
	uint8_t IdleBreakstr=0;
	while(1)
	{
		// Update the sens data showed in Home
		uint8_t HomeUpdataStr;
		if(osMessageQueueGet(HomeUpdata_MessageQueue, &HomeUpdataStr, NULL, 0)==osOK)
		{
			//电池电量
			uint8_t value_strbuf[5];

			HWInterface.Power.power_remain = HWInterface.Power.BatCalculate();//计算电池剩余电量
			if(HWInterface.Power.power_remain>0 && HWInterface.Power.power_remain<=100)
			{}
			else
			{HWInterface.Power.power_remain = 0;}

			//步数
			if(!(HWInterface.IMU.ConnectionError))
			{
				HWInterface.IMU.Steps = HWInterface.IMU.GetSteps();
			}

			//温湿度
			if(!(HWInterface.AHT21.ConnectionError))
			{
				//temp and humi messure
				float humi,temp;
				HWInterface.AHT21.GetHumiTemp(&humi,&temp);
				//check
				if(temp>-10 && temp<50 && humi>0 && humi<100)
				{
					// ui_EnvTempValue = (int8_t)temp;
					// ui_EnvHumiValue = (int8_t)humi;
					HWInterface.AHT21.humidity = humi;
					HWInterface.AHT21.temperature = temp;
				}
			}

			//发送数据保存消息队列
			uint8_t Datastr = 3;
			osMessageQueuePut(DataSave_MessageQueue, &Datastr, 0, 1);

		}


		// SPO2页面
		if(Page_Get_NowPage()->page_obj == &ui_SPO2Page)
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
			//sensor wake up

			//receive the sensor wakeup message, sensor wakeup
			if(0)
			{
				//SPO2 messure
			}
		}
		// 环境页面
		else if(Page_Get_NowPage()->page_obj == &ui_EnvPage)
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
			//receive the sensor wakeup message, sensor wakeup
			if(!HWInterface.AHT21.ConnectionError)
			{
				//temp and humi messure
				float humi,temp;
				HWInterface.AHT21.GetHumiTemp(&humi,&temp);
				//check
				if(temp>-10 && temp<50 && humi>0 && humi<100)
				{
					HWInterface.AHT21.temperature = (int8_t)temp;
					HWInterface.AHT21.humidity = (int8_t)humi;
				}
			}

		}
		//指南针页面
		else if(Page_Get_NowPage()->page_obj == &ui_CompassPage)
		{
			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
			//receive the sensor wakeup message, sensor wakeup
			LSM303DLH_Wakeup();
			//SPL_Wakeup();
			//if the sensor is no problem
			if(!HWInterface.Ecompass.ConnectionError)
			{
				//messure
				int16_t Xa,Ya,Za,Xm,Ym,Zm;
				LSM303_ReadAcceleration(&Xa,&Ya,&Za);
				LSM303_ReadMagnetic(&Xm,&Ym,&Zm);
				float temp = Azimuth_Calculate(Xa,Ya,Za,Xm,Ym,Zm)+0;//0 offset
				if(temp<0)
				{temp+=360;}
				//check
				if(temp>=0 && temp<=360)
				{
					HWInterface.Ecompass.direction = (uint16_t)temp;
				}
			}
			//if the sensor is no problem
			if(!HWInterface.Barometer.ConnectionError)
			{
				//messure
				float alti = Altitude_Calculate();
				//check
				if(1)
				{
					HWInterface.Barometer.altitude = (int16_t)alti;
				}
			}
		}

		osDelay(500);
	}
}
