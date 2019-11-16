//ͷ�ļ�����
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "oled.h"
#include "myiic.h"
#include "adc.h"
#include "timer.h"
#include "gpio_init.h"

//�궨��
#define	TIMER		2				//��ʱ����ʱʱ��2ms
#define true 		1				//��
#define false 		0				//��
#define HEART_MAX	100				//��������
#define HEART_MIN	60				//��������
#define HEART_MAX_ERROR		160		//���ʵĲ��ɵ�ֵ��������ֵ��ʾ����������
#define HEART_MIN_ERROR		40		//���ʵĲ��ɵ�ֵ�����ڴ�ֵ��ʾ����������

//��������
void ADC_deal(void);				//ģ��ת������
void OLED_Main_display(void);		//OLED����ʾ����
void OLED_Value_display(void);		//OLEDֵ��ʾ����
void OLED_Waveform_display(void);	//OLEDֵ��ʾ����
void KEY_deal(void);				//����������
void ALARM_deal(void);				//����������
void HeartRate_deal(void);			//����ֵ���㴦����
void Waveform_deal(void);			//���δ�����
int myabs(int a);					//ȡ����ֵ����

//��������
float adcx;							//�ɼ��������ʵ�ԭʼADCֵ��ÿ���ַŵ��������ϵ�ʱ������һ��ʱ���0
u8 display_flag = 0;				//��ʾ�����־  0--��ʾ������	1--��ʾ���ʽ���
u8 alarm = 0;						//����������	0--������		1--����
u8 alarm_en = 0;					//����ʹ�ܱ�־λ�����ַ���֮���ʹ��

u8 waveform[128] = {0};				//���βɼ����飬�ɼ�128���㣬OLED�Ŀ��Ϊ128������
u8 waveform_copy[128] = {0};		//���βɼ����飬�ɼ�128���㣬OLED�Ŀ��Ϊ128������
u8 waveform_flag = 0;				//���β���ʱ�����������128��֮���һ������ʾ����

//���ʲɼ���ر���
int BPM;                   			//������==��������
int Signal;                			//�����ԭʼ���ݡ�
int IBI = 600;             			//���ļ�������ν���֮���ʱ�䣨ms�������㣺60/IBI(s)=���ʣ�BPM��
unsigned char Pulse = false;     	//����ߵͱ�־����������ʱΪ�棬��ʱΪ�١�
unsigned char QS = false;        	//������һ������ʱ���ͱ������ʵ��
int rate[10];                    	//���鱣�����10��IBIֵ��
unsigned long sampleCounter = 0;    //����ȷ�����嶨ʱ��
unsigned long lastBeatTime = 0;     //���ڲ���IBI
int P =512;                      	//���������岨��Ѱ�ҷ�ֵ
int T = 512;                     	//���������岨��Ѱ�Ҳ���
int thresh = 512;                	//����Ѱ��˲�������
int amp = 100;                   	//���ڱ������岨�ε����
int Num;
unsigned char firstBeat = true;     //��һ���������
unsigned char secondBeat = false;   //�ڶ���������ģ���������������ȷ����������


//������
int main(void)
{  
	delay_init();	    	 		//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	 
	KEY_Init();						//���س�ʼ��
	LED_Init();						//LED��ʼ��
	BEEP_Init();					//��������ʼ��
	IIC_Init();   					//IIC��ʼ��
	OLED_Init();					//OLED��ʼ��
	Adc_Init();		  				//ADC��ʼ��	
	 
	TIM3_Int_Init(TIMER*10-1,7199);//��ʱ����ʼ����10Khz�ļ���Ƶ�ʣ�������2000Ϊ200ms   
	 
	while(1)
	{	
		switch(display_flag)
		{
			case 0:OLED_Main_display();break;			//����ʾ���溯��
			case 1:OLED_Value_display();break;			//ֵ��ʾ���溯��
			case 2:OLED_Waveform_display();break;		//������ʾ���溯��
		}	
		
		ALARM_deal();
		KEY_deal();		//����������
	}
}
 
//OLED����ʾ����
void OLED_Main_display(void)
{
	u8 i;
	for(i=0;i<6;i++)
	{
		OLED_ShowHz(16+16*i,0,i,1); 		//��ʾ����
	}
	for(i=6;i<12;i++)
	{
		OLED_ShowHz(16*(i-6),48,i,1); 		//��ʾ����
	}

	OLED_Refresh_Gram();					//������ʾ��OLED
}

//OLEDֵ��ʾ����
void OLED_Value_display(void)
{
	u8 temp;
	float xiao;
	
	temp = (u8)adcx/1;		//adcȡ��������
	xiao = adcx;			
	xiao -= temp;			//adcȡС������
	xiao *= 1000;			//adc��С������ת��������,�Ŵ�һǧ����������ʾ
	OLED_ShowString(32,0,">>",16);
	OLED_ShowString(80,0,"<<",16);
	OLED_ShowHz(48,0,12,1);					//��
	OLED_ShowHz(64,0,13,1);					//��
	OLED_ShowHz(0,16,14,1);					//��
	OLED_ShowHz(16,16,15,1);				//ѹ
	OLED_ShowHz(32,16,8,1);					//��
	OLED_ShowNum(40,16,(int)temp,1,16);		//��ʾ�ɼ�����adc����������	
	OLED_ShowChar(48,16,'.',16,1);
	OLED_ShowNum(56,16,(int)xiao,3,16);		//��ʾ�ɼ�����adcֵ��С������
	OLED_ShowString(88,16,"S:",16);
	OLED_ShowNum(104,16,(int)Signal,3,16);	//��ʾ����������Signalֵ
	
	OLED_ShowString(0,32,"IBI:",16);
	OLED_ShowNum(32,32,(int)IBI,3,16);		//��ʾIBIֵ
	OLED_ShowString(0,48,"BPM:",16);
	OLED_ShowNum(32,48,(int)BPM,3,16);		//��ʾBPMֵ
	
	OLED_Refresh_Gram();					//������ʾ��OLED
}

//ȡ����ֵ����
int myabs(int a)
{
	if(a<0)
		return -a;
	else 
		return a;
}

//OLED������ʾ����
void OLED_Waveform_display(void)
{
	int i;	
	u8 n;
		  
	if(waveform_flag == 1)
	{
		waveform_flag = 0;
		for(i=127;i>=0;i--)
		{
			for(n=0;n<64;n++)
			{
				OLED_DrawPoint(i,n,0);
			}
			//�����߲��������������ã�����ע�͵�,��#if 1��Ϊ#if 0
			//���ò��β��㺯�������ǲ��ο�������������
			#if 1
			if(i!=0)
			{
				if(myabs((int)waveform[i]-(int)waveform[i-1])>1)
				{
					if(waveform[i] > waveform[i-1])
					{
						for(n=waveform[i-1];n<waveform[i];n++)
						{
							OLED_DrawPoint(i,n,1);		//����Ӧ�����ص��ϴ�ӡ
						}
					}else
					{
						for(n=waveform[i];n<waveform[i-1];n++)
						{
							OLED_DrawPoint(i,n,1);		//����Ӧ�����ص��ϴ�ӡ
						}
					}
					
				}			
			}
			OLED_DrawPoint(i,waveform[i],1);		//����Ӧ�����ص��ϴ�ӡ
			#endif
		}
		OLED_Refresh_Gram();						//������ʾ��OLED
	}
}
 
//ADC������
void ADC_deal(void)
{
	u16 temp;
	temp = Get_Adc(ADC_Channel_0);	//����AD
	adcx = (float)temp*(3.3/4096);				//����ADCֵ�Ĵ���
	Signal = temp>>2;					//����������ֵ
}

//����������
void KEY_deal(void)
{
	static u8 key1_state = 0;	//����1��״̬
	static u8 key2_state = 0;	//����2��״̬
	
	if(KEY1 == 0)
	{
		key1_state = 1;		//��ʾ����1������
	}else if(KEY2 == 0)
	{
		key2_state = 1;		//��ʾ����2������
	}
	
	if(key1_state == 1)		//�˰�������������ʾ����
	{
		if(KEY1 == 1)		//��ʾ�Ѿ�������
		{
			key1_state = 0;
			OLED_Clear(); //��ʼ����
			display_flag = (display_flag==0)?1:(display_flag==1?2:0);		//��ʾ�����л�����
		}
	}
	if(key2_state == 1)		//�˰��������رշ�����
	{
		if(KEY2 == 1)		//��ʾ�Ѿ�������
		{
			key2_state = 0;
			alarm = 0;		//�رձ����ź�
			alarm_en = 0;	//�رձ���ʹ��
		}
	}
}

//����������
void ALARM_deal(void)
{
	static u8 alarm_flag = 0;
	if(adcx == 0)		//������ʱ�Ѿ����ֽӴ����˴�����
	{
		alarm_flag = 1;		//��ʾ�����ˣ��ȴ���������ʼ������Ժ��ٸ��ź�
	}else
	{
		if(alarm_flag == 1)
		{
			alarm_flag = 0;
			alarm_en = 1;		//ʹ�ܱ����ź�
		}
	}
	
	if(alarm_en == 1)		//ֻ���ڱ����ź�ʹ��֮����ܿ�ʼ���б�������
	{
		if(display_flag==1 || display_flag==2)	//��������ʾ����Ͳ�����ʾ������������������
		{
			if((BPM>HEART_MAX && BPM<HEART_MAX_ERROR) || (BPM<HEART_MIN && BPM>HEART_MIN_ERROR))		//�ɼ���������ֵ������Χ�����������˷�Χ֮��
			{
				alarm = 1;							//���������ź�
			}else
			{
				alarm = 0;							//�رձ����ź�
			}
		}
		if(alarm == 1)
		{
			LED_ON();								//��LED
			BEEP_ON();								//�򿪷�����
		}else
		{
			LED_OFF();
			BEEP_OFF();
		}
	}else
	{	//�رձ����ź�
		LED_OFF();
		BEEP_OFF();
	}
}

//���ʲɼ�����㴦��
void HeartRate_deal(void)
{
	unsigned int runningTotal;
	u8 i;

	Num = sampleCounter - lastBeatTime; 			//������һ�ν��ĺ��ʱ�䣬�Ա�������
		
	//�ҵ����岨�Ĳ���Ͳ���
	if(Signal < thresh && Num > (IBI/5)*3)	//Ϊ�˱�����Ҫ�ȴ�3/5��IBI��ʱ��
	{       
		if(Signal < T)
		{                        				//T����ֵ
			T = Signal;                         //��������������͵㣬�ı���ֵ
		}
	}
	if(Signal > thresh && Signal > P)		//����ֵ������ֵ���Ҳ���ֵ���ڷ�ֵ
	{          
		P = Signal;                             //P�Ƿ�ֵ���ı��ֵ
	} 
	//���ڿ�ʼѰ����������
	if (Num > 250)				//�����Ƶ����
	{                                   
		if ((Signal > thresh) && (Pulse == false) && (Num > (IBI/5)*3))
		{        
			Pulse = true;                               //���������ʱ������������źš�
//				LED_ON();									//��LED����ʾ�Ѿ���������
			IBI = sampleCounter - lastBeatTime;         //�������ĵ�ms����ʱ��
			lastBeatTime = sampleCounter;               //��¼��һ�������ʱ�䡣
			if(secondBeat)			//������ǵڶ������ģ����secondBeat == TRUE����ʾ�ǵڶ�������
			{                        
				secondBeat = false;                  //���secondBeat���ı�־
				for(i=0; i<=9; i++)		//������ʱ�����ӵ����������õ�һ��ʵ�ֵ�BPM��
				{             
					rate[i] = IBI;                      
				}
			}
			if(firstBeat)			//������ǵ�һ�η��ֽ��ģ����firstBeat == TRUE��
			{                         
				firstBeat = false;                   //���firstBeat��־
				secondBeat = true;                   //����secongBeat��־
				return;                              //IBIֵ�ǲ��ɿ��ģ����Է�������
			}   
			//�������10��IBIֵ������������
			runningTotal = 0;                  //���runningTotal���� 

			for(i=0; i<=8; i++)				//ת�����ݵ�rate������
			{                
				rate[i] = rate[i+1];                  //ȥ���ɵĵ�IBIֵ�� 
				runningTotal += rate[i];              //���9����ǰ���ϵ�IBIֵ��
			}

			rate[9] = IBI;                          //�����µ�IBI��ӵ����������С�
			runningTotal += rate[9];                //������µ�IBI��runningTotal��
			runningTotal /= 10;                     //ƽ�����10��IBIֵ��
			BPM = 60000/runningTotal;               //һ�����ж����ġ�������BPM
			QS = true;                              //�����������ұ�־Quantified Self��־
			//�����ISR���жϷ�������У�QS��־û�б������
		}                      
	}

	if (Signal < thresh && Pulse == true)		//��ֵ�½�ʱ�����ľͽ����ˡ�
	{   
//			LED_OFF();								//�ر�LED
		Pulse = false;                         //���������ǣ�����������һ�εļ���
		amp = P - T;                           //�õ����岨�������
		thresh = amp/2 + T;                    //����threshΪ�����50%��
		P = thresh;                            //����������һ��ʱ��
		T = thresh;
	}

	if (Num > 2500)				//���2.5���ȥ�˻�û�н���
	{                           
		thresh = 512;                          //����Ĭ����ֵ
		P = 512;                               //����Ĭ��Pֵ
		T = 512;                               //����Ĭ��Tֵ
		lastBeatTime = sampleCounter;          //�����Ľ��ĸ�������       
		firstBeat = true;                      //����firstBeatΪtrue������һ�δ���
		secondBeat = false;                    
	}
}



//���δ����������ڶ�ʱ����ִ�У�2msִ��һ��
void Waveform_deal(void)
{
	u8 temp;
	float xiao;
	static u8 waveSample_times = 0;
	
	int i;
	
	waveSample_times ++;
	if(waveSample_times == 10)
	{
		waveSample_times = 0;
		waveform_flag = 1;		
	
		temp = (u8)adcx/1;		//adcȡ��������
		xiao = adcx;			
		xiao -= temp;			//adcȡС������
		xiao *= 100;			//adc��С������ת�����������Ŵ�һ�ٱ�
		
		//���濪ʼ�������ݴ����Ա�ʹ��Һ����ʾ
		if(xiao<45)				//��xiao��ֵ������50-80֮��
		{
			xiao = 45;
		}else if(xiao>80)
		{
			xiao = 80;
		}
		xiao -= 50;				
		xiao *= 1.8;		
		xiao = 64-xiao;			//ʹ���εĿ����0-60֮�䣬OLED�Ŀ��Ϊ64
		
		//��������ʾ�����Ĵ����������һ�����������ʾ����
		waveform_copy[127] = waveform[127];
		for(i=126;i>=0;i--)
		{
			waveform_copy[i] = waveform[i];
			waveform[i] = waveform_copy[i+1];
		}
		waveform[127] = xiao;
	}		
}

//TIM3�жϣ�TIMER��2ms��ʱ��ִ��һ��
void TIM3_IRQHandler(void)   
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) 	//���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  	//���TIMx���жϴ�����λ:TIM �ж�Դ 
		sampleCounter += TIMER;							//����CPU����ʱ��	
		
		ADC_deal();										//ADC������	
		
		//����ֵ���������ڶ�ʱ���д���ÿTIMER(2ms)����һ��
		HeartRate_deal();	
		
		if(display_flag == 2)//����ͼ�δ���,��ͼ����ʾ����ʱ������ͼ������
		{
			Waveform_deal();		//�������ݴ�����������ʾ��������������ִ��
		}								
	}
}


