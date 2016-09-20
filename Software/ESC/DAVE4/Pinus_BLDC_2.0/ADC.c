/*
 * ADC.c
 *
 *  Created on: Jun 25, 2016
 *      Author: Andreas Mark
 */

#include "ADC.h"

extern uint8_t ComPattern;
extern int8_t MotorState;
extern uint16_t ComDelta;

uint16_t PhaseU=0;
uint16_t PhaseV=0;
uint16_t PhaseW=0;
uint16_t ADCReference=0;

uint8_t ZeroCrossing=0;
uint8_t CrossingDetected=0;

uint16_t CurrentSpeed=0;

void InitADC()
{
	//Make protected bitfields available for modification
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk);
									// MODE = 0:		Bit protection scheme disable
						  	  	  	// PROTS = 0:		Software is able to write to all protected bits
						  	  	  	// PASS = 0x18: 	Enable writing of bit field MODE

	//Loop until the lock is removed
	while(((SCU_GENERAL->PASSWD) & SCU_GENERAL_PASSWD_PROTS_Msk));

	SCU_CLK->CGATCLR0	|= ((1 << SCU_CLK_CGATCLR0_VADC_Pos) & SCU_CLK_CGATCLR0_VADC_Msk);
						   	   	   //VADC = 1:		disable gating

	//Wait voltage supply stabilization
	while ((SCU_CLK->CLKCR) & SCU_CLK_CLKCR_VDDC2LOW_Msk);

	//disable protected bits
	SCU_GENERAL->PASSWD = ((0x18 << SCU_GENERAL_PASSWD_PASS_Pos) & SCU_GENERAL_PASSWD_PASS_Msk) |
									// PASS = 0x18: 	Enable writing of bit field MODE
						  ((0x03 << SCU_GENERAL_PASSWD_MODE_Pos) & SCU_GENERAL_PASSWD_MODE_Msk);
									// MODE = 3:		Bit protection scheme enabled

	//Global ADC Initialization
	VADC->CLC          = ((0 << VADC_CLC_DISR_Pos) & VADC_CLC_DISR_Msk);
										// DISR = 0: module clock enable
										// 0B On request: enable the module clock
	// Wait for module clock enabled
	while(VADC->CLC!=0);

	VADC->GLOBCFG      = ((0 << VADC_GLOBCFG_DIVA_Pos) & VADC_GLOBCFG_DIVA_Msk) |
										// DIVA = 0: Divider Factor for the Analog Internal Clock
										// fADCI = fADC / (DIVA + 1) = fADC / 1
						 ((1 << VADC_GLOBCFG_DIVWC_Pos) & VADC_GLOBCFG_DIVWC_Msk);
										// DIVWC = 1: Write Control for Divider Parameters
										// Bitfields DIVA, DCMSB, DIVD can be written

	//Arbitration
	VADC_G0->ARBCFG    = ((VADC_G0->ARBCFG &
						 ~(VADC_G_ARBCFG_ANONC_Msk | VADC_G_ARBCFG_ARBRND_Msk | VADC_G_ARBCFG_ARBM_Msk)) |
										// Clear relevant bits
						 (((3 << VADC_G_ARBCFG_ANONC_Pos) & VADC_G_ARBCFG_ANONC_Msk) |
										// ANONS = 11B: Analog Converter Control: Normal Operation
										//  The converter is active, conversions are started immediately.
										//  Requires no wakeup time.
						 ((0 << VADC_G_ARBCFG_ARBRND_Pos) & VADC_G_ARBCFG_ARBRND_Msk)|
										// ARBRND = 00B: Arbitration Round Length
										//  00B: 4 arbitration slots per round (tARB = 4 / fADCD)
						 ((0 << VADC_G_ARBCFG_ARBM_Pos) & VADC_G_ARBCFG_ARBM_Msk)));
										// ARBM = 0B: Arbitration Mode: The arbiter runs permanently
	VADC_G1->ARBCFG    = ((VADC_G0->ARBCFG &
						 ~(VADC_G_ARBCFG_ANONC_Msk | VADC_G_ARBCFG_ARBRND_Msk | VADC_G_ARBCFG_ARBM_Msk)) |
										// Clear relevant bits
						 (((3 << VADC_G_ARBCFG_ANONC_Pos) & VADC_G_ARBCFG_ANONC_Msk) |
										// ANONS = 11B: Analog Converter Control: Normal Operation
										//  The converter is active, conversions are started immediately.
										//  Requires no wakeup time.
						 ((0 << VADC_G_ARBCFG_ARBRND_Pos) & VADC_G_ARBCFG_ARBRND_Msk)|
										// ARBRND = 00B: Arbitration Round Length
										//  00B: 4 arbitration slots per round (tARB = 4 / fADCD)
						 ((0 << VADC_G_ARBCFG_ARBM_Pos) & VADC_G_ARBCFG_ARBM_Msk)));
										// ARBM = 0B: Arbitration Mode: The arbiter runs permanently

	VADC_G0->ARBPR     = ((VADC_G0->ARBPR &
						 ~(VADC_G_ARBPR_ASEN1_Msk | VADC_G_ARBPR_PRIO1_Msk)) |
										// clear relevant bits
						 (((1 << VADC_G_ARBPR_PRIO1_Pos) & VADC_G_ARBPR_PRIO1_Msk) |
										// PRIO1 = 1: Priority of Request Source x (x = 1)
										// Arbitration priority of request source x (in slot x)
						 ((1 << VADC_G_ARBPR_ASEN1_Pos) & VADC_G_ARBPR_ASEN1_Msk)));
										// ASEN1 = 1: Arbitration Slot 1 Enable
	VADC_G1->ARBPR     = ((VADC_G0->ARBPR &
						 ~(VADC_G_ARBPR_ASEN1_Msk | VADC_G_ARBPR_PRIO1_Msk)) |
										// clear relevant bits
						 (((1 << VADC_G_ARBPR_PRIO1_Pos) & VADC_G_ARBPR_PRIO1_Msk) |
										// PRIO1 = 1: Priority of Request Source x (x = 1)
										// Arbitration priority of request source x (in slot x)
						 ((1 << VADC_G_ARBPR_ASEN1_Pos) & VADC_G_ARBPR_ASEN1_Msk)));
										// ASEN1 = 1: Arbitration Slot 1 Enable


	// Auto Scan source
	VADC_G0->ASSEL 	   = (((1 << VADC_G_ASSEL_CHSEL0_Pos) & VADC_G_ASSEL_CHSEL0_Msk) |
										// CHSEL0 = 0: Select channel 0
						  ((1 << VADC_G_ASSEL_CHSEL1_Pos) & VADC_G_ASSEL_CHSEL1_Msk) |
										// CHSEL1 = 0: Select channel 1
						  ((1 << VADC_G_ASSEL_CHSEL2_Pos) & VADC_G_ASSEL_CHSEL2_Msk));
										// CHSEL2 = 1: Select channel 2
	VADC_G1->ASSEL 	   = (((1 << VADC_G_ASSEL_CHSEL0_Pos) & VADC_G_ASSEL_CHSEL0_Msk) |
										// CHSEL0 = 0: Select channel 0
						  ((1 << VADC_G_ASSEL_CHSEL1_Pos) & VADC_G_ASSEL_CHSEL1_Msk) |
										// CHSEL1 = 0: Select channel 1
						  ((1 << VADC_G_ASSEL_CHSEL2_Pos) & VADC_G_ASSEL_CHSEL2_Msk));
										// CHSEL2 = 1: Select channel 2

	VADC_G0->ASCTRL    = ((8 << VADC_G_ASCTRL_XTSEL_Pos) & VADC_G_ASCTRL_XTSEL_Msk) |
										// XTSEL = 8: External Trigger Input Selection
										//   Trigger Input I --> CCU80.SR2
						 ((2 << VADC_G_ASCTRL_XTMODE_Pos) & VADC_G_ASCTRL_XTMODE_Msk) |
										// XTMODE = 1: Trigger Operating Mode: Falling edge
						 ((1 << VADC_G_ASCTRL_XTWC_Pos) & VADC_G_ASCTRL_XTWC_Msk) |
										// XTWC = 1: Write Control for Trigger Configuration
						 ((0 << VADC_G_ASCTRL_GTSEL_Pos) & VADC_G_ASCTRL_GTSEL_Msk) |
										// GTSEL = 0: Gate Input Selection
						 ((1 << VADC_G_ASCTRL_GTWC_Pos) & VADC_G_ASCTRL_GTWC_Msk);
										// GTWC = 1: Write Control for Gate Configuration
	VADC_G1->ASCTRL    = ((8 << VADC_G_ASCTRL_XTSEL_Pos) & VADC_G_ASCTRL_XTSEL_Msk) |
										// XTSEL = 8: External Trigger Input Selection
										//   Trigger Input I --> CCU80.SR2
						 ((1 << VADC_G_ASCTRL_XTMODE_Pos) & VADC_G_ASCTRL_XTMODE_Msk) |
										// XTMODE = 1: Trigger Operating Mode: Falling edge
						 ((1 << VADC_G_ASCTRL_XTWC_Pos) & VADC_G_ASCTRL_XTWC_Msk) |
										// XTWC = 1: Write Control for Trigger Configuration
										//  Bitfields XTMODE and XTSEL can be written
						 ((0 << VADC_G_ASCTRL_GTSEL_Pos) & VADC_G_ASCTRL_GTSEL_Msk) |
										// GTSEL = 0: Gate Input Selection
						 ((1 << VADC_G_ASCTRL_GTWC_Pos) & VADC_G_ASCTRL_GTWC_Msk);
										// GTWC = 1: Write Control for Gate Configuration
										//  Bitfield GTSEL can be written

	VADC_G0->ASMR      = ((3 << VADC_G_ASMR_ENGT_Pos) & VADC_G_ASMR_ENGT_Msk) |
										// ENGT = 3: Enable Gate: 01B Conversion requests are
										//  issued if at least one pending bit is set and gate is inactive
						 ((1 << VADC_G_ASMR_ENTR_Pos) & VADC_G_ASMR_ENTR_Msk) |
										// ENTR = 1: Enable External Trigger: 1B External trigger enable
						 ((1 << VADC_G_ASMR_ENSI_Pos) & VADC_G_ASMR_ENSI_Msk);
										// ENSI=1: Enable source event interrupt
	VADC_G1->ASMR      = ((1 << VADC_G_ASMR_ENGT_Pos) & VADC_G_ASMR_ENGT_Msk) |
										// ENGT = 1: Enable Gate: 01B Conversion requests are
										//  issued if at least one pending bit is set
						 ((1 << VADC_G_ASMR_ENTR_Pos) & VADC_G_ASMR_ENTR_Msk) |
										// ENTR = 1: Enable External Trigger: 1B External trigger enable
						 ((1 << VADC_G_ASMR_ENSI_Pos) & VADC_G_ASMR_ENSI_Msk);
										// ENSI=1: Enable source event interrupt

	VADC_G0->CHCTR[0]	|= ((0 << VADC_G_CHCTR_RESREG_Pos) & VADC_G_CHCTR_RESREG_Msk);
										// RESREG=0: Store result in GxRES0
	VADC_G0->CHCTR[1]	|= ((1 << VADC_G_CHCTR_RESREG_Pos) & VADC_G_CHCTR_RESREG_Msk);
										// RESREG=1: Store result in GxRES1
	VADC_G0->CHCTR[2]	|= ((2 << VADC_G_CHCTR_RESREG_Pos) & VADC_G_CHCTR_RESREG_Msk);
										// RESREG=2: Store result in GxRES2
	VADC_G1->CHCTR[1]	|= ((1 << VADC_G_CHCTR_RESREG_Pos) & VADC_G_CHCTR_RESREG_Msk);
										// RESREG=0: Store result in GxRES1

	NVIC_SetPriority(VADC0_G0_0_IRQn,1);
	NVIC_EnableIRQ(VADC0_G0_0_IRQn);
	NVIC_SetPriority(VADC0_G1_0_IRQn,3);
	NVIC_EnableIRQ(VADC0_G1_0_IRQn);
}

void ZeroCrossing_ISR()
{
	if (VADC_G0->SEFLAG & VADC_G_SEFLAG_SEV1_Msk)
	{
		PhaseU = (uint16_t)VADC_G0->RES[0];
		PhaseV = (uint16_t)VADC_G0->RES[1];
		PhaseW = (uint16_t)VADC_G0->RES[2];

		if (MotorState > 1)
		{
			if (!CrossingDetected)
			{
				switch (ComPattern)
				{
					case 1:
						if (PhaseW < ADCReference)
						{
							ZeroCrossing=!ZeroCrossing;
							CrossingDetected=1;
						}
						break;
					case 2:
						if (PhaseV > ADCReference)
						{
							ZeroCrossing=!ZeroCrossing;
							CrossingDetected=1;
						}
						break;
					case 3:
						if (PhaseU < ADCReference)
						{
							ZeroCrossing=!ZeroCrossing;
							CrossingDetected=1;
						}
						break;
					case 4:
						if (PhaseW > ADCReference)
						{
							ZeroCrossing=!ZeroCrossing;
							CrossingDetected=1;
						}
						break;
					case 5:
						if (PhaseV < ADCReference)
						{
							ZeroCrossing=!ZeroCrossing;
							CrossingDetected=1;
						}
						break;
					case 6:
						if (PhaseU > ADCReference)
						{
							ZeroCrossing=!ZeroCrossing;
							CrossingDetected=1;
						}
						break;
				}
				if (CrossingDetected)
				{
					if (MotorState==3)
					{
						CurrentSpeed=CCU40_CC40->TIMER*6;
						CCU40_CC40->CRS = CCU40_CC40->TIMER>>1;
					}
					else
					{
						if (CCU40_CC40->CR > CCU40_CC40->TIMER>>1)
							CCU40_CC40->CRS = CCU40_CC40->CR - 1;
						else
						{
							CCU40_CC40->PRS = 0xffff;
							MotorState=3;
						}
					}
					CCU40->GCSS 	=  ((1 << CCU4_GCSS_S0SE_Pos) & CCU4_GCSS_S0SE_Msk);
											// S0SE: Slice 0 shadow transfer set enable
					CCU40_CC40->TCCLR |= ((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);
												// TCC = 1: Timer Clear

					DIGITAL_IO_ToggleOutput(&DIGITAL_IO_ZeroCrossing);
				}
			}
		}

		VADC_G0->SEFCLR |= ((1 << VADC_G_SEFCLR_SEV1_Pos) & VADC_G_SEFCLR_SEV1_Msk);
								//SEV0=1: Clear source event 0
	}
}

void ReferenceResult_ISR()
{
	if (VADC_G1->SEFLAG & VADC_G_SEFLAG_SEV1_Msk)
	{
		ADCReference = (uint16_t)VADC_G1->RES[1];

		VADC_G1->SEFCLR |= ((1 << VADC_G_SEFCLR_SEV1_Pos) & VADC_G_SEFCLR_SEV1_Msk);
									//SEV0=1: Clear source event 0
	}
}
