//Document initialize functionettes
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

void CTMU_Initialize(void)
{
	//CTMUCON
	CTMUCONbits.CTMUEN		= 0;
	CTMUCONbits.CTMUSIDL	= 0;
	CTMUCONbits.TGEN		= 0;
	CTMUCONbits.EDGEN		= 0;
	CTMUCONbits.EDGSEQEN	= 0;
	CTMUCONbits.IDISSEN		= 0;
	CTMUCONbits.CTTRIG		= 0;
	CTMUCONbits.EDG2POL		= 0;
	CTMUCONbits.EDG2SEL		= 0;
	CTMUCONbits.EDG1POL		= 0;
	CTMUCONbits.EDG1SEL		= 0;

	//CTMUICON
	CTMUICONbits.IRNG		= 3; //55uA
	CTMUICONbits.ITRIM		= 0; //Nominal - No Adjustment
	CTMUCONbits.CTMUEN		= 1; //Enable CTMU

	return;
}

void CTMU_Start(int channel)
{
	int loop;

	AD1CON1bits.ADON = 0;
	AD1CON3bits.SAMC = 0b00001;
	AD1CON1bits.ADON = 1;

	//1.
//	AD1PCFGLbits.PCFG3 = ~0;
//	AD1PCFGL |= 1 << channel;
//	LATBbits.LATB3 = 0;
//	TRISBbits.TRISB3 = 0;
//	for (loop = 0; loop < 100; loop++) //Delay for CTMU charge time
		Nop();

	//2.
//	AD1PCFGLbits.PCFG3 = 0;
//	AD1PCFGL &= ~(1 << channel);
//	TRISBbits.TRISB3 = ~0;
	AD1CHS = channel;

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
