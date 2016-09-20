/*
 * CCU8.h
 *
 *  Created on: Jun 24, 2016
 *      Author: aamark
 */

#ifndef CCU8_H_
#define CCU8_H_

#include <XMC1300.h>

#define CompareAdjustment_ISR IRQ_Hdlr_26

void InitCCU8();
void SetCommutationPattern(uint8_t pattern);
void SynchronousStartCCU8();
void StopSlicesCCU8();

#endif /* CCU8_H_ */
