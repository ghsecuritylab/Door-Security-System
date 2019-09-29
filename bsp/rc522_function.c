#include "rc522_function.h"
#include "rc522_config.h"
#include "stm32f1xx_hal.h"
#include "public.h"

extern rt_sem_t match_thread_lock;
uint8_t ucArray_ID[4];  /*�Ⱥ���IC�������ͺ�UID(IC�����к�)*/   
#define   RC522_DELAY()  delay_2us ( 170 )

/**
  * @brief  ��RC522����1 Byte ����
  * @param  byte��Ҫ���͵�����
  * @retval RC522���ص�����
  */
void SPI_RC522_SendByte ( uint8_t byte )
{
  uint8_t counter;

  for(counter=0;counter<8;counter++)
  {     
    if ( byte & 0x80 )
      RC522_MOSI_1 ();
    else 
      RC522_MOSI_0 ();
    
    RC522_DELAY();
    RC522_SCK_0 ();
    
    RC522_DELAY();
    RC522_SCK_1();
    
    RC522_DELAY();
    byte <<= 1; 
  } 	
}


/**
  * @brief  ��RC522����1 Byte ����
  * @param  ��
  * @retval RC522���ص�����
  */
uint8_t SPI_RC522_ReadByte ( void )
{
  uint8_t counter;
  uint8_t SPI_Data;

  for(counter=0;counter<8;counter++)
  {
    SPI_Data <<= 1;
    RC522_SCK_0 ();
   
    RC522_DELAY();
    if ( RC522_MISO_GET() == 1)
     SPI_Data |= 0x01;
    
    RC522_DELAY();
    RC522_SCK_1 ();
    
    RC522_DELAY();
  }
  return SPI_Data;
	
}



/**
  * @brief  ��RC522�Ĵ���
  * @param  ucAddress���Ĵ�����ַ
  * @retval �Ĵ����ĵ�ǰֵ
  */
uint8_t ReadRawRC ( uint8_t ucAddress )
{
	uint8_t ucAddr, ucReturn;
	
	ucAddr = ( ( ucAddress << 1 ) & 0x7E ) | 0x80;	
	RC522_CS_Enable();
  
	SPI_RC522_SendByte ( ucAddr );
	ucReturn = SPI_RC522_ReadByte();
  
	RC522_CS_Disable();
	
	return ucReturn;	
}

/**
  * @brief  дRC522�Ĵ���
  * @param  ucAddress���Ĵ�����ַ
  * @param  ucValue��д��Ĵ�����ֵ
  * @retval ��
  */
void WriteRawRC ( uint8_t ucAddress, uint8_t ucValue )
{  
	uint8_t ucAddr;
	
	ucAddr = ( ucAddress << 1 ) & 0x7E;	
	RC522_CS_Enable();
	
	SPI_RC522_SendByte ( ucAddr );	
	SPI_RC522_SendByte ( ucValue );
  
	RC522_CS_Disable();		
}


/**
  * @brief  ��RC522�Ĵ�����λ
  * @param  ucReg���Ĵ�����ַ
  * @param   ucMask����λֵ
  * @retval ��
  */
void SetBitMask ( uint8_t ucReg, uint8_t ucMask )  
{
  uint8_t ucTemp;

  ucTemp = ReadRawRC ( ucReg );
  WriteRawRC ( ucReg, ucTemp | ucMask ); // set bit mask
}


/**
  * @brief  ��RC522�Ĵ�����λ
  * @param  ucReg���Ĵ�����ַ
  * @param  ucMask����λֵ
  * @retval ��
  */
void ClearBitMask ( uint8_t ucReg, uint8_t ucMask )  
{
  uint8_t ucTemp;

  ucTemp = ReadRawRC ( ucReg );
  WriteRawRC ( ucReg, ucTemp & ( ~ ucMask) ); // clear bit mask
}


/**
  * @brief  �������� 
  * @param  ��
  * @retval ��
  */
void PcdAntennaOn ( void )
{
  uint8_t uc;

  uc = ReadRawRC ( TxControlReg );
  if ( ! ( uc & 0x03 ) )
   SetBitMask(TxControlReg, 0x03);		
}


/**
  * @brief  �ر�����
  * @param  ��
  * @retval ��
  */
void PcdAntennaOff ( void )
{
  ClearBitMask ( TxControlReg, 0x03 );	
}


/**
  * @brief  ��λRC522 
  * @param  ��
  * @retval ��
  */
void PcdReset ( void )
{
	RC522_Reset_Disable();
	Delay1us ( 10 );
	RC522_Reset_Enable();
	Delay1us ( 10 );
	RC522_Reset_Disable();
	Delay1us (10 );
	WriteRawRC ( CommandReg, 0x0f );
	while ( ReadRawRC ( CommandReg ) & 0x10 );
	Delay1us ( 10 );
	//���巢�ͺͽ��ճ���ģʽ ��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC ( ModeReg, 0x3D );   
    
    WriteRawRC ( TReloadRegL, 30 );      //16λ��ʱ����λ    
	WriteRawRC ( TReloadRegH, 0 );		 //16λ��ʱ����λ
    WriteRawRC ( TModeReg, 0x8D );	     //�����ڲ���ʱ��������
    WriteRawRC ( TPrescalerReg, 0x3E );	 //���ö�ʱ����Ƶϵ��
	WriteRawRC ( TxAutoReg, 0x40 );	     //���Ʒ����ź�Ϊ100%ASK	
}



/**
  * @brief  ����RC522�Ĺ�����ʽ
  * @param  ucType��������ʽ
  * @retval ��
  */
void M500PcdConfigISOType ( uint8_t ucType )
{
  if ( ucType == 'A')                     //ISO14443_A
  {
		ClearBitMask ( Status2Reg, 0x08 );
		
        WriteRawRC ( ModeReg, 0x3D );         //3F
		
		WriteRawRC ( RxSelReg, 0x86 );        //84
		
		WriteRawRC( RFCfgReg, 0x7F );         //4F
		
		WriteRawRC( TReloadRegL, 30 );        
		
		WriteRawRC ( TReloadRegH, 0 );
		
		WriteRawRC ( TModeReg, 0x8D );
		
		WriteRawRC ( TPrescalerReg, 0x3E );
		
		Delay1us ( 5 );
		
		PcdAntennaOn ();//������
		
   }	 
}



/**
  * @brief  ͨ��RC522��ISO14443��ͨѶ
  * @param  ucCommand��RC522������
  * @param  pInData��ͨ��RC522���͵���Ƭ������
  * @param  ucInLenByte���������ݵ��ֽڳ���
  * @param  pOutData�����յ��Ŀ�Ƭ��������
  * @param  pOutLenBit���������ݵ�λ����
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdComMF522 ( uint8_t ucCommand,
                   uint8_t * pInData, 
                   uint8_t ucInLenByte, 
                   uint8_t * pOutData,
                   uint32_t * pOutLenBit )		
{
  char cStatus = MI_ERR;
  uint8_t ucIrqEn   = 0x00;
  uint8_t ucWaitFor = 0x00;
  uint8_t ucLastBits;
  uint8_t ucN;
  uint32_t ul;

  switch ( ucCommand )
  {
     case PCD_AUTHENT:		  //Mifare��֤
        ucIrqEn   = 0x12;		//��������ж�����ErrIEn  ��������ж�IdleIEn
        ucWaitFor = 0x10;		//��֤Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ
        break;
     
     case PCD_TRANSCEIVE:		//���շ��� ���ͽ���
        ucIrqEn   = 0x77;		//����TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        ucWaitFor = 0x30;		//Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ�� �����жϱ�־λ
        break;
     
     default:
       break;     
  }
  //IRqInv��λ�ܽ�IRQ��Status1Reg��IRqλ��ֵ�෴ 
  WriteRawRC ( ComIEnReg, ucIrqEn | 0x80 );
  //Set1��λ����ʱ��CommIRqReg������λ����
  ClearBitMask ( ComIrqReg, 0x80 );	 
  //д��������
  WriteRawRC ( CommandReg, PCD_IDLE );		 
  
  //��λFlushBuffer����ڲ�FIFO�Ķ���дָ���Լ�ErrReg��BufferOvfl��־λ�����
  SetBitMask ( FIFOLevelReg, 0x80 );			

  for ( ul = 0; ul < ucInLenByte; ul ++ )
    WriteRawRC ( FIFODataReg, pInData [ ul ] ); //д���ݽ�FIFOdata
    
  WriteRawRC ( CommandReg, ucCommand );					//д����


  if ( ucCommand == PCD_TRANSCEIVE )
    
    //StartSend��λ�������ݷ��� ��λ���շ�����ʹ��ʱ����Ч
    SetBitMask(BitFramingReg,0x80);  				  

  ul = 1000;                             //����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms

  do 														         //��֤ ��Ѱ���ȴ�ʱ��	
  {
       ucN = ReadRawRC ( ComIrqReg );		 //��ѯ�¼��ж�
       ul --;
  } while ( ( ul != 0 ) && ( ! ( ucN & 0x01 ) ) && ( ! ( ucN & ucWaitFor ) ) );	

  ClearBitMask ( BitFramingReg, 0x80 );	 //��������StartSendλ

  if ( ul != 0 )
  {
    //�������־�Ĵ���BufferOfI CollErr ParityErr ProtocolErr
    if ( ! ( ReadRawRC ( ErrorReg ) & 0x1B ) )	
    {
      cStatus = MI_OK;
      
      if ( ucN & ucIrqEn & 0x01 )				//�Ƿ�����ʱ���ж�
        cStatus = MI_NOTAGERR;   
        
      if ( ucCommand == PCD_TRANSCEIVE )
      {
        //��FIFO�б�����ֽ���
        ucN = ReadRawRC ( FIFOLevelReg );		          
        
        //�����յ����ֽڵ���Чλ��
        ucLastBits = ReadRawRC ( ControlReg ) & 0x07;	
        
        if ( ucLastBits )
          
          //N���ֽ�����ȥ1�����һ���ֽڣ�+���һλ��λ�� ��ȡ����������λ��
          * pOutLenBit = ( ucN - 1 ) * 8 + ucLastBits;   	
        else
          * pOutLenBit = ucN * 8;      //�����յ����ֽ������ֽ���Ч
        
        if ( ucN == 0 )		
          ucN = 1;    
        
        if ( ucN > MAXRLEN )
          ucN = MAXRLEN;   
        
        for ( ul = 0; ul < ucN; ul ++ )
          pOutData [ ul ] = ReadRawRC ( FIFODataReg );   
        
        }        
    }   
    else
      cStatus = MI_ERR;       
  }

  SetBitMask ( ControlReg, 0x80 );           // stop timer now
  WriteRawRC ( CommandReg, PCD_IDLE ); 
   
  return cStatus;
}

/**
  * @brief Ѱ��
  * @param  ucReq_code��Ѱ����ʽ = 0x52��Ѱ��Ӧ�������з���14443A��׼�Ŀ���
            Ѱ����ʽ= 0x26��Ѱδ��������״̬�Ŀ�
  * @param  pTagType����Ƭ���ʹ���
             = 0x4400��Mifare_UltraLight
             = 0x0400��Mifare_One(S50)
             = 0x0200��Mifare_One(S70)
             = 0x0800��Mifare_Pro(X))
             = 0x4403��Mifare_DESFire
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdRequest ( uint8_t ucReq_code, uint8_t * pTagType )
{
  char cStatus;  
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;

  //����ָʾMIFARECyptol��Ԫ��ͨ�Լ����п�������ͨ�ű����ܵ����
  ClearBitMask ( Status2Reg, 0x08 );
	//���͵����һ���ֽڵ� ��λ
  WriteRawRC ( BitFramingReg, 0x07 );
  //TX1,TX2�ܽŵ�����źŴ��ݾ����͵��Ƶ�13.56�������ز��ź�
  SetBitMask ( TxControlReg, 0x03 );	

  ucComMF522Buf [ 0 ] = ucReq_code;		//���� ��Ƭ������

  cStatus = PcdComMF522 ( PCD_TRANSCEIVE,	
                          ucComMF522Buf,
                          1, 
                          ucComMF522Buf,
                          &ulLen );	//Ѱ��  
  if ( ( cStatus == MI_OK ) && ( ulLen == 0x10 ) )	//Ѱ���ɹ����ؿ����� 
  {    
     * pTagType = ucComMF522Buf [ 0 ];
     * ( pTagType + 1 ) = ucComMF522Buf [ 1 ];
  }
  else
   cStatus = MI_ERR;
   return cStatus;	 
}

/**
  * @brief  ����ײ
  * @param  pSnr����Ƭ���кţ�4�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdAnticoll ( uint8_t * pSnr )
{
  char cStatus;
  uint8_t uc, ucSnr_check = 0;
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;
  
  //��MFCryptol Onλ ֻ�гɹ�ִ��MFAuthent����󣬸�λ������λ
  ClearBitMask ( Status2Reg, 0x08 );
  //����Ĵ��� ֹͣ�շ�
  WriteRawRC ( BitFramingReg, 0x00);	
	//��ValuesAfterColl���н��յ�λ�ڳ�ͻ�����
  ClearBitMask ( CollReg, 0x80 );			  
 
  ucComMF522Buf [ 0 ] = 0x93;	          //��Ƭ����ͻ����
  ucComMF522Buf [ 1 ] = 0x20;
 
  cStatus = PcdComMF522 ( PCD_TRANSCEIVE, 
                          ucComMF522Buf,
                          2, 
                          ucComMF522Buf,
                          & ulLen);      //�뿨Ƭͨ��

  if ( cStatus == MI_OK)		            //ͨ�ųɹ�
  {
    for ( uc = 0; uc < 4; uc ++ )
    {
       * ( pSnr + uc )  = ucComMF522Buf [ uc ]; //����UID
       ucSnr_check ^= ucComMF522Buf [ uc ];
    }
    
    if ( ucSnr_check != ucComMF522Buf [ uc ] )
      cStatus = MI_ERR;    				 
  }
  
  SetBitMask ( CollReg, 0x80 );
      
  return cStatus;		
}


/**
  * @brief  ��RC522����CRC16
  * @param  pIndata������CRC16������
  * @param  ucLen������CRC16�������ֽڳ���
  * @param  pOutData����ż�������ŵ��׵�ַ
  * @retval ��
  */
void CalulateCRC ( uint8_t * pIndata, 
                 uint8_t ucLen, 
                 uint8_t * pOutData )
{
  uint8_t uc, ucN;

  ClearBitMask(DivIrqReg,0x04);

  WriteRawRC(CommandReg,PCD_IDLE);

  SetBitMask(FIFOLevelReg,0x80);

  for ( uc = 0; uc < ucLen; uc ++)
    WriteRawRC ( FIFODataReg, * ( pIndata + uc ) );   

  WriteRawRC ( CommandReg, PCD_CALCCRC );

  uc = 0xFF;

  do 
  {
      ucN = ReadRawRC ( DivIrqReg );
      uc --;
  } while ( ( uc != 0 ) && ! ( ucN & 0x04 ) );
  
  pOutData [ 0 ] = ReadRawRC ( CRCResultRegL );
  pOutData [ 1 ] = ReadRawRC ( CRCResultRegM );		
}


/**
  * @brief  ѡ����Ƭ
  * @param  pSnr����Ƭ���кţ�4�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdSelect ( uint8_t * pSnr )
{
  char ucN;
  uint8_t uc;
  uint8_t ucComMF522Buf [ MAXRLEN ]; 
  uint32_t  ulLen;
  ucComMF522Buf [ 0 ] = PICC_ANTICOLL1;
  ucComMF522Buf [ 1 ] = 0x70;
  ucComMF522Buf [ 6 ] = 0;

  for ( uc = 0; uc < 4; uc ++ )
  {
    ucComMF522Buf [ uc + 2 ] = * ( pSnr + uc );
    ucComMF522Buf [ 6 ] ^= * ( pSnr + uc );
  }
  
  CalulateCRC ( ucComMF522Buf, 7, & ucComMF522Buf [ 7 ] );

  ClearBitMask ( Status2Reg, 0x08 );

  ucN = PcdComMF522 ( PCD_TRANSCEIVE,
                     ucComMF522Buf,
                     9,
                     ucComMF522Buf, 
                     & ulLen );
  
  if ( ( ucN == MI_OK ) && ( ulLen == 0x18 ) )
    ucN = MI_OK;  
  else
    ucN = MI_ERR;    
  
  return ucN;		
}



/**
  * @brief  ��֤��Ƭ����
  * @param  ucAuth_mode��������֤ģʽ= 0x60����֤A��Կ��
            ������֤ģʽ= 0x61����֤B��Կ
  * @param  uint8_t ucAddr�����ַ
  * @param  pKey������ 
  * @param  pSnr����Ƭ���кţ�4�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdAuthState ( uint8_t ucAuth_mode, 
                    uint8_t ucAddr, 
                    uint8_t * pKey,
                    uint8_t * pSnr )
{
  char cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
  uint32_t ulLen;
  

  ucComMF522Buf [ 0 ] = ucAuth_mode;
  ucComMF522Buf [ 1 ] = ucAddr;

  for ( uc = 0; uc < 6; uc ++ )
    ucComMF522Buf [ uc + 2 ] = * ( pKey + uc );   

  for ( uc = 0; uc < 6; uc ++ )
    ucComMF522Buf [ uc + 8 ] = * ( pSnr + uc );   

  cStatus = PcdComMF522 ( PCD_AUTHENT,
                          ucComMF522Buf, 
                          12,
                          ucComMF522Buf,
                          & ulLen );

  if ( ( cStatus != MI_OK ) || ( ! ( ReadRawRC ( Status2Reg ) & 0x08 ) ) )
    cStatus = MI_ERR;   
    
  return cStatus;
}


/**
  * @brief  д���ݵ�M1��һ��
  * @param  uint8_t ucAddr�����ַ
  * @param  pData��д������ݣ�16�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdWrite ( uint8_t ucAddr, uint8_t * pData )
{
  char cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ];
  uint32_t ulLen;
   
  
  ucComMF522Buf [ 0 ] = PICC_WRITE;
  ucComMF522Buf [ 1 ] = ucAddr;

  CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );

  cStatus = PcdComMF522 ( PCD_TRANSCEIVE,
                          ucComMF522Buf,
                          4, 
                          ucComMF522Buf,
                          & ulLen );

  if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || 
         ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
    cStatus = MI_ERR;   
      
  if ( cStatus == MI_OK )
  {
    //memcpy(ucComMF522Buf, pData, 16);
    for ( uc = 0; uc < 16; uc ++ )
      ucComMF522Buf [ uc ] = * ( pData + uc );  
    CalulateCRC ( ucComMF522Buf, 16, & ucComMF522Buf [ 16 ] );

    cStatus = PcdComMF522 ( PCD_TRANSCEIVE,
                           ucComMF522Buf, 
                           18, 
                           ucComMF522Buf,
                           & ulLen );
    if ( ( cStatus != MI_OK ) || ( ulLen != 4 ) || 
         ( ( ucComMF522Buf [ 0 ] & 0x0F ) != 0x0A ) )
      cStatus = MI_ERR;    
  } 	
  return cStatus;		
}


/**
  * @brief  ��ȡM1��һ������
  * @param  ucAddr�����ַ
  * @param  pData�����������ݣ�16�ֽ�
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdRead ( uint8_t ucAddr, uint8_t * pData )
{
  char cStatus;
  uint8_t uc, ucComMF522Buf [ MAXRLEN ]; 
  uint32_t ulLen;
  ucComMF522Buf [ 0 ] = PICC_READ;
  ucComMF522Buf [ 1 ] = ucAddr;
  CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
  cStatus = PcdComMF522 ( PCD_TRANSCEIVE,
                          ucComMF522Buf,
                          4, 
                          ucComMF522Buf,
                          & ulLen );
  if ( ( cStatus == MI_OK ) && ( ulLen == 0x90 ) )
  {
    for ( uc = 0; uc < 16; uc ++ )
      * ( pData + uc ) = ucComMF522Buf [ uc ];   
  }
  else
    cStatus = MI_ERR;   
  return cStatus;		
}


/**
  * @brief  ���Ƭ��������״̬
  * @param  ��
  * @retval ״ֵ̬= MI_OK���ɹ�
  */
char PcdHalt( void )
{
	uint8_t ucComMF522Buf [ MAXRLEN ]; 
	uint32_t  ulLen;
    ucComMF522Buf [ 0 ] = PICC_HALT;
    ucComMF522Buf [ 1 ] = 0;
    CalulateCRC ( ucComMF522Buf, 2, & ucComMF522Buf [ 2 ] );
 	PcdComMF522 ( PCD_TRANSCEIVE,
                ucComMF522Buf,
                4, 
                ucComMF522Buf, 
                & ulLen );
  return MI_OK;	
}


void IC_CMT ( uint8_t * UID,
              uint8_t * KEY,
              uint8_t RW,
              uint8_t * Dat )
{
  uint8_t ucArray_ID [ 4 ] = { 0 }; //�Ⱥ���IC�������ͺ�UID(IC�����к�)	
  PcdRequest ( 0x52, ucArray_ID ); //Ѱ��
  PcdAnticoll ( ucArray_ID );      //����ײ
  PcdSelect ( UID );               //ѡ����
  PcdAuthState ( 0x60, 0x10, KEY, UID );//У��
	if ( RW )                        //��дѡ��1�Ƕ���0��д
    PcdRead ( 0x10, Dat );
   else 
     PcdWrite ( 0x10, Dat );
   PcdHalt ();	 
}

/* ��������Read_ID
 * ����  ����ȡtag ID
 * ����  ����
 * ����  : MI_OK����ȡ�ɹ�
		   MI_ERR����ȡʧ��
		   MI_NOTAGERR��û�л�ȡ����ƬID
 * ����  ���ⲿ����              
*/
uint8_t Read_ID (void)
{
	char cStr [ 30 ];  
	uint8_t ucStatusReturn;    /*����״̬*/                                                                                         
	//while ( 1 )
	//{ 
		/*Ѱ��*/
		if ( ( ucStatusReturn = PcdRequest ( PICC_REQALL, ucArray_ID ) ) != MI_OK )  
		/*��ʧ���ٴ�Ѱ��*/
			ucStatusReturn = PcdRequest ( PICC_REQALL, ucArray_ID );	
		if ( ucStatusReturn == MI_OK  )
		{
		     /*����ײ�����ж��ſ������д��������Χʱ������ͻ���ƻ������ѡ��һ�Ž��в�����*/
			if ( PcdAnticoll ( ucArray_ID ) == MI_OK )                                                                   
			{
				  LED_Blink();
				  sprintf ( cStr, "The Card ID is: %02X%02X%02X%02X       ",
                  ucArray_ID [ 0 ], 
                  ucArray_ID [ 1 ], 
                  ucArray_ID [ 2 ], 
                  ucArray_ID [ 3 ]);
				  printf ( "%s\r\n",cStr ); 
				  LCD_ShowString( 0, 80,240,240,16 ,"                            ");
				  LCD_ShowString( 0, 80,240,240,16 ,cStr);
			    return MI_OK;
			}
			else 
			return MI_ERR;			
		}
		else
		return MI_NOTAGERR;
 // }	
}

/* ��������tag_match
 * ����  ���жϵ�ǰtag�Ƿ��еǼǹ�
 * ����  ����ȡ����tag ID
 * ����  : 1���еǼǣ�0��û�еǼ�
 * ����  ���ⲿ����              
*/
uint8_t tag_match(uint8_t* tag_id)
{
	char target_tag_id[16];//16����ֵת��Ϊ�ַ�������Ͻ�����һ����Ҫ16���ֽ�
    char stored_tag_id[16];
	uint8_t cnt,j=0;
	//��������IDת��Ϊ�ַ���
	sprintf(target_tag_id,"%02X%02X%02X%02X",*(tag_id),*(tag_id+1),*(tag_id+2),*(tag_id+3));
	printf("target_tag_id:%s\n",target_tag_id);
	//��ȡ��ǰ�Ǽǵ�ID��Ŀ
	cnt = AT24CXX_ReadOneByte(Mod_Cnt_Tag_Add)&Get_Tag_Cnt;
	printf("Matching cnt :%d\n",cnt);
	if(cnt==0)
	{
		printf("No tag is registered!\n");
		POINT_COLOR = RED;
		LCD_ShowString( 25, 200,240,240,16 ,"No tag registered!          ");
		POINT_COLOR = BLACK;
		return 0;
	}
	for(j=1;j<=cnt;j++)
	{
		//���Ǽǵ�ID���������ת��Ϊ�ַ���
		sprintf(stored_tag_id,"%02X%02X%02X%02X",AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+0),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+1),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+2),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+3));
		printf("Matching stored_tag_id %d:%s\n",j,stored_tag_id);
		//Ȼ�󽫶���ID��Ǽǵ�ID��������Ƚ�
		if(strcmp(stored_tag_id,target_tag_id)==0)
		{
			printf("Matched!ID:\n");
			printf("Stored at add:0x%02x,ID is:%s\n",Tag_Stored_Base_Add(j),stored_tag_id);
			return 1;
		}
	}
	printf("ID: %s is Not Registered!\n",target_tag_id);
	POINT_COLOR = RED;
	LCD_ShowString(20, 200,240,240,16 ,"                             ");
	LCD_ShowString(25, 200,240,240,16 ,"Not registered tag!          ");
	POINT_COLOR = BLACK;
	return 0;
}

/* ��������regist_tag
 * ����  ���Ǽǿ�Ƭ
 * ����  ����ȡ����tag ID
 * ����  : 1
 * ����  ���ⲿ����              
*/
uint8_t regist_tag(uint8_t* tag_id)
{
	uint8_t cnt,i=0;
	//��ȡ��ǰ�ԵǼǵĿ�Ƭ��Ŀ
	printf("Registering Tag........!\n");
	cnt = AT24CXX_ReadOneByte(Mod_Cnt_Tag_Add)&Get_Tag_Cnt;
	//�µǼ�һ����Ƭ����Ŀ��ԭ���Ļ����ϼ�һ
	cnt += 1;
	//��������������Ŀ
	if(cnt>56)
	  {
		cnt = 0&Get_Tag_Cnt;
		printf("Storage is full! New ID will overwrite old ID!\n");
	  }
	printf("regist cnt:%d\n",cnt);
	//�ٽ���Ŀд��EEPROM��ַ0x00�еĵ���λ
	AT24CXX_WriteOneByte(Mod_Cnt_Tag_Add,cnt|Modified); 
	//���¿�ƬIDд��EEPROM
	for(i=0;i<4;i++)
	{
		//Tag_Stored_Base_Add(cnt)Ϊ�¿�ƬID�洢��ַƫ�ƣ���ʼ��ַ0x01,����cnt����
		AT24CXX_WriteOneByte((Tag_Stored_Base_Add(cnt)+i),*(tag_id+i));
	}
	printf("Tag ID:%02X%02X%02X%02X is Registered\n",*(tag_id+0),*(tag_id+1),*(tag_id+2),*(tag_id+3));
	printf("Stored at address:0x%02x\n",Tag_Stored_Base_Add(cnt));
	return 1;
}

/* ��������delete_registered_tag
 * ����  ��ɾ��ϵͳ��ָ���Ŀ�ƬID
 * ����  ����ȡ����tag ID
 * ����  : 1���ɹ�ɾ���ǼǵĿ�Ƭ��0��û�еǼǵĿ�ƬҪɾ��
 * ����  ���ⲿ����              
*/
uint8_t delete_registered_tag(uint8_t* tag_id)
{
	char target_tag_id[16];//16����ֵת��Ϊ�ַ�������Ͻ�����һ����Ҫ16���ֽ�
    char stored_tag_id[16];
	int8_t cnt,j,k,x,y,z,tem=0;
	//��������IDת��Ϊ�ַ���
	sprintf(target_tag_id,"%02X%02X%02X%02X",*(tag_id),*(tag_id+1),*(tag_id+2),*(tag_id+3));
	//��ȡϵͳ�еǼǵĿ�ƬID����
	cnt = AT24CXX_ReadOneByte(Mod_Cnt_Tag_Add)&Get_Tag_Cnt;
	k= cnt;
	printf("delete cnt:%d\n",cnt);
	if(cnt==0)
	{
		printf("No tags to be deleted!\n");
		POINT_COLOR = RED;
		LCD_ShowString( 20, 200,240,240,16 ,"                                ");
		LCD_ShowString( 25, 200,240,240,16 ,"No tags to be deleted!          ");
		POINT_COLOR = BLACK;
		return 0;
	}
	printf("Delete Tag is in process....\n");
	//Tag_Stored_Base_Add(cnt)�е�cnt����0,���Դ˴���J��ʼֵ����Ϊ0�������޷���ȷ��ö�Ӧ��ַ�е�ֵ
	for(j=1;j<=cnt;j++)
	{
		//���Ǽǵ�ID���������ת��Ϊ�ַ���
		sprintf(stored_tag_id,"%02X%02X%02X%02X",AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+0),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+1),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+2),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+3));
		printf("Matching stored_tag_id %d:%s\n",j,stored_tag_id);
		//Ȼ�󽫶���ID��Ǽǵ�ID��������Ƚ�
		if(strcmp(stored_tag_id,target_tag_id)==0)
		{
			z=j;//��¼��ɾIDλ��
		    printf("Deleting The Tag....\n");
			LCD_ShowString( 25, 200,240,240,16 ,"Deleting The Tag....");
			for(uint8_t i=0;i<4;i++)
			{
				//���ID
				AT24CXX_WriteOneByte((Tag_Stored_Base_Add(j)+i),0x00);
			}
			printf("Deleted! \n");
			//��¼ɾ�����ٸ�ID
			k  = k-1;
		}
	}
	if(k==cnt)
	{
		printf("Target Tag Not found!\n");
		POINT_COLOR = RED;
		LCD_ShowString(20, 200,240,240,16 ,"                             ");
		LCD_ShowString( 25, 200,240,240,16 ,"Target Tag Not found!       ");
		POINT_COLOR = BLACK;
		return 0;
	}
	else
	{
		/**************����ɾ��IDλ��֮���ID��䵽��ǰλ��****************/
		for(x=0;x<=(cnt-z);x++)
		{
			printf("transfering data to EEPROM.....\n");
			for(y=0;y<4;y++)
			{
				//xҪ�ƶ���ID��Ŀ��y��ӦID���ֽ���
				tem = AT24CXX_ReadOneByte((((z+x+1)*4)-4+Base_Add)+y);
				AT24CXX_WriteOneByte(((((z+x)*4)-4+Base_Add)+y),tem);
				tem =0;
			}
		}
		cnt=k;
		//�ٽ��µ���Ŀд��EEPROM��ַ0x00�еĵ���λ
		AT24CXX_WriteOneByte(Mod_Cnt_Tag_Add,cnt|Modified); 
		printf("Delete tag process is completed!\n");
		POINT_COLOR = RED;
		LCD_ShowString(20, 200,240,240,16 ,"                          ");
		LCD_ShowString( 50, 200,240,240,16 ,"   Delete Done!         ");
		POINT_COLOR = BLACK;
		return  1;
	}
}
/* ��������Register_New_Tag
 * ����  ��ע���¿���ϵͳ������Ѿ�ע�����ִ��ע�����
 * ����  ����
 * ����  : ��
 * ����  ���ⲿ����              
*/
void Register_New_Tag(void)
{
	//��������LED��˸һ��
    LED_Blink();
	LCD_Clear(WHITE);
	register_delete_GUI(REGISTER);
	loop:	
	//��ʾ��ʾ��Ϣ
	LCD_ShowString( 20, 200,240,240,16 ,"                             ");
	while(1)
	{
			   //����ɨ��
			   tp_dev.scan(0); 
			   //�ж��Ƿ��а�������
			   if(tp_dev.sta&TP_PRES_DOWN)			
			   {	
					//��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
					if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) 
					{
						//������������  
						while(tp_dev.sta&TP_PRES_DOWN)	
						{
							tp_dev.scan(0); 
						}
						//��ʾ��
						beeponce(10);
						//��������LED��˸һ��
						LED_Blink();
						//������ؼ������򷵻�����ģʽ
						if(Back)
							goto loop2;
						//���NEW���¾���ִ��һ��ע���¿�����
						if(New)
						{
							goto loop;	
						}
						
					}
				}
			LCD_ShowString( 0, 80,280,280,16 ,"Put the tag on the antana area         ");
			//��ȡTAG ID
			switch(Read_ID())
			{
				//��ȡ�ɹ�
				case MI_OK:
							{   
								//ˢ����ʾ��
								beeponce(15);
								//��ʾ����ѡ��Back �� New
								//Show_BN;
								//���жϽ�Ҫע��Ŀ�Ƭ֮ǰ�Ƿ�ע�����ע����Ͳ���ע��
								if(tag_match(ucArray_ID)==1)
								{
									POINT_COLOR = RED;
									LCD_ShowString(20, 200,240,240,16 ,"                          ");
									LCD_ShowString(40, 200,240,240,16 ,"Already Registered!");
									printf("Already Registerd!\nRegistering rounting aborted!\n");
									POINT_COLOR = BLACK;
								}
								//û��ע�����ע�ῨƬ
								else
								{
									regist_tag(ucArray_ID);
									POINT_COLOR = RED;
									LCD_ShowString(20, 200,240,240,16 ,"                             ");
									LCD_ShowString(40, 200,240,240,16 ,"Registered Successfully!");
									POINT_COLOR = BLACK;
								}
								//�ж��û�ѡ�񣺷��ػ��Ǽ����Ǽ��¿�Ƭ
								while(1)
								{
								   //����ɨ��
								   tp_dev.scan(0); 
								   //�ж��Ƿ��а�������
								   if(tp_dev.sta&TP_PRES_DOWN)			
									{
										//��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
										if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) 
										{
											beeponce(10);
											LED_Blink();
											if(Back)
												goto loop2;
											if(New)
											{
												goto loop;	
											}
											//������������  
											while(tp_dev.sta&TP_PRES_DOWN)	
											{
												tp_dev.scan(0); 
											}
										}
									}
								 }
							}
				default:  
							{ 
									goto loop;	
							}
			}
		}
		  loop2:
		  //�ָ�������
		  security_door_window();		
}

/* ��������Delete_registered_Tag
 * ����  ��ɾ���ѵǼǵĿ�Ƭ
 * ����  ����
 * ����  : ��
 * ����  ���ⲿ����              
*/
void Delete_registered_Tag(void)
{
	//��������LED��˸һ��
    LED_Blink();
	LCD_Clear(WHITE);
	register_delete_GUI(DELETE);
	loop:
	//��ʾ��ʾ��Ϣ
	LCD_ShowString( 20, 200,240,240,16 ,"                                  ");
	LCD_ShowString( 0, 80,280,280,16 ,"Put the tag on the antana area        ");
	while(1)
	{
		   //����ɨ��
		   tp_dev.scan(0); 
		   //�ж��Ƿ��а�������
		   if(tp_dev.sta&TP_PRES_DOWN)			
			{	
				//��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
				if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) 
				{
					//������������  
					while(tp_dev.sta&TP_PRES_DOWN)	
					{
						tp_dev.scan(0); 
					}
					//��ʾ��
					beeponce(10);
					//��������LED��˸һ��
					LED_Blink();
					//������ؼ������򷵻�����ģʽ
					if(Back)
						goto loop2;
					//���NEW���¾���ִ��һ��ע���¿�����
					if(New)
					{
						goto loop;	
					}
					
				}
			}		
		
		//��ȡTAG ID
		switch(Read_ID())
		{
			//��ȡ�ɹ�
			case MI_OK:
						{   
							//��ʾ��
							beeponce(15);
							delete_registered_tag(ucArray_ID);
							//�ж��û�ѡ�񣺷��ػ��Ǽ����Ǽ��¿�Ƭ
							while(1)
							{
							   //����ɨ��
							   tp_dev.scan(0); 
							   //�ж��Ƿ��а�������
							   if(tp_dev.sta&TP_PRES_DOWN)			
								{
									//��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
									if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) 
									{
										
										beeponce(10);
										//LED��˸
										LED_Blink();
										if(Back)
											//�˳�
											goto loop2;
										if(New)
										{
											//���¿�ʼ
											goto loop;	
										}
										//������������  
										while(tp_dev.sta&TP_PRES_DOWN)	
										{
											tp_dev.scan(0); 
										}
									}
								}
							 }
						}
			default:  
						{ 
								goto loop;	
						}
		}
	}
	  loop2:
	  //�ָ�������
	  security_door_window();		
}


/* ��������DEL_ALL_TAG
 * ����  ��ͨ��PC���ڷ���msh����ɾ���ѵǼǵ����п�Ƭ
 * ����  ����
 * ����  : ��
 * ����  ���ⲿ����              
*/
uint8_t DEL_ALL_TAG(void)
{
	uint8_t cnt,i,j = 0;
	cnt = AT24CXX_ReadOneByte(Mod_Cnt_Tag_Add)&Get_Tag_Cnt;
	printf("delete cnt:%d\n",cnt);
	if(cnt==0)
	{
		printf("No tags to be deleted!\n");
		return 0;
	}
	for(j=1;j<=cnt;j++)
	{
		for(i=0;i<4;i++)
		{
			printf("Deleting......\n");
			AT24CXX_WriteOneByte((Tag_Stored_Base_Add(j)+i),0x00);
		}
	}
	cnt = 0;
	AT24CXX_WriteOneByte(Mod_Cnt_Tag_Add,cnt|Modified); 
	printf("All registered tags have been deleted!\n");
	return 1;
}
MSH_CMD_EXPORT(DEL_ALL_TAG,delet_all_tags);


/* ��������LIST_ALL_TAGS
 * ����  ��ͨ��PC���ڷ���msh�����г������ѵǼǵĿ�ƬID
 * ����  ����
 * ����  : ��
 * ����  ���ⲿ����              
*/
uint8_t LIST_ALL_TAGS(void)
{
	uint8_t cnt,j = 0;
	char stored_tag_id[16];
	cnt = AT24CXX_ReadOneByte(Mod_Cnt_Tag_Add)&Get_Tag_Cnt;
	printf("Total registered Tag ID:%d\n",cnt);
	if(cnt==0)
	{
		printf("No tag is registered!\n");
		return 0;
	}
	for(j=1;j<=cnt;j++)
	{
			//���Ǽǵ�ID���������ת��Ϊ�ַ���
			sprintf(stored_tag_id,"%02X%02X%02X%02X",AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+0),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+1),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+2),
												 AT24CXX_ReadOneByte(Tag_Stored_Base_Add(j)+3));
			printf("Registered Tags ID %d:%s\n",j,stored_tag_id);
	}
	return 1;
}
MSH_CMD_EXPORT(LIST_ALL_TAGS,list_all_tags);
