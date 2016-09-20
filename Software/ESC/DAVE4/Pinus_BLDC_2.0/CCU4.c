/*
 * CCU4.c
 *
 *  Created on: Jun 25, 2016
 *      Author: Andreas Mark
 */

#include "CCU4.h"

extern uint8_t CrossingDetected;

uint8_t ComPattern=1;
uint16_t ComStartPeriod=9999;
uint16_t ComDelta=50;
uint16_t ComEndPeriod=1999;

uint16_t ADCGatingPeriod=1920;

int8_t MotorState=0;

void InitCCU4()
{
	//Make protected bitfields available for modification
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk);
									// MODE = 0:		Bit protection scheme disable
						  	  	  	// PROTS = 0:		Software is able to wirte to all protected bits
						  	  	  	// PASS = 0x18: 	Enable writing of bit field MODE

	//Loop until the lock is removed
	while(((SCU_GENERAL->PASSWD) & SCU_GENERAL_PASSWD_PROTS_Msk));

	SCU_CLK->CGATCLR0	|= ((1 << SCU_CLK_CGATCLR0_CCU40_Pos) & SCU_CLK_CGATCLR0_CCU40_Msk);
						   	   	   //CCU40 = 1:		disable gating

	//Wait voltage supply stabilization
	while ((SCU_CLK->CLKCR) & SCU_CLK_CLKCR_VDDC2LOW_Msk);

	//disable protected bits
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk) |
									// PASS = 0x18: 	Enable writing of bit field MODE
						  ((0x03 << SCU_GENERAL_PASSWD_MODE_Pos) & SCU_GENERAL_PASSWD_MODE_Msk);
									// MODE = 3:		Bit protection scheme enabled

	//Global IDLE clear
	CCU40->GIDLC	=  ((1 << CCU4_GIDLC_CS0I_Pos) & CCU4_GIDLC_CS0I_Msk) |
									// GIDLC = 1: Remove IDLE mode from slice 0
					   ((1 << CCU4_GIDLC_CS1I_Pos) & CCU4_GIDLC_CS1I_Msk) |
					   	   	   	    // GIDLC = 1: Remove IDLE mode from slice 1
					   ((1 << CCU4_GIDLC_CS2I_Pos) & CCU4_GIDLC_CS2I_Msk) |
					   				// GIDLC = 1: Remove IDLE mode from slice 2
					   ((1 << CCU4_GIDLC_CS3I_Pos) & CCU4_GIDLC_CS3I_Msk) |
					   				// GIDLC = 1: Remove IDLE mode from slice 3
					   ((1 << CCU4_GIDLC_SPRB_Pos) & CCU4_GIDLC_SPRB_Msk);
									// SPRB = 1: Set run bit of prescaler

	CCU40_CC40->TC |= ((1 << CCU4_CC4_TC_CLST_Pos) & CCU4_CC4_TC_CLST_Msk);
									//CLST = 1: Shadow transfer on clear
	CCU40_CC42->TC |= ((1 << CCU4_CC4_TC_TSSM_Pos) & CCU4_CC4_TC_TSSM_Msk);
									//TSSM = 1: Timer Single Shot Mode
	CCU40_CC43->TC |= ((1 << CCU4_CC4_TC_TSSM_Pos) & CCU4_CC4_TC_TSSM_Msk);
									//TSSM = 1: Timer Single Shot Mode

	//Prescaler Configuration
	CCU40_CC40->PSC |= ((5 << CCU4_CC4_PSC_PSIV_Pos) & CCU4_CC4_PSC_PSIV_Msk);
									//PSIV = 5: Prescaler initial value
	CCU40_CC41->PSC |= ((5 << CCU4_CC4_PSC_PSIV_Pos) & CCU4_CC4_PSC_PSIV_Msk);
									//PSIV = 5: Prescaler initial value

	CCU40_CC42->PSC |= ((10 << CCU4_CC4_PSC_PSIV_Pos) & CCU4_CC4_PSC_PSIV_Msk);
									//PSIV = 4: Prescaler initial value

	//Period and Compare
	CCU40_CC40->PRS = ComStartPeriod;
	CCU40_CC40->CRS = ComEndPeriod-ComDelta;

	CCU40_CC41->PRS = ComStartPeriod;
	CCU40_CC42->PRS = 0xffff;
	CCU40_CC43->PRS = ADCGatingPeriod;


	//Global channel set
	CCU40->GCSS 	=  ((1 << CCU4_GCSS_S0SE_Pos) & CCU4_GCSS_S0SE_Msk) |
									// S0SE: Slice 0 shadow transfer set enable
					   ((1 << CCU4_GCSS_S1SE_Pos) & CCU4_GCSS_S1SE_Msk) |
									// S1SE: Slice 1 shadow transfer set enable
					   ((1 << CCU4_GCSS_S2SE_Pos) & CCU4_GCSS_S2SE_Msk) |
									// S1SE: Slice 2 shadow transfer set enable
					   ((1 << CCU4_GCSS_S3SE_Pos) & CCU4_GCSS_S3SE_Msk);
					   				// S1SE: Slice 3 shadow transfer set enable

	//Interrupt Configuration
	CCU40_CC40->SRS		=  ((0 << CCU4_CC4_SRS_CMSR_Pos) & CCU4_CC4_SRS_CMSR_Msk);
									// CMSR = 0: Forward compare match up to CC4ySR0
	CCU40_CC41->SRS		=  ((1 << CCU4_CC4_SRS_POSR_Pos) & CCU4_CC4_SRS_POSR_Msk);
									// POSR = 1: Forward period match to CC4ySR1
	CCU40_CC42->SRS		=  ((2 << CCU4_CC4_SRS_POSR_Pos) & CCU4_CC4_SRS_POSR_Msk);
									// POSR = 2: Forward period match to CC4ySR2

	//Interrupt Enable Control
	CCU40_CC40->INTE	=  ((1 << CCU4_CC4_INTE_CMUE_Pos) & CCU4_CC4_INTE_CMUE_Msk);
									// CMUE = 1: Compare Match while counting up enabled
	CCU40_CC41->INTE	=  ((1 << CCU4_CC4_INTE_PME_Pos) & CCU4_CC4_INTE_PME_Msk);
									// PME = 1: Period/One-Match enable
	CCU40_CC42->INTE	=  ((1 << CCU4_CC4_INTE_PME_Pos) & CCU4_CC4_INTE_PME_Msk);
									// PME = 1: Period/One-Match enable

	NVIC_SetPriority(CCU40_0_IRQn,0);
	NVIC_EnableIRQ(CCU40_0_IRQn);
	NVIC_SetPriority(CCU40_1_IRQn,3);
	NVIC_EnableIRQ(CCU40_1_IRQn);
	NVIC_SetPriority(CCU40_2_IRQn,2);
	NVIC_EnableIRQ(CCU40_2_IRQn);
}

void BlockCommutation_ISR()
{
	if (CCU40_CC40->INTS & CCU4_CC4_INTS_CMUS_Msk)
	{
		CCU40_CC43->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk);
										// TRBS = 1: Timer Run Bit set

		ComPattern++;

		if (ComPattern > 6)
			ComPattern=1;

		SetCommutationPattern(ComPattern);
		CrossingDetected=0;

		CCU40_CC40->SWR |= ((1 << CCU4_CC4_SWR_RCMU_Pos) & CCU4_CC4_SWR_RCMU_Msk);
								//RCMU=1:	Compare Match while counting up clear
	}
}

void RampUp_ISR()
{
	if (CCU40_CC41->INTS & CCU4_CC4_INTS_PMUS_Msk)
	{
		if (CCU40_CC40->PR > ComEndPeriod)
		{
			CCU40_CC40->PRS = CCU40_CC40->PR - ComDelta;

			CCU40->GCSS 	=  ((1 << CCU4_GCSS_S0SE_Pos) & CCU4_GCSS_S0SE_Msk);
									// S0SE: Slice 0 shadow transfer set enable

			while (CCU40->GCST & CCU4_GCST_S0SS_Msk);
		}
		else
		{
			MotorState=2;
			NVIC_DisableIRQ((IRQn_Type)22);
		}

		CCU40_CC41->SWR |= ((1 << CCU4_CC4_SWR_RPM_Pos) & CCU4_CC4_SWR_RPM_Msk);
									//RPM=1:	Period/One Match while counting up clear
	}
}

void Daisy_WatchDog_ISR()
{
	if (CCU40_CC42->INTS & CCU4_CC4_INTS_PMUS_Msk)
	{
		StopSlicesCCU4();
		StopSlicesCCU8();

		CCU40_CC42->SWR |= ((1 << CCU4_CC4_SWR_RPM_Pos) & CCU4_CC4_SWR_RPM_Msk);
								//RPM=1:	Period/One Match while counting up clear
	}
}

void StartSlicesCCU4()
{
	CCU40_CC40->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk);
									// TRBS = 1: Timer Run Bit set
	CCU40_CC41->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk);
									// TRBS = 1: Timer Run Bit set
	CCU40_CC42->TCSET	|=	((1 << CCU4_CC4_TCSET_TRBS_Pos) & CCU4_CC4_TCSET_TRBS_Msk);
									// TRBS = 1: Timer Run Bit set
	MotorState=1;
}

void StopSlicesCCU4()
{
	CCU40_CC40->TCCLR	=	((1 << CCU4_CC4_TCCLR_TRBC_Pos) & CCU4_CC4_TCCLR_TRBC_Msk) |
										// TRBC=1: Timer Run Bit Clear
							((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);
										// TCC=1: Timer Clear
	CCU40_CC41->TCCLR	=	((1 << CCU4_CC4_TCCLR_TRBC_Pos) & CCU4_CC4_TCCLR_TRBC_Msk) |
										// TRBC=1: Timer Run Bit Clear
							((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);
										// TCC=1: Timer Clear
	CCU40_CC42->TCCLR	=	((1 << CCU4_CC4_TCCLR_TRBC_Pos) & CCU4_CC4_TCCLR_TRBC_Msk) |
										// TRBC=1: Timer Run Bit Clear
							((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);
										// TCC=1: Timer Clear

	NVIC_EnableIRQ((IRQn_Type)22);

	//Period and Compare
	CCU40_CC40->PRS = ComStartPeriod;
	CCU40_CC40->CRS = ComEndPeriod-ComDelta;

	CCU40_CC41->PRS = ComStartPeriod;

	//Global channel set
	CCU40->GCSS 	=  ((1 << CCU4_GCSS_S0SE_Pos) & CCU4_GCSS_S0SE_Msk) |
									// S0SE: Slice 0 shadow transfer set enable
					   ((1 << CCU4_GCSS_S1SE_Pos) & CCU4_GCSS_S1SE_Msk);
									// S1SE: Slice 1 shadow transfer set enable
	MotorState=0;
}
