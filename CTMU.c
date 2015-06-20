/**************************************************************************************************
Target Hardware:		PIC24Fxxxx
Code assumptions:
Purpose:
Notes:

Version History:
vnext	Y-M-D	Craig Comberbach	Compiler: C30 v3.31	Optimization: 0	IDE: MPLABx 1.95	Tool: RealICE	Computer: Intel Xeon CPU 3.07 GHz, 6 GB RAM, Windows 7 64 bit Professional SP1
	First version
**************************************************************************************************/
/*************    Header Files    ***************/
#include "Config.h"
#include "CTMU.h"
#include "../Pins/Pins.h"
#include "../A2D/A2D.h"

/************* Library Definition ***************/
/************* Semantic Versioning***************/
#if CTMU_MAJOR != 0
	#error "lib-p_* has had a change that loses some previously supported functionality"
#elif CTMU_MINOR != 0
	#error "lib-p_* has new features that your code may benefit from"
#elif CTMU_PATCH != 0
	#error "lib-p_* has had a bug fix, you should check to see that you weren't relying on a bug for functionality"
#endif

//This code requires a specific version of the A2D library code to be guaranteed to work correctly
#ifndef A2D_LIBRARY
	#error "You need to include the A2D library for this code to compile"
#elif A2D_MAJOR != 1
	#error "A2D.c has had a change that loses some previously supported functionality"
#elif A2D_MINOR != 1
	#error "A2D.c has new features that this code may benefit from"
#elif A2D_PATCH != 0
	#error "A2D.c has had a bug fix, you should check to see that we weren't relying on a bug for functionality"
#endif

//This code requires a specific version of the Pins library code to be guaranteed to work correctly
#ifndef PINS_LIBRARY
	#error "You need to include the Pins library for this code to compile"
#elif PINS_MAJOR != 2
	#error "Pins.c has had a change that loses some previously supported functionality"
#elif PINS_MINOR != 0
	#error "Pins.c has new features that this code may benefit from"
#elif PINS_PATCH != 0
	#error "Pins.c has had a bug fix, you should check to see that we weren't relying on a bug for functionality"
#endif

/************Arbitrary Functionality*************/
/*************   Magic  Numbers   ***************/
/*************    Enumeration     ***************/
/***********  Structure Definitions  ************/
/***********State Machine Definitions************/
/*************  Global Variables  ***************/
int pinTranslator[NUMBER_OF_A2D_PINS];
int calibratedTrigger[NUMBER_OF_A2D_PINS];
unsigned long current_nA = 0;
unsigned long calibrationResistorOhms = 0;

/*************Interrupt Prototypes***************/
/*************Function  Prototypes***************/
/************* Device Definitions ***************/
/************* Module Definitions ***************/
/************* Other  Definitions ***************/

int CTMU_Auto_Calibrate(int channel, int currentValue, int targetValue)
{
	static unsigned int born = 1;

	if(born)
	{
		if(currentValue >= targetValue)
		{
			born = 0;
			calibratedTrigger[channel]--;
		}
		else if(currentValue < targetValue)
			calibratedTrigger[channel]++;
	}

	return born;
}

void CTMU_Initialize(unsigned long valueOhms)
{
	int loop;

	for(loop = 0; loop < NUMBER_OF_A2D_PINS; ++loop)
	{
		pinTranslator[loop] = -1;
		calibratedTrigger[loop] = 0;
	}

	//Specify the value of the calibration resistor in ohms
	calibrationResistorOhms = valueOhms;

	//CTMUCON
	CTMUCONbits.CTMUEN		= 0; //make sure CTMU is disabled
	CTMUCONbits.CTMUSIDL	= 0; //CTMU continues to run in idle mode
	CTMUCONbits.TGEN		= 0; //disable edge delay generation mode of the CTMU
	CTMUCONbits.EDGEN		= 0; //edges are blocked
	CTMUCONbits.EDGSEQEN	= 0; //edge sequence not needed
	CTMUCONbits.IDISSEN		= 0; //Do not ground the current source
	CTMUCONbits.CTTRIG		= 0; //Trigger Output is disabled
	CTMUCONbits.EDG2POL		= 0;
	CTMUCONbits.EDG2SEL		= 0; //Edge2 Src = OC1 (don't care)
	CTMUCONbits.EDG1POL		= 0;
	CTMUCONbits.EDG1SEL		= 0; //Edge1 Src = Timer1 (don?t care)

	//CTMUICON
	CTMUICONbits.IRNG		= 3; //55uA
	CTMUICONbits.ITRIM		= 0; //Nominal - No Adjustment
	CTMUCONbits.CTMUEN		= 1; //Enable CTMU

	return;
}

void Pin_Translation(enum A2D_PIN_DEFINITIONS analogPin, enum PIN_DEFINITIONS physicalPin)
{
	pinTranslator[analogPin] = physicalPin;

	return;
}

void CTMU_Start(int channel)
{
	int loop;

	//1. Configure for sampling
	AD1PCFG &= ~(1 << channel);					//Configure pin for analog mode
	Pin_Set_TRIS(pinTranslator[channel], INPUT);//Set pin to input

	//2. Start sampling the A2D
	AD1CON1bits.SAMP = 1;

	//3. initiate charging of circuit
	CTMUCONbits.EDG2STAT = 0; //Make sure edge2 is 0
	CTMUCONbits.EDG1STAT = 1; //Set edge1 - Start Charge
ClrWdt();
	//4. Wait for circuit to charge
	for (loop = 0; loop < calibratedTrigger[channel]; loop++) //Delay for CTMU charge time
		Nop();
ClrWdt();
	//5. Stop charging and finish sampling
//	CTMUCONbits.EDG1STAT = 0; //Clear edge1 - Stop Charge
	AD1CON1bits.SAMP = 0;

	return;
}

void CTMU_Discharge(int channel)
{
	//Clear all residual charge from the circuit
	//Configure pin for digital mode
	AD1PCFG |= 1 << channel;

	//Set pin to output, then low
	Pin_Low(pinTranslator[channel]);
	Pin_Set_TRIS(pinTranslator[channel],OUTPUT);

	return;
}

void CTMU_Calibrate_Constant_Current_Supply(int channel)
{
	//We need to ensure that the calibration resistor has been accounted for
	if(calibrationResistorOhms != 0)
	{
		//Convert the A2D reading into a known calibrated current
		current_nA = (A2D_Value(channel) * 62500);	//Make it as big as possible to retain resolution later
		current_nA += 55095;						//Add an algorithmically chosen value to allow for proper rounding and reduce the error to +/- 887 pA
//		current_nA /= calibrationResistorOhms;		//Divide by the known resistance
		current_nA *= 25;							//Final multiplier to allow the final division to occur
		current_nA /= 31;							//Final answer, accurate to 1nA +/- 0.887nA
	}

	return;
}

int CTMU_Calibration_Format(int value)
{
	/*Don't do anything. I have to use raw format, because the A2D library will not accept a uV format because it exceeds
	 and integer in size. But I don't want to have to explain that to whoever uses the library. It is easier to just say
	 "Use the specific CTMU Calibration Format" and have everything work out. Besides, it makes the code easier to read
	 that way.*/
	return value;
}
