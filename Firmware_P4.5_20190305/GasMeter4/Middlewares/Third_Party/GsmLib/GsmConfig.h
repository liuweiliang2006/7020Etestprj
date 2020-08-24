#ifndef	_GSMCONFIG_H
#define	_GSMCONFIG_H
//	Please enable FreerRtos
//	Please Config your usart and enable interrupt on CubeMX
//	2 control Pin	 PowerKey>>>>output--open drain : default Value>>1         Power status>>>>>input---pulldown
// Select "General peripheral Initalizion as a pair of '.c/.h' file per peripheral" on project settings

#define		_GSM_DUAL_SIM_SUPPORT				0

#define		_GSM_USART									huart2

#define		_GSM_POWER_PORT							GSM_POWER_KEY_GPIO_Port
#define		_GSM_POWER_PIN							GSM_POWER_KEY_Pin
#define		_GSM_POWER_STATUS_PORT			GSM_STATUS_GPIO_Port
#define		_GSM_POWER_STATUS_PIN				GSM_STATUS_Pin

#define		_GSM_RX_SIZE								512
#define		_GSM_TX_SIZE								256
#define		_GSM_TASK_SIZE							1024


#define		_GSM_WAIT_TIME_LOW					1000
#define		_GSM_WAIT_TIME_MED					10000
#define		_GSM_WAIT_TIME_HIGH					25000
#define		_GSM_WAIT_TIME_VERYHIGH			80000



#endif
