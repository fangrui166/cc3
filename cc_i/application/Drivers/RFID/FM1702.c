#include "FM1702.h"
#include "fm1702_drv.h"
#include "common.h"


unsigned char 	FM1702DataBuffer[16];	        //���պͷ������ݻ�����
unsigned char gBuff[16];

static unsigned char 	CardSnr[5];  	        //��Ƭ���к�
//static unsigned char    Random[4];              //0���� ��2�µ������
static unsigned char 	CardKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //Ĭ�Ͽ�Ƭ����
const unsigned char 	UserKey[6] = {0x08, 0x02, 0x01, 0x00, 0x01, 0x07}; //��Ƭ����
const unsigned char 	CompanyKey[6] = {'b', 'x', 'i', 'a', 'n', 'g'}; //��˾��Ƭ����
//************************************��ʱ����**************************************
//��������: void delay(uint32_t nCount)
//��    ������ʱ
//��ڲ�����nCount ����ʱ�����У�ѭ���Ĵ���
//���ڲ�������
//��    ע��Editor��Armink 2011-03-18    Company: BXXJS
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
/*����: Send_1_Char                                             */
/*����: ��FM1702����1���ֽ�                                      */
/*����: Data-���͵�ֵ                                           */
/*���: N/A                                                     */
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
/*����: Rec_1_Char                                              */
/*����: ����FM1702���͵�1���ֽ�                                  */
/*����: N/A                                                     */
/*���: ������ֵ                                                */
/****************************************************************/
unsigned char  Rec_1_Char(void)
{
    unsigned char i,Temp=0;
    for (i = 0; i < 8; i++)
     {//����FM1702����1���ֽ�
        Temp <<= 1;
        Temp |=FM1702_MISO;
        FM1702_SCK_H;
        delay(10);
        FM1702_SCK_L;
     }
    return Temp;
}
/****************************************************************/
/*����: ReadReg                                               */
/*����: ��FM1702�Ĵ���                                           */
/*����: Address-�Ĵ�����ַ                                      */
/*���: ������ֵ                                                */
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
/*����: WriteReg                                              */
/*����: дFM1702�Ĵ���                                           */
/*����: Address - �Ĵ�����ַ; value - д���ֵ                  */
/*���: N/A                                                     */
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
/*����: Clear_FIFO                                              */
/*����: ���FIFO����                                            */
/*����: N/A                                                     */
/*���: N/A                                                     */
/****************************************************************/
void Clear_FIFO(void)
{
    unsigned char i=0,Temp;
    Temp =ReadReg(Control);						//���FIFO
    Temp = (Temp | 0x01);
    WriteReg(Control, Temp);
    do{
	if(ReadReg(FIFOLength)==0)
	break;
    }while(i++<200);
}
/****************************************************************/
/*����: Write_FIFO                                              */
/*����: дFIFO����                                            */
/*����: N/A                                                     */
/*���: N/A                                                     */
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
//���ƣ�FM1702Write_FIFO
//���ܣ�������д��FM1702��FIFO
//����:	buff��Ҫд���������Ram�е��׵�ַ
//		count��	Ҫд����ֽ���
//���:
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
/*����: Read_FIFO                                               */
/*����: ��FIFO����                                              */
/*����: N/A                                                     */
/*���: N/A                                                     */
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
//	����: ReadFIFO
//	����: �ú���ʵ�ִ�FM1702��FIFO�ж���x bytes����
//	����:	buff, ָ��������ݵ�ָ��
//	���: 	���ݳ��ȣ���λ���ֽڣ�
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
/*����: Command_Send                                            */
/*����: ͨ��FM1702��ISO14443��ͨѶ                              */
/*����: Comm_Code-�����룻Number-���ݸ���						*/
/*���: N/A                                                     */
/****************************************************************/
void Command_Send(unsigned char Number,unsigned char Comm_Code)
{
    unsigned char i=0,RetChar;
    WriteReg(Command,Idle);			//������ǰָ��
    Clear_FIFO();    		                //���FIFO
    Write_FIFO(Number);
    ReadReg(FIFOLength);
    WriteReg(Command, Comm_Code);		//����������
    do
    {
        delay(10);
        RetChar=ReadReg(Command);
    	if(RetChar==0)
    	break;
    }while(i++<10);
}
//=======================================================
//	����: drv_fm1702Command
//	����: �ú���ʵ����FM1702��������Ĺ���
//	����:	count, ����������ĳ���
//			buff, ָ����������ݵ�ָ��
//			Comm_Set, �����룺ָFM1702����IC��������
//	���: 	TRUE, �����ȷִ��
//			FALSE, ����ִ�д���
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
//	����: drv_fm1702HaltCard
//	����: �ú���ʵ����ͣMIFARE��
//	����:	N/A
//	���: 	FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_FRAMINGERR:FM1702֡����
//			FM1702_CRCERR: CRCУ���
//			FM1702_NOTAGERR: �޿�
//=======================================================
unsigned char FM1702HaltCard(void)
{
    unsigned char	temp;
    //ѡ��TX1��TX2�ķ��������迹
    WriteReg(CWConductance, 0x3f);
    //ѡ������У�������ģʽ
    WriteReg(ChannelRedundancy, 0x03);
    *gBuff = RF_CMD_HALT;
    *(gBuff + 1) = 0x00;
    temp = FM1702Command(Transmit, gBuff, 2);//����FIFO�����ַ
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
/*����: FM1702Reset                                             */
/*����: ��λFM1702                                              */
/*����: N/A                                                     */
/*���: �Ƿ�ɹ� 0���ɹ�                                        */
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
		if(CurTime - LastTime > 1000) return 1; //FM1702��λ����ʱ
	}
    WriteReg(Page,0x80);//����ѡ��
    Temp=ReadReg(0x01);
    xlogd("%s temp:%d\n",__func__,Temp);
	CurTime = HAL_GetTick();
	LastTime = CurTime;
    while(Temp!=0x00)
	{
		CurTime = HAL_GetTick();
		Temp=ReadReg(0x01);
		if(CurTime - LastTime > 1000) return 1; //FM1702��λ����ʱ
	}
    WriteReg(Page,0x00);


    WriteReg(InterruptEn,0x7f);  //  ��ֹ�����ж��������λ��0�� 0x7f
    WriteReg(InterruptRq,0x7f);  // ��ֹ�����ж������ʶ��0�����λ��0�� 0x7f
    //WriteReg(0x28,0x80); // page 5
    //WriteReg(IRQPinConfig, 0x03);

    //���õ�����������ԴΪ�ڲ�������, ��������TX1��TX2
    WriteReg(TxControl, 0x5B); 		// ���Ϳ��ƼĴ���
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
    WriteReg(BitFraming,0x07);          //����7bit
    WriteReg(ChannelRedundancy,0x03);   //�ر�CRC
    WriteReg(TxControl,0x5b);			//
    WriteReg(Control,0x01);	            //����CRYPTOLλ
    Command_Send(1,Transceive);         //ͨ��FM1702��ISO14443��ͨѶ

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
    WriteReg(ChannelRedundancy,0x03);       //�ر�CRC������żУ��
    FM1702DataBuffer[0] = ANTICOLL;
    FM1702DataBuffer[1] = 0x20;
    Command_Send(2,Transceive);

    //Read_FIFO();
    FM1702ReadFifo(FM1702DataBuffer);

    for (i=0; i<5; i++)//���ͷ���ײָ����տ�Ƭ���к�4�ֽ�
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
    WriteReg(ChannelRedundancy,0x0f);			//��CRC������żУ��
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
/*����: ReadBlock                                         	 	*/
/*����: ��ȡM1��һ������                                        */
/*����: Block_Addr:���ַ										*/
/*���: �ɹ�����MI_OK                                           */
/*˵�����������ݴ��FM1702DataBuffer[]��								*/
/****************************************************************/
unsigned char ReadBlock(unsigned char Block_Addr,unsigned char K)
{
    //K=1,�ж����������ԣ�K=0���ж�����������
    unsigned char i,XOR=0;
    WriteReg(CRCPresetLSB,0x63);
    WriteReg(CWConductance,0x3f);
    WriteReg(ModConductance,0x3f);
    WriteReg(ChannelRedundancy,0x0f);
    FM1702DataBuffer[0] = READ;                   	//Command
    FM1702DataBuffer[1] = Block_Addr;             	//���ַ
    Command_Send(2,Transceive);	                    //ͨѶ����
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
/*����: WriteBlock                                          	*/
/*����: д���ݵ�M1��һ��                                        */
/*����: Block_Addr:���ַ										*/
/*���: �ɹ�����MI_OK                                           */
/*˵����д����FM1702DataBuffer[]��ָ����                        */
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
/*����: Value                                          			*/
/*����: �ۿ�ͳ�ֵ	                                  	        */
/*����: Mode��		0xC0 = �ۿ�;0xC1 = ��ֵ						*/
/*	Block_Addr:	���ַ         									*/
/*     Data0~Data3:   	4�ֽ���(��)ֵ����λ��ǰ      			*/
/*���: �ɹ�����MI_OK                                           */
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
//	����: FM1702Request
//	����: 	�ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ��Request����
//			��������M1���ĸ�λ��Ӧ
//	����:	mode: ALL(�������FM1702������Χ֮�ڵĿ�Ƭ)
//			STD(�����FM1702������Χ֮�ڴ���HALT״̬�Ŀ�Ƭ)
//			Comm_Set, �����룺ָFM1702����IC��������
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_REQERR: Ӧ�����
//=======================================================
unsigned char FM1702Request(unsigned char mode)
{

    unsigned char temp;

    //ѡ��TX1��TX2�ķ��������迹
    WriteReg(CWConductance, 0x3f);
    //ѡ������У�������ģʽ
    WriteReg(ChannelRedundancy, 0x03);
    //��������bit�ĸ�ʽ
    WriteReg(BitFraming, 0x07);
    gBuff[0] = mode;		//Requestģʽѡ��
    temp = ReadReg(Control);
    temp = temp & (0xf7);
    WriteReg(Control, temp);			//Control reset value is 00
    temp = FM1702Command(Transceive, gBuff, 1);   //���ͽ�������
    if (temp == 0)
    {
        return FM1702_NOTAGERR;
    }

    temp = FM1702ReadFifo(gBuff);		//��FIFO�ж�ȡӦ����Ϣ��gBuff��
    // �ж�Ӧ���ź��Ƿ���ȷ
    //2  	Mifare Pro ��
    //4 	Mifare One ��
    if ((gBuff[0] == 0x04) & (gBuff[1] == 0x0) & (temp == 2))
    {
        return FM1702_OK;
    }
    return FM1702_REQERR;
}

//=======================================================
//	����: Authentication
//	����: 	�ú���ʵ��������֤�Ĺ���
//	����:	UID: ��Ƭ���кŵ�ַ
//			SecNR: ������
//			mode: ģʽ
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_AUTHERR: Ȩ����֤�д�
//=======================================================
unsigned char FM1702Authentication(unsigned char *UID, unsigned char SecNR, unsigned char mode)
{
    unsigned char i;
    unsigned char temp, temp1;

    if (SecNR >= 16)
        SecNR = SecNR % 16;

    //ѡ������У�������ģʽ
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
