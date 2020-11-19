// HEADER FILES
#include<stdint.h>
#include<stdbool.h>
#include"inc/hw_memmap.h"
#include"inc/hw_types.h"
#include"driverlib/gpio.h"
#include"driverlib/pin_map.h"
#include"driverlib/sysctl.h"
#include"driverlib/uart.h"

#include "utils/uartstdio.h"
#include <string.h>

#define GPIO_PA0_U0RX 0x00000001 // UART PIN ADDRESS FOR UART RX
#define GPIO_PA1_U0TX 0x00000401 // UART PIN ADDRESS FOR UART TX
int main(void)
{
    // SYSTEM CLOCK AT 40 MHZ
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|
    SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlDelay(3);
    // ENABLE GPIO PORT A,FOR UART
    // ENABLE PERIPHERAL UART 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX); // PA0 IS CONFIGURED TO UART RX
    GPIOPinConfigure(GPIO_PA1_U0TX); // PA1 IS CONFIGURED TO UART TX
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
    UARTprintf("Echo Characters:\n");
    while (1)
     {
        //UART ECHO - what is received is transmitted back //
        if (UARTCharsAvail(UART0_BASE)) UARTCharPut(UART0_BASE,
        UARTCharGet(UART0_BASE));
     }
}
