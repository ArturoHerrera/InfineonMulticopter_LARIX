/*
 * BlockCommutation.c
 *
 *  Created on: 01.09.2015
 *      Author: maan
 */


#include "BlockCommutation.h"
#include <stdbool.h>

#define Pole_Pairs			6
#define CC40Prescaler		8		//CCU40_CC40->PSC |= 0x03;
//#define CC40Prescaler		32		//CCU40_CC40->PSC |= 0x05;
#define OpenLoopRPM_start	324//432//216//180//144		//200
#define OpenLoopRPM_target	2160//1080//540		//550,540
#define AccelerateRounds	2//2.8		//6���ٵ�Ŀ���ٶ����õ�Ȧ��
#define OpenLoopDutyCycle	0.4		//0.2
#define LockTime			0.1		//��λs
#define LockDutyCycle		0.4

//CC40ʱ����Ƶ32Mhz��6�ε�ǶȻ���
#define OpenLoopPR_start	(uint16_t)( (32 * 1000 * 1000 / CC40Prescaler) / ((OpenLoopRPM_start * Pole_Pairs * 6) / 60) )
#define OpenLoopPR_target	(uint16_t)( (32 * 1000 * 1000 / CC40Prescaler) / ((OpenLoopRPM_target * Pole_Pairs * 6) / 60) )
#define OpenLoop_lockCNT	(uint32_t)( 32 * 1000 * 1000 / CC40Prescaler * LockTime ) / OpenLoopPR_start

volatile float InnerPWMFreq=25000;			//25k PWM
volatile float CurrentDutyCycleStart=0.1;	//10%ռ�ձ�

volatile uint32_t InnerPWMPeriod=0;
volatile uint32_t InnerPWMCompare=0;

volatile int8_t PhaseState=0;
volatile enum MotorState motorState = Stopped;

volatile bool OpenLoopFinish = false;
volatile bool OpenLoopLock = true;

extern uint8_t FifoTransBuffer[13];

extern void log(const char *pcString, ...);


void InitBlockCommutation()
{
	//Set Period and Compare for all slices
	InnerPWMPeriod=((uint32_t)(1000000000.0f/(InnerPWMFreq*31.25)))-(uint32_t)1; //Calculates Value for CCU Period Register at a timerspeed of 31.25ns per step (25.02KHz-->1279)
	InnerPWMCompare=InnerPWMPeriod*CurrentDutyCycleStart;//(127.9->127)
	CCU80_CC80->PRS = CCU80_CC81->PRS = CCU80_CC82->PRS = CCU80_CC83->PRS = InnerPWMPeriod; //Sets Period Register of Slice 0 to 3 to 1279 which means all slices have 25.02kHz
	CCU80_CC80->CR1S = InnerPWMCompare;
	CCU80_CC80->CR2S = InnerPWMPeriod+1;
	CCU80_CC81->CR1S = 0;
	CCU80_CC81->CR2S = 0;
	CCU80_CC82->CR1S = 0;
	CCU80_CC82->CR2S = InnerPWMPeriod+1;
	CCU80_CC83->CR1S = InnerPWMCompare-1;//>>1;
	CCU80_CC83->CR2S = InnerPWMCompare-1;//>>1;
	//��ʱ��Ƭ0��1��2��3ӳ�䴫����λʹ��
	CCU80->GCSS |= 0x1111UL; //Execute Shadow Transfer --> from shadow to actual Register

	//Synchronos start
	CCU80_CC80->INS |= 0x10007UL;	//Selects CCU8x.INyH (SCU.GSC80-->CCUCON) als Input f�r Event0 and Event0 active on Rising Edge
	CCU80_CC80->CMC |= 0x1UL;		//External Start triggered by Event 0
	CCU80_CC81->INS |= 0x10007UL;
	CCU80_CC81->CMC |= 0x1UL;
	CCU80_CC82->INS |= 0x10007UL;
	CCU80_CC82->CMC |= 0x1UL;
	CCU80_CC83->INS |= 0x10007UL;	//�¼�0ѡ��CCU80.IN3H�������봥��,�¼�0�ź�����������Ч
	CCU80_CC83->CMC |= 0x1UL;		//�ⲿ�����������¼�0����

	//Enable slices
	CCU80->GIDLC |= 0xFUL;			//CC80��CC81��CC82��CC83�˳�����ģʽ

	//CCU8 Channel Selection
	CCU80_CC80->CHC |= 0x1E;		//����CC80ST1���ӵ�CCU80.OUT00������CC80ST2���ӵ�CCU80.OUT03
	CCU80_CC81->CHC |= 0x1E;		//����CC81ST1���ӵ�CCU80.OUT10������CC81ST2���ӵ�CCU80.OUT13
	CCU80_CC82->CHC |= 0x1E;

	//IO CCU8
	PORT0->IOCR0  |= 0x15UL << 3;		//CCU80.OUT00��P0.0  UH
	PORT0->IOCR0  |= 0x15UL << 27;		//CCU80.OUT03��P0.3  UL
	PORT0->IOCR4  |= 0x15UL << 27;		//P0.7  VH
	PORT0->IOCR4  |= 0x15UL << 3;		//P0.4  VL
	//The startup software (SSW) will change the PC8 value to input pull-up device active, 00010b.
	PORT0->IOCR8 = 0x00UL;
	PORT0->IOCR8  |= 0x15UL << 3;		//P0.8  WH
	PORT0->IOCR8  |= 0x15UL << 27;		//P0.11 WL
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------------------------------
	//Blockcommutation
	//CCU40_CC40->PSC |= 0x08;		//Clock = Fccu4/256=32MHz/256=125kHz siehe App
	//CCU40_CC40->PRS = 0xFFFF; 	//Setzt Period Register auf maximal Wert Shadow
	//CCU40_CC40->PSC |= 0x07;		//Ԥ��Ƶ��ֵ,(Fccu4)/128,(32mhz/128)/65536=262.144ms
	CCU40_CC40->PSC |= 0x03;		//Ԥ��Ƶ��ֵ,(Fccu4)/8,(32mhz/8)/65536=16.384msԼ����6�Լ�101_RPM
	//CCU40_CC40->PSC |= 0x05;		//Ԥ��Ƶ��ֵ,(Fccu4)/8,(32mhz/32)/65536=65.536msԼ����6�Լ�25_RPM
//	CCU40_CC40->PRS = 0xFFFF;		//��ʱ��ӳ������ֵ,
	CCU40_CC40->PRS = OpenLoopPR_start;
	CCU40_CC40->CRS = 0;			//��ʱ��ӳ��Ƚ�ֵ������ռ�ձ�
	CCU40->GCSS |= (0x01UL << 0);	//��ʱ��Ƭ0ӳ�䴫����λʹ��

	//Interrupt Compare Match Slice 0
	CCU40_CC40->INTE |= 0x04UL;		//���ϼ���ʱ�Ƚ�ƥ���ж�ʹ��
	//CCU40_CC40->INTE |= 0x01UL;			//���ϼ���ʱ����ƥ���ж�ʹ��
	NVIC_SetPriority((IRQn_Type)21, 0);/*!< CCU40 SR0 Interrupt per Reset at SR0 */ //Block Commutation ISR
	NVIC_EnableIRQ((IRQn_Type)21);

	//Enable slice
	CCU40->GIDLC |= 0x01UL;		//CC40�˳�����ģʽ

	OpenLoopFinish = false;		//���ÿ�����־������������ÿ����϶�
	
//	log("InitBlockCommutation finish.\r\n");

	//ADC Trigger
	//ͨ��2�Ƚ�ƥ���������ת����SR2����������
	CCU80_CC83->SRS |= 2UL<<4; //Channel 2 der CCU83 compare Match auf CC83SR2 ->VADC
	//ͨ��1�Ƚ�ƥ���������ת����SR1����������
	CCU80_CC83->SRS |= 1UL<<2; //Channel 1 compare Match auf CC83SR1 --> NVIC
	//���ϼ���ʱͨ��1�Ƚ�ƥ���ж�ʹ��
	CCU80_CC83->INTE |= 1UL<<2; //Channel 1 compare Match while Counting up enable
}

//#define mutiple6 6
void BlockCommutation_ISR()
{
	static uint32_t openloopCNT = 0;
	if (motorState > Stopped)
	{

		if ( OpenLoopFinish == false )		//����δ����
		{
			//InnerPWMCompare = InnerPWMPeriod * OpenLoopDutyCycle;
			InnerPWMCompare = InnerPWMPeriod*OpenLoopDutyCycle - \
					( ((InnerPWMPeriod*(OpenLoopDutyCycle-0.2)) / (6*Pole_Pairs*AccelerateRounds)) * (openloopCNT-OpenLoop_lockCNT) );
			
			openloopCNT++;
			if ( openloopCNT <= OpenLoop_lockCNT )	//��0.2s
			{
				OpenLoopLock = true;
				InnerPWMCompare = InnerPWMPeriod * LockDutyCycle;
				PhaseState = 0;
				
				VADC_G0->ASSEL = 0x04UL;	//׼������W��
				CCU80_CC80->CR1S = InnerPWMCompare;		//U����PWM
				CCU80_CC80->CR2S = InnerPWMPeriod+1;
				CCU80_CC81->CR1S = 0;					//V���Źر�
				CCU80_CC81->CR2S = 0;					//V���ų���
				CCU80_CC82->CR1S = 0;					//W���Źر�
				CCU80_CC82->CR2S = InnerPWMPeriod+1;	//W���Źر�
				CCU80_CC83->CR1S = InnerPWMPeriod+1;
				CCU80_CC83->CR2S = InnerPWMPeriod+1;
				CCU80->GCSS |= 0x1111;//��ʱ��Ƭ0��1��2��3ӳ�䴫����λ

				// --- all shadow registers set?
				while ((CCU80->GCST & 0x1111) != 0);

				CCU80_CC83->INTE |= 1UL<<4;	//���ϼ���ʱ��ͨ��2�Ƚ�ƥ���ж�ʹ��
				return;
			}

			OpenLoopLock = false;
			//6�λ��ࡢ6�Լ���2Ȧ���ٵ�λ
			CCU40_CC40->PRS = OpenLoopPR_start - \
					( ((OpenLoopPR_start - OpenLoopPR_target) / (6*Pole_Pairs*AccelerateRounds)) * (openloopCNT-OpenLoop_lockCNT) );
			if ( (openloopCNT-OpenLoop_lockCNT) >= 6*Pole_Pairs*AccelerateRounds )
			{
				openloopCNT = 0;
				//openloopCNT--;
				OpenLoopFinish = true;		//�ÿ���������־
				//CCU40_CC40->PRS = OpenLoopPR_target;
				CCU40_CC40->PRS = 0xFFFF;
				//InnerPWMCompare = InnerPWMPeriod * 0.2;
			}
			
			CCU40->GCSS |= 0x01;//��ʱ��Ƭ0ӳ�䴫����λ
			// --- all shadow registers set?
			while ((CCU40->GCST & 0x01) != 0);
		}
		else	//�϶���ɺ�������մ���ָ��
			motorState = Running;

		
		PhaseState++;
		if (PhaseState > 5)
			PhaseState = 0;

		//if (PhaseState > 0)
		//	PhaseState--;
		//else
		//	PhaseState = 5;

		switch (PhaseState)
		{
			case 0:
				VADC_G0->ASSEL = 0x04UL;	//׼������W��
				CCU80_CC80->CR1S = InnerPWMCompare;		//U����PWM
				CCU80_CC80->CR2S = InnerPWMPeriod+1;
				CCU80_CC81->CR1S = 0;					//V���Źر�
				CCU80_CC81->CR2S = 0;					//V���ų���
				CCU80_CC82->CR1S = 0;					//W���Źر�
				CCU80_CC82->CR2S = InnerPWMPeriod+1;	//W���Źر�
				break;
			case 1:
				VADC_G0->ASSEL = 0x02UL;	//׼������V��
				CCU80_CC80->CR1S = InnerPWMCompare;		//U����PWM
				CCU80_CC80->CR2S = InnerPWMPeriod+1;
				CCU80_CC81->CR1S = 0;					//V���Źر�
				CCU80_CC81->CR2S = InnerPWMPeriod+1;	//V���Źر�
				CCU80_CC82->CR1S = 0;
				CCU80_CC82->CR2S = 0;					//W���ų���
				break;
			case 2:
				VADC_G0->ASSEL = 0x01UL;	//׼������U��
				CCU80_CC80->CR1S = 0;
				CCU80_CC80->CR2S = InnerPWMPeriod+1;	//U���Źر�
				CCU80_CC81->CR1S = InnerPWMCompare;		//V����PWM
				CCU80_CC81->CR2S = InnerPWMPeriod+1;
				CCU80_CC82->CR1S = 0;
				CCU80_CC82->CR2S = 0;					//W���ų���
				break;
			case 3:
				VADC_G0->ASSEL = 0x04UL;	//׼������W��
				CCU80_CC80->CR1S = 0;
				CCU80_CC80->CR2S = 0;					//U���ų���
				CCU80_CC81->CR1S = InnerPWMCompare;		//V����PWM
				CCU80_CC81->CR2S = InnerPWMPeriod+1;	//V���Źر�
				CCU80_CC82->CR1S = 0;
				CCU80_CC82->CR2S = InnerPWMPeriod+1;
				break;
			case 4:
				VADC_G0->ASSEL = 0x02UL;	//׼������V��
				CCU80_CC80->CR1S = 0;
				CCU80_CC80->CR2S = 0;					//U���ų���
				CCU80_CC81->CR1S = 0;
				CCU80_CC81->CR2S = InnerPWMPeriod+1;	//V���Źر�
				CCU80_CC82->CR1S = InnerPWMCompare;		//W����PWM
				CCU80_CC82->CR2S = InnerPWMPeriod+1;
				break;
			case 5:
				VADC_G0->ASSEL = 0x01UL;	//׼������U��
				CCU80_CC80->CR1S = 0;
				CCU80_CC80->CR2S = InnerPWMPeriod+1;	//U���Źر�
				CCU80_CC81->CR1S = 0;
				CCU80_CC81->CR2S = 0;					//V���ų���
				CCU80_CC82->CR1S = InnerPWMCompare;		//W����PWM
				CCU80_CC82->CR2S = InnerPWMPeriod+1;
				break;
		}
		CCU80_CC83->CR1S = InnerPWMCompare-1;//>>1;
		CCU80_CC83->CR2S = InnerPWMCompare-1;//>>1;
		CCU80->GCSS |= 0x1111;//��ʱ��Ƭ0��1��2��3ӳ�䴫����λ

		// --- all shadow registers set?
		while ((CCU80->GCST & 0x1111) != 0);

		CCU80_CC83->INTE |= 1UL<<4;	//���ϼ���ʱ��ͨ��2�Ƚ�ƥ���ж�ʹ��
/*
		static uint16_t cnt=0;
		if (motorState==StartUp)	//����1000�κ��ٽ��մ��ڵ�ռ�ձ�����
		{
			cnt++;
			if (cnt > 1000){
				cnt = 0;
				motorState=Running;
			}
		}
*/
	}
	else
		motorState=StartUp;
}

//void BlockCommutationPeriodMatch_ISR(){
//	StopMotor();
//	StopMotor();
//}

uint8_t GetPhaseState()
{
	return PhaseState;
}

void StartMotor()
{
	if (motorState==Stopped)
	{
		//Start slicesȫ��ʹ��CCU4
		SCU_GENERAL->CCUCON |= 0x100UL;
		SCU_GENERAL->CCUCON &= ~(0x100UL);
		//Start slicesCCU4��ʱ������
		CCU40_CC40->TCSET |= 0x01UL;
		OpenLoopFinish = false;		//���ÿ�����־������������ÿ����϶�
		OpenLoopLock = true;
	}
}

void StopMotor()
{
	if (motorState != Stopped)
	{
		InnerPWMCompare=InnerPWMPeriod*CurrentDutyCycleStart;

		//Stop slices CCU4���㶨ʱ����ֹͣ����
		CCU40_CC40->TCCLR |= 3UL; //Stops timer clears timer flag

		//Stop slices CCU8
		CCU80_CC80->TCCLR |= 3UL;//Stops timer clears timer flag
		CCU80_CC81->TCCLR |= 3UL;//Stops timer clears timer flag
		CCU80_CC82->TCCLR |= 3UL;//Stops timer clears timer flag
		CCU80_CC83->TCCLR |= 3UL;//Stops timer clears timer flag

		motorState=Stopped;
		OpenLoopFinish = false;		//���ÿ�����־������������ÿ����϶�
		OpenLoopLock = true;
	}
}

uint8_t SetReferenceCurrent(uint16_t ref)
{
	if (motorState == Stopped && ref > 125)
		StartMotor();
	else if(motorState != Stopped && ref <= 125)
		StopMotor();

	//StartMotor();

	if (motorState == Running)
	{
		InnerPWMCompare=ref;
		return 1;
	}
	else
		return 0;
}
