//********************************************************************************
/*!
\author
\date

\file       OS_SelfTest
\details    Handles the self-tests.
*/

#include "OS_SelfTest.h"
#include "OS_ErrorDebouncer.h"
#include "OS_StateManager.h"

#include "HAL_MemoryInit.h"
#include "HAL_SelfTest_Stack.h"
#include "HAL_SelfTest_TimeBase.h"
#include "HAL_SelfTest_Memory.h"
#include "HAL_SelfTest_CPU.h"

/****************************************** Defines ******************************************************/
//*******************************************
//* Defines for error generation in tests   *
// If error in SelfTests should be generated set the directive to 1
// For errors in SelfTest_SRAM_March.s file set the apropriate in assembler file too
#define SET_ERROR_IN_CPUREG             0       // Set failure during CPU_Reg test
#define SET_ERROR_IN_CPUPC              0       // Set failure during CPU_PC test
#define SET_ERROR_IN_INTERRUPT          0       // Set failure during Interrupt test
#define SET_ERROR_IN_RAM                0       // Set failure during SRAM March tests
#define SET_ERROR_IN_STACK              0       // Set failure during Stack March tests
#define SET_ERROR_IN_FLASH              0       // Set failure during Flash test
#define SET_ERROR_IN_IO                 0       // Set failure during IO test
#define SET_ERROR_IN_UREG               0       // Set failure during UDB config register test
#define SET_ERROR_IN_ADC                0       // Set failure during ADC test
#define SET_ERROR_IN_UART               0       // Set failure during UART test


#define SELFTEST_ID_DELTA   10


typedef enum                                 // Always seperate additional tests witch SELFTEST_ID_DELTA
{
   eSelfTest_ID_INIT   = 0u,                                            // STest init
   eSelfTest_ID_CPUREG = eSelfTest_ID_INIT + SELFTEST_ID_DELTA,         // STest CPU register
   eSelfTest_ID_CPUPC = eSelfTest_ID_CPUREG + SELFTEST_ID_DELTA,        // STest CPU program counter
   eSelfTest_ID_TIMEBASE = eSelfTest_ID_CPUPC + SELFTEST_ID_DELTA,    // STest Timebase test
   eSelfTest_ID_RAM0 = eSelfTest_ID_TIMEBASE + SELFTEST_ID_DELTA,     // STest SRAM area
   eSelfTest_ID_RAM1,                                                   // STest SRAM exec
   eSelfTest_ID_RAM2,                                                   // STest SRAM exec
   eSelfTest_ID_STACK = eSelfTest_ID_RAM0 + SELFTEST_ID_DELTA,         // STest STACK exec
   eSelfTest_ID_STACKOVF = eSelfTest_ID_STACK + SELFTEST_ID_DELTA,     // STest STACK limit exec
   eSelfTest_ID_FLASH0 = eSelfTest_ID_STACKOVF + SELFTEST_ID_DELTA,     // STest Flash 0
   eSelfTest_ID_FLASH1,                                                 // STest Flash 1
   eSelfTest_ID_IO0 = eSelfTest_ID_FLASH0 + SELFTEST_ID_DELTA,          // STest IO
   eSelfTest_ID_IO1,                                                    // STest IO
   eSelfTest_ID_UREG0 = eSelfTest_ID_IO0 + SELFTEST_ID_DELTA,           // STest UDB Config Register
   eSelfTest_ID_UREG1,                                                  // STest UDB Config Register
   eSelfTest_ID_ADC0 = eSelfTest_ID_UREG0 + SELFTEST_ID_DELTA,          // STest ADC
   eSelfTest_ID_UART0 = eSelfTest_ID_ADC0 + SELFTEST_ID_DELTA,          // STest UART0
   eSelfTest_ID_UART1,                                                  // STest Uart1
   eSelfTest_ID_EXIT = eSelfTest_ID_UART0 + SELFTEST_ID_DELTA,          // STest exit
}teSelfTest_ID;

//***** Structure for cyclic test *********
//
typedef struct
{
    s32 slTestCount;
    u32 ulTestAddress;
    u32 ulTestSize;
    u32 ulTestOffset;
    u32 ulTestResult;
    teSelfTest_ID eTestID;
    teSelfTest_ResultCode eResultCode;
} tsSelfTest_State;

//*** Structure for cyclic test logging *****
//
typedef struct
{
    teSelfTest_ID eTestID;
    teSelfTest_ResultCode eResultCode;
} tsSelfTest_LogVal;

typedef struct
{
        tsSelfTest_LogVal sActualResult;
        tsSelfTest_LogVal sLastResult;
} tsSelfTest_LOG;





/****************************************** Variables ****************************************************/
static volatile u32 ulResetReason = 0;

// Timertick counter
static u32 ulSfT_Time_0 = 0;   //Self-Test start time tick
static u32 ulSfT_Time_1 = 0;   //Duration result for the cyclic self test


// Test State controlling the Sequence of Selftests
static tsSelfTest_State sSfT_State;
static tsSelfTest_State *psSfT_State  = &sSfT_State;


// Result set containing the test results of the last test executed,
// the actual running test and the next text to execute
static tsSelfTest_LOG sSfT_Log;
static tsSelfTest_LOG *psSfT_Log = &sSfT_Log;

// Variables used in flash tests
extern u32 ulAppCodeLength;
extern u32 ulAppCodeStart;





/****************************************** Function prototypes ******************************************/

extern u32 OS_SW_Timer_GetSystemTickCount( void );

/****************************************** local functions *********************************************/




//********************************************************************************
/*!
\author     Kraemer E.
\date       13.04.2021
\brief      Logs the state of the self-test
\return     eSelfTest_OK
\param      eNextID - The next self-test step
\param      eResultCode - The result code of the current step
**********************************************************************************/
void TestLog( teSelfTest_ID eNextID, teSelfTest_ResultCode eResultCode)
{
    psSfT_Log->sActualResult.eTestID = psSfT_State->eTestID;
    psSfT_Log->sActualResult.eResultCode = eResultCode;
    psSfT_Log->sLastResult = psSfT_Log->sActualResult;                // Shift actual result to old, new result to last active
    psSfT_State->eTestID = eNextID;
    psSfT_Log->sActualResult.eResultCode = eSelfTest_NotExecuted;
}




/****************************************** External visible functions **********************************/
//********************************************************************************
/*!
\author     Kraemer E.
\date       20.01.2019
\brief      Handles the self tests on the start up of the system.
            When everything is fine the main taks is called.
\return     none
\param      none
***********************************************************************************/
void OS_SelfTest_StartCallback(void)
{

    #if SELFTEST_S_ENABLE
        /***** do CPU Register test before memory initialization *******/
        #if EXEC_STARTUP_CPUREG
            if(HAL_SelfTest_CPU_Reg())
            {
                 while(1u);               // !!! Stop on error.
            }
        #endif

        /***** do CPU program counter test before memory initialization ******/
        #if EXEC_STARTUP_CPUPC
            if (HAL_SelfTest_CPU_PC())
            {
                while(1u);                // !!! Stop on error.
            }
        #endif

        /****** do SRAM test before memory initialization *******/
        #if EXEC_STARTUP_RAM
            if (HAL_SelfTest_RAM())          // !!! don't return on error.
            {
                while(1u);                // !!! Stop on error.
            }
        #endif

        /****** do Stack test before memory initialization *******/
        #if EXEC_STARTUP_STACK
            /* Initialize self-test stack */
            HAL_SelfTest_Stack_Init();

            if (HAL_SelfTest_Stack_Check())         // !!! don't return on error.
            {
                while(1u);               // !!! Stop on error.
            }
        #endif
    #endif  // SELFTEST_S_ENABLE

    /* Initialize standard memory */
    HAL_MemoryInit_Init();
    HAL_SelfTest_Stack_Init();

    #if SELFTEST_S_ENABLE

        //***** do Interrupt and time-base test AFTER memory initialization
        // Using CyDelayUS needs initialized memory !!! //
        #if EXEC_STARTUP_INTERRUPT
            if(HAL_SelfTest_TimeBase())
            {
                while(1u);                // !!! Stop on error.
            }
        #endif

        //***** do FLASH test AFTER memory initialization
        // Needs initialized memory !!! //
        #if EXEC_STARTUP_FLASH
            if ( HAL_SelfTest_Flash()!= eSelfTest_OK )
            {
                while(1u);               // !!! Stop on error.
            }
        #endif

        //***** do ADC SAR test AFTER memory initialization ! //
        #if EXEC_STARTUP_ADC
            if(HAL_SelfTest_ADC())
            {
                while(1u);                // !!! Stop on error.
            }
        #endif

        //***** do UART_1 test AFTER memory initialization ! //
        #if EXEC_STARTUP_UART
            HAL_SelfTest_UART_Init()
            if (HAL_SelfTest_UART_Check())
            {
                while(1u);                // !!! Stop on error.
            }
        #endif

    #endif  // SELFTEST_S_ENABLE

     /* Start with OS-Main-Task */
    (void)main();

    while(1);        // If main returns, make sure we don't return.
}


//********************************************************************************
/*!
\author     Kraemer E.
\date       20.01.2019
\brief      Handles the self tests on the start up of the system.
            When everything is fine the main taks is called.
\return     none
\param      none
***********************************************************************************/
void OS_SelfTest_Cyclic_Run(void)                       // Run the sequence of cyclic tests
{
    ulSfT_Time_0 = OS_SW_Timer_GetSystemTickCount();     // Start duration timer

    switch(psSfT_State->eTestID)
    {
        case eSelfTest_ID_INIT:
        {
            psSfT_State->ulTestResult = 0u;
            psSfT_State->eTestID = eSelfTest_ID_CPUREG; // Go to next test, do not log
            break;
        }

        case eSelfTest_ID_CPUREG:                       // Test CPU register
        {
            #if EXEC_CYCLIC_CPUREG == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                if(HAL_SelfTest_CPU_Reg() != eSelfTest_OK)
                {
                    psSfT_Log->sActualResult.eResultCode = eSelfTest_ERROR;
                    while(1u);               // Stop on error
                }
                TestLog(eSelfTest_ID_CPUPC, eSelfTest_OK);   // Go to next test, log
            #endif
            break;
        }

        case eSelfTest_ID_CPUPC:                    // STest CPU program counter
        {
            #if EXEC_CYCLIC_CPUPC == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                psSfT_State->ulTestResult = 0u;
                if(HAL_SelfTest_CPU_PC() != eSelfTest_OK)
                {
                    psSfT_Log->sActualResult.eResultCode = eSelfTest_ERROR;
                    while(1u);      //Stop on error
                }

                TestLog(eSelfTest_ID_TIMEBASE, eSelfTest_OK);
            #endif
            break;
        }

        case eSelfTest_ID_TIMEBASE:                  // STest  timebase
        {
            /* If cyclic interrupt test is disabled, jump over to eSelfTest_ID_RAM0 */
            #if EXEC_CYCLIC_TIMEBASE == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                    #warning check in standby state
                    /* Check for standby state */
                    #if 0
                    if (OS_StateManager_GetCurrentState() == eSM_State_Reset)
                    {
                        psSfT_State->eTestID += SELFTEST_ID_DELTA;
                    }
                    else
                    #endif
                    {
                        /* Interrupt test is - start with interrupt0_start up */
                        if(HAL_SelfTest_TimeBase() != eSelfTest_OK)
                        {
                            psSfT_Log->sActualResult.eResultCode = eSelfTest_ERROR;
                            while(1u);                // Stop on error
                        }

                        /* Next test state is interrupt 1 */
                        TestLog(eSelfTest_ID_RAM0, eSelfTest_OK);
                    }
            #endif
            break;
        }

        case eSelfTest_ID_RAM0:         // Test the save buffer ( size of one tile )
        {
            #if EXEC_CYCLIC_RAM == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                if(SelfTest_March_Buffer(TEST_BUFFER_STARTADR, TEST_BLOCK_SRAM_SIZE) == ERROR_STATUS)
                {
                    psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                    while(1u);  // Stop on error ( probably the test stops in situ )
                }

                psSfT_State->ulTestAddress = TEST_SRAM_STARTADR;      // Prepare the SRAM test
                psSfT_State->ulTestSize = TEST_BLOCK_SRAM_SIZE;
                psSfT_State->ulTestOffset = 0u;            // Start this session with offset 0
                psSfT_State->eTestID = eSelfTest_ID_RAM1;
            #endif
            break;
        }

        case eSelfTest_ID_RAM1:
        {
            #if EXEC_CYCLIC_RAM == true
                psSfT_State->ulTestResult = SelfTest_March_SRAM((psSfT_State->ulTestAddress + psSfT_State->ulTestOffset), psSfT_State->ulTestSize,TEST_SRAM_ENDADR);
                psSfT_State->eTestID = eSelfTest_ID_RAM2;
            #endif
            break;
        }

        case eSelfTest_ID_RAM2:
        {
            #if EXEC_CYCLIC_RAM == true
                if(psSfT_State->ulTestResult == PASS_STILL_TESTING_STATUS)
                {
                    psSfT_State->ulTestAddress += psSfT_State->ulTestSize;
                    psSfT_State->eTestID = eSelfTest_ID_RAM1;
                }
                else if(psSfT_State->ulTestResult == PASS_COMPLETE_STATUS)
                {
                    TestLog(eSelfTest_ID_STACK0, eSelfTest_OK);
                }
                else if(psSfT_State->ulTestResult == ERROR_STATUS)
                {
                    psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                    while(1u);
                }
                else    // Program Error
                {
                    while(1u);
                }
            #endif
            break;
        }

        case eSelfTest_ID_STACK:                      // First test the save buffer
        {
            #if EXEC_CYCLIC_STACK == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                if(HAL_SelfTest_Stack_Check() == ERROR_STATUS)
                {
                    psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                    while(1u);               // Stop on error
                }
                TestLog(eSelfTest_ID_STACKOVF, eSelfTest_OK);
            #endif
            break;
        }

        case eSelfTest_ID_STACKOVF:
        {
            #if EXEC_CYCLIC_STACKOVF == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                if(!(SelfTest_StackOverflow()))       // Stack limit test
                {
                    TestLog(eSelfTest_ID_FLASH0, eSelfTest_OK);
                }
                else
                {
                    psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                    while(1u);    // Stop on error
                }
            #endif
            break;
        }

        case eSelfTest_ID_FLASH0:         // Test Segments 1 - 4, 0 is not used here
        {
            #if EXEC_CYCLIC_FLASH == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                psSfT_State->ulTestOffset = ST_FLASH_SEGIDX_S1;      // Used as segment number
                psSfT_State->slTestCount = uAppCodeLength;       // Used as overall byte count

                if(0 == psSfT_State->slTestCount)
                {
                    psSfT_State->eTestID += SELFTEST_ID_DELTA;             // Go to next test
                }
                else
                {
                    psSfT_State->ulTestAddress = uAppCodeStart;       // Start with segment 1
                    psSfT_State->ulTestResult = CRC_INIT_VALUE;
                    psSfT_State->eTestID = eSelfTest_ID_FLASH1;          // Execute CRC check
                }
            #endif
            break;
        }

        case eSelfTest_ID_FLASH1:
        {
            #if EXEC_CYCLIC_FLASH == true
                if(psSfT_State->slTestCount > (s32)ST_FLASH_TILE_SIZE)  // Not null
                {
                    psSfT_State->ulTestSize = ST_FLASH_TILE_SIZE; // Number of bytes is bigger than a tile size
                }
                else
                {
                    psSfT_State->ulTestSize = psSfT_State->slTestCount; // Number of bytes is already equal or smaller than a tile size
                }
                psSfT_State->ulTestResult = eSelfTest_C_FlashCRC((u8*)psSfT_State->ulTestAddress, psSfT_State->ulTestSize, psSfT_State->ulTestResult);
                psSfT_State->slTestCount = psSfT_State->slTestCount - ST_FLASH_TILE_SIZE;

                if(psSfT_State->slTestCount < 1)             // If it has reached 0 now, check CRC
                {
                    if(SelfTest_FlashCRCRead(psSfT_State->ulTestOffset) != ~psSfT_State->ulTestResult) // Compare with last inversion
                    {
                        psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                        while(1u);           // Stop on CRC error
                    }
                    TestLog(eSelfTest_ID_IO0, eSelfTest_OK);
                }
                else
                {
                    psSfT_State->ulTestAddress += ST_FLASH_TILE_SIZE;  // Next row in same segment
                }
            #endif
            break;
        }

        case eSelfTest_ID_IO0:
        {
            #if EXEC_CYCLIC_IO == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                psSfT_State->ulTestOffset = 0u;                // Use TestOffset as PortNumber
                psSfT_State->eTestID = eSelfTest_ID_IO1;
            #endif
            break;
        }

        case eSelfTest_ID_IO1:
        {
            #if EXEC_CYCLIC_IO == true
                if(eSelfTest_C_IO(psSfT_State->ulTestOffset) != eSelfTest_OK) // TestOffset holds the port number
                {
                    psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                    while(1u);           // Stop on error
                }
                psSfT_State->ulTestOffset++;                              // Increment PortNumber

                if(psSfT_State->ulTestOffset >= IO_PORTS)             // Max port index reached ?
                {
                    TestLog(eSelfTest_ID_UREG0, eSelfTest_OK);
                }
            #endif
            break;
        }

        case eSelfTest_ID_ADC0:                                 // ADC test
        {
            /* If ADC-Test is disabled skip this test an test UART0 */
            #if EXEC_CYCLIC_ADC == false
                psSfT_State->eTestID += SELFTEST_ID_DELTA;
            #else
                // If ADC is in use, this adds extra ADC conversion cycles, which may influence regular conversion results !
                if(eSelfTest_S_ADC())
                {
                    psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                    while(1u);  // Stop on error
                }
                TestLog(eSelfTest_ID_UART0, eSelfTest_OK);
            }
            #endif
            break;
        }

        case eSelfTest_ID_UART0:                                // UART test
        {
            #if EXEC_CYCLIC_UART == false
               psSfT_State->eTestID += SELFTEST_ID_DELTA;
           #else
                // Do NOT use UART test!
                // Communication on Wolf Bus is life test, do not disturb with extra characters !
                if(HAL_SelfTest_UART_Check())
                {
                    psSfT_Log->sActualResult.eResultCode  = eSelfTest_ERROR;
                    while(1u);// Stop on error
                }
                else
                {
                    TestLog(eSelfTest_ID_EXIT, eSelfTest_OK);
                }
            #endif
            break;
        }

        case eSelfTest_ID_EXIT: // Not a real test. Terminates the sequence of real tests.
        {
             // If enabled creates testpattern to locate end of test sequence
            psSfT_State->eTestID = eSelfTest_ID_INIT;
            TestLog(eSelfTest_ID_INIT, eSelfTest_NotExecuted);
            ulSfT_Time_1 = OS_SW_Timer_GetSystemTickCount() - ulSfT_Time_0; // Time in msec for the duration of all cyclic self tests
            break;
        }

        default:                                            // Should not happen
        {
            psSfT_State->eTestID = eSelfTest_ID_INIT;
        }

    } // end switch (  psSfT_State->eTestID )
}



//********************************************************************************
/*!
\author     Kraemer E.
\date       19.04.2020
\brief      Initializes the structure for the cyclical callback
\return     none
\param      none
***********************************************************************************/
void OS_SelfTest_InitCyclic(void)                         // Prepare the sequence of cyclic tests
{
    psSfT_State = &sSfT_State;
    psSfT_State->eTestID = eSelfTest_ID_INIT;
    psSfT_Log  = &sSfT_Log;
    psSfT_Log->sLastResult.eResultCode = 0u;
    psSfT_Log->sLastResult.eTestID = 0u;
    psSfT_State->slTestCount = 0u;
    psSfT_State->ulTestOffset = 0u;
}
