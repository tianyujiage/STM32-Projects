/* Private includes -----------------------------------------------------------*/
//includes
#include "user_DataSaveTask.h"
//APP SYS setting
#include "ui_DateTimeSetPage.h"
#include "ui_HomePage.h"

#include "main.h"
#include "rtc.h"
#include "DataSave.h"
#include "inv_mpu_dmp_motion_driver.h"

#include "HWDataAccess.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/******************************************
EEPROM 数据说明:
[0x00]:0x55 用于校验
[0x01]:0xAA 用于校验

[0x10]:存储用户手腕设置信息,对应 HWInterface.IMU.wrist_is_enabled
[0x11]:存储用户 ui_APPSy_EN 信息

[0x20]:存储上次保存数据的日期(0-31)
[0x21]:存储当天的步数

*******************************************/


/* Private function prototypes -----------------------------------------------*/

/* Tasks ---------------------------------------------------------------------*/

/**
  * @brief  Data Save in the EEPROM
  * @param  argument: Not used
  * @retval None
  */
void DataSaveTask(void *argument)
{

	while(1)
	{
		uint8_t Datastr=0;
		if(osMessageQueueGet(DataSave_MessageQueue,&Datastr,NULL,1)==osOK)
		{
			/****************
			设置更改
			日期更改
			步数更改
			****************/
			uint8_t dat[3];
			dat[0] = HWInterface.IMU.wrist_is_enabled;
			dat[1] = ui_APPSy_EN;
			SettingSave(dat,0x10,2);

			RTC_DateTypeDef nowdate;
			HAL_RTC_GetDate(&hrtc,&nowdate,RTC_FORMAT_BIN);

			SettingGet(dat,0x20,3);
			if(dat[0] != nowdate.Date)
			{
				if(!HWInterface.IMU.ConnectionError)
					HWInterface.IMU.SetSteps(0);

				dat[0] = nowdate.Date;
				dat[2] = 0;
				dat[1] = 0;
				SettingSave(dat,0x20,3);
			}
			else
			{
				uint16_t temp = HWInterface.IMU.GetSteps();
				dat[0] = nowdate.Date;
				dat[2] = temp & 0xff;
				dat[1] = temp>>8 & 0xff;
				SettingSave(dat,0x20,3);
			}

		}
		osDelay(100);
	}
}


