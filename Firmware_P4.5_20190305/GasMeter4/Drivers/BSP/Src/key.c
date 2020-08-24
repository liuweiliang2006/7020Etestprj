/**
  ******************************************************************************
  * File Name          : key.c
  * Description        :
	* xudong
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "key.h"

/*============= 
低层按键（I/0）扫描函数，即低层按键设备驱动，只返回无键、短按和长按。具体双击不在此处判断。参考本人教材的例9-1，稍微有变化。教材中为连_发。 
===============*/ 

unsigned char key_driver(key_Status_t * key_Status) 
{ 
    //static unsigned char key_state = key_state_0, key_time = 0; 
    unsigned char key_press, key_return = N_key; 

	  if(key_Status->index_input == 1)
		{
			key_press = key_input;                    // 读按键I/O电平  有按键动作 置零
		}
		else if(key_Status->index_input == 2)
		{
			key_press = Key2_input;                   // 
		}
		else if(key_Status->index_input == 3)
		{
			key_press = NEEDLE_input; 
		}
		else if(key_Status->index_input == 4)
		{
			key_press = Opening_input; 
		}
		
    switch (key_Status->key_state) 
    { 
      case key_state_0:                       // 按键初始态 
        if (!key_press) 
				{
					key_Status->key_state = key_state_1;            // 键被按下，状态转换到按键消抖和确认状态 
				}
        break; 
       
      case key_state_1:                       // 按键消抖与确认态 
        if (!key_press) 
        { 
             key_Status->key_time = 0;                    // 时间置零 
             key_Status->key_state = key_state_2;         // 按键仍然处于按下，消抖完成，状态转换到按下键时间的计时状态，但返回的还是无键事件 
        } 
        else 
				{
             key_Status->key_state = key_state_0;         // 按键已抬起，转换到按键初始态。此处完成和实现软件消抖，其实按键的按下和释放都在此消抖的。 
				}
        break; 
       
      case key_state_2: 
        if(key_press) 
        { 
             key_return = S_key;              // 此时按键释放，说明是产生一次短操作，回送S_key 应该是第三次进入这个函数 间隔两个25毫秒,共50秒
             key_Status->key_state = key_state_0;         // 转换到按键初始态 
        } 
        else if (++(key_Status->key_time) >= 100)           // 继续按下，计时加25ms（25ms为本函数循环执行间隔） 
        { 
             key_return = L_key;              // 按下时间>2500ms，此按键为长按操作，返回长键事件 
             key_Status->key_state = key_state_3;         // 转换到等待按键释放状态 
        } 
        break; 

      case key_state_3:                       // 等待按键释放状态，此状态只返回无按键事件 
        if (key_press) 
				{
						key_Status->key_state = key_state_0; 					//按键已释放，转换到按键初始态 
				}																			
        break; 
    } 
    return key_return; 
} 

/*============= 
中间层按键处理函数，调用低层函数一次，处理双击事件的判断，返回上层正确的无键、单键、双键、长键4个按键事件。 
本函数由上层循环调用，间隔10ms 
===============*/ 

unsigned char key_read(key_Status_t * key_Status) 
{ 
    //static unsigned char key_m = key_state_0, key_time_1 = 0; 
    unsigned char key_return = N_key,key_temp; 
     
    key_temp = key_driver(key_Status); 
     
    switch(key_Status->key_m) 
    { 
        case key_state_0: 
            if (key_temp == S_key ) 
            { 
                 key_Status->key_time_1 = 0;               // 第1次单击，不返回，到下个状态判断后面是否出现双击 
                 key_Status->key_m = key_state_1; 
            } 
            else 
						{
                 key_return = key_temp;        // 对于无键、长键，返回原事件 
						}
            break; 

        case key_state_1: 
            if (key_temp == S_key)             // 又一次单击（间隔肯定<500ms） 
            { 
                 key_return = D_key;           // 返回双击键事件，回初始状态 
                 key_Status->key_m = key_state_0; 
            } 
            else                                
            {                                  // 这里500ms内肯定读到的都是无键事件，因为长键>1000ms，在1s前低层返回的都是无键 
                 if(++(key_Status->key_time_1) >= 10) 			 //提高单键 响应速度 改为200ms
                 { 
                      key_return = S_key;      // 500ms内没有再次出现单键事件，返回上一次的单键事件 
                      key_Status->key_m = key_state_0;     // 返回初始状态 
                 } 
             } 
             break; 
    }
    return key_return; 
}     

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
