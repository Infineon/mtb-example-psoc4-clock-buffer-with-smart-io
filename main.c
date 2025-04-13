/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the PSoC 4 Clock Buffer with
* Smart IO code example for ModusToolbox.
*
* Related Document: See README.md 
*
*******************************************************************************
* Copyright 2023-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"

/*******************************************************************************
 * Macros
 ********************************************************************************/
#define SWITCH_INTR_PRIORITY    (3u)
#define ONE_SEC_DELAY           (1000u)       

/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/
void switch_isr();
void smart_io_start(void);

/*******************************************************************************
 * Global Variables
 ********************************************************************************/
volatile uint32_t interrupt_flag = 0;

/******************************************************************************
 * Switch interrupt configuration structure
 *******************************************************************************/
const cy_stc_sysint_t switch_intr_config = {
    .intrSrc = INTR_PIN_IRQ,                  /* Source of interrupt signal */
    .intrPriority = SWITCH_INTR_PRIORITY            /* Interrupt priority */
};

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* System entrance point. This function performs
*   1. Initializes the BSP.
*   2. Initializes and enable GPIO interrupt.
*   3. Calls the functions to set up Smart IO block.
*   4. Put the CPU to deepsleep mode.
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize and enable GPIO interrupt */
    result = Cy_SysInt_Init(&switch_intr_config, switch_isr);

    if(result != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Clearing and enabling the GPIO interrupt in NVIC */
    NVIC_ClearPendingIRQ(switch_intr_config.intrSrc);
    NVIC_EnableIRQ(switch_intr_config.intrSrc);

    smart_io_start();

    for (;;)
    {
        /* Hold override functionality must be enabled before entering deep-sleep */
        Cy_SmartIO_HoldOverride(PRGIO_PRT0, true);
        Cy_SysPm_CpuEnterDeepSleep();
        Cy_SmartIO_HoldOverride(PRGIO_PRT0, false);
        if (interrupt_flag == 1u)
        {
            interrupt_flag = 0u;
            Cy_GPIO_Write(STATUS_PIN_PORT, STATUS_PIN_NUM, 0UL);
            Cy_SysLib_Delay(ONE_SEC_DELAY);
            Cy_GPIO_Write(STATUS_PIN_PORT, STATUS_PIN_NUM, 1UL);
        }
    }
}

/*******************************************************************************
 * Function Name: switch_isr
 ********************************************************************************
 * Summary: 
 *  Interrupt service routine for the GPIO interrupt triggered from INTR_PIN.
 *  This interrupt is the result of an AND operation in the SMART_IO of IN_PIN
 *  and TRIG_PIN.
 *  This function clears the triggered GPIO pin interrupt.
 *
 * Parameters:
 *  None
 *
 * Return
 *  void
 *
 *******************************************************************************/
void switch_isr()
{
    /* Clears the triggered pin interrupt */
    Cy_GPIO_ClearInterrupt(INTR_PIN_PORT, INTR_PIN_NUM);

    /* Set interrupt flag */
    interrupt_flag = 1;
}

/*******************************************************************************
 * Function Name: smart_io_start
 ********************************************************************************
 * Summary: This function initializes and enable SMART_IO block.
 *
 * Parameters:
 *  None
 *
 * Return
 *  void
 *
 *******************************************************************************/
void smart_io_start(void)
{
    /* Configure the SMART_IO block */
    if (CY_SMARTIO_SUCCESS != Cy_SmartIO_Init(PRGIO_PRT0, &SMART_IO_config))
    {
        CY_ASSERT(0);
    }

    /* Enable SMART_IO block */
    Cy_SmartIO_Enable(PRGIO_PRT0);
}


/* [] END OF FILE */
