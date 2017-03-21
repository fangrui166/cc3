/******************************************************************************
* @ File name --> ds3231.c
* @ Author    --> By@ Rui Fang
* @ Version   --> V1.0
* @ Date      --> 08 - 22 - 2016
* @ Brief     --> 高精度时钟芯片DS3231驱动代码
*
* @ Copyright (C) 20**
* @ All rights reserved
*******************************************************************************
*
*                                  File Update
* @ Version   --> V1.
* @ Author    -->
* @ Date      -->
* @ Revise    -->
*
******************************************************************************/

#include "ds3231.h"
#include "rtc_drv.h"
#include "common.h"

static I2C_HandleTypeDef hi2c2;

/* I2C2 init function */
void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = DS3231_ADD_BASS;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c2);

}
HAL_StatusTypeDef DS3231_Write_Byte(uint8_t REG_ADD,uint8_t dat)
{
    HAL_StatusTypeDef status = HAL_OK;
    status = HAL_I2C_Mem_Write(&hi2c2, DS3231_Write_ADD, REG_ADD, I2C_MEMADD_SIZE_8BIT, &dat, 1, 100);
    if(status != HAL_OK){
        xloge("%s error:%d ***\n",__func__,status);
    }
    return status;
}

uint8_t DS3231_Read_Byte(uint8_t REG_ADD)
{
    uint8_t data;
    HAL_StatusTypeDef status = HAL_OK;
    status = HAL_I2C_Mem_Read(&hi2c2, DS3231_Write_ADD, REG_ADD, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK){
        xloge("%s error:%d ***\n",__func__,status);
    }
    return data;
}
HAL_StatusTypeDef DS3231_Operate_Register(uint8_t REG_ADD,uint8_t *pBuff,uint8_t num,uint8_t mode)
{
    HAL_StatusTypeDef status = HAL_OK;
    if(mode == DS3231_I2C_WRITE){
        status = HAL_I2C_Mem_Write(&hi2c2, DS3231_Write_ADD, REG_ADD, I2C_MEMADD_SIZE_8BIT, pBuff, num, 100);
        if(status != HAL_OK){
            xloge("%s(%d) error:%d ***\n",__func__, mode,status);
        }
    }
    else{
        status = HAL_I2C_Mem_Read(&hi2c2, DS3231_Write_ADD, REG_ADD, I2C_MEMADD_SIZE_8BIT, pBuff, num, 100);
        if(status != HAL_OK){
            xloge("%s(%d) error:%d ***\n",__func__, mode,status);
        }
    }
    return status;
}

/******************************************************************************
* Function Name --> DS3231读取或者写入时间信息
* Description   --> 连续写入n字节或者连续读取n字节数据
* Input         --> *pBuff：写入数据缓存
*                   mode：操作模式。0：写入数据操作。1：读取数据操作
* Output        --> none
* Reaturn       --> none
******************************************************************************/
void DS3231_ReadWrite_Time(Time_Typedef * TimeValue, uint8_t mode)
{
	uint8_t Time_Register[8];	//定义时间缓存

	if(mode == DS3231_I2C_READ)	//读取时间信息
	{
		DS3231_Operate_Register(Address_second,Time_Register,7,DS3231_I2C_READ);	//从秒地址（0x00）开始读取时间日历数据

		/******将数据复制到时间结构体中，方便后面程序调用******/
		TimeValue->second = ((Time_Register[0] & 0x0F) + (((Time_Register[0] >> 4) & 0x07) * 10));	//秒数据
		TimeValue->minute = ((Time_Register[1] & 0x0F) + (((Time_Register[1] >> 4) & 0x07) * 10));	//分钟数据
		TimeValue->hour   = ((Time_Register[2] & 0x0F) + (((Time_Register[2] >> 4) & 0x03) * 10));	//小时数据
		TimeValue->week   = Time_Register[3] & Shield_weekBit;	//星期数据
		TimeValue->date   = ((Time_Register[4] & 0x0F) + (((Time_Register[4] >> 4) & 0x03) * 10));	//日数据
		TimeValue->month  = ((Time_Register[5] & 0x0F) + (((Time_Register[5] >> 4) & 0x01) * 10));	//月数据
		TimeValue->year   = ((Time_Register[6] & 0x0F) + (((Time_Register[6] >> 4) & 0x0F) * 10));	//年数据
		TimeValue->year = TimeValue->year + 2000;
        //xlogd("Read :%d-%d-%d %d:%d:%d\n",TimeValue->year,TimeValue->month,TimeValue->date,
            //TimeValue->hour,TimeValue->minute,TimeValue->second);
	}
	else
	{

        //xlogd("Write :%d-%d-%d %d:%d:%d\n",TimeValue->year,TimeValue->month,TimeValue->date,
            //TimeValue->hour,TimeValue->minute,TimeValue->second);
		/******从时间结构体中复制数据进来******/
        TimeValue->year = TimeValue->year % 100;
		Time_Register[0] = ((TimeValue->second % 10) | ((TimeValue->second / 10) << 4));	//秒
		Time_Register[1] = ((TimeValue->minute % 10) | ((TimeValue->minute / 10) << 4));	//分钟
		Time_Register[2] = ((TimeValue->hour % 10) | (((TimeValue->hour / 10) << 4) & 0x30));	//小时 Hour_Mode24
		Time_Register[3] = TimeValue->week;	//星期
		Time_Register[4] = ((TimeValue->date % 10) | (((TimeValue->date / 10) << 4) & 0x30));	//日
		Time_Register[5] = ((TimeValue->month % 10) | (((TimeValue->month / 10) << 4) & 0x10));	//月
		Time_Register[6] = ((TimeValue->year % 10) | ((TimeValue->year / 10) << 4));	//年

		DS3231_Operate_Register(Address_second,Time_Register,7,DS3231_I2C_WRITE);	//从秒地址（0x00）开始写入时间日历数据
	}
}
/******************************************************************************
* Function Name --> 时间日历初始化
* Description   --> none
* Input         --> *TimeVAL：RTC芯片寄存器值指针
* Output        --> none
* Reaturn       --> none
******************************************************************************/
void DS3231_Time_Init(Time_Typedef *TimeVAL)
{

	DS3231_ReadWrite_Time(TimeVAL, DS3231_I2C_WRITE);
	DS3231_Write_Byte(Address_control, OSC_Enable);
	DS3231_Write_Byte(Address_control_status, Clear_OSF_Flag);
}
/******************************************************************************
* Function Name --> DS3231检测函数
* Description   --> 将读取到的时间日期信息转换成ASCII后保存到时间格式数组中
* Input         --> none
* Output        --> none
* Reaturn       --> 0: 正常
*                   1: 不正常或者需要初始化时间信息
******************************************************************************/
uint8_t DS3231_Check(void)
{
	if(DS3231_Read_Byte(Address_control_status) & 0x80)  //晶振停止工作了
	{
        printf("%s 1\n",__func__);
		return 1;  //异常
	}
	else if(DS3231_Read_Byte(Address_control) & 0x80)  //或者 EOSC被禁止了
	{
        printf("%s 2\n",__func__);
		return 1;  //异常
	}
	else	return 0;  //正常
}
/******************************************************************************
* Function Name --> 时间日历数据处理函数
* Description   --> 将读取到的时间日期信息转换成ASCII后保存到时间格式数组中
* Input         --> none
* Output        --> none
* Reaturn       --> none
******************************************************************************/
void DS3231_Get_Date_Time(Time_Typedef *TimeValue)
{
	/******************************************************
	                   读取时间日期信息
	******************************************************/

	DS3231_ReadWrite_Time(TimeValue, DS3231_I2C_READ);	//获取时间日历数据


}
void DS3231_Set_Date(uint16_t year, uint8_t month, uint8_t day)
{
    Time_Typedef TimeValue;
    DS3231_ReadWrite_Time(&TimeValue, DS3231_I2C_READ);
    TimeValue.year = year;
    TimeValue.month = month;
    TimeValue.date = day;
    TimeValue.week = RTCWeekDayNum(TimeValue.year,
                        TimeValue.month, TimeValue.date);
    if(TimeValue.week == 0){
        TimeValue.week = 7;
    }
    DS3231_ReadWrite_Time(&TimeValue, DS3231_I2C_WRITE);

}
void DS3231_Set_Time(uint8_t hour, uint8_t min, uint8_t sec)
{
    Time_Typedef TimeValue;
    DS3231_ReadWrite_Time(&TimeValue, DS3231_I2C_READ);
    TimeValue.hour= hour;
    TimeValue.minute= min;
    TimeValue.second= sec;
    DS3231_ReadWrite_Time(&TimeValue, DS3231_I2C_WRITE);

}

/******************************************************************************
* Function Name --> 读取芯片温度寄存器
* Description   --> 温度寄存器地址为0x11和0x12，这两寄存器为只读
* Input         --> none
* Output        --> *Temp：最终温度显示字符缓存
* Reaturn       --> none
******************************************************************************/
void DS3231_Read_Temp(uint8_t *Temp)
{
	uint8_t temph,templ;
	float temp_dec;

	temph = DS3231_Read_Byte(Address_temp_MSB);	//读取温度高8bits
	templ = DS3231_Read_Byte(Address_temp_LSB) >> 6;	//读取温度低2bits

	//温度值转换
	if(temph & 0x80)	//判断温度值的正负
	{	//负温度值
		temph = ~temph;	//高位取反
		templ = ~templ + 0x01;	//低位取反加1
		Temp[0] = 0x2d;	//显示“-”
	}
	else	Temp[0] = 0x20;	//正温度不显示符号，显示正号填0x2b

	//小数部分计算处理
	temp_dec = (float)templ * (float)0.25;	//0.25℃分辨率

	//整数部分计算处理
	temph = temph & 0x70;	//去掉符号位
	Temp[1] = temph % 1000 / 100 + 0x30;	//百位
	Temp[2] = temph % 100 / 10 + 0x30;	//十位
	Temp[3] = temph % 10 + 0x30;	//个位
	Temp[4] = 0x2e;	//.

	//小数部分处理
	Temp[5] = (uint8_t)(temp_dec * 10) + 0x30;	//小数点后一位
	Temp[6] = (uint8_t)(temp_dec * 100) % 10 + 0x30;	//小数点后二位

	if(Temp[1] == 0x30)	Temp[1] = 0x20;	//百位为0时不显示
	if(Temp[2] == 0x30)	Temp[2] = 0x20;	//十位为0时不显示
}
void DS3231_GPIO_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_Delay(1);
}
void DS3231_init(void)
{
    uint8_t temp[8] = {0};
    MX_I2C2_Init();
    DS3231_GPIO_init();
    if(DS3231_Check()){
        struct tm build_time;
        Time_Typedef init_time;
        GetCompileDateTime(&build_time);
        init_time.year = build_time.tm_year;
        init_time.month = build_time.tm_mon;
        init_time.date = build_time.tm_mday;
        init_time.hour = build_time.tm_hour;
        init_time.minute = build_time.tm_min;
        init_time.second = build_time.tm_sec;
        init_time.week = RTCWeekDayNum(build_time.tm_year,
                            build_time.tm_mon, build_time.tm_mday);

        DS3231_Time_Init(&init_time);

        xlogd("Config RTC-DS3231 by first time\n");
    }
    else{
        xlogd("RTC-DS3231 was config\n");
    }

    DS3231_Read_Temp(temp);
    xlogd("Current temperature:%s ℃\n",(char *)temp);
}

