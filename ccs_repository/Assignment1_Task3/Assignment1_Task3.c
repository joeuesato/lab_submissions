#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/tm4c123gh6pm.h"   //def. for the interrupt and register assignments on the Tiva C Series device on the launchPad board
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_adc.h"
#include "inc/hw_types.h"
#include "inc/hw_udma.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/udma.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"

#define ADC_SAMPLE_BUF_SIZE     64

enum BUFFERSTATUS
                  { EMPTY,
                    FILLING,
                    FULL
                  };

#pragma DATA_ALIGN(ucControlTable, 1024)
uint8_t ucControlTable[1024];
volatile enum BUFFERSTATUS BufferStatus[1];

uint16_t ADC_Out1[ADC_SAMPLE_BUF_SIZE];

void ConfigureUART(void);
void init_ADC(void);
void init_TIMER(void);
void init_DMA(void);
void LEDSetup(void);

static uint32_t g_ui32DMAErrCount = 0u;
static uint32_t g_ui32SysTickCount;
static uint32_t g_ui32ADCCount;

void getTemp(void);
volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;

uint8_t ui8LEDData=2;                   // For LED colors
char cmd;                               // For UART input
uint32_t ui32PinStatus = 0x00000000;    // Variable to store pin status of GPIO Port F

void uDMAErrorHandler(void)
{
    uint32_t ui32Status;
    ui32Status = MAP_uDMAErrorStatusGet();
    if(ui32Status)
    {
        MAP_uDMAErrorStatusClear();
        g_ui32DMAErrCount++;
    }
}

// Not used in this example, but used to debug to make sure timer interrupts happen
void Timer0AIntHandler(void)
{
    //
    // Clear the timer interrupt flag.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void SysTickIntHandler(void)
{
    // Update our system tick counter.
    g_ui32SysTickCount++;
}

void ADCseq0Handler()
{
    ADCIntClear(ADC0_BASE, 0);
    g_ui32ADCCount++;
    if ((uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT) == UDMA_MODE_STOP)
            && (BufferStatus[0] == FILLING) )
    {
        BufferStatus[0] = FULL;
    }
   // uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_BASIC, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &ADC_Out1, ADC_SAMPLE_BUF_SIZE);
   // uDMAChannelEnable(UDMA_CHANNEL_ADC0); // Enables DMA channel so it can perform transfers
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

int main(void)
{
    // Set the system clock to run at 80MHz from the PLL.
    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    SysCtlDelay(20u);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  //Enable the clock to TIMER0
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);    //Enable the clock to ADC module
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);    //Enable the clock to uDMA
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);   //Enables the clock to PORT E
    MAP_SysCtlDelay(30u);

    MAP_SysTickPeriodSet(SysCtlClockGet() / 100000u); //Sets the period of the SysTic counter to 10us
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();

    BufferStatus[0] = FILLING;

    ConfigureUART();

    init_DMA();
    init_ADC();
    init_TIMER();
    LEDSetup();

    MAP_IntMasterEnable();
    MAP_TimerEnable(TIMER0_BASE, TIMER_A); // Start everything

    getTemp();  // Skip first faulty input

    while(1)
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

void getTemp(void)
{
    uint32_t i;
    // Do something with data in ADC_OUT1
    ui32TempAvg = 0u;
    for(i =0u; i < ADC_SAMPLE_BUF_SIZE; i++)
    {
        ui32TempAvg += ADC_Out1[i];
        ADC_Out1[i] = 0u;
    }
    ui32TempAvg = (ui32TempAvg+ (ADC_SAMPLE_BUF_SIZE / 2u)) / ADC_SAMPLE_BUF_SIZE;
    ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
    ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;
}


void init_TIMER()
{
    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
    // Set sample frequency to 16KHz (every 62.5uS)
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, MAP_SysCtlClockGet()/16000 -1);   //TODO: Timer Load Value is set here
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
    MAP_TimerControlStall(TIMER0_BASE, TIMER_A, true); //Assist in debug by stalling timer at breakpoints
}

void init_ADC()
{
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
    SysCtlDelay(80u);

    // Use ADC0 sequence 0 to sample channel 0 once for each timer period
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_HALF, 1);

    SysCtlDelay(10); // Time for the clock configuration to set

    IntDisable(INT_ADC0SS0);
    ADCIntDisable(ADC0_BASE, 0u);
    ADCSequenceDisable(ADC0_BASE,0u);
    // With sequence disabled, it is now safe to load the new configuration parameters

    ADCSequenceConfigure(ADC0_BASE, 0u, ADC_TRIGGER_TIMER, 0u);
    ADCSequenceStepConfigure(ADC0_BASE,0u,0u,ADC_CTL_CH0| ADC_CTL_END | ADC_CTL_IE);
    ADCSequenceEnable(ADC0_BASE,0u); //Once configuration is set, re-enable the sequencer
    ADCIntClear(ADC0_BASE,0u);
    ADCSequenceDMAEnable(ADC0_BASE,0);
    IntEnable(INT_ADC0SS0);

}

void init_DMA()
{
    uDMAEnable(); // Enables uDMA
    uDMAControlBaseSet(ucControlTable);

    uDMAChannelAttributeDisable(UDMA_CHANNEL_ADC0, UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

    //uDMAChannelAttributeEnable(UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST);
    // Only allow burst transfers

    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);

    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_BASIC, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &ADC_Out1, ADC_SAMPLE_BUF_SIZE);

    uDMAChannelEnable(UDMA_CHANNEL_ADC0); // Enables DMA channel so it can perform transfers

}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
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
