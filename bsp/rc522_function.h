#ifndef __RC522_FUNCTION_H
#define	__RC522_FUNCTION_H
#include "stm32f1xx_hal.h"

/**************��Ƭ�����궨��************/
#define Mod_Cnt_Tag_Add            0x00              //�õ�ַ����λ��ʾ�Ƿ��޸�,10��ʾ�޸Ĺ���������ֵ��ʾû���޸Ĺ�
										             //��6λ��ֵΪ�Ǽǵ�tag��Ŀ
#define Base_Add                   0x01    	         //��Ƭ�洢����ַ							
#define Tag_Stored_Base_Add(cnt) ((cnt*4)-4+Base_Add) //ÿ���µǼǵ�tag ID��ʼ�����ַ��
                                                     //���ݵǼǸ������㣬��ʼ��ַ0x01
#define If_Modified                0xc0				 //�жϵ�ַ0x00����λ�Ƿ��޸�	
#define Get_Tag_Cnt                0x3f			     //��ȡ��ַ0x00��6λ�е���Ŀ
#define Modified                   0x80				 //����ַ0x00����λ���޸ı�־λ��λ
#define macDummy_Data              0x00

/**************��������****************/
void PcdReset( void );                                   //��λ
void M500PcdConfigISOType( uint8_t type );               //������ʽ
char PcdRequest( uint8_t req_code, uint8_t * pTagType ); //Ѱ��
char PcdAnticoll( uint8_t * pSnr);                       //������
void SPI_RC522_SendByte ( uint8_t byte );
uint8_t ReadRawRC( uint8_t ucAddress );
void WriteRawRC( uint8_t ucAddress, uint8_t ucValue );
void SPI_RC522_SendByte ( uint8_t byte );
uint8_t SPI_RC522_ReadByte ( void );
uint8_t Read_ID (void);
uint8_t tag_match(uint8_t* tag_id);
uint8_t regist_tag(uint8_t* tag_id);
void Register_New_Tag(void);
void Delete_registered_Tag(void);
#endif /* __RC522_FUNCTION_H */
