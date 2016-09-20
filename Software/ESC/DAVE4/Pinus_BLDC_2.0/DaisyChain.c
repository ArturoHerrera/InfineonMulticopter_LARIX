/*
 * DaisyChain.c
 *
 *  Created on: 02.05.2015
 *      Author: Andreas
 */

#include "DaisyChain.h"

uint8_t FifoRecBuffer[DAISY_BUFFER_SIZE] = {0};
uint8_t FifoTransBuffer[DAISY_BUFFER_SIZE] = {0};

extern uint16_t PWMPeriod;
extern uint16_t PWMCompare;
extern uint16_t PWMCompareStart;
extern uint16_t PWMCompareRef;
extern int8_t MotorState;

void DaisyChain(void)
{
	uint8_t i=0;
	uint32_t data=0;

	//Read data from UART buffer
	UART_Receive(&UART_DaisyChain, FifoRecBuffer, DAISY_BUFFER_SIZE);

	//Reset WatchDog Slice
	CCU40_CC42->TCCLR |= ((1 << CCU4_CC4_TCCLR_TCC_Pos) & CCU4_CC4_TCCLR_TCC_Msk);
													// TCC = 1: Timer Clear

	//Assumption that communication is lost --> emtpy Receive Buffer
	if (FifoRecBuffer[DAISY_BUFFER_SIZE-1] != DAISY_STOP_BYTE)
	{
		DIGITAL_IO_SetOutputHigh(&DIGITAL_IO_DY_Error);
		while((uint16_t)UART_DaisyChain.channel->RBUF != DAISY_STOP_BYTE);
		return;
	}

	DIGITAL_IO_SetOutputLow(&DIGITAL_IO_DY_Error);

	uint8_t cmd = FifoRecBuffer[0];
	uint16_t params =  (FifoRecBuffer[1] << 8 | FifoRecBuffer[2]);

	switch (cmd)
	{
		case START_MOTOR:
			SynchronousStartCCU8();
			StartSlicesCCU4();
			break;
		case STOP_MOTOR:
			StopSlicesCCU4();
			StopSlicesCCU8();
			break;
		case SET_REF_CURRENT:
			if (params)
			{
				if (MotorState == 3)
					PWMCompareRef=(PWMPeriod+1-PWMCompareStart)/65535.0*params+PWMCompareStart;
				else if (MotorState == 0)
				{
					SynchronousStartCCU8();
					StartSlicesCCU4();
				}
			}
			else
			{
				StopSlicesCCU4();
				StopSlicesCCU8();
			}
			break;
	}

	for(i=DAISY_MESSAGE_LENGTH; i<DAISY_BUFFER_SIZE-1; i++)
		FifoTransBuffer[i-DAISY_MESSAGE_LENGTH]=FifoRecBuffer[i];

	//Status-Code
	FifoTransBuffer[i-DAISY_MESSAGE_LENGTH]=0;
	i++;
	//Data
	FifoTransBuffer[i-DAISY_MESSAGE_LENGTH]=(uint8_t)(data >> 8);
	i++;
	FifoTransBuffer[i-DAISY_MESSAGE_LENGTH]=(uint8_t)data;
	i++;
	FifoTransBuffer[i-DAISY_MESSAGE_LENGTH]=DAISY_STOP_BYTE;

	UART_Transmit(&UART_DaisyChain, FifoTransBuffer, DAISY_BUFFER_SIZE);
}
