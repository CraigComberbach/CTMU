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

/************* Library Definition ***************/
/************* Semantic Versioning***************/
#if CTMU_MAJOR != 0
	#error "lib-p_* has had a change that loses some previously supported functionality"
#elif CTMU_MINOR != 0
	#error "lib-p_* has new features that your code may benefit from"
#elif CTMU_PATCH != 0
	#error "lib-p_* has had a bug fix, you should check to see that you weren't relying on a bug for functionality"
#endif

/************Arbitrary Functionality*************/
/*************   Magic  Numbers   ***************/
/*************    Enumeration     ***************/
/***********  Structure Definitions  ************/
/***********State Machine Definitions************/
/*************  Global Variables  ***************/
/*************Interrupt Prototypes***************/
/*************Function  Prototypes***************/
/************* Device Definitions ***************/
/************* Module Definitions ***************/
/************* Other  Definitions ***************/

void Configure_CTMU(void)
{
	//CTMUCON
	CTMUCONbits.CTMUEN		= 0; //make sure CTMU is disabled
	CTMUCONbits.CTMUSIDL	= 0; //CTMU continues to run in idle mode
	CTMUCONbits.TGEN		= 0; //disable edge delay generation mode of the CTMU
	CTMUCONbits.EDGEN		= 0; //edges are blocked
	CTMUCONbits.EDGSEQEN	= 0; //edge sequence not needed
	CTMUCONbits.IDISSEN		= 0; //Do not ground the current source
	CTMUCONbits.CTTRIG		= 0; //Trigger Output is disabled
	CTMUCONbits.EDG2POL		= 0;
	CTMUCONbits.EDG2SEL		= 0; //Edge2 Src = OC1 (don?t care)
	CTMUCONbits.EDG1POL		= 0;
	CTMUCONbits.EDG1SEL		= 0; //Edge1 Src = Timer1 (don?t care)

	//CTMUICON
	CTMUICONbits.IRNG		= 3; //55uA
	CTMUICONbits.ITRIM		= 0; //Nominal - No Adjustment
	CTMUCONbits.CTMUEN		= 1; //Enable CTMU

	return;
}

void CTMU_Start(int channel)
{
	int loop;
	capacitivePresense = A2D_Value(channel);

	TRISBbits.TRISB4 = 0;
	if(capacitivePresense < 715)
		LATBbits.LATB4 = 1;
	else
		LATBbits.LATB4 = 0;

	AD1CON1bits.ADON = 0;
	AD1CON3bits.SAMC = 0b00001;
	AD1CON1bits.ADON = 1;

	//1.
	AD1PCFGLbits.PCFG3 = ~0;
	LATBbits.LATB3 = 0;
	TRISBbits.TRISB3 = 0;
	for (loop = 0; loop < 100; loop++) //Delay for CTMU charge time
		Nop();

	//2.
	AD1PCFGLbits.PCFG3 = 0;
	TRISBbits.TRISB3 = ~0;
	AD1CHS = 3;

	//3.
	AD1CSSL |= 1 << channel;

	//4.
	CTMUCONbits.IDISSEN = 1;

	//5.
	Nop();	Nop();	Nop();	Nop();	Nop();

	//6.
	CTMUCONbits.IDISSEN = 0;

//	//7.
	AD1CON1bits.SAMP = 1;

	//8.
	CTMUCONbits.EDG2STAT = 0; // Make sure edge2 is 0
	CTMUCONbits.EDG1STAT = 1; // Set edge1 - Start Charge

	//9.
	for (loop = 0; loop < 10; loop++) //Delay for CTMU charge time
		Nop();

	//10.
	CTMUCONbits.EDG1STAT = 0; //Clear edge1 - Stop Charge
	AD1CON1bits.SAMP = 0;

	//11.

	return;
}

void CTMU_Stop(int channel)
{
//	//Set the pin to digital
////	AD1PCFGL = 0xFFFF;
//	AD1PCFGL |= (1 << CAPACITIVE_PRESENSE_SENSOR);
//	Pin_Low(PIN_PRESENCE_SENSOR);
//	Pin_Set_TRIS(PIN_PRESENCE_SENSOR, OUTPUT);

	return;
}
