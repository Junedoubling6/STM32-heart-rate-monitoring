#ifndef __GPIO_INIT_H
#define __GPIO_INIT_H
#include "sys.h"

//�궨��
#define BEEP PBout(15)			//�����˷�����������
#define LED  PBout(12)			//������led�Ƶ�����
#define KEY1 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)			//KEY1������������
#define KEY2 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10)			//KEY2������������

//��������
void LED_Init(void);			//LED���ų�ʼ��
void BEEP_Init(void);			//���������ų�ʼ��
void LED_ON(void);				//��LED
void LED_OFF(void);				//�ر�LED
void BEEP_ON(void);				//�򿪷�����
void BEEP_OFF(void);			//�رշ�����
void KEY_Init(void);			//�������ų�ʼ��

#endif
