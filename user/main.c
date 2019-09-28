#include "public.h"

/***********************************�û�����****************************************/

extern uint8_t ucArray_ID[4];            //���ڴ��RFIDģ������Ŀ�ƬID
static uint8_t register_lock = 1;		 //ע�������
static uint8_t delete_lock = 1;			 //ɾ��������
static uint8_t psd_lock = 1;             //�Զ������������
static uint8_t reg_err_cnt = 0;          //ע���������������������
static uint8_t del_err_cnt = 0;          //ɾ����������������������
static uint8_t psd_err_cnt = 0;          //�Զ��������������������������
char register_remain_time[40];           //ע����������ʱ��ʾ����
char delete_remain_time[40];             //ɾ����������ʱ��ʾ����
char psd_remain_time[40];                //�����Զ�����������ʱ��ʾ����
uint8_t flag = 0;                        //��ʶҪ��ʾ��ģ�鵹��ʱ

/*****************************�̶߳�ջ�����ȼ���ʱ��Ƭ�궨��************************/

#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 21
#define THREAD_TIMESLICE 500

/***********************************�����߳̿��ƿ�**********************************/

static struct rt_thread Touch_Thread;    //�����߳̿��ƿ�
static struct rt_thread Match_Thread;    //��Ƭƥ���߳̿��ƿ�
static struct rt_thread Door_Open_Thread;//�����߳̿��ƿ�
static struct rt_thread Err_Psw_Thread;  //������������ʱ�߳̿��ƿ�
static struct rt_thread Err_Reg_Thread;  //ע����������ʱ�߳̿��ƿ�
static struct rt_thread Err_Del_Thread;  //ɾ����������ʱ�߳̿��ƿ�

/***********************************�����ź���***************************************/

rt_sem_t match_thread_lock= RT_NULL;	 //��Ƭƥ���߳���
rt_sem_t door_open_thread_lock= RT_NULL; //�����߳���
rt_sem_t err_psd_thread_lock= RT_NULL;   //�����Զ���ģ��������֤���󵹼�ʱ�߳���
rt_sem_t err_reg_thread_lock= RT_NULL;   //ע��ģ��������֤���󵹼�ʱ�߳���
rt_sem_t err_del_thread_lock= RT_NULL;   //ɾ��ģ��������֤���󵹼�ʱ�߳���


/****************************������̬��ʱ�����ƿ����******************************/
rt_timer_t register_countdown_timer1;    //ע�������������ʱ
rt_timer_t delete_countdown_timer2;      //ɾ��������������ʱ
rt_timer_t password_countdown_timer3;    //�Զ������������������ʱ

//ע�����������ʱ����ʱ����--ע�����
void register_timeout1(void *parameter)
{
	register_lock = 1;
}

//ɾ������������ʱ����ʱ����--ɾ������
void delete_timeout2(void *parameter)
{
	delete_lock = 1;
}

//�Զ����������������ʱ����ʱ����--�����趨����
void psd_timeout3(void *parameter)
{
	psd_lock = 1;
}

//����flag��ʾ����ʱ
void down_count(uint8_t flag)
{
	 uint8_t cnt = 0;
	 //����
	 LCD_Clear(WHITE);
	 POINT_COLOR = RED;
	 //��ʾ��ʱ5����˳�
	 while(cnt<5)
	 {   
		  //��ʱ1��
		  rt_thread_mdelay(1000);
		  cnt++;
		  switch(flag)
		  {
			  case 1: LCD_ShowString(20,120,200,24,24,register_remain_time);break;
			  case 2: LCD_ShowString(20,120,200,24,24,delete_remain_time); break;
			  case 3: LCD_ShowString(20,120,200,24,24,psd_remain_time); break;
			  default:break;
		  }
	 }
	 POINT_COLOR = BLACK;
	 //����
	 LCD_Clear(WHITE);
}

//�Զ���������������ʱֵ�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char err_psd_thread_stack[512];
/******************�Զ���������������ʱֵ�����߳�****************/
static void err_psd_thread(void *parameter)
{
	static rt_tick_t psd_tick = 300000;
	uint8_t min,sec;
	while(1)
	{
		 if(rt_sem_take(err_psd_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
		 {
			while(psd_tick!=0)
			{
				 //�������ʾ����ʱ�Ĺ�����������λ���˳�����ʱѭ��
				if(psd_lock==1)
					{
						 psd_tick = 300000;
						 break;
					}
				rt_thread_mdelay(1);
				//��1��
				psd_tick -=1; 
				//printf("psd_tick:%lu\n",psd_tick);
				//������ת��Ϊ�ֺ���
				min = psd_tick/1000/60;
				sec = psd_tick/1000%60;
				//�����ת��Ϊ�ַ�������յ���ʾ����������ʾ����������ʾ
				sprintf(psd_remain_time,"Please try %dmin  %dsec later!   ",min,sec);
			}
		 }
		 rt_thread_mdelay(10);
	 }
}
						  

//ע����������ʱֵ�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char err_reg_thread_stack[512];
/******************ע����������ʱֵ�����߳�****************/
static void err_reg_thread(void *parameter)
{
	static rt_tick_t register_tick = 300000;
	uint8_t min,sec;
	while(1)
	{
		 if(rt_sem_take(err_reg_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
		 {
			while(register_tick!=0)
			{
				 //�������ʾ����ʱ�Ĺ�����������λ���˳�����ʱѭ��
				if(register_lock==1)
					{
						 register_tick = 300000;
						 break;
					}
				rt_thread_mdelay(1);
				//��1��
				register_tick -=1; 
				//printf("register_tick:%lu\n",register_tick);
				//������ת��Ϊ�ֺ���
				min = register_tick/1000/60;
				sec = register_tick/1000%60;
				//�����ת��Ϊ�ַ�����ʾ��LCD
				sprintf(register_remain_time,"Please try %dmin  %dsec later!   ",min,sec);
		    }
		 }
		 rt_thread_mdelay(10);
	 }
}


//ɾ����������ʱֵ�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char err_del_thread_stack[512];
/******************ɾ����������ʱֵ�����߳�****************/
static void err_del_thread(void *parameter)
{
	static rt_tick_t delete_tick = 300000;
	uint8_t min,sec;
	while(1)
	{
		 if(rt_sem_take(err_del_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
		 {
			while(delete_tick!=0)
			{
				 //�������ʾ����ʱ�Ĺ�����������λ���˳�����ʱѭ��
				if(delete_lock==1)
					{
						 delete_tick = 300000;
						 break;
					}
				rt_thread_mdelay(1);
				//��1��
				delete_tick -=1; 
				//printf("delete_tick:%lu\n",delete_tick);
				//������ת��Ϊ�ֺ���
				min = delete_tick/1000/60;
				sec = delete_tick/1000%60;
				//�����ת��Ϊ�ַ�����ʾ��LCD
				sprintf(delete_remain_time,"Please try %dmin  %dsec later!   ",min,sec);
		    }
		 }
		 rt_thread_mdelay(10);
	 }
}

//�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char door_open_thread_stack[512];
/******************�����߳�****************/
static void door_open_thread(void *parameter)
{
	while(1)
	{
		if(rt_sem_take(door_open_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
		{
			//��ʼ
			HAL_TIM_PWM_Start(&TIM3_Handler,TIM_CHANNEL_2);
			//����
			TIM_SetTIM3Compare2(4000);
			rt_thread_mdelay(3500);
			//��ͣ
			HAL_TIM_PWM_Stop(&TIM3_Handler,TIM_CHANNEL_2);
			rt_thread_mdelay(2500);
			//����
			HAL_TIM_PWM_Start(&TIM3_Handler,TIM_CHANNEL_2);
			TIM_SetTIM3Compare2(3000);
			rt_thread_mdelay(4500);
			//ֹͣ
			HAL_TIM_PWM_Stop(&TIM3_Handler,TIM_CHANNEL_2);
		}
		rt_thread_mdelay(10);
	}
}

//tagƥ���̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char match_thread_stack[1024];
/******************tagƥ���߳�****************/
static void match_thread(void *parameter)
{
	while(1)
	{
		if(rt_sem_take(match_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
		{
			if(Read_ID()==MI_OK)
			{
				if(tag_match(ucArray_ID)==1)
				{
					//�ͷ��ź�������
					rt_sem_release(door_open_thread_lock);
					POINT_COLOR = RED;
					beeponce(35);
					LCD_ShowString(20, 200,240,240,16 ,"                        ");
					LCD_ShowString(70,120,200,16,24,"Welcome!");
					LCD_ShowString(70,150,100,16,24,"~      ~");
					LCD_ShowString(70,170,100,16,24,"   __   ");
					rt_thread_mdelay(2500);
					POINT_COLOR = BLACK;
					LCD_ShowString(70,120,200,16,24,"         ");
					LCD_ShowString(70,150,100,16,24,"         ");
					LCD_ShowString(70,170,100,16,24,"         ");
					LCD_ShowString( 0, 80,240,240,16 ,"The Card ID is:               ");
				}
				else
					beeponce(150);
			}
			rt_sem_release(match_thread_lock);
			rt_thread_mdelay(5);
		}
	}
}
	
//�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char touch_thread_stack[1024];
/******************�����߳�****************/
static void touch_thread(void *parameter)
{
	
	while(1)
   {
		//������ɨ��
		tp_dev.scan(0); 	
        //�жϴ�����������		  
		if(tp_dev.sta&TP_PRES_DOWN)			
		{	
			//��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) 
			{	
				
				// һ. �ж�Register�Ƿ񱻰���
				if(Register)
				{
					//������ʾ��
					beeponce(10);
					//����match_thread�߳�
					if(rt_sem_take(match_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
					{
						//��ֹRTC�ж�
						HAL_NVIC_DisableIRQ(RTC_IRQn);
						//������־λΪ1ʱ�����Խ���ע�����
						if(register_lock)
						{
								//ע���¿�Ƭǰ����֤����
								switch(Match_psw())
								{
									case 1:
										  {
											  //����������ȷʱ���������������
											  reg_err_cnt = 0;
											  //ע���¿�Ƭ
											  Register_New_Tag();
											  break;
										  }
									case 2:
										  {
											  //��¼�����������
											   reg_err_cnt++;
											   //�������Σ�����ע�Ṧ�ܽ���5��������״̬
											   if(reg_err_cnt>2)
											   {
												    //�������������
													reg_err_cnt = 0;
												    //������־λ����
													register_lock = 0;
												    //�ͷ��ź�������������ʱ����ֵ��ȡ�߳�
													rt_sem_release(err_reg_thread_lock);
												    //������ʱ��
												    rt_timer_start(register_countdown_timer1);
											   } 
											   POINT_COLOR = RED;
											   LCD_ShowString(0,46,200,24,24,"  Wrong Password!"); 
											   POINT_COLOR = BLACK;	
											   rt_thread_mdelay(2000);
											   break;
										  }
									case 0:
										  {
											   break;
										  }
									default: break;
								 }
						}
						//������־λΪ0ʱ����ʾ�ȴ�����ʱ��
						else
						{
							down_count(1);
						}
						HAL_NVIC_EnableIRQ(RTC_IRQn);	
						//�ָ�������
						security_door_window(); 
						//�ͷ��ź���������match_thread�߳�
						rt_sem_release(match_thread_lock);
				  }
				   goto loop;
			  }					
				//��. �ж�Delete�Ƿ񱻰���
				if(Delete)
				{
					//������ʾ��
					beeponce(10);
					//����match_thread�߳�
					if(rt_sem_take(match_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
					{
						//��ֹRTC�ж�
						HAL_NVIC_DisableIRQ(RTC_IRQn);
						//������־λΪ1ʱ�����Խ���ע�����
						if(delete_lock)
						{
								//ע���¿�Ƭǰ����֤����
								switch(Match_psw())
								{
									case 1:
										  {
									         //�������������
											  del_err_cnt = 0;
											  Delete_registered_Tag();
											  break;
										  }
									case 2:
										  {
											   //��¼�����������
											   del_err_cnt++;
											   //�������Σ�����ע�Ṧ�ܽ���5��������״̬
											   if(del_err_cnt>2)
											   {
												    //�������������
													del_err_cnt = 0;
												    //������־λ����
													delete_lock = 0;
												     //�ͷ��ź�������������ʱ����ֵ��ȡ�߳�
													rt_sem_release(err_del_thread_lock);
												    rt_timer_start(delete_countdown_timer2);
											   } 
											   POINT_COLOR = RED;
											   LCD_ShowString(0,46,200,24,24,"  Wrong Password!"); 
											   POINT_COLOR = BLACK;	
											   rt_thread_mdelay(2000);
											   break;
										  }
									case 0:
										  {
											   break;
										  }
									default: break;
								 }
							 }
						//������־λΪ0ʱ����ʾ�ȴ�����ʱ��
						else
						{
							 down_count(2);
						}
						HAL_NVIC_EnableIRQ(RTC_IRQn);
						//�ָ�������
						security_door_window(); 
						//�ͷ��ź���������match_thread�߳�
						rt_sem_release(match_thread_lock);
				  }
				   goto loop;
				}
				//��. ����������ü��Ƿ񱻰���
				if(Set_Psw)
				{
					//������ʾ��
					beeponce(10);
					//����match_thread�߳�
					if(rt_sem_take(match_thread_lock,RT_WAITING_FOREVER)==RT_EOK)
					{
						//��ֹRTC�ж�
						HAL_NVIC_DisableIRQ(RTC_IRQn);
						//������־λΪ1ʱ�����Խ���ע�����
						if(psd_lock)
						{
								//ע���¿�Ƭǰ����֤����
								switch(Match_psw())
								{
									case 1:
											{
											    //�������������
												psd_err_cnt = 0;
												//��֤�ɹ�������������
												if(password_setting()==1)
												{
													POINT_COLOR = RED;
													LCD_ShowString(0,46,200,24,24,"Password Set OK!");
													LCD_ShowString(0,70,200,24,24,"                ");
													POINT_COLOR = BLACK;
													rt_thread_mdelay(2000);
												}
												//���أ��������κβ���
												else
												{
													;
												}
												break;
											}
									case 2:
											{
												//��¼�����������
											   psd_err_cnt++;
											   //�������Σ�����ע�Ṧ�ܽ���5��������״̬
											   if(psd_err_cnt>2)
											   {
												    //�������������
													psd_err_cnt = 0;
												    //������־λ����
													psd_lock = 0;
												     //�ͷ��ź�������������ʱ����ֵ��ȡ�߳�
													rt_sem_release(err_psd_thread_lock);
												    //������ʱ��
												    rt_timer_start(password_countdown_timer3);
											   } 
												//������֤ʧ�ܣ���ʾ������Ϣ��Ȼ���˳�
												POINT_COLOR = RED;
												LCD_ShowString(0,46,200,24,24," Wrong  Password!");
												LCD_ShowString(0,70,200,24,24,"                ");
												POINT_COLOR = BLACK;
												rt_thread_mdelay(2000);
												break;
											}
									case 0:
											{
												//�û�ȡ������������֤��ֱ���˳�
												break;
											}
									default:break;
								}
						}
						//������־λΪ0ʱ����ʾ�ȴ�����ʱ��
						else
						{
							down_count(3);
						}
						HAL_NVIC_EnableIRQ(RTC_IRQn);	
						//�ָ�������
						security_door_window();
						//�ͷ��ź���������match_thread�߳�
						rt_sem_release(match_thread_lock);
					}
				}
				loop:
			    //������������  
				while(tp_dev.sta&TP_PRES_DOWN)	
				{
					tp_dev.scan(0); 
				}
			}
		}
		rt_thread_mdelay(5);
	}
}

int Door_Security_Demo(void)
{
	rt_err_t result = 0;
	//������ʱ��1�߳�--register����
	register_countdown_timer1 = rt_timer_create("register_countdown_timer1",register_timeout1,RT_NULL,300000,RT_TIMER_CTRL_SET_ONESHOT|RT_TIMER_FLAG_SOFT_TIMER);
	//������ʱ��1�߳�
	if(register_countdown_timer1 != RT_NULL)
		rt_kprintf("register_countdown_timer1 created!\n");
		
	
	//������ʱ��2�߳�--delete����
	delete_countdown_timer2 = rt_timer_create("delete_countdown_timer2",delete_timeout2,RT_NULL,300000,RT_TIMER_CTRL_SET_ONESHOT|RT_TIMER_FLAG_SOFT_TIMER);
	//������ʱ��2�߳�
	if(delete_countdown_timer2 != RT_NULL)
		rt_kprintf("delete_countdown_timer2 created!\n");
		//
	
	//������ʱ��3�߳�--psd����
	password_countdown_timer3 = rt_timer_create("password_countdown_timer3",psd_timeout3,RT_NULL,300000,RT_TIMER_CTRL_SET_ONESHOT|RT_TIMER_FLAG_SOFT_TIMER);
	//������ʱ��2�߳�
	if(password_countdown_timer3 != RT_NULL)
		rt_kprintf("password_countdown_timer3 created!\n");

	
	//������Ƭƥ���߳����ź���
	match_thread_lock = rt_sem_create("match_thread_lock",1,RT_IPC_FLAG_FIFO);
	if(match_thread_lock!=RT_NULL)
	{
		rt_kprintf("match_thread_lock created!\n");
	}
	//���������߳����ź���
	door_open_thread_lock = rt_sem_create("door_open_thread_lock",0,RT_IPC_FLAG_FIFO);
	if(door_open_thread_lock!=RT_NULL)
	{
		rt_kprintf("door_open_thread_lock created!\n");
	}
	
	//��������ģ�鵹��ʱֵ�����߳����ź���
	err_psd_thread_lock = rt_sem_create("err_psd_thread_lock",0,RT_IPC_FLAG_FIFO);
	if(err_psd_thread_lock!=RT_NULL)
	{
		rt_kprintf("err_psd_thread_lock created!\n");
	}
	
	//����ע��ģ�鵹��ʱֵ�����߳����ź���
	err_reg_thread_lock = rt_sem_create("err_reg_thread_lock",0,RT_IPC_FLAG_FIFO);
	if(err_reg_thread_lock!=RT_NULL)
	{
		rt_kprintf("err_reg_thread_lock created!\n");
	}
	
	//����ɾ��ģ�鵹��ʱֵ�����߳����ź���
	err_del_thread_lock = rt_sem_create("err_del_thread_lock",0,RT_IPC_FLAG_FIFO);
	if(err_reg_thread_lock!=RT_NULL)
	{
		rt_kprintf("err_del_thread_lock created!\n");
	}
	
	//tagƥ���߳�
    result = rt_thread_init( &Match_Thread,
							"match_thread",
							 match_thread,
							 RT_NULL,
							 &match_thread_stack[0],
							 sizeof(match_thread_stack),
							 THREAD_PRIORITY+1,
							 THREAD_TIMESLICE  	
							);
	//����tagƥ���߳�
	if(result==RT_EOK)
	{
		rt_thread_startup(&Match_Thread);
	}
	
	//�����߳�
    result = rt_thread_init( &Door_Open_Thread,
							"Door_Open_Thread",
							 door_open_thread,
							 RT_NULL,
							 &door_open_thread_stack[0],
							 sizeof(door_open_thread_stack),
							 THREAD_PRIORITY+2,
							 THREAD_TIMESLICE  	
							);
	//���������߳�
	if(result==RT_EOK)
	{
		rt_thread_startup(&Door_Open_Thread);
	}
	
	//���������߳�
    result = rt_thread_init( &Touch_Thread,
							"touch_thread",
							 touch_thread,
							 RT_NULL,
							 &touch_thread_stack[0],
							 sizeof(touch_thread_stack),
							 THREAD_PRIORITY-18,//�����ȼ�����Match_Thread�̴߳��������Գٶ�
							 THREAD_TIMESLICE  	
							);
	//���������߳� 
	if(result==RT_EOK)
	{
	  rt_thread_startup(&Touch_Thread);
	}
	
	//��������ģ�鵹��ʱֵ�����߳�
    result = rt_thread_init( &Err_Psw_Thread,
							"Err_Psw_Thread",
							 err_psd_thread,
							 RT_NULL,
							 &err_psd_thread_stack[0],
							 sizeof(err_psd_thread_stack),
							 THREAD_PRIORITY-15, 
							 THREAD_TIMESLICE  	
							);
	//��������ģ�鵹��ʱֵ�����߳�
	if(result==RT_EOK)
	{
	  rt_thread_startup(&Err_Psw_Thread);
	}
	
	//����ע��ģ�鵹��ʱֵ�����߳�
    result = rt_thread_init( &Err_Reg_Thread,
							"Err_Reg_Thread",
							 err_reg_thread,
							 RT_NULL,
							 &err_reg_thread_stack[0],
							 sizeof(err_reg_thread_stack),
							 THREAD_PRIORITY-16, 
							 THREAD_TIMESLICE  	
							);
	//����ע��ģ�鵹��ʱֵ�����߳�
	if(result==RT_EOK)
	{
	  rt_thread_startup(&Err_Reg_Thread);
	}
	
	//����ɾ��ģ�鵹��ʱֵ�����߳�
    result = rt_thread_init( &Err_Del_Thread,
							"Err_Del_Thread",
							 err_del_thread,
							 RT_NULL,
							 &err_del_thread_stack[0],
							 sizeof(err_del_thread_stack),
							 THREAD_PRIORITY-17, 
							 THREAD_TIMESLICE  	
							);
	//����ɾ��ģ�鵹��ʱֵ�����߳�
	if(result==RT_EOK)
	{
	  rt_thread_startup(&Err_Del_Thread);
	}
	return 0;
}



//������
int main ( void )
{  
	psw_check();
	Door_Security_Demo();
}





/****************************END OF FILE**********************/


