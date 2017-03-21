#include "FM1702.h"
#include "fm1702_drv.h"
#include "common.h"


unsigned char 	FM1702DataBuffer[16];	        //接收和发送数据缓冲区
unsigned char gBuff[16];

static unsigned char 	CardSnr[5];  	        //卡片序列号
//static unsigned char    Random[4];              //0扇区 块2下的随机数
static unsigned char 	CardKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //默认卡片密码
const unsigned char 	UserKey[6] = {0x08, 0x02, 0x01, 0x00, 0x01, 0x07}; //卡片密码
const unsigned char 	CompanyKey[6] = {'b', 'x', 'i', 'a', 'n', 'g'}; //公司卡片密码
//************************************延时函数**************************************
//函数定义: void delay(uint32_t nCount)
//描    述：延时
//入口参数：nCount ：延时函数中，循环的次数
//出口参数：无
//备    注：Editor：Armink 2011-03-18    Company: BXXJS
//**********************************************************************************
void delay(uint32_t nCount)
{
//	nCount *= 100;
	nCount *= 5;
//	nCount *= 10;
	for(; nCount!= 0;nCount--);

//	rt_thread_delay(nCount/100);//nCount ms
}
#if USE_STM_HW_SPI

#define SPI1_TIMEOUT_MAX   1000
#define MASK_DATA1	0xFF
#define MASK_DATA2	0x80

extern SPI_HandleTypeDef hspi1;

unsigned char ReadReg(unsigned char Address)
{
	uint8_t array_tx[2] = {0};
	uint8_t array_rx[2] = {0};
    array_tx[0] = (Address<<1) | MASK_DATA2;
    FM1702_CS_L;
    HAL_SPI_TransmitReceive(&hspi1, array_tx, array_rx, 2, SPI1_TIMEOUT_MAX);
    //HAL_SPI_Transmit(&hspi1, array_tx, 1, SPI1_TIMEOUT_MAX);
    //HAL_SPI_Receive(&hspi1, array_rx, 1, SPI1_TIMEOUT_MAX);

    FM1702_CS_H;
    return array_rx[1];
}
unsigned char ReadBuffer(uint8_t *pTxData, uint8_t *pRxData, uint8_t len)
{
    FM1702_CS_L;
    HAL_SPI_Transmit(&hspi1, pTxData, 1, SPI1_TIMEOUT_MAX);
    HAL_SPI_Receive(&hspi1, pRxData, len, SPI1_TIMEOUT_MAX);
    FM1702_CS_H;
    return 0;
}

void WriteReg(unsigned char Address, unsigned char Data)
{

	uint8_t array[2];
    array[0] = (uint8_t)((Address<<1)&0x7E);
    array[1] = (uint8_t)Data;
    FM1702_CS_L;
    HAL_SPI_Transmit(&hspi1, array, 2, SPI1_TIMEOUT_MAX);
    FM1702_CS_H;

}
#else
/****************************************************************/
/*名称: Send_1_Char                                             */
/*功能: 向FM1702发送1个字节                                      */
/*输入: Data-发送的值                                           */
/*输出: N/A                                                     */
/****************************************************************/
void Send_1_Char(unsigned char Data)
{
  unsigned char i,Temp;
  for (i=0;i<8;i++)
  {
    delay(10);
    Temp=Data&0x80;
    if (Temp==0x80)
    FM1702_MOSI_H;
    else FM1702_MOSI_L;
    delay(10);
    FM1702_SCK_H;
    Data<<=1;
    delay(10);
    FM1702_SCK_L;
  }
}
/****************************************************************/
/*名称: Rec_1_Char                                              */
/*功能: 接收FM1702发送的1个字节                                  */
/*输入: N/A                                                     */
/*输出: 读出的值                                                */
/****************************************************************/
unsigned char  Rec_1_Char(void)
{
    unsigned char i,Temp=0;
    for (i = 0; i < 8; i++)
     {//接收FM1702发送1个字节
        Temp <<= 1;
        Temp |=FM1702_MISO;
        FM1702_SCK_H;
        delay(10);
        FM1702_SCK_L;
     }
    return Temp;
}
/****************************************************************/
/*名称: ReadReg                                               */
/*功能: 读FM1702寄存器                                           */
/*输入: Address-寄存器地址                                      */
/*输出: 读出的值                                                */
/****************************************************************/
unsigned char ReadReg(unsigned char Address)
{
    unsigned char Temp;
    Address=(Address<<1)|0x80;
    FM1702_CS_L;
    Send_1_Char(Address);
    Temp=Rec_1_Char();
    FM1702_CS_H;
    return Temp;
}
/****************************************************************/
/*名称: WriteReg                                              */
/*功能: 写FM1702寄存器                                           */
/*输入: Address - 寄存器地址; value - 写入的值                  */
/*输出: N/A                                                     */
/****************************************************************/
void WriteReg(unsigned char Address, unsigned char Data)
{
    Address = ((Address<<1)&0x7E);
    FM1702_CS_L;
    Send_1_Char(Address);
    Send_1_Char(Data);
    FM1702_CS_H;
}
#endif
/****************************************************************/
/*名称: Clear_FIFO                                              */
/*功能: 清除FIFO数据                                            */
/*输入: N/A                                                     */
/*输出: N/A                                                     */
/****************************************************************/
void Clear_FIFO(void)
{
    unsigned char i=0,Temp;
    Temp =ReadReg(Control);						//清空FIFO
    Temp = (Temp | 0x01);
    WriteReg(Control, Temp);
    do{
	if(ReadReg(FIFOLength)==0)
	break;
    }while(i++<200);
}
/****************************************************************/
/*名称: Write_FIFO                                              */
/*功能: 写FIFO数据                                            */
/*输入: N/A                                                     */
/*输出: N/A                                                     */
/****************************************************************/
void Write_FIFO(unsigned char Number)
{
    unsigned char i;
    if(Number>0)
    {
      for (i=0; i<Number; i++)
      WriteReg(FIFOData, FM1702DataBuffer[i]);
    }
}
//=======================================================
//名称：FM1702Write_FIFO
//功能：将数据写入FM1702的FIFO
//输入:	buff：要写入的数据在Ram中的首地址
//		count：	要写入的字节数
//输出:
//========================================================
void FM1702Write_FIFO(unsigned char *buff, unsigned char count)
{
    unsigned char i;
//	drv_fm1702ItfWrite(FIFO_Reg, buff, count);
    for (i = 0; i < count; i++)
        WriteReg(FIFOData, buff[i]);
    //		SPIWrite(FIFO_Reg,buff,count);
}

/****************************************************************/
/*名称: Read_FIFO                                               */
/*功能: 读FIFO数据                                              */
/*输入: N/A                                                     */
/*输出: N/A                                                     */
/****************************************************************/
void Read_FIFO(void)
{
    unsigned char i,Temp;
    Temp=ReadReg(FIFOLength);
    if(Temp>18)
    Temp=18;
    for (i=0; i<Temp; i++)
    FM1702DataBuffer[i]=ReadReg(FIFOData);
}
//=======================================================
//	名称: ReadFIFO
//	功能: 该函数实现从FM1702的FIFO中读出x bytes数据
//	输入:	buff, 指向读出数据的指针
//	输出: 	数据长度（单位：字节）
//=======================================================
unsigned char FM1702ReadFifo(unsigned char *buff)
{
    unsigned char	ucResult, i;
    ucResult = ReadReg(FIFOLength);
    if (ucResult == 0 || ucResult > 16)
        return 0;
//	drv_fm1702ItfRead(FIFO_Reg, buff, ucResult);
    for (i = 0; i < ucResult; i++)
    {
        buff[i] = ReadReg(FIFOData);
    }

    return ucResult;
}

/****************************************************************/
/*名称: Command_Send                                            */
/*功能: 通过FM1702和ISO14443卡通讯                              */
/*输入: Comm_Code-命令码；Number-数据个数						*/
/*输出: N/A                                                     */
/****************************************************************/
void Command_Send(unsigned char Number,unsigned char Comm_Code)
{
    unsigned char i=0,RetChar;
    WriteReg(Command,Idle);			//结束当前指令
    Clear_FIFO();    		                //清除FIFO
    Write_FIFO(Number);
    ReadReg(FIFOLength);
    WriteReg(Command, Comm_Code);		//发送命令码
    do
    {
        delay(10);
        RetChar=ReadReg(Command);
    	if(RetChar==0)
    	break;
    }while(i++<10);
}
//=======================================================
//	名称: drv_fm1702Command
//	功能: 该函数实现向FM1702发送命令集的功能
//	输入:	count, 待发送命令集的长度
//			buff, 指向待发送数据的指针
//			Comm_Set, 命令码：指FM1702发给IC卡的命令
//	输出: 	TRUE, 命令被正确执行
//			FALSE, 命令执行错误
//=======================================================
unsigned char FM1702Command(unsigned char Comm_Set, unsigned char *buff, unsigned char count)
{
    unsigned char ucResult1, ucResult2, i;
    WriteReg(Command, 0x00);
    Clear_FIFO();
    FM1702Write_FIFO(buff, count);
    WriteReg(Command, Comm_Set);
    for (i = 0; i < 0xA0; i++)
    {
        ucResult1 = ReadReg(Command);
        ucResult2 = ReadReg(InterruptRq) & 0x80;
        if (ucResult1 == 0 || ucResult2 == 0x80)
            return 1;
    }
    return 0;
}

unsigned char FM1702LoadKey(unsigned char *ramadr)
{
    unsigned char acktemp, temp, i;
    unsigned char ucBuff[12];
    for (i = 0; i < 6; i++)
    {
        temp = ramadr[i];
        ucBuff[2 * i] = (((ramadr[i] & 0xf0) >> 4) | ((~ramadr[i]) & 0xf0));
        ucBuff[2 * i + 1] = ((temp & 0xf) | (~(temp & 0xf) << 4));
    }
    acktemp = FM1702Command(LoadKey, ucBuff, 12);
    if(!acktemp){
        xloge("%s failed\n",__func__);
    }
    temp = ReadReg(SecondaryStatus);
    if (temp & 0x40)
    {
        temp = 0x0;
        WriteReg(Command, temp);
        return(0);
    }
    temp = 0x0;
    WriteReg(Command, temp);
    return(1);
}
//=======================================================
//	名称: drv_fm1702HaltCard
//	功能: 该函数实现暂停MIFARE卡
//	输入:	N/A
//	输出: 	FM1702_OK: 应答正确
//			FM1702_PARITYERR: 奇偶校验错
//			FM1702_FRAMINGERR:FM1702帧错误
//			FM1702_CRCERR: CRC校验错
//			FM1702_NOTAGERR: 无卡
//=======================================================
unsigned char FM1702HaltCard(void)
{
    unsigned char	temp;
    //选择TX1、TX2的发射天线阻抗
    WriteReg(CWConductance, 0x3f);
    //选择数据校验种类和模式
    WriteReg(ChannelRedundancy, 0x03);
    *gBuff = RF_CMD_HALT;
    *(gBuff + 1) = 0x00;
    temp = FM1702Command(Transmit, gBuff, 2);//发送FIFO缓存地址
    if (temp == 1)
        return FM1702_OK;
    else
    {
        temp = ReadReg(ErrorFlag);
        if ((temp & 0x02) == 0x02)
        {
            return(FM1702_PARITYERR);
        }

        if ((temp & 0x04) == 0x04)
        {
            return(FM1702_FRAMINGERR);
        }
        return(FM1702_NOTAGERR);
    }
}

/****************************************************************/
/*名称: FM1702Reset                                             */
/*功能: 复位FM1702                                              */
/*输入: N/A                                                     */
/*输出: 是否成功 0：成功                                        */
/****************************************************************/
unsigned char FM1702Reset(void)
{
	unsigned long CurTime , LastTime;
    unsigned char Temp;

    FM1702_Reset_H;
    delay(10);
    FM1702_Reset_L;
    delay(10);
    Temp=ReadReg(0x01);
	CurTime = HAL_GetTick();
	LastTime = CurTime;
    while(Temp!=0x00)
	{
		CurTime = HAL_GetTick();
		Temp=ReadReg(0x01);
		if(CurTime - LastTime > 1000) return 1; //FM1702复位请求超时
	}
    WriteReg(Page,0x80);//总线选择
    Temp=ReadReg(0x01);
    xlogd("%s temp:%d\n",__func__,Temp);
	CurTime = HAL_GetTick();
	LastTime = CurTime;
    while(Temp!=0x00)
	{
		CurTime = HAL_GetTick();
		Temp=ReadReg(0x01);
		if(CurTime - LastTime > 1000) return 1; //FM1702复位请求超时
	}
    WriteReg(Page,0x00);


    WriteReg(InterruptEn,0x7f);  //  禁止所有中断请求（最高位置0） 0x7f
    WriteReg(InterruptRq,0x7f);  // 禁止所有中断请求标识置0（最高位置0） 0x7f
    //WriteReg(0x28,0x80); // page 5
    //WriteReg(IRQPinConfig, 0x03);

    //设置调制器的输入源为内部编码器, 并且设置TX1和TX2
    WriteReg(TxControl, 0x5B); 		// 发送控制寄存器
    WriteReg(RxControl2, 0x01);
    WriteReg(RxWait, 5);


    FM1702LoadKey(CardKey);
    FM1702HaltCard();
	return 0;
}
unsigned char Request(void)
{
    WriteReg(TxControl,0x58);
    delay(10);
    WriteReg(TxControl,0x5b);
    WriteReg(CRCPresetLSB,0x63);		//
    WriteReg(CWConductance,0x3f);
    FM1702DataBuffer[0]=REQALL;
    WriteReg(BitFraming,0x07);          //发送7bit
    WriteReg(ChannelRedundancy,0x03);   //关闭CRC
    WriteReg(TxControl,0x5b);			//
    WriteReg(Control,0x01);	            //屏蔽CRYPTOL位
    Command_Send(1,Transceive);         //通过FM1702和ISO14443卡通讯

    //Read_FIFO();
    FM1702ReadFifo(FM1702DataBuffer);

    if(FM1702DataBuffer[0]==4)
    	return(MI_OK);
    else
   		return(MI_NOTHERE);
}
unsigned char Anticoll(void)
{
    unsigned char RetChar=0,i;
    WriteReg(CRCPresetLSB,0x63);			//
    WriteReg(CWConductance,0x3f);			//
    WriteReg(ModConductance,0x3f);			//
    WriteReg(ChannelRedundancy,0x03);       //关闭CRC，打开奇偶校验
    FM1702DataBuffer[0] = ANTICOLL;
    FM1702DataBuffer[1] = 0x20;
    Command_Send(2,Transceive);

    //Read_FIFO();
    FM1702ReadFifo(FM1702DataBuffer);

    for (i=0; i<5; i++)//发送防冲撞指令，接收卡片序列号4字节
    {
      CardSnr[i] = FM1702DataBuffer[i];
      RetChar = RetChar^CardSnr[i];
    }
    if(RetChar==0)
    return(MI_OK);
    else
    return(MI_ERR);
}

unsigned char Select(void)
{
    unsigned char i;
    WriteReg(CRCPresetLSB,0x63);			//
    WriteReg(CWConductance,0x3f);			//
    FM1702DataBuffer[0] = SELECT;
    FM1702DataBuffer[1] = 0x70;
    for (i=0; i<5; i++)
    {
      FM1702DataBuffer[i+2] = CardSnr[i];
    }
    WriteReg(ChannelRedundancy,0x0f);			//开CRC，打开奇偶校验
    Command_Send(7,Transceive);

    //Read_FIFO();
    FM1702ReadFifo(FM1702DataBuffer);

    if(FM1702DataBuffer[0]==0x08)
    return(MI_OK);
    else{
        dump_hex((char*)FM1702DataBuffer, 16);
    return(MI_ERR);
    }
}

/****************************************************************/
/*名称: ReadBlock                                         	 	*/
/*功能: 读取M1卡一块数据                                        */
/*输入: Block_Addr:块地址										*/
/*输出: 成功返回MI_OK                                           */
/*说明：返回数据存放FM1702DataBuffer[]中								*/
/****************************************************************/
unsigned char ReadBlock(unsigned char Block_Addr,unsigned char K)
{
    //K=1,判断数据完整性，K=0不判断数据完整性
    unsigned char i,XOR=0;
    WriteReg(CRCPresetLSB,0x63);
    WriteReg(CWConductance,0x3f);
    WriteReg(ModConductance,0x3f);
    WriteReg(ChannelRedundancy,0x0f);
    FM1702DataBuffer[0] = READ;                   	//Command
    FM1702DataBuffer[1] = Block_Addr;             	//块地址
    Command_Send(2,Transceive);	                    //通讯操作
    i = ReadReg(ErrorFlag);
    if((i&0x02)||(i&0x04)||(i&0x08))
    return(MI_ERR);

    //Read_FIFO();
    FM1702ReadFifo(FM1702DataBuffer);

    if(K==1)
    {
      for(i=1;i<15;i++)
      XOR+=FM1702DataBuffer[i];
      if((FM1702DataBuffer[0]!=0x68)||(FM1702DataBuffer[15]!=XOR))
      return(MI_ERR);
    }
    return(MI_OK);

}
/****************************************************************/
/*名称: WriteBlock                                          	*/
/*功能: 写数据到M1卡一块                                        */
/*输入: Block_Addr:块地址										*/
/*输出: 成功返回MI_OK                                           */
/*说明：写数据FM1702DataBuffer[]到指定块                        */
/****************************************************************/
unsigned char WriteBlock(unsigned char Block_Addr)
{
    unsigned char i,Temp[2];
    WriteReg(CRCPresetLSB,0x63);
    WriteReg(CWConductance,0x3f);
    WriteReg(ChannelRedundancy,0x07);
    Temp[0]=FM1702DataBuffer[0];
    Temp[1]=FM1702DataBuffer[1];
    FM1702DataBuffer[0]=WRITE;
    FM1702DataBuffer[1]=Block_Addr;
    Command_Send(2,Transceive);
    FM1702DataBuffer[0]=Temp[0];
    FM1702DataBuffer[1]=Temp[1];
    Command_Send(16,Transceive);
    i = ReadReg(ErrorFlag);
    if((i&0x02)||(i&0x04)||(i&0x08))
    return(MI_ERR);
    else
    return(MI_OK);
}

/****************************************************************/
/*名称: Value                                          			*/
/*功能: 扣款和充值	                                  	        */
/*输入: Mode：		0xC0 = 扣款;0xC1 = 充值						*/
/*	Block_Addr:	块地址         									*/
/*     Data0~Data3:   	4字节增(减)值，低位在前      			*/
/*输出: 成功返回MI_OK                                           */
/****************************************************************/
unsigned char Value(unsigned char Mode,unsigned char Block_Addr,
                      unsigned char Data0,unsigned char Data1,
                      unsigned char Data2,unsigned char Data3)
{
    unsigned char Temp;
    WriteReg(0x23,0x63);
    WriteReg(0x12,0x3f);
    FM1702DataBuffer[0]=Mode;
    FM1702DataBuffer[1]=Block_Addr;
    WriteReg(0x22,0x07);
    Command_Send(2, Transceive);
    Temp = ReadReg(FIFOLength);
    if(Temp>0)
    {

      //Read_FIFO();
      FM1702ReadFifo(FM1702DataBuffer);

      if(FM1702DataBuffer[0]==0x0A)
      {
        FM1702DataBuffer[0]=Data0;
        FM1702DataBuffer[1]=Data1;
        FM1702DataBuffer[2]=Data2;
        FM1702DataBuffer[3]=Data3;
        Command_Send(4, Transmit);
        FM1702DataBuffer[0] = TRANSFER;
        FM1702DataBuffer[1] = Block_Addr;
        Command_Send(2, Transceive);
        Temp = ReadReg(FIFOLength);
        if(Temp>0)
        {
          //Read_FIFO();
          FM1702ReadFifo(FM1702DataBuffer);
          if(FM1702DataBuffer[0]==0x0A)
          return(MI_OK);
        }
      }
    }
    return MI_ERR;
}

//=======================================================
//	名称: FM1702Request
//	功能: 	该函数实现对放入FM1702操作范围之内的卡片的Request操作
//			即：请求M1卡的复位响应
//	输入:	mode: ALL(监测所以FM1702操作范围之内的卡片)
//			STD(监测在FM1702操作范围之内处于HALT状态的卡片)
//			Comm_Set, 命令码：指FM1702发给IC卡的命令
//	输出: 	FM1702_NOTAGERR: 无卡
//			FM1702_OK: 应答正确
//			FM1702_REQERR: 应答错误
//=======================================================
unsigned char FM1702Request(unsigned char mode)
{

    unsigned char temp;

    //选择TX1、TX2的发射天线阻抗
    WriteReg(CWConductance, 0x3f);
    //选择数据校验种类和模式
    WriteReg(ChannelRedundancy, 0x03);
    //调整面向bit的格式
    WriteReg(BitFraming, 0x07);
    gBuff[0] = mode;		//Request模式选择
    temp = ReadReg(Control);
    temp = temp & (0xf7);
    WriteReg(Control, temp);			//Control reset value is 00
    temp = FM1702Command(Transceive, gBuff, 1);   //发送接收命令
    if (temp == 0)
    {
        return FM1702_NOTAGERR;
    }

    temp = FM1702ReadFifo(gBuff);		//从FIFO中读取应答信息到gBuff中
    // 判断应答信号是否正确
    //2  	Mifare Pro 卡
    //4 	Mifare One 卡
    if ((gBuff[0] == 0x04) & (gBuff[1] == 0x0) & (temp == 2))
    {
        return FM1702_OK;
    }
    return FM1702_REQERR;
}

//=======================================================
//	名称: Authentication
//	功能: 	该函数实现密码认证的过程
//	输入:	UID: 卡片序列号地址
//			SecNR: 扇区号
//			mode: 模式
//	输出: 	FM1702_NOTAGERR: 无卡
//			FM1702_OK: 应答正确
//			FM1702_PARITYERR: 奇偶校验错
//			FM1702_CRCERR: CRC校验错
//			FM1702_AUTHERR: 权威认证有错
//=======================================================
unsigned char FM1702Authentication(unsigned char *UID, unsigned char SecNR, unsigned char mode)
{
    unsigned char i;
    unsigned char temp, temp1;

    if (SecNR >= 16)
        SecNR = SecNR % 16;

    //选择数据校验种类和模式
    WriteReg(ChannelRedundancy, 0x0F);
    gBuff[0] = mode;
    gBuff[1] = SecNR * 4 + 3;
    for (i = 0; i < 4; i++)
    {
        gBuff[2 + i] = UID[i];
    }

    temp = FM1702Command(Authent1, gBuff, 6);
    if (temp == 0)
    {
        return 0x99;
    }

    temp = ReadReg(ErrorFlag);
    if ((temp & 0x02) == 0x02) return FM1702_PARITYERR;
    if ((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
    if ((temp & 0x08) == 0x08) return FM1702_CRCERR;
    temp = FM1702Command(Authent2, gBuff, 0);
    if (temp == 0)
    {
        return 0x88;
    }

    temp = ReadReg(ErrorFlag);
    //	Show(temp,0);
    if ((temp & 0x02) == 0x02) return FM1702_PARITYERR;
    if ((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
    if ((temp & 0x08) == 0x08) return FM1702_CRCERR;
    temp1 = ReadReg(Control);
    temp1 = temp1 & 0x08;
    if (temp1 == 0x08)
    {
        return FM1702_OK;
    }

    return FM1702_AUTHERR;
}

uint32_t BrushCard(void)
{
    uint8_t ret;
    uint32_t card_id = 0;
    //FM1702LoadKey(CardKey);
    //FM1702HaltCard();
    #if 0
    if(FM1702Request(RF_CMD_REQUEST_STD) == FM1702_OK
        && Anticoll() == FM1702_OK
        && Select() == FM1702_OK
        && FM1702Authentication(CardSnr, 1, 0x60) == FM1702_OK
      ){
      dump_hex((char *)CardSnr,5);
    }
    #else
    ret = FM1702Request(RF_CMD_REQUEST_STD);
    if(ret == FM1702_OK){
        ret = Anticoll();
        if(ret == FM1702_OK){
            ret = Select();
            if(ret == FM1702_OK){
                memcpy(&card_id, CardSnr, 4);
                dump_hex((char *)CardSnr,5);
            }
        }
    }

    #endif
    return card_id;
}
