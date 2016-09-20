/*******************************************************************************
**                      Author(s) Identity                                    **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** AM           Andreas Mark                                                  **
** DW           Dominik Wieland                                               **
**                                                                            **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/
/*
 * V0.0: 29-12-2014, AM:  Initial Version
 * V0.1: 21-07-2016, DW:  Port of SW from DAVE3 to DAVE4
 * V0.2: 20.09.2016, RT:  Moved Setup in another folder
 */


/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/
#include "AttitudeController.h"

/*******************************************************************************
**                      Private Constant Definitions to be changed            **
*******************************************************************************/

/*******************************************************************************
**                      Private Macro Definitions                             **
*******************************************************************************/

/*******************************************************************************
**                      Global Type Definitions                               **
*******************************************************************************/

/*******************************************************************************
**                      Private Type Definitions                              **
*******************************************************************************/

/*******************************************************************************
**                      Global Function Declarations                          **
*******************************************************************************/

/*******************************************************************************
**                      Private Function Declarations                         **
*******************************************************************************/

/*******************************************************************************
**                      Global Constant Definitions                           **
*******************************************************************************/

/*******************************************************************************
**                      Private Constant Definitions                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Variable Definitions                           **
*******************************************************************************/
extern const ADC_MEASUREMENT_ISR_t backgnd_rs_intr_handle; /**< global ADC-NVIC struct  */

CONTROLLERPOLYNOMIALS polynoms;/**< global structure for controler polynomials*/

//DPS3100 Pressure-Sensor
float pressure = 0.0f;/**< global variable for storing current pressure value*/
float temperature = 0.0f;/**< global variable for storing current temperature value*/
struct structFIR PressureFIR;/**< global struct for filtering pressure values*/
float YPR[3];
float powerD = 0.0f;/* throttle value of remote, range: 0 - 100*/
float yawD_dot = 0.0f;/* yaw value of remote, range: -90 - +90*/
float pitchD = 0.0f;/* pitchD value of remote, range: -30 - +30*/
float rollD = 0.0f;/* rollD value of remote, range: -30 - +30*/
float u_yaw_dot = 0.0f;/* output of angle-rate controller for yaw*/
float u_pitch = 0.0f;/* output of angle controller for pitch*/
float u_roll = 0.0f;/* output of angle controller for roll*/
float PWM_percent[4];/* scaled motor power from controller output, range: 0 - 100*/
uint8_t height_control = 0;/* for enabling height-control, activated: 0xff, disabled: 0x00*/

#ifdef Plexi
CONTROLLERPARAMETER parameter =
{
		.T=0.002f,
		.P_roll=70.0f,
		.I_roll=0.2f,
		.D_roll=1.7f,
		.N_roll=400.0f,
		.P_pitch=70.0f,
		.I_pitch=0.2f,
		.D_pitch=1.7f,
		.N_pitch=400.0f,
		.P_yaw=200.0f
};
#elif defined HTL
CONTROLLERPARAMETER parameter =
{
             .T=0.002f,
             .P_roll=40.0f,
             .I_roll=0.1f,
             .D_roll=1.6f,
             .N_roll=400.0f,
             .P_pitch=40.0f,
             .I_pitch=0.1f,
             .D_pitch=1.6f,
             .N_pitch=400.0f,
             .P_yaw=100.0f
};
#elif defined Flatcopter
CONTROLLERPARAMETER parameter =
{
             .T=0.002f,
             .P_roll=35.0f,
             .I_roll=0.1f,
             .D_roll=1.2f,
             .N_roll=400.0f,
             .P_pitch=35.0f,
             .I_pitch=0.1f,
             .D_pitch=1.2f,
             .N_pitch=400.0f,
             .P_yaw=100.0f
};
#elif defined Carbon
CONTROLLERPARAMETER parameter =
{
		.T=0.002f,
		.P_roll=125.0f,
		.I_roll=0.0f,
		.D_roll=4.0f,
		.N_roll=400.0f,
		.P_pitch=125.0f,
		.I_pitch=0.0f,
		.D_pitch=4.0f,
		.N_pitch=400.0f,
		.P_yaw=200.0f
};
#endif


/*******************************************************************************
**                      Private Variable Definitions                          **
*******************************************************************************/

/*******************************************************************************
**                      Global Function Definitions                           **
*******************************************************************************/

/**
 *  \brief Interrupt-Service-Routine of the Control_Timer
 *
 *  \details Central Control Routine for Quadcopter\n
 *  This routine gets the momentary position and the desired position \n
 *  and calculates new output for motors and transmits it via DaisyChain to the \n
 *  Electric Speed Controller
 *
 */
void Control_Timer_ISR(void)
{
	static float x_pitch[CONTROL_ORDER];
	static float x_roll[CONTROL_ORDER];

	GetAngles(YPR);
	GetRCData(&powerD, &height_control, &yawD_dot, &pitchD, &rollD);
	//yaw control
	AngleRateController(&yawD_dot, &YPR[0], &parameter.P_yaw, &u_yaw_dot);
	//pitch control
	AngleController(&pitchD, &YPR[1], CONTROL_ORDER, polynoms.a_pitch, polynoms.b_pitch, x_pitch, &u_pitch);
	//roll control
	AngleController(&rollD, &YPR[2], CONTROL_ORDER, polynoms.a_roll, polynoms.b_roll, x_roll, &u_roll);
	//generate actuator values
	CreatePulseWidth(&u_roll, &u_pitch, &u_yaw_dot, &powerD, PWM_percent);

	if (powerD > 5.0f)
		SendDaisyPWMPercentages(PWM_percent);
	else
		SendDaisyStopCommand();

}

/**
 *  \brief PID controller for angle based quadcopter stabilization
 *  
 *  \param [in] r Reference inputs (RC)
 *  \param [in] y Current output value of the sensor
 *  \param [in] n Degree of control polynomial
 *  \param [in] a Coefficient of the numerator and denominator polynomials of the controller
 *  \param [in] b Coefficient of the numerator and denominator polynomials of the controller
 *  \param [in] x
 *  \param [in] u Controller output
 *  
 */
void AngleController(float *r, float *y, int n, const float *a, const float *b,float *x, float *u)
{
	//PID-Controller

	//control error
	float e = (*r - *y) * (float)M_PI / 180.0f;

	//calculate plant input
	*u = x[n - 1] + b[n] * e;

	//calculate new coefficients
	for (int i = n - 1; i > 0; i--)
		x[i] = b[i] * e - a[i] * (*u) + x[i - 1];

	x[0] = b[0] * e - a[0] * (*u);

	*u /= 4.0f;
}
/**
 *  \brief This function represents the proportional controller for yaw-movement of the quadcopter
 *  
 *  \param [in] r Reference inputs (RC)
 *  \param [in] y Current output value of the sensor
 *  \param [in] P Value of P_yaw
 *  \param [in] u Controller output
 *  
 */
void AngleRateController(float *r, float *y, const float *P, float *u)
{
	//P-Controller
	*u = (*r - *y) * M_PI / (180.0f * 4.0f) * *P;
}
/**
 *  \brief This function calculates and limits the PWM signals of the 4 motors, based on the 3 controller outputs
 *  
 *  \param [in] u_phi Output of third controller (u_roll)
 *  \param [in] u_theta Output of second controller (u_pitch)
 *  \param [in] u_psi_dot Output of third controller (u_yaw_dot)
 *  \param [in] u_hover Power 
 *  \param [in] PWM_width Desired width of the PWM
 *  
 *  \details Roll, pitch and yaw movements are are based on the linearized mathematical model of the system
 */
void CreatePulseWidth(float *u_phi, float *u_theta, float *u_psi_dot,float *u_hover, float *PWM_width)
{
	float saturationMax = 100.0f;
	float saturationMin = 10.0f;

	if (*u_hover > 5.0f)
	{
		PWM_width[0] = -*u_theta + *u_phi - *u_psi_dot + *u_hover;
		PWM_width[1] = -*u_theta - *u_phi + *u_psi_dot + *u_hover;
		PWM_width[2] = *u_theta + *u_phi + *u_psi_dot + *u_hover;
		PWM_width[3] = *u_theta - *u_phi - *u_psi_dot + *u_hover;
	}
	else
	{
		PWM_width[0] = *u_hover;
		PWM_width[1] = *u_hover;
		PWM_width[2] = *u_hover;
		PWM_width[3] = *u_hover;
	}

	if (PWM_width[0] > saturationMax)
		PWM_width[0] = saturationMax;

	if (PWM_width[0] < saturationMin)
		PWM_width[0] = saturationMin;

	if (PWM_width[1] > saturationMax)
		PWM_width[1] = saturationMax;

	if (PWM_width[1] < saturationMin)
		PWM_width[1] = saturationMin;

	if (PWM_width[2] > saturationMax)
		PWM_width[2] = saturationMax;

	if (PWM_width[2] < saturationMin)
		PWM_width[2] = saturationMin;

	if (PWM_width[3] > saturationMax)
		PWM_width[3] = saturationMax;

	if (PWM_width[3] < saturationMin)
		PWM_width[3] = saturationMin;
}
