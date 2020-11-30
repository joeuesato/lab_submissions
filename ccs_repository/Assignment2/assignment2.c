#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "TSL2591_def.h"
#include "HAL_I2C.h"
#include "HAL_TMP006.h"
#include "driverlib/debug.h"
#include "utils/uartstdio.h"
#include <string.h>

/* Variable for storing temperature value returned from TMP006 */
float temp;
void ConfigureUART0(void)

//Configures the UART to run at 115200 baud rate
{
    // ENABLE PERIPHERAL UART 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // ENABLE GPIO PORT A,FOR UART
    GPIOPinConfigure(GPIO_PA0_U0RX); // PA0 IS CONFIGURED TO UART RX
    GPIOPinConfigure(GPIO_PA1_U0TX); // PA1 IS CONFIGURED TO UART TX
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}

void I2C1_Init ()
    //Configure/initialize the I2C1
    {
        SysCtlPeripheralEnable (SYSCTL_PERIPH_I2C1); //enables I2C0
        SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOA); //enable PORTA as peripheral
        GPIOPinTypeI2C (GPIO_PORTA_BASE, GPIO_PIN_7); //set I2C PA7 as SDA
        GPIOPinConfigure (GPIO_PA7_I2C1SDA);
        GPIOPinTypeI2CSCL (GPIO_PORTA_BASE, GPIO_PIN_6); //set I2C PA6 as SCLK
        GPIOPinConfigure (GPIO_PA6_I2C1SCL);
        // Enable and initialize I2C1 master module
        // Use system clock for the I2C1 module
        // false
        I2CMasterInitExpClk (I2C1_BASE, SysCtlClockGet(), false); //Set the clock of the I2C to ensure proper connection
        while (I2CMasterBusy (I2C1_BASE)); //wait while the master SDA is busy
    }

void I2C1_Write (uint8_t addr, uint8_t N, ...)
//Writes data from master to slave
//Takes the address of the device, the number of arguments, and a variable amount of register addresses to write to
{
    I2CMasterSlaveAddrSet (I2C1_BASE, addr, false); //Find the device based on the address given
    while (I2CMasterBusy (I2C1_BASE));

    va_list vargs; //variable list to hold the register addresses passed
    va_start (vargs, N); //initialize the variable list with the number of arguments
    I2CMasterDataPut (I2C1_BASE, va_arg(vargs, uint8_t)); //put the first argument in the list in to the I2C bus

    while (I2CMasterBusy (I2C1_BASE));

    if (N == 1) //if only 1 argument is passed, send that register command then stop
    {
        I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
        while (I2CMasterBusy (I2C1_BASE));
        va_end (vargs);
    }
    else //if more than 1, loop through all the commands until they are all sent
    {
        I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        while (I2CMasterBusy (I2C1_BASE));

        uint8_t i;
        for (i = 1; i < N - 1; i++)
        {
            I2CMasterDataPut (I2C1_BASE, va_arg(vargs, uint8_t)); //send the next register address to the bus

            while (I2CMasterBusy (I2C1_BASE));

            I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT); //burst send, keeps receiving until the stop signal is received

            while (I2CMasterBusy (I2C1_BASE));

        }
        I2CMasterDataPut (I2C1_BASE, va_arg(vargs, uint8_t)); //puts the last argument on the SDA bus

        while (I2CMasterBusy (I2C1_BASE));

        I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH); //send the finish signal to stop transmission

        while (I2CMasterBusy (I2C1_BASE));

        va_end (vargs);
    }
}

//Read data from slave to master
//Takes in the address of the device and the register to read from
uint32_t I2C1_Read (uint8_t addr, uint8_t reg)
{
    I2CMasterSlaveAddrSet (I2C1_BASE, addr, false); //find the device based on the address given

    while (I2CMasterBusy (I2C1_BASE));

    I2CMasterDataPut (I2C1_BASE, reg); //send the register to be read on to the I2C bus

    while (I2CMasterBusy (I2C1_BASE));

    I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND); //send the send signal to send the register value

    while (I2CMasterBusy (I2C1_BASE));

    I2CMasterSlaveAddrSet (I2C1_BASE, addr, true); //set the master to read from the device

    while (I2CMasterBusy (I2C1_BASE));

    I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE); //send the receive signal to the device

    while (I2CMasterBusy (I2C1_BASE));

    return I2CMasterDataGet (I2C1_BASE); //return the data read from the bus
}

//Initializes the TSL2591 to have a medium gain,
void TSL2591_init ()
{
    uint32_t x;
    x = I2C1_Read (TSL2591_ADDR, (TSL2591_COMMAND_BIT | TSL2591_ID)); //read the device ID
    if (x == 0x50)
    {
        UARTprintf("Device Found\n");
    }
    else
    {
        UARTprintf("Device Not Found\n");
        while (1){}; //loop here if the dev ID is not correct
    }
    I2C1_Write (TSL2591_ADDR, 2, (TSL2591_COMMAND_BIT | TSL2591_CONFIG), 0x10);
    //configures the TSL2591 to have medium gain and integration time of 100ms
    I2C1_Write (TSL2591_ADDR, 2, (TSL2591_COMMAND_BIT | TSL2591_ENABLE),
                (TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN |
                        TSL2591_ENABLE_NPIEN)); //enables proper interrupts and power to work with TSL2591
}
/*
uint32_t GetLuminosity ()
//This function will read the channels of the TSL and returns the calculated value to the caller
{
 float atime = 100.0f, again = 25.0f; //the variables to be used to calculate proper lux value
 uint16_t ch0, ch1; //variable to hold the channels of the TSL2591
 uint32_t cp1, lux1, lux2, lux;
 uint32_t x = 1;
 x = I2C0_Read (TSL2591_ADDR, (TSL2591_COMMAND_BIT | TSL2591_C0DATAH));
 x <<= 16;
 x |= I2C0_Read (TSL2591_ADDR, (TSL2591_COMMAND_BIT | TSL2591_C0DATAL));
 ch1 = x>>16;
 ch0 = x & 0xFFFF;
 cp1 = (uint32_t) (atime * again) / TSL2591_LUX_DF;
 lux1 = (uint32_t) ((float) ch0 - (TSL2591_LUX_COEFB * (float) ch1)) / cp1;
 lux2 = (uint32_t) ((TSL2591_LUX_COEFC * (float) ch0) - (TSL2591_LUX_COEFD * (float) ch1)) /
cp1;
 lux = (lux1 > lux2) ? lux1: lux2;
 return lux;
}
*/

uint32_t GetTemp ()
{
    uint16_t ch0, ch1; //variable to hold the channels of the TSL2591
    uint32_t x = 1;
    x = I2C0_Read (TSL2591_ADDR, (TSL2591_COMMAND_BIT | TSL2591_C0DATAH));
    x <<= 16;
    x |= I2C0_Read (TSL2591_ADDR, (TSL2591_COMMAND_BIT | TSL2591_C0DATAL));
    ch1 = x>>16;
    ch0 = x & 0xFFFF;
}

void main (void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    //set the main clock to runat 40MHz
    ConfigureUART0(); //configure the UART of Tiva C
    // I2C1_Init(); //initialize the I2C0 of Tiva C
    UARTprintf("Starting TSL2591 Initialization ..... \n");
    /* Initialize I2C communication */
    Init_I2C_GPIO();
    I2C_init();
    /* Initialize TMP006 temperature sensor */
    TMP006_init();
    __delay_cycles(100000);
    while(1)
    {
        /* Obtain temperature value from TMP006 */
        temp = TMP006_getTemp();
        /* Display temperature */
        char string[10];
        sprintf(string, "%f", temp);
        sprintf(string, "F");
    }
}
