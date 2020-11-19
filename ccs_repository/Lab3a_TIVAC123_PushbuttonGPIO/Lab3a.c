// This program uses the other pushbutton - PF0
// Blue LED is on if pushbutton is NOT pressed
// Red LED is on if pushbutton IS pressed

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"    // We need to include this. It's got the GPIO_LOCK_KEY macro, among others
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
int main(void)
{
  SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); // set up the clock
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // enable port F

    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;   // unlock the GPIOCR register for port F
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x01;      // Free up pin 0

  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); // enable outputs on the launchpad LED pins
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);    // make F0 an input

  GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); // enable F0's pullup, the drive strength won't affect the input
  while(1)
  {
    uint32_t pinVal=0;  // variable to hold the pinRead
    pinVal= GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0);    // read F0

    if( (pinVal & GPIO_PIN_0)==0){  // AND to strip out anything but the value read from F0
      GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);    // turn on one LED
    }

    else{
      GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);    // turn on a different LED
    }
    // Debouncing
    SysCtlDelay(7000000);

  }
}
