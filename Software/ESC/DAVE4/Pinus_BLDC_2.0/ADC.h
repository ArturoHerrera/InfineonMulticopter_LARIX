/*
 * ADC.h
 *
 *  Created on: Jun 25, 2016
 *      Author: Andreas Mark
 */

#ifndef ADC_H_
#define ADC_H_

#include <XMC1300.h>
#include "CCU4.h"

#define ZeroCrossing_ISR IRQ_Hdlr_17
#define ReferenceResult_ISR IRQ_Hdlr_19

void InitADC();

#endif /* ADC_H_ */
