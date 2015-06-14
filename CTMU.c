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

/*************Interrupt Prototypes***************/
/*************Function  Prototypes***************/
/************* Device Definitions ***************/
/************* Module Definitions ***************/
/************* Other  Definitions ***************/

void CTMU_Initialize(void)
{
	int loop;

	for(loop = 0; loop < NUMBER_OF_A2D_PINS; ++loop)
		pinTranslator[loop] = -1;

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
	AD1PCFG &= ~(1 << channel);	//Configure pin for analog mode
	if(pinTranslator[channel] = -1)
		TRISB |= 1 << (channel);	//Set pin to input
	else
		Pin_Set_TRIS(pinTranslator[channel], INPUT);

	//2. Start sampling the A2D
	AD1CON1bits.SAMP = 1;

	//3. initiate charging of circuit
	CTMUCONbits.EDG2STAT = 0; // Make sure edge2 is 0
	CTMUCONbits.EDG1STAT = 1; // Set edge1 - Start Charge

	//4. Wait for circuit to charge
	for (loop = 0; loop < 15; loop++) //Delay for CTMU charge time
		Nop();

	//5. Stop charging and finish sampling
	CTMUCONbits.EDG1STAT = 0; //Clear edge1 - Stop Charge
	AD1CON1bits.SAMP = 0;

	return;
}

void CTMU_Discharge(int channel)
{
	//Clear all residual charge from the circuit
	AD1PCFG |= 1 << channel;	//Configure pin for digital mode

	if(pinTranslator[channel] = -1)
	{
		LATB &= ~(1 << (channel));	//Set pin to low
		TRISB &= ~(1 << (channel));	//Set pin to output
	}
	else
	{
		Pin_Low(pinTranslator[channel]);
		Pin_Set_TRIS(pinTranslator[channel],OUTPUT);
	}

	return;
}
