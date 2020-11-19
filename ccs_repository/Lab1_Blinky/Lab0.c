//*****************************************************************************
//
// This example demonstrates the use of TivaWare to setup the clocks and
// toggle GPIO pins to make the LED's blink.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

//*****************************************************************************
//
// Define pin to LED color mapping.
//
//*****************************************************************************

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// Main 'C' Language entry point.  Toggle an LED using TivaWare.
//
//*****************************************************************************
int
main(void)
{
    //
    // Setup the system clock to run at 50 Mhz from PLL with crystal reference
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
                    SYSCTL_OSC_MAIN);

    //
    // Enable and wait for the port to be ready for access
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    // Configure the GPIO port for the LED operation.
    // PF1 : RED_LED, PF2 : BLUE_LED, PF3 : GREEN_LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);

    //
    // Loop Forever
    //
    while(1)
    {
        //
        // Turn on the LED
        //
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);

        //
        // Delay for a bit
        //
        SysCtlDelay(2000000);

        //
        // Turn on the LED
        //
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);

        //
        // Delay for a bit
        //
        SysCtlDelay(2000000);
    }
}
