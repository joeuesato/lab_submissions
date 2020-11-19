#include <stdbool.h>
#include <stdint.h>

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

static uint16_t ADC_Out1[ADC_SAMPLE_BUF_SIZE];
static uint16_t ADC_Out2[ADC_SAMPLE_BUF_SIZE];
static enum BUFFERSTATUS BufferStatus[2];

void ConfigureUART(void);
void init_ADC(void);
void init_TIMER(void);
void init_DMA(void);
static uint32_t g_ui32DMAErrCount = 0u;
static uint32_t g_ui32SysTickCount;


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

    if ((uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT) == UDMA_MODE_STOP)
            && (BufferStatus[0] == FILLING))
    {
        BufferStatus[0] = FULL;
        BufferStatus[1] = FILLING;
    }
    else if ((uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT) == UDMA_MODE_STOP)
            && (BufferStatus[1] == FILLING))

    {
        BufferStatus[0] = FILLING;
        BufferStatus[1] = FULL;
    }
}

int main(void)
{
    uint32_t i, average1, average2, samples_taken;
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
    BufferStatus[1] = EMPTY;
    samples_taken = 0u;

    ConfigureUART();
    UARTprintf("Timer->ADC->uDMA demo!\n");
    UARTprintf("\tAverage1\tAverage2\tTotal Samples\n");

    init_DMA();
    init_ADC();
    init_TIMER();

    MAP_IntMasterEnable();
    MAP_TimerEnable(TIMER0_BASE, TIMER_A); // Start everything


    while(1)
    {
        if(BufferStatus[0u] == FULL)
        {
            // Do something with data in ADC_OUT1
            average1 = 0u;
            for(i =0u; i < ADC_SAMPLE_BUF_SIZE; i++)
            {
                average1 += ADC_Out1[i];
                ADC_Out1[i] = 0u;
            }
            BufferStatus[0u] = EMPTY;
            // Enable for another uDMA block transfer
            uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &ADC_Out1, ADC_SAMPLE_BUF_SIZE);
            uDMAChannelEnable(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT); // Enables DMA channel so it can perform transfers
            samples_taken += ADC_SAMPLE_BUF_SIZE;
            average1 = (average1 + (ADC_SAMPLE_BUF_SIZE / 2u)) / ADC_SAMPLE_BUF_SIZE;
        }
        if(BufferStatus[1u] == FULL)
        {
            // Do something with data in ADC_OUT2
            average2 = 0u;
            for(i =0u; i < ADC_SAMPLE_BUF_SIZE; i++)
            {
                average2 += ADC_Out2[i];
                ADC_Out2[i] = 0u;
            }
            BufferStatus[1u] = EMPTY;
            // Enable for another uDMA block transfer
            uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &ADC_Out2, ADC_SAMPLE_BUF_SIZE);
            uDMAChannelEnable(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT);
            samples_taken += ADC_SAMPLE_BUF_SIZE;
            average2 = (average2 + (ADC_SAMPLE_BUF_SIZE / 2u)) / ADC_SAMPLE_BUF_SIZE;
            UARTprintf("\t%d\t\t%d\t\t%d\r", average1,average2,samples_taken);
        }
    }
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

    uDMAChannelAttributeDisable(UDMA_CHANNEL_ADC0, UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

    uDMAChannelAttributeEnable(UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST);
    // Only allow burst transfers

    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);

    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &ADC_Out1, ADC_SAMPLE_BUF_SIZE);
    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG, (void *)(ADC0_BASE + ADC_O_SSFIFO0), &ADC_Out2, ADC_SAMPLE_BUF_SIZE);

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
