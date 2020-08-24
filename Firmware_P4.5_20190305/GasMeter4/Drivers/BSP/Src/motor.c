#include "motor.h"
#include "cmsis_os.h"
#include <string.h>


#define Lock_RUNTIME 	600
#define MOTOR_RUNTIME 600
#define Disassemble_RUNTIME 600

MotorStatus_t glockSet = motor_stop;//这个值存储指令发送过来的开关,只有确实开盖,或者时间到了才会赋值为空

MotorStatus_t glockStatus = motor_stop;	
MotorStatus_t gmotorStatus = motor_stop;	
MotorStatus_t gassembleStatus = motor_stop;

volatile uint32_t beginMotoTime = 0;

void lock_test()
{	
	switch (glockStatus)
  {
  	case motor_stop:
			//printf("lockStatus:%d\r\n",glockStatus);
			LOCK_HANDLE(motor_stop);
		  glockStatus = motor_null;
			MOTOR_PWR(0);
  		break;
  	case motor_close:
			//beginMotoTime = HAL_GetTick();
			MOTOR_PWR(1);
		  //if(strcmp(CONFIG_Meter.Lid_Type, "2")==0)
			{
			  osDelay(1000);
		    MOTOR_PWR(0);
			}
		  //printf("begiTime:%d\r\n",HAL_GetTick());
			LOCK_HANDLE(motor_close);	
		
//		  if(strcmp(CONFIG_Meter.Lid_Type, "1")==0)
//			{
//			  osDelay(3000);
//			}
//			else
			{
			  osDelay(Lock_RUNTIME);
				//ulTaskNotifyTake(pdTRUE,Lock_RUNTIME);
			}	
			printf("stopTime1:%d\r\n",HAL_GetTick());
			glockStatus = motor_stop;
  		break;
		case motor_open:
			//beginMotoTime = HAL_GetTick();
			MOTOR_PWR(1);
		  //if(strcmp(CONFIG_Meter.Lid_Type, "2")==0)
			{
			  osDelay(1000);
		    MOTOR_PWR(0);
			}

		  //printf("begiTime:%d\r\n",beginMotoTime);
			LOCK_HANDLE(motor_open); 
			//printf("lockStatus:%d\r\n",glockStatus);
//			if(strcmp(CONFIG_Meter.Lid_Type, "1")==0)
//			{
//			  osDelay(3000);
//			}
//			else
			{
			  osDelay(Lock_RUNTIME);
				//ulTaskNotifyTake(pdTRUE,Lock_RUNTIME);
			}
			printf("stopTime2:%d\r\n",HAL_GetTick());
			glockStatus = motor_stop;			
			break;
  	default:
  		break;
  }
}

void motor_test()
{	
	switch (gmotorStatus)
  {
  	case motor_stop:
			MOTOR2_HANDLE(motor_stop);
		  gmotorStatus = motor_null;
			MOTOR_PWR(0);
		  //printf("motor2_stop\r\n");
  		break;
  	case motor_close:
			MOTOR_PWR(1);
		  //if(strcmp(CONFIG_Meter.Lid_Type, "2")==0)
			{
			  osDelay(1000);
		    MOTOR_PWR(0);
			}
			MOTOR2_HANDLE(motor_close);	
			osDelay(MOTOR_RUNTIME);
			gmotorStatus = motor_stop;
		  //printf("motor2_close\r\n");
  		break;
		case motor_open:
			MOTOR_PWR(1);
		  //if(strcmp(CONFIG_Meter.Lid_Type, "2")==0)
			{
			  osDelay(1000);
		    MOTOR_PWR(0);
			}
			MOTOR2_HANDLE(motor_open);
			osDelay(MOTOR_RUNTIME);
			gmotorStatus = motor_stop;		
		  //printf("motor2_open\r\n");
			break;
  	default:
  		break;
  }
}

void assemble_test()
{	
	switch (gassembleStatus)
  {
  	case motor_stop:
			Assemble_HANDLE(motor_stop);
		  gassembleStatus = motor_null;
			MOTOR_PWR(0);
  		break;
  	case motor_close:
			MOTOR_PWR(1);
		  //if(strcmp(CONFIG_Meter.Lid_Type, "2")==0)
			{
			  osDelay(1000);
		    MOTOR_PWR(0);
			}
			Assemble_HANDLE(motor_close);	
			osDelay(Disassemble_RUNTIME);
			gassembleStatus = motor_stop;
  		break;
		case motor_open:
			MOTOR_PWR(1);
		  //if(strcmp(CONFIG_Meter.Lid_Type, "2")==0)
			{
			  osDelay(1000);
		    MOTOR_PWR(0);
			}
			Assemble_HANDLE(motor_open);
			osDelay(Disassemble_RUNTIME);
			gassembleStatus = motor_stop;		
			break;
  	default:
  		break;
  }
}

