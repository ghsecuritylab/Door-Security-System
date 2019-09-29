#include "rc522_config.h"
#include "stm32f1xx_hal.h"

static void RC522_SPI_Config( void );

/**
  * @brief  RC522_Init
  * @param  ��
  * @retval ��
  */
void RC522_Init ( void )
{
	RC522_SPI_Config();	
	RC522_Reset_Disable();	
	RC522_CS_Disable();
	PcdReset();
    /*���ù�����ʽ*/
	M500PcdConfigISOType( 'A' );
	printf("RC522 Initilized!\n");
}

/**
  * @brief  R522 SPI����
  * @param  ��
  * @retval ��
  */
static void RC522_SPI_Config ( void )
{
    GPIO_InitTypeDef GPIO_Initure;
	//__HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();       //ʹ��GPIOBʱ��
    __HAL_RCC_GPIOA_CLK_ENABLE(); 
    
    //PA4,5,7
    GPIO_Initure.Pin=GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;          //һ��Ҫ����Ϊ���������������������ᵼ��SPI�޷�����
    GPIO_Initure.Pull=GPIO_PULLUP;                  //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;        //����            
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	 //PA6
	GPIO_Initure.Pin=GPIO_PIN_6;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;              //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;        //����            
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
	//PB12
    GPIO_Initure.Pin=GPIO_PIN_12;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;          //һ��Ҫ����Ϊ���������������������ᵼ��SPI�޷�����
    GPIO_Initure.Pull=GPIO_PULLUP;                  //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;        //����            
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
}


