/*
 * main.c
 *
 *  Created on: 2016 Jun 24 13:04:06
 *  Author: aamark
 */

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include "CCU8.h"
#include "CCU4.h"
#include "ADC.h"
#include "DaisyChain.h"

/**
 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code. It is responsible for
 * invoking the APP initialization dispatcher routine - DAVE_Init() and hosting the place-holder for user application
 * code.
 */

int main(void)
{
	DAVE_STATUS_t status;

	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	InitADC();
	InitCCU8();
	InitCCU4();

	if(status == DAVE_STATUS_FAILURE)
	{
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1U)
		{

		}
	}

	while(1U)
	{
		DaisyChain();
	}
}
