/**
  ******************************************************************************
  * @file    transport.c
  * $Author: �ɺ�̤ѩ $
  * $Revision: 17 $
  * $Date:: 2014-10-25 11:16:48 +0800 #$
  * @brief   ������.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, EmbedNet</center>
  *<center><a href="http:\\www.embed-net.com">http://www.embed-net.com</a></center>
  *<center>All Rights Reserved</center></h3>
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "MQTTPacket.h"
#include "common.h"
#include "transport.h"
#include "sim900a.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  ͨ��TCP��ʽ�������ݵ�TCP������
  * @param  buf �����׵�ַ
  * @param  buflen ���ݳ���
  * @retval С��0��ʾ����ʧ��
  */
int transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
    xlogi("%s %s\n",__func__,buf);
  return (int)sim900a_send_data(buf,buflen);
}
/**
  * @brief  ������ʽ����TCP���������͵�����
  * @param  buf ���ݴ洢�׵�ַ
  * @param  count ���ݻ���������
  * @retval С��0��ʾ��������ʧ��
  */
int transport_getdata(unsigned char* buf, int count)
{
    memcpy(buf,"abcdefgh\r\n",10);
  return 5;
}


/**
  * @brief  ��һ��socket�����ӵ�������
  * @param  ��
  * @retval С��0��ʾ��ʧ��
  */
int transport_open(void)
{
    #if 0
  int32_t ret;
  //�½�һ��Socket���󶨱��ض˿�5000
  ret = socket(SOCK_TCPS,Sn_MR_TCP,5000,0x00);
  if(ret != SOCK_TCPS){
    printf("%d:Socket Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Opened\r\n",SOCK_TCPS);
  }

  //����TCP������
  ret = connect(SOCK_TCPS,domain_ip,1883);//�˿ڱ���Ϊ1883
  if(ret != SOCK_OK){
    printf("%d:Socket Connect Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Connected\r\n",SOCK_TCPS);
  }
  #endif
	return 0;
}
/**
  * @brief  �ر�socket
  * @param  ��
  * @retval С��0��ʾ�ر�ʧ��
  */
int transport_close(void)
{
  //close(SOCK_TCPS);
  return 0;
}
