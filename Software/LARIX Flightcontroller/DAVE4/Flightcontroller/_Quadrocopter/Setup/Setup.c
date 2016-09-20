/* *****************************************************************************
**                      Author(s) Identity                                    **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** RT           Raffael Tschinder                                               **
**                                                                            **
**                                                                            **
***************************************************************************** */

/* *****************************************************************************
**                      Revision Control History                              **
***************************************************************************** */
/*
 * V0.0: 			 RT:  Initial Version
*/


/* *****************************************************************************
**                      Includes                                              **
***************************************************************************** */

#include "Setup.h"

/* *****************************************************************************
**                      Private Constant Definitions to be changed            **
***************************************************************************** */

/* *****************************************************************************
**                      Private Macro Definitions                             **
***************************************************************************** */

/* *****************************************************************************
**                      Global Type Definitions                               **
***************************************************************************** */

/* *****************************************************************************
**                      Private Type Definitions                              **
***************************************************************************** */

/* *****************************************************************************
**                      Global Function Declarations                          **
***************************************************************************** */
extern CONTROLLERPOLYNOMIALS polynoms;
/* *****************************************************************************
**                      Private Function Declarations                         **
***************************************************************************** */

/* *****************************************************************************
**                      Global Constant Definitions                           **
***************************************************************************** */

/* *****************************************************************************
**                      Private Constant Definitions                          **
***************************************************************************** */

/* *****************************************************************************
**                      Global Variable Definitions                           **
***************************************************************************** */

/* *****************************************************************************
**                      Private Variable Definitions                          **
***************************************************************************** */

/* *****************************************************************************
**                      Global Function Definitions                           **
***************************************************************************** */

void setup(void)
{
	initBluetoothStorage();//initialize space for variables used for Bluetooth Communication
	//-------------------Dave Setup---------------------
	DAVE_STATUS_t status;
	status = DAVE_Init();//Initialization of DAVE APPs
	if (status == DAVE_STATUS_FAILURE)
	{
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");
		while (1U);
	}
	disableIRQ();//disables all Interrupts
	delay(2000);//waits 2000ms
	enableIRQ();//enables configurated Interrupts
	setup_STATE_LEDs();//setup Status-LED's
	WatchRC_Init();//initialize RC-Watchdog
	setup_UART_Trigger_limits();//setup Trigger-Limits of UART-FIFO
	PressureFIR = Initialize_FIR_Filter(PressureFIR, MOVING_AVERAGE);//initialize FIR Filter
	setupDPS310I2C();//initialize DPS310
    setupDPS310();//setup DPS Hardware
    getCoefficients();//get Coefficients of DPS310

	MPU9250_Setup();//configures the IMU
	delay(3000);//wait 3000ms to wait for ESC's to startup
	// initialize controller polynomials
	polynoms.b_roll[0]=parameter.P_roll-parameter.I_roll*parameter.T-parameter.P_roll*parameter.N_roll*parameter.T+parameter.N_roll*parameter.I_roll*parameter.T*parameter.T+parameter.D_roll*parameter.N_roll;
	polynoms.b_roll[1]=parameter.I_roll*parameter.T-2*parameter.P_roll+parameter.P_roll*parameter.N_roll*parameter.T-2*parameter.D_roll*parameter.N_roll;
	polynoms.b_roll[2]=parameter.P_roll+parameter.D_roll*parameter.N_roll;
	polynoms.a_roll[0]=1-parameter.N_roll*parameter.T;
	polynoms.a_roll[1]=parameter.N_roll*parameter.T-2;

	polynoms.b_pitch[0]=parameter.P_pitch-parameter.I_pitch*parameter.T-parameter.P_pitch*parameter.N_pitch*parameter.T+parameter.N_pitch*parameter.I_pitch*parameter.T*parameter.T+parameter.D_pitch*parameter.N_pitch;
	polynoms.b_pitch[1]=parameter.I_pitch*parameter.T-2*parameter.P_pitch+parameter.P_pitch*parameter.N_pitch*parameter.T-2*parameter.D_pitch*parameter.N_pitch;
	polynoms.b_pitch[2]=parameter.P_pitch+parameter.D_pitch*parameter.N_pitch;
	polynoms.a_pitch[0]=1-parameter.N_pitch*parameter.T;
	polynoms.a_pitch[1]=parameter.N_pitch*parameter.T-2;

	TIMER_Start(&Control_Timer);//start Timer for Controller
}
/**
 *  \brief Function for setup of the pins for the Battery-LEDs on the LARIX-Board
 */
void setup_STATE_LEDs(void)
{
	Control_P3_0(OUTPUT_PP_GP, VERYSTRONG);	//Configure Pin 3.0 -->BatteryState1 (See: _Quadrocopter/BatterySafety/BatterySafety.h)
	Control_P3_1(OUTPUT_PP_GP, VERYSTRONG); //Configure Pin 3.1 -->BatteryState2
	Control_P3_2(OUTPUT_PP_GP, VERYSTRONG); //Configure Pin 3.2 -->BatteryState3
}

/**
 *  \brief Function for setting the trigger limits
 *
 *  \details Receive FIFO trigger limit is configured for Bluetooth & the Remote Control
 *  \details When the FIFO filling level rises above the trigger limit -> Interrupt will be generated
 */
void setup_UART_Trigger_limits(void)
{
	UART_SetRXFIFOTriggerLimit(&RemoteControl_Handle, 31);
	UART_SetRXFIFOTriggerLimit(&Bluetooth_Handle, 18);
}
/**
 *  \brief Function for disabling all Interrupt-Service-Routines
 *  *
 *  \details This function disables all Interrupt-Service-Routines\n
 *  except the Util_Timer_ISR
 */
void disableIRQ(void)
{
	INTERRUPT_Disable(&Control_Timer_ISR_Handle);
	INTERRUPT_Disable(&GeneralPurpose_Timer_ISR_Handle);
	INTERRUPT_Disable(&MagnetometerCal_Timer_ISR_Handle);
	NVIC_DisableIRQ(backgnd_rs_intr_handle.node_id);//Disable ADC Interrupt
	INTERRUPT_Disable(&DPS310_Ext_Int_ISR_Handle);
	INTERRUPT_Disable(&MPU9X50_Ext_Int_ISR_Handle);
	INTERRUPT_Disable(&DaisyChain_RX_ISR_Handle);
	INTERRUPT_Disable(&Bluetooth_RX_ISR_Handle);
	INTERRUPT_Disable(&RemoteControl_RX_ISR_Handle);
}
/**
 *  \brief Function for enabling all Interrupt-Service-Routines
 *
 *  \details This function enables all Interrupt-Service-Routines
 */
void enableIRQ(void)
{
	INTERRUPT_Enable(&GeneralPurpose_Timer_ISR_Handle);
	NVIC_EnableIRQ(backgnd_rs_intr_handle.node_id); //Enables ADC Interrupt
	INTERRUPT_Enable(&Control_Timer_ISR_Handle);
    INTERRUPT_Enable(&Bluetooth_RX_ISR_Handle);
    INTERRUPT_Enable(&DPS310_Ext_Int_ISR_Handle);
	INTERRUPT_Enable(&RemoteControl_RX_ISR_Handle);
}

/* *****************************************************************************
**                      Private Function Definitions                          **
***************************************************************************** */


