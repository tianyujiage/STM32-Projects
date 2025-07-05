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
EEPROM ����˵��:
[0x00]:0x55 ����У��
[0x01]:0xAA ����У��

[0x10]:�洢�û�����������Ϣ,��Ӧ HWInterface.IMU.wrist_is_enabled
[0x11]:�洢�û� ui_APPSy_EN ��Ϣ

[0x20]:�洢�ϴα������ݵ�����(0-31)
[0x21]:�洢����Ĳ���

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
			���ø���
			���ڸ���
			��������
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


