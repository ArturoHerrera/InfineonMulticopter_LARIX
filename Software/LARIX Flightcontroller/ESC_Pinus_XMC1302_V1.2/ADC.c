/*
 * ADC.c
 *
 *  Created on: 01.09.2015
 *      Author: maan
 */

#include "ADC.h"
#include <stdbool.h>

volatile uint16_t ADCReference=0;
volatile uint16_t ADCBEMF=0;

extern void log(const char *pcString, ...);

void InitADC()
{
	//Converter is active and arbiter is only activated if a conversion is needed (no permanent conversion)
	VADC_G0->ARBCFG |= 0x83UL;
	VADC_G1->ARBCFG |= 0x83UL;

	//Start-Up CalibrationУ׼AD
	VADC->GLOBCFG |= 1UL << 31;
	VADC->GLOBCFG |= 1UL << 31; //Same?
	while(VADC_G0->ARBCFG & (1UL<<28) || VADC_G1->ARBCFG & (1UL<<28)); //Wait till start up calibration is finished

	//G0��0��1��2ͨ����G1��1ͨ��Ϊ����ͨ��
	//Priority Channel can only be activated by Group request
	VADC_G0->CHASS |= 0x07UL; //Channel 0-2 are priority channels within group0
	//VADC_G0->CHASS |= 0x02UL; //Channel 0-2 are priority channels within group0
	VADC_G1->CHASS |= 0x02UL; //Channel 1 priority channel within group1

	//Input class
	VADC_G0->CHCTR[0] |= 0x341UL; //693ʹ����ר����1���ϱ߽�ʹ����ר�ñ߽�1���±߽�ʹ����ר�ñ߽�0
	VADC_G0->CHCTR[1] |= 0x341UL;
	VADC_G0->CHCTR[2] |= 0x341UL;
	VADC_G0->RCR[0] |= 1UL <<31;	//��������¼��������������

	VADC_G1->CHCTR[1] |= 1UL;		//ʹ����ר����1
	VADC_G1->CHCTR[1] |= 1UL << 16;	//�������������Ĵ���1
	VADC_G1->RCR[1] |= 1UL <<31;	//��������¼��������������

	//External Trigger
	VADC_G0->ASCTRL |= 0xC800UL;	//CCU80.SR2�������룬�������ز�������
	VADC_G1->ASCTRL |= 0xC800UL;

	//Gating mode//ʹ���ⲿ����
	VADC_G0->ASMR |= 0x05UL; //Conversion is started if at least one pending bit is set Edge of trigger event generates load event (Starts channel scan sequence converts from highest to lowest channel number)
	VADC_G1->ASMR |= 0x05UL;

	//Channel Select
	VADC_G0->ASSEL = 0x04UL; //Enables Channels 0-2 in Scan process (ADC Conversion)
	//VADC_G0->ASSEL = 0x02UL; //Enables Channels 0-2 in Scan process (ADC Conversion)
	VADC_G1->ASSEL = 0x02UL; //Enables Channel 1 in Scan Request

	//Enable Arbitration slot
	VADC_G0->ARBPR |= 0x01UL << 25;	//�ٲ�ʱ϶ʹ��
	VADC_G1->ARBPR |= 0x01UL << 25;

	//Service Request Software Activation Trigger
	VADC_G0->SRACT |= 0x02UL;	//�������������ڵ�1
	//VADC_G0->SRACT = 0x02UL;	//�������������ڵ�1
	VADC_G0->REVNP0 |= 1UL << 0; //Service Request Node Pointer Result Event i Routes the corresponding event trigger to one of the service request lines (nodes).0000BSelect service request line 0 of group x
	NVIC_SetPriority((IRQn_Type)18, 0); /*!< VADC SR3 Interrupt   VADC0_G0_1_IRQn*/
	NVIC_EnableIRQ((IRQn_Type)18);
	VADC_G1->SRACT |= 0x02UL;
	VADC_G1->REVNP0 |= 1UL << 4; //Service request Line 1 Group1//G1����������1(SR1?)
	NVIC_SetPriority((IRQn_Type)20, 2);
	NVIC_EnableIRQ((IRQn_Type)20);

	//IO enable
	//PORT2->PDISC &= (~((uint32_t)0x1U << 6));		//Pin 2.6 //Activate analog input path
	//PORT2->PDISC &= (~((uint32_t)0x1U << 8));		//Pin 2.8
	//PORT2->PDISC &= (~((uint32_t)0x1U << 9));		//Pin 2.9

	//PORT2->PDISC &= (~((uint32_t)0x1U << 7));		//Pin 2.7

	PORT2->PDISC = (0xf << 6);
}

extern volatile bool OpenLoopFinish;
extern volatile bool OpenLoopLock;

extern volatile uint32_t InnerPWMCompare;
extern volatile enum MotorState motorState;

//Interrupt wenn ADC neues Ergebnis, Phase U,V,W
void ZeroCrossing_ISR()
{
	static uint8_t Valuefilter = 0;

	if ( OpenLoopLock == true )
		return;
	if (OpenLoopFinish == false)
		return;
	
	if(VADC_G0->RES[0] & (1UL << 31))	//AD���½��
	{
		if((GetPhaseState() % 2 == 0 && ((uint16_t) VADC_G0->RES[0]) < ADCReference*8/10) ||
		   (GetPhaseState() % 2 != 0 && ((uint16_t) VADC_G0->RES[0]) > ADCReference*10/8))
		{
			Valuefilter++;
			if ( Valuefilter > 1 )
			{
				CCU40_CC40->TCCLR|=0x02;	//��ʱ�����㣬������ñȽ�ƥ��ֵΪ0��ʹ�ܱȽ�ƥ���жϣ�
											//��ᴥ��CCU4�Ƚ��ж�SR0��BlockCommutation_ISR��
											//ADC�жϱ�CCU4���ȣ����Բ���������ת�������ж��˳�����������CC40
				CCU80_CC83->INTE &= ~(1UL<<4);	//���ϼ���ʱ��ͨ��2�Ƚ�ƥ���жϳ���
			}
			IO004_TogglePin(IO004_Handle0);
		}
		else
		{
			Valuefilter = 0;
		}
	}
}

//Interrupt Wandlung von Referenzspannung
void ReferenceResult_ISR()
{
	if(VADC_G1->RES[1] & (1UL << 31))
		ADCReference = VADC_G1->RES[1];
}
