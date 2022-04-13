//********************************************************************************
/*!
\author     Kraemere E
\date       13.04.2021

\file       SelfTest.handler
\brief      Module for Testroutines
            Tests routines are executed at startup or during normal operation.
*           The routines where implemented to fullfil the requirements of DIN 60730.
*           There are two groups of tests: Tests that run once befor entry of main()
*           Tests called cyclically during execution of main()- loop
***********************************************************************************/
#ifndef SELFTEST_H
#define SELFTEST_H


#include "OS_Config.h"
#ifdef USE_OS_SELF_TEST
/********************************* includes **********************************/

#include "BaseTypes.h"
/***************************** defines / macros ******************************/
//Status flags to return function results
#define OK_STATUS                   (0u)
#define ERROR_STATUS                (1u)
#define PASS_STILL_TESTING_STATUS   (2u)
#define PASS_COMPLETE_STATUS        (3u)
    
    
// Defines for timing tests 
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
    
/****************************** type definitions *****************************/
typedef enum
{
   eSelfTest_OK,                        // STest ok
   eSelfTest_ERROR,                     // STest Error
   eSelfTest_NotExecuted                // STest Not executed
} teSelfTest_ResultCode;

/***************************** global variables ******************************/
// Variables used in isr_1 test interrupt handler
static volatile u32 uSfTtInterruptCnt;

/************************ externally visible functions ***********************/
#ifdef __cplusplus
extern "C"
{
#endif

void OS_SelfTest_InitCyclic(void);
void OS_SelfTest_Cyclic_Run(void);
void OS_SelfTest_StartCallback(void);

#ifdef __cplusplus
}
#endif

#endif //USE_OS_SELF_TEST

#endif // SELFTEST_H


