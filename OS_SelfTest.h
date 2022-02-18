//********************************************************************************
/*!
\author     Kraemere E
\date       13.04.2021

\file       SelfTest.handler
\brief      Module for Testroutines

***********************************************************************************/
#ifndef SELFTEST_H
#define SELFTEST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "BaseTypes.h"
#include "OS_Config.h"


#ifdef USE_SELFTEST
/**************************************************************
*
* Tests routines are executed at startup or during normal operation.
* The routines where implemented to fullfil the requirements of DIN 60730.
* There are two groups of tests: Tests that run once befor entry of main()
*                                Tests called cyclically during execution of main()- loop
*
***************************************************************/

/***************************************
 * Type Defs for all Selftests
 ***************************************/

typedef enum
{
   eSelfTest_OK,                        // STest ok
   eSelfTest_ERROR,                     // STest Error
   eSelfTest_NotExecuted                // STest Not executed
} teSelfTest_ResultCode;


/*******************************************
 * Status flags to return function results *
 *******************************************/

#define OK_STATUS                   (0u)
#define ERROR_STATUS                (1u)
#define PASS_STILL_TESTING_STATUS   (2u)
#define PASS_COMPLETE_STATUS        (3u)

//******************************
//* Defines for timing tests   *
//******************************

#if ( ST_TESTPIN_ACTIVE )
    #if ( CONTROL_LOW_VOLT )
        #define ST_TESTPIN_ON             Pin_6_0_Write( 1 );     // Switch on test pin
        #define ST_TESTPIN_OFF            Pin_6_0_Write( 0 );     // Switch on test pin
    #endif
    #if ( CONTROL_HIGH_VOLT )
        #define ST_TESTPIN_ON             CyPins_SetPin(Triac_T_9);   // Switch on test pin
        #define ST_TESTPIN_OFF            CyPins_ClearPin(Triac_T_9); // Switch on test pin
    #endif
    #if ( CONTROL_SYS_CON )
        #define ST_TESTPIN_ON             Pin_4_0_Write(1);          // Switch on test pin
        #define ST_TESTPIN_OFF            Pin_4_0_Write(0);          // Switch on test pin
    #endif
#else
    #define ST_TESTPIN_ON                                     // Switch on test pin
    #define ST_TESTPIN_OFF                                    // Switch on test pin
#endif

/***************************************
 * Variables
 ***************************************/

// Variables used in isr_1 test interrupt handler
static volatile u32 uSfTtInterruptCnt;

/***************************************
* Function Prototypes
***************************************/



void OS_SelfTest_InitCyclic(void);
void OS_SelfTest_Cyclic_Run(void);
void OS_SelfTest_StartCallback(void);

#endif

#ifdef __cplusplus
}
#endif

#endif // SELFTEST_H

/* [] END OF FILE */

