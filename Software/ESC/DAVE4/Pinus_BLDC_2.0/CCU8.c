/*
 * CCU8.c
 *
 *  Created on: Jun 24, 2016
 *      Author: aamark
 */

#include "CCU8.h"

uint16_t PWMPeriod = 1279;
uint16_t PWMCompare = 320;
uint16_t PWMCompareStart = 320;
uint16_t PWMCompareRef = 320;

void InitCCU8()
{
	//Make protected bitfields available for modification
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk);
									// MODE = 0:		Bit protection scheme disable
						  	  	  	// PROTS = 0:		Software is able to write to all protected bits
						  	  	  	// PASS = 0x18: 	Enable writing of bit field MODE

	//Loop until the lock is removed
	while(((SCU_GENERAL->PASSWD) & SCU_GENERAL_PASSWD_PROTS_Msk));

	SCU_CLK->CGATCLR0	|= ((1 << SCU_CLK_CGATCLR0_CCU80_Pos) & SCU_CLK_CGATCLR0_CCU80_Msk);
						   	   	   //CCU80 = 1:		disable gating

	//Wait voltage supply stabilization
	while ((SCU_CLK->CLKCR) & SCU_CLK_CLKCR_VDDC2LOW_Msk);

	//disable protected bits
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk) |
									// PASS = 0x18: 	Enable writing of bit field MODE
						  ((0x03 << SCU_GENERAL_PASSWD_MODE_Pos) & SCU_GENERAL_PASSWD_MODE_Msk);
									// MODE = 3:		Bit protection scheme enabled

	//Global IDLE clear
	CCU80->GIDLC	=  ((1 << CCU8_GIDLC_CS0I_Pos) & CCU8_GIDLC_CS0I_Msk) |
									// GIDLC = 1: Remove IDLE mode from slice 0
					   ((1 << CCU8_GIDLC_CS1I_Pos) & CCU8_GIDLC_CS1I_Msk) |
									// GIDLC = 1: Remove IDLE mode from slice 1
					   ((1 << CCU8_GIDLC_CS2I_Pos) & CCU8_GIDLC_CS2I_Msk) |
									// GIDLC = 1: Remove IDLE mode from slice 2
					   ((1 << CCU8_GIDLC_CS3I_Pos) & CCU8_GIDLC_CS3I_Msk) |
					   				// GIDLC = 1: Remove IDLE mode from slice 3
					   ((1 << CCU8_GIDLC_SPRB_Pos) & CCU8_GIDLC_SPRB_Msk);
									// SPRB = 1: Set run bit of prescaler

	CCU80_CC80->PRS = CCU80_CC81->PRS = CCU80_CC82->PRS = CCU80_CC83->PRS = PWMPeriod;
	CCU80_CC80->CR1S = PWMCompare;
	CCU80_CC80->CR2S = PWMCompare;
	CCU80_CC81->CR1S = 0;
	CCU80_CC81->CR2S = 0;
	CCU80_CC82->CR1S = 0;
	CCU80_CC82->CR2S = PWMPeriod+1;
	CCU80_CC83->CR1S = PWMCompare >> 2;

	//Global channel set
	CCU80->GCSS 	=  ((1 << CCU8_GCSS_S0SE_Pos) & CCU8_GCSS_S0SE_Msk) |
									// S0SE: Slice 0 shadow transfer set enable
					   ((1 << CCU8_GCSS_S1SE_Pos) & CCU8_GCSS_S1SE_Msk) |
									// S1SE: Slice 1 shadow transfer set enable
					   ((1 << CCU8_GCSS_S2SE_Pos) & CCU8_GCSS_S2SE_Msk) |
									// S2SE: Slice 2 shadow transfer set enable
					   ((1 << CCU8_GCSS_S3SE_Pos) & CCU8_GCSS_S3SE_Msk);
									// S3SE: Slice 3 shadow transfer set enable

	//Input Selector
	CCU80_CC80->INS	=  ((7 << CCU8_CC8_INS_EV0IS_Pos) & CCU8_CC8_INS_EV0IS_Msk) |
									// EV0IS = 7: INyH --> Synchronous start
					   ((1 << CCU8_CC8_INS_EV0EM_Pos) & CCU8_CC8_INS_EV0EM_Msk);
									// EV0EM = 1: Signal active on rising edge
	CCU80_CC81->INS	=  ((7 << CCU8_CC8_INS_EV0IS_Pos) & CCU8_CC8_INS_EV0IS_Msk) |
									// EV0IS = 7: INyH --> Synchronous start
					   ((1 << CCU8_CC8_INS_EV0EM_Pos) & CCU8_CC8_INS_EV0EM_Msk);
									// EV0EM = 1: Signal active on rising edge
	CCU80_CC82->INS	=  ((7 << CCU8_CC8_INS_EV0IS_Pos) & CCU8_CC8_INS_EV0IS_Msk) |
									// EV0IS = 7: INyH --> Synchronous start
					   ((1 << CCU8_CC8_INS_EV0EM_Pos) & CCU8_CC8_INS_EV0EM_Msk);
									// EV0EM = 1: Signal active on rising edge
	CCU80_CC83->INS	=  ((7 << CCU8_CC8_INS_EV0IS_Pos) & CCU8_CC8_INS_EV0IS_Msk) |
										// EV0IS = 7: INyH --> Synchronous start
					   ((1 << CCU8_CC8_INS_EV0EM_Pos) & CCU8_CC8_INS_EV0EM_Msk);
									// EV0EM = 1: Signal active on rising edge

	//Connection Matrix Control
	CCU80_CC80->CMC	=  ((1 << CCU8_CC8_CMC_STRTS_Pos) & CCU8_CC8_CMC_STRTS_Msk);
									// STRTS = 1: External Start Function triggered by Event 0
	CCU80_CC81->CMC	=  ((1 << CCU8_CC8_CMC_STRTS_Pos) & CCU8_CC8_CMC_STRTS_Msk);
									// STRTS = 1: External Start Function triggered by Event 0
	CCU80_CC82->CMC	=  ((1 << CCU8_CC8_CMC_STRTS_Pos) & CCU8_CC8_CMC_STRTS_Msk);
									// STRTS = 1: External Start Function triggered by Event 0
	CCU80_CC83->CMC	=  ((1 << CCU8_CC8_CMC_STRTS_Pos) & CCU8_CC8_CMC_STRTS_Msk);
									// STRTS = 1: External Start Function triggered by Event 0

	//Channel control
	CCU80_CC80->CHC	=	((1 << CCU8_CC8_CHC_OCS1_Pos) & CCU8_CC8_CHC_OCS1_Msk) |
									// OCS1 = 1: Inverted ST1 is conneccted to out0
						((1 << CCU8_CC8_CHC_OCS2_Pos) & CCU8_CC8_CHC_OCS2_Msk) |
									// OCS2 = 1: ST1 is conneccted to out1
						((1 << CCU8_CC8_CHC_OCS3_Pos) & CCU8_CC8_CHC_OCS3_Msk) |
									// OCS2 = 1: Inverted ST2 is conneccted to out2
						((1 << CCU8_CC8_CHC_OCS4_Pos) & CCU8_CC8_CHC_OCS4_Msk);
									// OCS2 = 1: ST2 is conneccted to out3
	CCU80_CC81->CHC	=	((1 << CCU8_CC8_CHC_OCS1_Pos) & CCU8_CC8_CHC_OCS1_Msk) |
									// OCS1 = 1: Inverted ST1 is conneccted to out0
						((1 << CCU8_CC8_CHC_OCS2_Pos) & CCU8_CC8_CHC_OCS2_Msk) |
									// OCS2 = 1: ST1 is conneccted to out1
						((1 << CCU8_CC8_CHC_OCS3_Pos) & CCU8_CC8_CHC_OCS3_Msk) |
									// OCS2 = 1: Inverted ST2 is conneccted to out2
						((1 << CCU8_CC8_CHC_OCS4_Pos) & CCU8_CC8_CHC_OCS4_Msk);
									// OCS2 = 1: ST2 is conneccted to out3
	CCU80_CC82->CHC	=	((1 << CCU8_CC8_CHC_OCS1_Pos) & CCU8_CC8_CHC_OCS1_Msk) |
									// OCS1 = 1: Inverted ST1 is conneccted to out0
						((1 << CCU8_CC8_CHC_OCS2_Pos) & CCU8_CC8_CHC_OCS2_Msk) |
									// OCS2 = 1: ST1 is conneccted to out1
						((1 << CCU8_CC8_CHC_OCS3_Pos) & CCU8_CC8_CHC_OCS3_Msk) |
									// OCS2 = 1: Inverted ST2 is conneccted to out2
						((1 << CCU8_CC8_CHC_OCS4_Pos) & CCU8_CC8_CHC_OCS4_Msk);
									// OCS2 = 1: ST2 is conneccted to out3

	//Dead-Time-Generation
	CCU80_CC80->DC1R = ((36 << CCU8_CC8_DC1R_DT1F_Pos) & CCU8_CC8_DC1R_DT1F_Msk) |
									//DT1F=36: Fall Value for Dead Time of Channel 1
					   ((36 << CCU8_CC8_DC1R_DT1R_Pos) & CCU8_CC8_DC1R_DT1R_Msk);
									//DT1R=36: Rise Value for Dead Time of Channel 1
	CCU80_CC81->DC1R = ((36 << CCU8_CC8_DC1R_DT1F_Pos) & CCU8_CC8_DC1R_DT1F_Msk) |
									//DT1F=36: Fall Value for Dead Time of Channel 1
					   ((36 << CCU8_CC8_DC1R_DT1R_Pos) & CCU8_CC8_DC1R_DT1R_Msk);
									//DT1R=36: Rise Value for Dead Time of Channel 1
	CCU80_CC82->DC1R = ((36 << CCU8_CC8_DC1R_DT1F_Pos) & CCU8_CC8_DC1R_DT1F_Msk) |
									//DT1F=36: Fall Value for Dead Time of Channel 1
					   ((36 << CCU8_CC8_DC1R_DT1R_Pos) & CCU8_CC8_DC1R_DT1R_Msk);
									//DT1R=36: Rise Value for Dead Time of Channel 1

	CCU80_CC80->DC2R = ((36 << CCU8_CC8_DC2R_DT2F_Pos) & CCU8_CC8_DC2R_DT2F_Msk) |
									//DT1F=36: Fall Value for Dead Time of Channel 2
					   ((36 << CCU8_CC8_DC2R_DT2R_Pos) & CCU8_CC8_DC2R_DT2R_Msk);
									//DT1R=36: Rise Value for Dead Time of Channel 2
	CCU80_CC81->DC2R = ((36 << CCU8_CC8_DC2R_DT2F_Pos) & CCU8_CC8_DC2R_DT2F_Msk) |
									//DT1F=36: Fall Value for Dead Time of Channel 2
					   ((36 << CCU8_CC8_DC2R_DT2R_Pos) & CCU8_CC8_DC2R_DT2R_Msk);
									//DT1R=36: Rise Value for Dead Time of Channel 2
	CCU80_CC82->DC2R = ((36 << CCU8_CC8_DC2R_DT2F_Pos) & CCU8_CC8_DC2R_DT2F_Msk) |
									//DT1F=36: Fall Value for Dead Time of Channel 2
					   ((36 << CCU8_CC8_DC2R_DT2R_Pos) & CCU8_CC8_DC2R_DT2R_Msk);
									//DT1R=36: Rise Value for Dead Time of Channel 2

	CCU80_CC80->DTC  = ((1 << CCU8_CC8_DTC_DTE1_Pos) & CCU8_CC8_DTC_DTE1_Msk) |
									//DTE1=1: Dead Time Enable for Channel 1
					   ((1 << CCU8_CC8_DTC_DTE2_Pos) & CCU8_CC8_DTC_DTE2_Msk) |
									//DTE2=1: Dead Time Enable for Channel 2
					   ((1 << CCU8_CC8_DTC_DCEN1_Pos) & CCU8_CC8_DTC_DCEN1_Msk) |
									//DCEN1=1: Dead Time Enable for CC8yST1
					   ((1 << CCU8_CC8_DTC_DCEN2_Pos) & CCU8_CC8_DTC_DCEN2_Msk) |
									//DCEN2=1: Dead Time Enable for inverted CC8yST1
					   ((1 << CCU8_CC8_DTC_DCEN3_Pos) & CCU8_CC8_DTC_DCEN3_Msk) |
									//DCEN3=1: Dead Time Enable for CC8yST2
					   ((1 << CCU8_CC8_DTC_DCEN4_Pos) & CCU8_CC8_DTC_DCEN4_Msk);
					   	   	   	    //DCEN4=1: Dead Time Enable for inverted CC8yST2
   CCU80_CC81->DTC  = ((1 << CCU8_CC8_DTC_DTE1_Pos) & CCU8_CC8_DTC_DTE1_Msk) |
									//DTE1=1: Dead Time Enable for Channel 1
					   ((1 << CCU8_CC8_DTC_DTE2_Pos) & CCU8_CC8_DTC_DTE2_Msk) |
									//DTE2=1: Dead Time Enable for Channel 2
					   ((1 << CCU8_CC8_DTC_DCEN1_Pos) & CCU8_CC8_DTC_DCEN1_Msk) |
									//DCEN1=1: Dead Time Enable for CC8yST1
					   ((1 << CCU8_CC8_DTC_DCEN2_Pos) & CCU8_CC8_DTC_DCEN2_Msk) |
									//DCEN2=1: Dead Time Enable for inverted CC8yST1
					   ((1 << CCU8_CC8_DTC_DCEN3_Pos) & CCU8_CC8_DTC_DCEN3_Msk) |
									//DCEN3=1: Dead Time Enable for CC8yST2
					   ((1 << CCU8_CC8_DTC_DCEN4_Pos) & CCU8_CC8_DTC_DCEN4_Msk);
									//DCEN4=1: Dead Time Enable for inverted CC8yST2
	CCU80_CC82->DTC  = ((1 << CCU8_CC8_DTC_DTE1_Pos) & CCU8_CC8_DTC_DTE1_Msk) |
									//DTE1=1: Dead Time Enable for Channel 1
					   ((1 << CCU8_CC8_DTC_DTE2_Pos) & CCU8_CC8_DTC_DTE2_Msk) |
									//DTE2=1: Dead Time Enable for Channel 2
					   ((1 << CCU8_CC8_DTC_DCEN1_Pos) & CCU8_CC8_DTC_DCEN1_Msk) |
									//DCEN1=1: Dead Time Enable for CC8yST1
					   ((1 << CCU8_CC8_DTC_DCEN2_Pos) & CCU8_CC8_DTC_DCEN2_Msk) |
									//DCEN2=1: Dead Time Enable for inverted CC8yST1
					   ((1 << CCU8_CC8_DTC_DCEN3_Pos) & CCU8_CC8_DTC_DCEN3_Msk) |
									//DCEN3=1: Dead Time Enable for CC8yST2
					   ((1 << CCU8_CC8_DTC_DCEN4_Pos) & CCU8_CC8_DTC_DCEN4_Msk);
									//DCEN4=1: Dead Time Enable for inverted CC8yST2

	//IO CCU8
	PORT0->IOCR0 	=	((0x15 << PORT0_IOCR0_PC0_Pos) & PORT0_IOCR0_PC0_Msk) |
									//PC0 = 0x15: Use ALT5 --> P0.0 UH
						((0x15 << PORT0_IOCR0_PC3_Pos) & PORT0_IOCR0_PC3_Msk);
									//PC3 = 0x15: Use ALT5 --> P0.3 UL
	PORT0->IOCR4 	=	((0x15 << PORT0_IOCR4_PC7_Pos) & PORT0_IOCR4_PC7_Msk) |
									//PC7 = 0x15: Use ALT5 --> P0.7 VH
						((0x15 << PORT0_IOCR4_PC4_Pos) & PORT0_IOCR4_PC4_Msk);
									//PC4 = 0x15: Use ALT5 --> P0.4 VL
	PORT0->IOCR8 	=	((0x15 << PORT0_IOCR8_PC8_Pos) & PORT0_IOCR8_PC8_Msk) |
									//PC8 = 0x15: Use ALT5 --> P0.8 WH
						((0x15 << PORT0_IOCR8_PC11_Pos) & PORT0_IOCR8_PC11_Msk);
									//PC11 = 0x15: Use ALT5 --> P0.11 WL

	//Interrupt Configuration
	CCU80_CC83->SRS		=  	((1 << CCU8_CC8_SRS_POSR_Pos) & CCU8_CC8_SRS_POSR_Msk) |
									// POSR = 1: Forward to CC8ySR1
							((2 << CCU8_CC8_SRS_CM1SR_Pos) & CCU8_CC8_SRS_CM1SR_Msk);
									// CM1SR = 2: Forward to CC8ySR2

	//Interrupt Enable Control
	CCU80_CC83->INTE	|=  ((1 << CCU8_CC8_INTE_PME_Pos) & CCU8_CC8_INTE_PME_Msk) |
									// PME = 1: Period Match enabled
							((1 << CCU8_CC8_INTE_CMU1E_Pos) & CCU8_CC8_INTE_CMU1E_Msk);
									// CMU1E = 1: Compare Match 1 while counting up enabled
}

void SetCommutationPattern(uint8_t pattern)
{
	switch (pattern)
	{
		case 1:
			CCU80_CC80->CR1S = PWMCompare;
			CCU80_CC80->CR2S = PWMCompare;
			CCU80_CC81->CR1S = 0;
			CCU80_CC81->CR2S = 0;
			CCU80_CC82->CR1S = 0;
			CCU80_CC82->CR2S = PWMPeriod+1;
			break;
		case 2:
			CCU80_CC80->CR1S = PWMCompare;
			CCU80_CC80->CR2S = PWMCompare;
			CCU80_CC81->CR1S = 0;
			CCU80_CC81->CR2S = PWMPeriod+1;
			CCU80_CC82->CR1S = 0;
			CCU80_CC82->CR2S = 0;
			break;
		case 3:
			CCU80_CC80->CR1S = 0;
			CCU80_CC80->CR2S = PWMPeriod+1;
			CCU80_CC81->CR1S = PWMCompare;
			CCU80_CC81->CR2S = PWMCompare;
			CCU80_CC82->CR1S = 0;
			CCU80_CC82->CR2S = 0;
			break;
		case 4:
			CCU80_CC80->CR1S = 0;
			CCU80_CC80->CR2S = 0;
			CCU80_CC81->CR1S = PWMCompare;
			CCU80_CC81->CR2S = PWMCompare;
			CCU80_CC82->CR1S = 0;
			CCU80_CC82->CR2S = PWMPeriod+1;
			break;
		case 5:
			CCU80_CC80->CR1S = 0;
			CCU80_CC80->CR2S = 0;
			CCU80_CC81->CR1S = 0;
			CCU80_CC81->CR2S = PWMPeriod+1;
			CCU80_CC82->CR1S = PWMCompare;
			CCU80_CC82->CR2S = PWMCompare;
			break;
		case 6:
			CCU80_CC80->CR1S = 0;
			CCU80_CC80->CR2S = PWMPeriod+1;
			CCU80_CC81->CR1S = 0;
			CCU80_CC81->CR2S = 0;
			CCU80_CC82->CR1S = PWMCompare;
			CCU80_CC82->CR2S = PWMCompare;
			break;
	}

	CCU80_CC83->CR1S = PWMCompare >> 2;

	//Global channel set
	CCU80->GCSS 	=  ((1 << CCU8_GCSS_S0SE_Pos) & CCU8_GCSS_S0SE_Msk) |
									// S0SE: Slice 0 shadow transfer set enable
					   ((1 << CCU8_GCSS_S1SE_Pos) & CCU8_GCSS_S1SE_Msk) |
									// S1SE: Slice 1 shadow transfer set enable
					   ((1 << CCU8_GCSS_S2SE_Pos) & CCU8_GCSS_S2SE_Msk) |
									// S2SE: Slice 2 shadow transfer set enable
					   ((1 << CCU8_GCSS_S3SE_Pos) & CCU8_GCSS_S3SE_Msk);
									// S3SE: Slice 3 shadow transfer set enable

	NVIC_SetPriority(CCU80_1_IRQn,3);
	NVIC_EnableIRQ(CCU80_1_IRQn);
}

void CompareAdjustment_ISR()
{
	if (CCU80_CC83->INTS & CCU8_CC8_INTS_PMUS_Msk)
	{
		static uint8_t prescale=0;
		prescale++;

		if (prescale % 4 == 0)
		{
			if (PWMCompareRef > PWMCompare)
				PWMCompare++;
			else if (PWMCompareRef < PWMCompare)
				PWMCompare--;
		}

		CCU80_CC83->SWR |= ((1 << CCU8_CC8_SWR_RPM_Pos) & CCU8_CC8_SWR_RPM_Msk);
								//RPM=1:	Period/One Match clear
	}
}

void SynchronousStartCCU8()
{
	//Start slices
	SCU_GENERAL->CCUCON	=  ((1 << SCU_GENERAL_CCUCON_GSC80_Pos) & SCU_GENERAL_CCUCON_GSC80_Msk);
									// GSC80 = 1: Synchronous start of CCU80
	SCU_GENERAL->CCUCON	=  ((0 << SCU_GENERAL_CCUCON_GSC80_Pos) & SCU_GENERAL_CCUCON_GSC80_Msk);
}

void StopSlicesCCU8()
{
	CCU80_CC80->TCCLR	=	((1 << CCU8_CC8_TCCLR_TRBC_Pos) & CCU8_CC8_TCCLR_TRBC_Msk) |
									// TRBC=1: Timer Run Bit Clear
							((1 << CCU8_CC8_TCCLR_TCC_Pos) & CCU8_CC8_TCCLR_TCC_Msk);
									// TCC=1: Timer Clear
	CCU80_CC81->TCCLR	=	((1 << CCU8_CC8_TCCLR_TRBC_Pos) & CCU8_CC8_TCCLR_TRBC_Msk) |
									// TRBC=1: Timer Run Bit Clear
							((1 << CCU8_CC8_TCCLR_TCC_Pos) & CCU8_CC8_TCCLR_TCC_Msk);
									// TCC=1: Timer Clear
	CCU80_CC82->TCCLR	=	((1 << CCU8_CC8_TCCLR_TRBC_Pos) & CCU8_CC8_TCCLR_TRBC_Msk) |
									// TRBC=1: Timer Run Bit Clear
							((1 << CCU8_CC8_TCCLR_TCC_Pos) & CCU8_CC8_TCCLR_TCC_Msk);
									// TCC=1: Timer Clear
	CCU80_CC83->TCCLR	=	((1 << CCU8_CC8_TCCLR_TRBC_Pos) & CCU8_CC8_TCCLR_TRBC_Msk) |
									// TRBC=1: Timer Run Bit Clear
							((1 << CCU8_CC8_TCCLR_TCC_Pos) & CCU8_CC8_TCCLR_TCC_Msk);
									// TCC=1: Timer Clear

	PWMCompareRef = PWMCompareStart;
	PWMCompare = PWMCompareStart;
}
