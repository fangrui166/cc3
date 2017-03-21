#include "ds_iic.h"
#include "common.h"

void DS_IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_10|GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP ;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);

}

void DS_IIC_Start(void)
{
	DS_SDA_OUT();
	DS_IIC_SDA=1;
	delay_us(1);
	DS_IIC_SCL=1;
	delay_us(5);
 	DS_IIC_SDA=0;//START:when CLK is high,DATA change form high to low
	delay_us(5);
	DS_IIC_SCL=0;
	delay_us(2);
}

void DS_IIC_Stop(void)
{
	DS_SDA_OUT();
	DS_IIC_SCL=0;
	DS_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	DS_IIC_SCL=1;
	delay_us(5);
	DS_IIC_SDA=1;
	delay_us(4);
}

uint8_t DS_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	DS_SDA_IN();
	DS_IIC_SDA=1;
	delay_us(1);
	DS_IIC_SCL=1;
	delay_us(1);
	while(DS_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			DS_IIC_Stop();
			return 1;
		}
	}
	DS_IIC_SCL=0;
	return 0;
}

void DS_IIC_Ack(void)
{
	DS_IIC_SCL=0;
	DS_SDA_OUT();
	DS_IIC_SDA=0;
	delay_us(2);
	DS_IIC_SCL=1;
	delay_us(2);
	DS_IIC_SCL=0;
}

void DS_IIC_NAck(void)
{
	DS_IIC_SCL=0;
	DS_SDA_OUT();
	DS_IIC_SDA=1;
	delay_us(2);
	DS_IIC_SCL=1;
	delay_us(2);
	DS_IIC_SCL=0;
}

void DS_IIC_Write_Byte(uint8_t txd)
{
    uint8_t t;
    DS_SDA_OUT();
    DS_IIC_SCL=0;
    for(t=0;t<8;t++)
    {
		if((txd&0x80)>>7)
			DS_IIC_SDA=1;
		else
			DS_IIC_SDA=0;
		txd<<=1;
		delay_us(2);
		DS_IIC_SCL=1;
		delay_us(2);
		DS_IIC_SCL=0;
		delay_us(2);
    }
}

uint8_t DS_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	DS_SDA_IN();
    for(i=0;i<8;i++ )
	{
        DS_IIC_SCL=0;
        delay_us(2);
		DS_IIC_SCL=1;
        receive<<=1;
        if(DS_READ_SDA)
			receive++;
		delay_us(1);
    }
/*
    if (!ack)
        DS_IIC_NAck();
    else
        DS_IIC_Ack();
*/
    return receive;
}

