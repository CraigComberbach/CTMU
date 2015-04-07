/**************************************************************************************************
Target Hardware:		PIC24FJ256GA1xx
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
	CTMUCONbits.CTMUEN =  0; //make sure CTMU is disabled
	CTMUCONbits.CTMUSIDL = 0; //CTMU continues to run in idle mode
	CTMUCONbits.TGEN =  0; //disable edge delay generation mode of the CTMU
	CTMUCONbits.EDGEN =  0; //edges are blocked
	CTMUCONbits.EDGSEQEN = 0; //edge sequence not needed
	CTMUCONbits.IDISSEN =  0; //Do not ground the current source
	CTMUCONbits.CTTRIG =  0; //Trigger Output is disabled
	CTMUCONbits.EDG2POL =  0;
	CTMUCONbits.EDG2SEL =  0x3; //Edge2 Src = OC1 (don?t care)
	CTMUCONbits.EDG1POL =  1;
	CTMUCONbits.EDG1SEL =  0x3; //Edge1 Src = Timer1 (don?t care)

	//CTMUICON
	CTMUICONbits.IRNG = 3; //55uA
	CTMUICONbits.ITRIM = 0; //Nominal - No Adjustment
	CTMUCONbits.CTMUEN = 1; //Enable CTMU

	return;
}

void CTMU_Start(int channel)
{
	int loop;
	int loop2;
	//Set the pin to Analog
	Pin_Set_TRIS(PIN_PRESENCE_SENSOR, INPUT);
	AD1CON1bits.ADON = 0;
	AD1PCFG &= ~(1 << CAPACITIVE_PRESENSE_SENSOR);
	AD1CON1bits.ADON = 1;
	Nop();	Nop();	Nop();	Nop();
	Nop();	Nop();	Nop();	Nop();
	Nop();	Nop();	Nop();	Nop();
	Nop();	Nop();	Nop();	Nop();

	CTMUCONbits.IDISSEN = 1;

	Nop();	Nop();	Nop();	Nop();
	Nop();	Nop();	Nop();	Nop();

	CTMUCONbits.IDISSEN = 0;

	CTMUCONbits.EDG2STAT = 0; // Make sure edge2 is 0
	CTMUCONbits.EDG1STAT = 1; // Set edge1 - Start Charge

//	for (loop = 0; loop < 11988; loop++) // Delay for CTMU charge time
	for (loop2 = 0; loop2 < 40; loop2++) // Delay for CTMU charge time
	{
		for (loop = 0; loop < 20000; loop++) // Delay for CTMU charge time
			asm volatile("nop");
		ClrWdt();
	}
	CTMUCONbits.EDG1STAT = 0; //Clear edge1 - Stop Charge
	return;
}

void CTMU_Stop(int channel)
{
	//Set the pin to digital
	AD1CON1bits.ADON = 0;
	AD1PCFG |= (1 << CAPACITIVE_PRESENSE_SENSOR);
	AD1CON1bits.ADON = 1;
	Pin_Low(PIN_PRESENCE_SENSOR);
	Pin_Set_TRIS(PIN_PRESENCE_SENSOR, OUTPUT);

	return;
}
