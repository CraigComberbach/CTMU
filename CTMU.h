#ifndef CTMU_H
#define	CTMU_H

/***********Add to config file header************/
/*
//* Library
#define CTMU_MAJOR	0
#define CTMU_MINOR	0
#define CTMU_PATCH	0
*/

/***************Add to config file***************/
/*
#ifndef CTMU_LIBRARY
	#error "You need to include the CTMU library for this code to compile"
#endif
 */

/************* Semantic Versioning***************/
#define CTMU_LIBRARY

/*************   Magic  Numbers   ***************/
/*************    Enumeration     ***************/
/***********State Machine Definitions************/
/*************Function  Prototypes***************/
void CTMU_Initialize(void);
void CTMU_Start(int channel);
void CTMU_Discharge(int channel);
void Pin_Translation(enum A2D_PIN_DEFINITIONS analogPin, enum PIN_DEFINITIONS physicalPin);
int CTMU_Auto_Calibrate(int channel, int currentValue, int targetValue);

#endif	/* CTMU_H */
