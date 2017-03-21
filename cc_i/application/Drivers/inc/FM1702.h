#ifndef FM1702_H
#define FM1702_H


#include "cmsis_os.h"

/*********************FM1702ͨѶʱ���صĴ������*********************/
#define MI_OK			0
#define MI_NOTHERE		1
#define MI_ERR			2

//;==============================================
//;����������붨��
//;==============================================
#define FM1702_OK			0		// ��ȷ
#define FM1702_NOTAGERR		1		// �޿�
#define FM1702_CRCERR		2		// ��ƬCRCУ�����
#define FM1702_EMPTY		3		// ��ֵ�������
#define FM1702_AUTHERR		4		// ��֤���ɹ�
#define FM1702_PARITYERR    5		// ��Ƭ��żУ�����
#define FM1702_CODEERR		6		// ͨѶ����(BCCУ���)
#define FM1702_SERNRERR		8		// ��Ƭ���кŴ���(anti-collision ����)
#define FM1702_SELECTERR    9		// ��Ƭ���ݳ����ֽڴ���(SELECT����)
#define FM1702_NOTAUTHERR	10		// 0x0A ��Ƭû��ͨ����֤
#define FM1702_BITCOUNTERR	11		// �ӿ�Ƭ���յ���λ������
#define FM1702_BYTECOUNTERR	12		// �ӿ�Ƭ���յ����ֽ����������������Ч
#define FM1702_RESTERR		13		// ����restore��������
#define FM1702_TRANSERR		14		// ����transfer��������
#define FM1702_WRITEERR		15		// 0x0F ����write��������
#define FM1702_INCRERR		16		// 0x10 ����increment��������
#define FM1702_DECRERR		17      // 0x11 ����decrement��������
#define FM1702_READERR		18      // 0x12 ����read��������
#define FM1702_LOADKEYERR	19      // 0x13 ����LOADKEY��������
#define FM1702_FRAMINGERR	20      // 0x14 FM1702֡����
#define FM1702_REQERR		21      // 0x15 ����req��������
#define FM1702_SELERR		22      // 0x16 ����sel��������
#define FM1702_ANTICOLLERR	23      // 0x17 ����anticoll��������
#define FM1702_INTIVALERR	24      // 0x18 ���ó�ʼ����������
#define FM1702_READVALERR	25      // 0x19 ���ø߼�����ֵ��������
#define FM1702_DESELECTERR	26      // 0x1A
#define FM1702_CMD_ERR		42      // 0x2A �������

/************************FM1702 FIFO���ȶ���*************************/
#define DEF_FIFO_LENGTH		64          //FIFO size=64byte
#define MAXRLEN				18 			//���ݳ���18�ֽ�
/****************************�˿ڶ���*******************************/

/****************************FM1702������***************************/
#define Idle			0x00			//ȡ����ǰ����
#define Transmit		0x1A			//��������
#define Receive			0x16			//��������
#define Transceive		0x1E			//���Ͳ���������
#define WriteE2			0x01			//дFM1702EEPROM
#define ReadE2			0x03			//��FM1702EEPROM
#define LoadKeyE2		0x0B			//����Կ��EEPROM���Ƶ�KEY����
#define LoadKey			0x19			//����Կ��FIFO���Ƶ�KEY����
#define Authent1		0x0C			//��֤����1
#define Authent2		0x14			//��֤����2
/*************************Mifare_One��Ƭ������**********************/
#define REQIDL			0x26               	//Ѱ��������δ��������״̬
#define REQALL			0x52               	//Ѱ��������ȫ����
#define ANTICOLL		0x93               	//����ײ
#define SELECT			0x93               	//ѡ��Ƭ
#define AUTHENTA		0x60               	//��֤A��Կ
#define AUTHENTB		0x61               	//��֤B��Կ
#define READ			0x30               	//����
#define WRITE			0xA0               	//д��
#define DECREMENT		0xC0               	//�ۿ�
#define INCREMENT		0xC1               	//��ֵ
#define PRESTORE		0xC2               	//�������ݵ�������
#define TRANSFER		0xB0               	//���滺����������
#define HALT			0x50               	//����
/*************************FM1702�Ĵ�������**********************/
#define  Page 			0x00			//ѡ��Ĵ�����
#define  Command		0x01			//ָ��Ĵ���
#define  FIFOData 		0x02			//64byte FIFO ����������Ĵ���
#define  PrimaryStatus 		0x03			//��������������FIFO �ı�ʶλ�Ĵ���
#define  FIFOLength 		0x04			//��ǰFIFO ��byte��
#define  SecondaryStatus 	0x05			//����״̬��ʶ�Ĵ���
#define  InterruptEn 		0x06			//�ж�ʹ��/��ֹ���ƼĴ���
#define  InterruptRq 		0x07			//�ж������ʶ�Ĵ���
#define  Control 		0x09			//���ֿ��Ʊ�ʶ�Ĵ���
#define  ErrorFlag 		0x0A			//��һ��ָ�����������ʶ
#define  CollPos 		0x0B			//��⵽�ĵ�һ����ͻλ��λ��
#define  TimerValue 	0x0C			//��ǰTimerֵ
#define  CRCResultLSB  	0x0D			//Э��������8λ
#define  CRCResultMSB  	0x0E			//Э��������8λ
#define  BitFraming 	0x0F			//��������bit��֡��ʽ
#define  TxControl 		0x11 			//���������ƼĴ���
#define  CWConductance 	0x12 			//ѡ�����TX1��TX2�������ߵ��迹
#define  ModConductance 0x13 			//Ԥ��Ĵ�����Ҫ�ı�����
#define  PreSet14 		0x14			//Ԥ��Ĵ�����Ҫ�ı�����
#define  ModWidth 		0x15			//ѡ���ز����ƿ��
#define  PreSet16 		0x16			//Ԥ��Ĵ�����Ҫ�ı�����
#define  PreSet17 		0x17			//Ԥ��Ĵ�����Ҫ�ı�����
#define  RXControl1 		0x19			//���������ƼĴ���
#define  DecoderControl 	0x1A			//������ƼĴ���
#define  BitPhase 			0x1B			//�����������ͽ�����ʱ�����
#define  Rxthreshold 		0x1C			//ѡ��bit�������ֵ
#define  PreSet1D 			0x1D			//Ԥ��Ĵ�����Ҫ�ı�����
#define  RxControl2 		0x1E			//������Ƽ�ѡ�����Դ
#define  ClockQControl 		0x1F			//ʱ�Ӳ������ƼĴ���
#define  RxWait 			0x21			//ѡ����ͽ���֮���ʱ����
#define  ChannelRedundancy 	0x22			//ѡ������У�������ģʽ
#define  CRCPresetLSB  	    0x23			//Ԥ�üĴ�����8λ
#define  CRCPresetMSB  	    0x24			//Ԥ�üĴ�����8λ
#define  PreSet25 		0x25			//Ԥ��Ĵ�����Ҫ�ı�����
#define  MFOUTSelect 	0x26			//ѡ��MFOUT �ź�Դ
#define  PreSet27 		0x27			//Ԥ��Ĵ�����Ҫ�ı�����
#define  FIFOLevel 		0x29			//����FIFO �������
#define  TimerClock 	0x2A			//ѡ��Timerʱ�ӵķ�Ƶ
#define  TimerControl 	0x2B			//ѡ��Timer����/ֹͣ����
#define  TimerReload 	0x2C			//TimerԤ��ֵ
#define  IRQPinConfig 	0x2D			//IRQ ��
#define  TypeSH 		0x31			//�Ϻ���׼ѡ��Ĵ���
#define  TestDigiSelect 0x3D			//���Թܽ����üĴ���

//;==============================================
//;��Ƶ��ͨ�������붨��
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
