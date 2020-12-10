#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "sensorlib/i2cm_drv.h"
#include "sensorlib/hw_tmp006.h"
#include "sensorlib/tmp006.h"

#include "uartstdio.h"
#include "math.h"
#include "IQmath/IQmathLib.h"

#define TMP006_I2C_ADDRESS      0x41

// Instance of drivers
tI2CMInstance g_sI2CInst;
tTMP006 g_sTMP006Inst;

// Variables
volatile uint_fast8_t g_vui8DataFlag;       // Global new data flag to alert main that TMP006 data is ready.
volatile uint_fast8_t g_vui8ErrorFlag;      // Global new error flag to store the error condition if encountered.

// Interrupt Handlers
void TMP006AppCallback(void *pvCallbackData, uint_fast8_t ui8Status);

// Other functions
void ConfigureUART(void);
void InitI2C0(void);

int main(void) {
    float fAmbient, fObject;
    double IQ_tempFarenheit;
    int_fast32_t i32IntegerPart;
    int_fast32_t i32FractionPart;

    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    InitI2C0();
    ConfigureUART();

    TMP006Init(&g_sTMP006Inst, &g_sI2CInst, TMP006_I2C_ADDRESS, TMP006AppCallback, &g_sTMP006Inst);
    I2CMInit(&g_sI2CInst, I2C0_BASE, INT_I2C0, 0xff, 0xff, SysCtlClockGet());

    UARTprintf("Assignment 2!!\n");

    while (1)
    {
        // Reset the flag
        g_vui8DataFlag = 0;

        // Get a local copy of the latest data in float format.
        TMP006DataTemperatureGetFloat(&g_sTMP006Inst, &fAmbient, &fObject);

        // Convert the floating point object temperature  to an integer part and fraction part for easy printing.
        i32IntegerPart = (int32_t)fObject;
        i32FractionPart = (int32_t)(fObject * 1000.0f);
        i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
        if(i32FractionPart < 0)
        {
            i32FractionPart *= -1;
        }
        UARTprintf("Object Temperature Celcius: %3d.%03d\n", i32IntegerPart, i32FractionPart);

        // Convert to Farenheit
        fObject = ((fObject * 9) + 160) / 5;

        // Convert the floating point ambient temperature  to an integer part and fraction part for easy printing.
        i32IntegerPart = (int32_t)fObject;
        i32FractionPart = (int32_t)(fObject * 1000.0f);
        i32FractionPart = i32FractionPart - (i32IntegerPart * 1000);
        if(i32FractionPart < 0)
        {
            i32FractionPart *= -1;
        }
        UARTprintf("Object Temperature Farenheit: %3d.%03d\n", i32IntegerPart, i32FractionPart);

        SysCtlDelay(10000000);
    }
}

void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    // Enable the GPIO Peripheral used by the UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);    // Enable UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);                // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}

void InitI2C0(void) {
    SysCtlPeripheralEnable (SYSCTL_PERIPH_I2C0);    //enables I2C0
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOB);   //enable PORTB as peripheral

    GPIOPinTypeI2C (GPIO_PORTB_BASE, GPIO_PIN_3);   //set I2C PB3 as SDA
    GPIOPinConfigure (GPIO_PB3_I2C0SDA);

    GPIOPinTypeI2CSCL (GPIO_PORTB_BASE, GPIO_PIN_2);    //set I2C PB2 as SCLK
    GPIOPinConfigure (GPIO_PB2_I2C0SCL);

    I2CMasterInitExpClk (I2C0_BASE, SysCtlClockGet(), false);   //Set the clock of the I2C to ensure proper connection
    while (I2CMasterBusy (I2C0_BASE));  //wait while the master SDA is busy
}

void TMP006AppCallback(void *pvCallbackData, uint_fast8_t ui8Status)
{
    // If the transaction succeeded set the data flag to indicate to
    // application that this transaction is complete and data may be ready.
    if(ui8Status == I2CM_STATUS_SUCCESS)
    {
        g_vui8DataFlag = 1;
    }

    // Store the most recent status in case it was an error condition
    g_vui8ErrorFlag = ui8Status;
}

