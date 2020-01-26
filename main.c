#include "main.h"



#define TMAX 10		 // Configures how often (sec) the ADC is triggered
//#define TMAX 600	 // Configures how often (sec) the ADC is triggered

/**
 * Globals
 */
uint8_t tcount;
uint16_t tMax;
uint8_t temperature;

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	initGPIO();
	initA2D();
}

void initGPIO(void)
{
	/*
	 * Set unused pins to pullup/down enabled to avoid floating inputs
	 */
    P1->REN |= 0xFF;
    P2->REN |= 0xFF;
    P3->REN |= 0xFF;
    P4->REN |= 0xFF;
    P5->REN |= 0xFF;
    P6->REN |= 0xFF;
    P7->REN |= 0xFF;
    P8->REN |= 0xFF;
    P9->REN |= 0xFF;
    P10->REN |= 0xFF;
}

/**
 * AD2 will be used to take a temperature reading from the sensor every 10 min.
 * For testing purposes I will want to make this like every min or half min.
 */
void initA2D(void)
{
    ADC14->CTL0 |= ADC14_CTL0_SHT0_5 | ADC14_CTL0_SHP | ADC14_CTL0_SSEL_2 | ADC14_CTL0_ON;
    ADC14->CTL1 |= ADC14_CTL1_RES_2;  // 12-bit conversion
    ADC14->CTL1 &= ~ADC14_CTL1_RES_1;  // 12-bit conversion
    ADC14->CTL1 |= BIT0  // use MEM[0]
    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_6;  // input on A6
    ADC14->IER0 |= ADC14_IER0_IE4;  // enable interrupt

    NVIC->ISER[0] |=(1<<24);    // enable interrupt for ADC
    return;
}


void initTimer(void)
{
	tcount = 0;
    TIMER_A0->CTL |= TIMER_A_CTL_SSEL__ACLK | TIMER_A_CTL_MC__UP
                       | TIMER_A_CTL_CLR | TIMER_A_CTL_IE;
    TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE | TIMER_A_CCTLN_OUTMOD_7;
    NVIC->ISER[0] |= (1<<8);
    TIMER_A0->CCR[0] = 4096; // 1 sec

}

/**
 * Timer interrupt handling the ADC Start conversion
 */
void TA0_0_IRQHandler(void)
{
    if(tcount >= TMAX)
    {
        tcount++;
        ADC14->CTL0 |= ADC14_CTL0_ENC | ADC14_CTL0_SC;  // enable and start conversion
    }
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;      // clear interrupt flag
}

/**
 * Triggered by conversion complete flag
 */
void ADC14_IRQHandler(void)
{
    // Temperature conversion complete flag for MEM[0]
    if(ADC14->IV == 0x0E)
    {
		//Temp in °C = [(Vout in mV) - 500] / 10
		int inputVoltage = ADC14->MEM[0];
		temperature = (inputVoltage - 500.0) / 10.0;
	}
}
