/* *****************************************************************************
* File Name: SelfTest_Stack.c
* Version 1.1
*
* Description:
*  This file provides the source code to the API for runtime Stack self tests for 
*  PSoC 4
*
* Owner:
*  JOBI
* 
* Related Document: 
*  AN89056: PSoC 4 - IEC 60730 Class B and IEC 61508 Safety Software Library
*
* Hardware Dependency:
*  CY8C40XX, CY8C42XX, CY8C42XXM, CY8C42XXL, CY8C40XXS, CY8C41XXS and 
*  CY8C41XXS Plus Devices
*
* Note:
*
********************************************************************************
* Copyright (2018-2019), Cypress Semiconductor Corporation.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress product in a life
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*******************************************************************************/





/********************************* includes **********************************/
#include "CyLib.h"
#include "SelfTest_Stack.h"
//#include "TestConfiguration.h"

/***************************** defines / macros ******************************/

/************************ local data type definitions ************************/

/************************* local function prototypes *************************/

/************************* local data (const and var) ************************/

/************************ export data (const and var) ************************/


/****************************** local functions ******************************/


/************************ externally visible functions ***********************/
//********************************************************************************
/*!
\author     Kraemer E
\date       29.11.2020
\brief      This function initializes the upper stack area with 0xAA and 0x55 pattern.
\return     none
\param      none
***********************************************************************************/
void SelfTest_Stack_Init(void)
{
	/* Pointer to the last word in the stack*/
	uint16 *uiStackAddr = (uint16 *)PSOC_STACK_END;
		
	/* Fill test stack block with predefined pattern */
    uint8 ucStIdx;
	for(ucStIdx = 0u; ucStIdx < (STACK_TEST_BLOCK_SIZE / sizeof(uint16)); ucStIdx++)
	{
		#if (ERROR_IN_STACK)
			*stack = STACK_TEST_PATTERN + 1u;
            stack++;
		#else
			*uiStackAddr = STACK_TEST_PATTERN;
            uiStackAddr++;
		#endif /* End (ERROR_IN_STACK) */
	}
}

//********************************************************************************
/*!
\author     Kraemer E
\date       29.11.2020
\brief      This function performs stack self test. It checks upper stack area for 0xAA 
            and 0x55 pattern.
\return     Result of test:  "0" - pass test; "1" - fail test.
\param      none
***********************************************************************************/
uint8 SelfTest_Stack_Check(void)
{
	uint8 ucRetVal = OK_STATUS;
		
	/* Pointer to the last word in the stack */
	uint16 *uiStackAddr = (uint16 *)PSOC_STACK_END;
		
	/* Check test stack block for pattern and return error if no pattern found */
    uint8 ucStIdx;
	for (ucStIdx = 0u; ucStIdx < (STACK_TEST_BLOCK_SIZE / sizeof(uint16)); ucStIdx++)
	{
		if(*uiStackAddr != STACK_TEST_PATTERN)
		{
            uiStackAddr++;
			ucRetVal = ERROR_STATUS;
			break;
		}
	}
		
	return ucRetVal;
}

