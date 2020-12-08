#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"   //def. for the interrupt and register assignments on the Tiva C Series device on the launchPad board
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_adc.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"


#ifdef DEBUG
void__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

// Globals
uint32_t ui32Period;
char     buffer[4];

uint32_t ui32ADC0Value[4];
volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;

uint8_t ui8LEDData=2;                   // For LED colors
char cmd;                               // For UART input
uint32_t ui32PinStatus = 0x00000000;    // Variable to store pin status of GPIO Port F

// Interrupt Handlers
void GPIOF0IntHandler(void);

void getTemp(void);
void LEDSetup(void);
void timerSetup(void);
void UARTinit(void);
void ADCinit(void);

int main(void) {

    // Configure Clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Configure peripherals
    LEDSetup();
    UARTinit();
    ADCinit();

    // Enable interrupts
    IntMasterEnable();
    ADCSequenceEnable(ADC0_BASE, 2);

    getTemp();  // Skip inital bad input

    while (1)  // Wait forever
    {
        UARTprintf("Enter Command: ");
        char cmd = UARTgetc();

        UARTprintf("\n");
        switch(cmd) {
        case 'R':
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
            break;
        case 'r':
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
            break;
        case 'G':
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);
            break;
        case 'g':
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
            break;
        case 'B':
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
            break;
        case 'b':
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
            break;
        case 'T':
            getTemp();
            UARTprintf("C %3d\t\n",ui32TempValueC);
            break;
        case 't':
            getTemp();
            UARTprintf("F %3d\t\n",ui32TempValueF);
            break;
        case 'S':
            ui32PinStatus = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
            switch(ui32PinStatus) {
            case 0:
                UARTprintf("RGB Status: Black (off)\n");
                break;
            case 2:
                UARTprintf("RGB Status: Red\n");
                break;
            case 4:
                UARTprintf("RGB Status: Blue\n");
                break;
            case 8:
                UARTprintf("RGB Status: Green\n");
                break;
            case 6:
                UARTprintf("RGB Status: Magenta\n");
                break;
            case 10:
                UARTprintf("RGB Status: Yellow\n");
                break;
            case 12:
                UARTprintf("RGB Status: Cyan\n");
                break;
            case 14:
                UARTprintf("RGB Status: White\n");
                break;
            default:
                UARTprintf("Invalid Color? There is an error\n");
            }
            break;
            default:
                UARTprintf("Invalid Input. Please try again.\n");

        }
    }

}

// Get temperature without the timer
void getTemp(void)
{
    ADCIntClear(ADC0_BASE, 2);
    ADCProcessorTrigger(ADC0_BASE, 2);

    ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0Value);

    ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
    ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
    ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

}

void GPIOF0IntHandler(void) //interrupt handler for GPIO pin F0
{
    //clear interrupt flag on pin F0
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
    // Update RGB color
    if(ui8LEDData==8) {ui8LEDData=2;} else {ui8LEDData=ui8LEDData*2;}
    // Change Color
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, ui8LEDData);
    SysCtlDelay(1000000);    // Debounce
}

void LEDSetup(void)
{
    //Port configuration (LEDS)
    //Enable GPIOF port
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //set LEDS connected to pins as outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //Unlock Pin F0 to use an interrupt on SW2
    SYSCTL_RCGC2_R |= 0x00000020; // activate clock for Port F
    GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
    GPIO_PORTF_CR_R = 0x1F; // allow changes to PF4-0
    // only PF0 needs to be unlocked, other bits can't be locked
    GPIO_PORTF_AMSEL_R = 0x00; // disable analog on PF
    GPIO_PORTF_PCTL_R = 0x00000000; // PCTL GPIO on PF4-0
    GPIO_PORTF_DIR_R = 0x0E; // PF4,PF0 in, PF3-1 out
    GPIO_PORTF_AFSEL_R = 0x00; // disable alt funct on PF7-0
    GPIO_PORTF_PUR_R = 0x11; // enable pull-up on PF0 and PF4
    GPIO_PORTF_DEN_R = 0x1F; // enable digital I/O on PF4-0
    // Enable PF0 interrupt
    // Register the interrupt handler for PF0
    GPIOIntRegister(GPIO_PORTF_BASE, GPIOF0IntHandler);
    //SW2 goes low when pressed
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    //enable interrupts on PF0
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0);
    // Start on Red LED
    // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8LEDData); // Disable for tasks 2 and 3
}

void timerSetup(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  // Enabling Timer 1
    // Configure Timer 1 module
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    ui32Period = SysCtlClockGet()/2;   // Period of 0.5s 2Hz
    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period -1);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}
void UARTinit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure pins for UART
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);

}

void ADCinit(void)
{
    // Configure ADC
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCHardwareOversampleConfigure(ADC0_BASE, 32);

    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0); // Changed to sequencer #2

    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_TS);

    ADCSequenceStepConfigure(ADC0_BASE, 2, 3, ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 2);
}
