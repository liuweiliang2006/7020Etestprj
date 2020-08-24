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
�Ͳ㰴����I/0��ɨ�躯�������Ͳ㰴���豸������ֻ�����޼����̰��ͳ���������˫�����ڴ˴��жϡ��ο����˽̲ĵ���9-1����΢�б仯���̲���Ϊ��_���� 
===============*/ 

unsigned char key_driver(key_Status_t * key_Status) 
{ 
    //static unsigned char key_state = key_state_0, key_time = 0; 
    unsigned char key_press, key_return = N_key; 

	  if(key_Status->index_input == 1)
		{
			key_press = key_input;                    // ������I/O��ƽ  �а������� ����
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
      case key_state_0:                       // ������ʼ̬ 
        if (!key_press) 
				{
					key_Status->key_state = key_state_1;            // �������£�״̬ת��������������ȷ��״̬ 
				}
        break; 
       
      case key_state_1:                       // ����������ȷ��̬ 
        if (!key_press) 
        { 
             key_Status->key_time = 0;                    // ʱ������ 
             key_Status->key_state = key_state_2;         // ������Ȼ���ڰ��£�������ɣ�״̬ת�������¼�ʱ��ļ�ʱ״̬�������صĻ����޼��¼� 
        } 
        else 
				{
             key_Status->key_state = key_state_0;         // ������̧��ת����������ʼ̬���˴���ɺ�ʵ�������������ʵ�����İ��º��ͷŶ��ڴ������ġ� 
				}
        break; 
       
      case key_state_2: 
        if(key_press) 
        { 
             key_return = S_key;              // ��ʱ�����ͷţ�˵���ǲ���һ�ζ̲���������S_key Ӧ���ǵ����ν���������� �������25����,��50��
             key_Status->key_state = key_state_0;         // ת����������ʼ̬ 
        } 
        else if (++(key_Status->key_time) >= 100)           // �������£���ʱ��25ms��25msΪ������ѭ��ִ�м���� 
        { 
             key_return = L_key;              // ����ʱ��>2500ms���˰���Ϊ�������������س����¼� 
             key_Status->key_state = key_state_3;         // ת�����ȴ������ͷ�״̬ 
        } 
        break; 

      case key_state_3:                       // �ȴ������ͷ�״̬����״ֻ̬�����ް����¼� 
        if (key_press) 
				{
						key_Status->key_state = key_state_0; 					//�������ͷţ�ת����������ʼ̬ 
				}																			
        break; 
    } 
    return key_return; 
} 

/*============= 
�м�㰴�������������õͲ㺯��һ�Σ�����˫���¼����жϣ������ϲ���ȷ���޼���������˫��������4�������¼��� 
���������ϲ�ѭ�����ã����10ms 
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
                 key_Status->key_time_1 = 0;               // ��1�ε����������أ����¸�״̬�жϺ����Ƿ����˫�� 
                 key_Status->key_m = key_state_1; 
            } 
            else 
						{
                 key_return = key_temp;        // �����޼�������������ԭ�¼� 
						}
            break; 

        case key_state_1: 
            if (key_temp == S_key)             // ��һ�ε���������϶�<500ms�� 
            { 
                 key_return = D_key;           // ����˫�����¼����س�ʼ״̬ 
                 key_Status->key_m = key_state_0; 
            } 
            else                                
            {                                  // ����500ms�ڿ϶������Ķ����޼��¼�����Ϊ����>1000ms����1sǰ�Ͳ㷵�صĶ����޼� 
                 if(++(key_Status->key_time_1) >= 10) 			 //��ߵ��� ��Ӧ�ٶ� ��Ϊ200ms
                 { 
                      key_return = S_key;      // 500ms��û���ٴγ��ֵ����¼���������һ�εĵ����¼� 
                      key_Status->key_m = key_state_0;     // ���س�ʼ״̬ 
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
