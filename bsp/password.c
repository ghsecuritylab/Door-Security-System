#include "public.h"

/* ��������psw_check
 * ����  ������ʱ��������Ƿ��޸ģ���־:0xaa
 * ����  ����
 * ����  : ��
 * ����  ����ʼ��ʱ����              
*/
void psw_check(void)
{
    uint8_t tem = 0;
	tem = AT24CXX_ReadOneByte(PSW_FLAG);
	//���֮ǰû�������룬������������û���
	if(tem!=0xaa)
	{
		//����
		LCD_Clear(WHITE);
		//��ʾ��������GUI
		Psd_setting_GUI(SET_PSW);
		//��������
		if(password_setting())
		{
			//��������������ñ�־λ
			AT24CXX_WriteOneByte(PSW_FLAG,0xaa);
			POINT_COLOR = RED;
			LCD_ShowString(0,46,200,24,24," Password Set OK! ");
			POINT_COLOR = BLACK;
			rt_thread_mdelay(3500);
			LCD_Clear(WHITE);
		}
		else
		{
			POINT_COLOR = RED;
			LCD_ShowString(0,46,200,24,24,"Password Setting cancelled!");
			POINT_COLOR = BLACK;
			rt_thread_mdelay(3500);
			LCD_Clear(WHITE);
		}
	}
}


/* ��������password_setting
 * ����  ����������
 * ����  ����
 * ����  : ��
 * ����  ��psw_check()��������              
*/
uint8_t password_setting(void)
{
	char psw[6];
	uint8_t i;
	//��ȡ�û����������
    if(Get_User_Input_psw(psw)==1)
	{
		//printf("psw:%s\n",psw);
		//������д�뵽ָ��EEPROM��ַ
		for(i=0;i<6;i++)
		{
		 AT24CXX_WriteOneByte(PSW_ADD_BASE+i,psw[i]);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}


/* ��������Get_Touch_KeyValue
 * ����  ����������ֵ��ȡ
 * ����  ����
 * ����  : ��
 * ����  ��password_setting()��������              
*/
uint8_t Get_Touch_KeyValue(void)
{
	    //������ɨ��
		tp_dev.scan(0); 	
        //�жϴ�����������		  
		if(tp_dev.sta&TP_PRES_DOWN)			
		{	
			//��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
		 	if((tp_dev.x[0]<lcddev.width)&&(tp_dev.y[0]<lcddev.height)) 
			{	
				//������������  
				while(tp_dev.sta&TP_PRES_DOWN)	
				{
					tp_dev.scan(0); 
				}
				//������ʾ��
				beeponce(10);
				//�ж�One�Ƿ񱻰���
				if(One)
				{
					return 1;
				}	
				//�ж�Two�Ƿ񱻰���
				if(Two)
				{
					return 2;
				}
				//�ж�Three�Ƿ񱻰���
				if(Three)
				{
					return 3;
				}
				//�ж�Four�Ƿ񱻰���
				if(Four)
				{
					return 4;
				}
			    //�ж�Five�Ƿ񱻰���
				if(Five)
				{
					return 5;
				}
			    //�ж�Six�Ƿ񱻰���
				if(Six)
				{
					return 6;
				}
			    //�ж�Seven�Ƿ񱻰���
				if(Seven)
				{
					return 7;
				}
			    //�ж�Eight�Ƿ񱻰���
				if(Eight)
				{
					return 8;
				}
			    //�ж�Nine�Ƿ񱻰���
				if(Nine)
				{
					return 9;
				}
			    //�ж�D/B�Ƿ񱻰���
				if(DB)
				{
					 
					return 67;
				}
			    //�ж�Zero�Ƿ񱻰���
				if(Zero)
				{
					return 0;
				}
			    //�ж�OK�Ƿ񱻰���
				if(OK)
				{
					 
					return 69;
				}
			}
		}
	return 'a';	
}



/* ��������Get_User_Input_psw
 * ����  ����ȡ�û����������
 * ����  ����
*  ����  : uint8_t psw[6]
		   1:�ɹ���ȡ�û�����
		   0:��������ȡ������
 * ����  ����Ƭɾ����ע�ắ������              
*/
uint8_t Get_User_Input_psw(char psw[6])
{
	uint8_t tem,i = 0;
	while(1)
	{
		loop:
		//��ȡ������������
		tem = Get_Touch_KeyValue();
		if(tem!='a')
		{
			//һ. ������������ֵ������ת��Ϊ�ַ���Ȼ����ʾ��LCD
			if((tem>=0)&&(tem<=9))
			{
				psw[i++] = tem+'0';
				//����λ������6λ��������
				if(i>6)      
				{  
				    i = 0;
					//���֮ǰ������
				    for(uint8_t k=0;k<=6;k++)
					     { psw[k] = ' ';}
				}			
				//��ʾ��LCD
                LCD_ShowString(0,70,200,24,24,psw);	
			}
			//��. ������Ƿ��ػ�ɾ������ʾB/D
			if(tem==67)
			{
				//û�������κ���ֵʱ����B/D���˳������趨����
				if(i==0)
				{		
                    LCD_ShowString(0,70,200,24,24,"                   ");							
					return 0;
				}
				else
				{
					//û�����������°�B/D����
//					if(i==0)
//					{
//						LCD_ShowString(0,70,200,24,24,"                  ");	
//					    return 0;
//					}
					//�����һ���������ɾ��
					psw[(--i)] =' ';
					//��ʾ��LCD
					LCD_ShowString(0,70,200,24,24,psw);		
				}
			}
			//��.�������ȷ�ϼ����򷵻ؽ��
			if(tem==69)
			{
				//û�����������£���E��������
				if(i==0)
				{goto loop;}
				//��ȡ�û�����󷵻�
				//printf("psw:%s\n",psw);
				LCD_ShowString(0,70,200,24,24,"          ");	
				return 1;
			}
		}
	}
}

/* ��������Match_psw
 * ����  �����û���������EEPROM�б�����������ƥ��
 * ����  ����
*  ����  : 1:ƥ��ɹ�
		   2:ƥ��ʧ�ܷ���2
		   0:�û�ȡ������������֤
 * ����  ����Ƭɾ����ע�ắ������              
*/
uint8_t Match_psw(void)
{
	uint8_t i = 0;
	char eeprpm_psw[7];
	char user_psw[7];
	//����
	LCD_Clear(WHITE);
	//��ʾ��������GUI
	Psd_setting_GUI(MATCH_PSW);
	//��EEPROM�б�������������
	for(i=0;i<6;i++)
	eeprpm_psw[i]=AT24CXX_ReadOneByte(PSW_ADD_BASE+i);
	eeprpm_psw[6]= '\0';
	//��ȡ�û����������
	if(Get_User_Input_psw(user_psw)==1)
	{
		//��ӽ�����
		user_psw[6]= '\0';
		//��ʼƥ��
		if(rt_strcmp(eeprpm_psw,user_psw)==0)
			return 1; //ƥ��ɹ�����1
		else
		{
			
			return 2; //ƥ��ʧ�ܷ���2
		}
	}
	else
	{
		 return 0;   //�û�ȡ������������֤
	}
		
}


/* ��������Psd_setting_GUI
 * ����  ����ʾ�����趨����
 * ����  ����
 * ����  : ��
 * ����  ��psd_check()��������              
*/
void Psd_setting_GUI(uint8_t mode)
{
  POINT_COLOR = RED;
  if(mode==SET_PSW)
  {
    LCD_ShowString(20,0,200,24,24,"Setting  Password");//��ʾ����	
	LCD_ShowString(0,46,200,24,24,"New Password:");
  }
  else
  {
    LCD_ShowString(20,0,200,24,24,"Verify   Password");
	LCD_ShowString(0,46,200,24,24,"Password:");  
  }
  POINT_COLOR = BLACK;
  LCD_DrawLine(0, 24, 240, 24);//��һ������
  LCD_ShowString(0,70,200,24,24,"            ");
  LCD_DrawLine(1, 160, 240, 160);//����yi������
  LCD_DrawLine(0, 200, 240, 200);//����er������
  LCD_DrawLine(0, 240, 240, 240);//����san������
  LCD_DrawLine(0, 280, 240, 280);//����si������
  LCD_DrawLine(80, 160, 80, 320);//����1������	
  LCD_DrawLine(160, 160, 160, 320);	//����2������
  LCD_ShowString(38,178-8,200,24,24,"1");
  LCD_ShowString(116,178-8,200,24,24,"2");	
  LCD_ShowString(199,178-8,200,24,24,"3");
  LCD_ShowString(38,221-8,200,24,24,"4");
  LCD_ShowString(116,221-8,200,24,24,"5");
  LCD_ShowString(199,221-8,200,24,24,"6");	
  LCD_ShowString(38,257-8,200,24,24,"7");
  LCD_ShowString(116,257-8,200,24,24,"8");
  LCD_ShowString(199,257-8,200,24,24,"9");
  LCD_ShowString(25,297-8,200,24,24,"B/D");	
  LCD_ShowString(113,297-8,200,24,24,"0");
  LCD_ShowString(199,297-8,200,24,24,"E");	
}

/* ��������Get_User_Psd
 * ����  ��ͨ��PC���ڷ���msh����鿴�û�����
 * ����  ����
 * ����  : ��
 * ����  ��MSH����              
*/
uint8_t Get_User_Psd(void)
{
	uint8_t i = 0;
	char str[7];
	//��EEPROM�б�������������
	for(i=0;i<6;i++)
	str[i]=AT24CXX_ReadOneByte(PSW_ADD_BASE+i);
	str[6]= '\0';
	printf("User Password is:%s\n",str);
	return 1;
}
MSH_CMD_EXPORT(Get_User_Psd,Get_User_Psd);
