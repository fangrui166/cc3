#ifndef FM1702_H
#define FM1702_H


#include "cmsis_os.h"

/*********************FM1702通讯时返回的错误代码*********************/
#define MI_OK			0
#define MI_NOTHERE		1
#define MI_ERR			2

//;==============================================
//;函数错误代码定义
//;==============================================
#define FM1702_OK			0		// 正确
#define FM1702_NOTAGERR		1		// 无卡
#define FM1702_CRCERR		2		// 卡片CRC校验错误
#define FM1702_EMPTY		3		// 数值溢出错误
#define FM1702_AUTHERR		4		// 验证不成功
#define FM1702_PARITYERR    5		// 卡片奇偶校验错误
#define FM1702_CODEERR		6		// 通讯错误(BCC校验错)
#define FM1702_SERNRERR		8		// 卡片序列号错误(anti-collision 错误)
#define FM1702_SELECTERR    9		// 卡片数据长度字节错误(SELECT错误)
#define FM1702_NOTAUTHERR	10		// 0x0A 卡片没有通过验证
#define FM1702_BITCOUNTERR	11		// 从卡片接收到的位数错误
#define FM1702_BYTECOUNTERR	12		// 从卡片接收到的字节数错误仅读函数有效
#define FM1702_RESTERR		13		// 调用restore函数出错
#define FM1702_TRANSERR		14		// 调用transfer函数出错
#define FM1702_WRITEERR		15		// 0x0F 调用write函数出错
#define FM1702_INCRERR		16		// 0x10 调用increment函数出错
#define FM1702_DECRERR		17      // 0x11 调用decrement函数出错
#define FM1702_READERR		18      // 0x12 调用read函数出错
#define FM1702_LOADKEYERR	19      // 0x13 调用LOADKEY函数出错
#define FM1702_FRAMINGERR	20      // 0x14 FM1702帧错误
#define FM1702_REQERR		21      // 0x15 调用req函数出错
#define FM1702_SELERR		22      // 0x16 调用sel函数出错
#define FM1702_ANTICOLLERR	23      // 0x17 调用anticoll函数出错
#define FM1702_INTIVALERR	24      // 0x18 调用初始化函数出错
#define FM1702_READVALERR	25      // 0x19 调用高级读块值函数出错
#define FM1702_DESELECTERR	26      // 0x1A
#define FM1702_CMD_ERR		42      // 0x2A 命令错误

/************************FM1702 FIFO长度定义*************************/
#define DEF_FIFO_LENGTH		64          //FIFO size=64byte
#define MAXRLEN				18 			//数据长度18字节
/****************************端口定义*******************************/

/****************************FM1702命令字***************************/
#define Idle			0x00			//取消当前命令
#define Transmit		0x1A			//发送命令
#define Receive			0x16			//接收数据
#define Transceive		0x1E			//发送并接收数据
#define WriteE2			0x01			//写FM1702EEPROM
#define ReadE2			0x03			//读FM1702EEPROM
#define LoadKeyE2		0x0B			//将密钥从EEPROM复制到KEY缓存
#define LoadKey			0x19			//将密钥从FIFO复制到KEY缓存
#define Authent1		0x0C			//认证过程1
#define Authent2		0x14			//认证过程2
/*************************Mifare_One卡片命令字**********************/
#define REQIDL			0x26               	//寻天线区内未进入休眠状态
#define REQALL			0x52               	//寻天线区内全部卡
#define ANTICOLL		0x93               	//防冲撞
#define SELECT			0x93               	//选择卡片
#define AUTHENTA		0x60               	//验证A密钥
#define AUTHENTB		0x61               	//验证B密钥
#define READ			0x30               	//读块
#define WRITE			0xA0               	//写块
#define DECREMENT		0xC0               	//扣款
#define INCREMENT		0xC1               	//充值
#define PRESTORE		0xC2               	//调块数据到缓冲区
#define TRANSFER		0xB0               	//保存缓冲区中数据
#define HALT			0x50               	//休眠
/*************************FM1702寄存器定义**********************/
#define  Page 			0x00			//选择寄存器组
#define  Command		0x01			//指令寄存器
#define  FIFOData 		0x02			//64byte FIFO 的输入输出寄存器
#define  PrimaryStatus 		0x03			//发射器接收器及FIFO 的标识位寄存器
#define  FIFOLength 		0x04			//当前FIFO 内byte数
#define  SecondaryStatus 	0x05			//各种状态标识寄存器
#define  InterruptEn 		0x06			//中断使能/禁止控制寄存器
#define  InterruptRq 		0x07			//中断请求标识寄存器
#define  Control 		0x09			//各种控制标识寄存器
#define  ErrorFlag 		0x0A			//上一条指令结束后错误标识
#define  CollPos 		0x0B			//侦测到的第一个冲突位的位置
#define  TimerValue 	0x0C			//当前Timer值
#define  CRCResultLSB  	0x0D			//协处理器低8位
#define  CRCResultMSB  	0x0E			//协处理器高8位
#define  BitFraming 	0x0F			//调整面向bit的帧格式
#define  TxControl 		0x11 			//发射器控制寄存器
#define  CWConductance 	0x12 			//选择发射脚TX1和TX2发射天线的阻抗
#define  ModConductance 0x13 			//预设寄存器不要改变内容
#define  PreSet14 		0x14			//预设寄存器不要改变内容
#define  ModWidth 		0x15			//选择载波调制宽度
#define  PreSet16 		0x16			//预设寄存器不要改变内容
#define  PreSet17 		0x17			//预设寄存器不要改变内容
#define  RXControl1 		0x19			//接收器控制寄存器
#define  DecoderControl 	0x1A			//解码控制寄存器
#define  BitPhase 			0x1B			//调整发射器和接收器时钟相差
#define  Rxthreshold 		0x1C			//选择bit解码的阈值
#define  PreSet1D 			0x1D			//预设寄存器不要改变内容
#define  RxControl2 		0x1E			//解码控制及选择接收源
#define  ClockQControl 		0x1F			//时钟产生控制寄存器
#define  RxWait 			0x21			//选择发射和接收之间的时间间隔
#define  ChannelRedundancy 	0x22			//选择数据校验种类和模式
#define  CRCPresetLSB  	    0x23			//预置寄存器低8位
#define  CRCPresetMSB  	    0x24			//预置寄存器高8位
#define  PreSet25 		0x25			//预设寄存器不要改变内容
#define  MFOUTSelect 	0x26			//选择MFOUT 信号源
#define  PreSet27 		0x27			//预设寄存器不要改变内容
#define  FIFOLevel 		0x29			//定义FIFO 溢出级别
#define  TimerClock 	0x2A			//选择Timer时钟的分频
#define  TimerControl 	0x2B			//选择Timer启动/停止条件
#define  TimerReload 	0x2C			//Timer预置值
#define  IRQPinConfig 	0x2D			//IRQ 输
#define  TypeSH 		0x31			//上海标准选择寄存器
#define  TestDigiSelect 0x3D			//测试管脚配置寄存器

//;==============================================
//;射频卡通信命令码定义
//;==============================================
#define RF_CMD_REQUEST_STD	0x26
#define RF_CMD_REQUEST_ALL	0x52
#define RF_CMD_ANTICOL		0x93
#define RF_CMD_SELECT		0x93
#define RF_CMD_AUTH_LA		0x60
#define RF_CMD_AUTH_LB		0x61
#define RF_CMD_READ         0x30
#define RF_CMD_WRITE		0xa0
#define RF_CMD_INC		    0xc1
#define RF_CMD_DEC		    0xc0
#define RF_CMD_RESTORE		0xc2
#define RF_CMD_TRANSFER		0xb0
#define RF_CMD_HALT		    0x50

void Clear_FIFO(void);
void Write_FIFO(unsigned char Number);
void Read_FIFO(void);
void Command_Send(unsigned char Number,unsigned char Comm_Code);
unsigned char FM1702Reset(void);
unsigned char Request(void);
unsigned char Anticoll(void);
unsigned char Select(void);
unsigned char Authentication(unsigned char Auth_mode,unsigned char SecNr);
unsigned char ReadBlock(unsigned char Block_Addr,unsigned char K);
unsigned char WriteBlock(unsigned char Block_Addr);
unsigned char Value(unsigned char Mode,unsigned char Block_Addr,unsigned char Data0,unsigned char Data1,unsigned char Data2,unsigned char Data3);
uint32_t BrushCard(void);

#endif
