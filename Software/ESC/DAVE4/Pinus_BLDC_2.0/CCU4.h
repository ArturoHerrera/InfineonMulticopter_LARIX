/*
 * CCU4.h
 *
 *  Created on: Jun 25, 2016
 *      Author: Andreas Mark
 */

#ifndef CCU4_H_
#define CCU4_H_

#include <DAVE.h>
#include <XMC1300.h>
#include "CCU8.h"

#define BlockCommutation_ISR IRQ_Hdlr_21
#define RampUp_ISR			 IRQ_Hdlr_22
#define Daisy_WatchDog_ISR	 IRQ_Hdlr_23

void InitCCU4();
void NextCommutationPattern();
void StartSlicesCCU4();
void StopSlicesCCU4();

#endif /* CCU4_H_ */
