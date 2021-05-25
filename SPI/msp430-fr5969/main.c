#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "./lib/functions.h"

char input[100];
unsigned int input_index = 0;
int *data;
int *rx_data;

void SetupClock(void);
void SetupUart(void);
void SetupSPI(void);

void SetupLedPins(void);
void SetupSpiPins(void);
void ToggleLed(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                       // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;

    SetupClock();
    SetupLedPins();
    SetupUart();
    SetupSPI();
    SetupSpiPins();

    __bis_SR_register(LPM0_bits + GIE);

    while(1){}
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_UART_ISR(void){
    if(UCA0RXBUF != 'z'){
        input[input_index] = UCA0RXBUF;
        input_index++;
    }else{
        if(strstr(input, "-s") != NULL){
            const char ch = 's';
            char *ret;

            ToggleLed();

            ret = memchr(input, ch, strlen(input));
            memmove(ret, ret + 1, strlen(ret));
            data = getDataFromTerminal(ret);

            TransmitDataSPI(data);

            memset(input, 0, sizeof(input));
            input_index = 0;
            ToggleLed();
        }else if(strstr(input, "-g") != NULL){
            const char ch = 'g';
            char *ret;
            int data_size = 0;

            ToggleLed();
            ret = memchr(input, ch, strlen(input));
            memmove(ret, ret + 2, strlen(ret));

            data_size = atoi(ret);
            ReceiveDataSPI(data_size);

            memset(input, 0, sizeof(input));
            input_index = 0;
            ToggleLed();
        }else if(strstr(input, "-a") != NULL){
            const char ch = 'a';
            char *ret;

            ToggleLed();

            ret = memchr(input, ch, strlen(input));
            memmove(ret, ret + 1, strlen(ret));
            data = getDataFromTerminal(ret);

            ReceiveDataPassingAddressSPI(data[0], data[1]);

            memset(input, 0, sizeof(input));
            input_index = 0;
            ToggleLed();
        }
    }
}

void SetupLedPins(void){
    /* Set Leds */
    P1DIR |= BIT0;                                  // Green Led
    P1OUT |= BIT0;                                  // Led on
    P4DIR |= BIT6;                                  // Red Led
    P4OUT &= ~BIT6;                                 // Led off
}

void ToggleLed(void){
    P1OUT ^= BIT0;                                  // Toggle Green Led
    __delay_cycles(2000);
    P4OUT ^= BIT6;                                  // Toggle Red Led
    __delay_cycles(2000);
}

void SetupClock(void){
    /* Setup SMCLK to 8MHz */
    CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
    CSCTL0_H = 0;                             // Lock CS registers
}

void SetupSPI(void){
    /* Setup SPI B0 */
    UCB0CTLW0 |= UCSWRST;                     // SW Reset

    UCB0CTLW0 |= UCSSEL__SMCLK;

    UCB0CTLW0 |= UCMST | UCSYNC | UCCKPH | UCMSB | UCMODE_1 | UCSTEM;

    UCB0BR0 |= 0x04;                          // 2 MHz SPI clock
    UCB0BR1 |= 0x00;

    UCB0CTLW0 &= ~UCSWRST;
}

void SetupUart(void){
    /* Setup UART A0: 115200 baud */
    UCA0CTLW0 |= UCSWRST;                     // SW Reset

    UCA0CTLW0 |= UCSSEL__SMCLK;               // BRCLK = SMCLK (8MHz)
    UCA0BRW = 4;                              // Prescalar = 4
    UCA0MCTLW = 0x5551;                       // Modulation = 0x5551; UCBRS = 0x55; UCBRF = 5; UCOS16 = 1

    P2SEL1 |= BIT0;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT0;                          // To transmit test result from computer terminal (/dev/ttyACM*)

    P2SEL1 |= BIT1;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT1;                          // To receive test command from terminal

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void SetupSpiPins(void){
    P1SEL1 |= BIT6;                           // SIMO
    P1SEL0 &= ~BIT6;

    P1SEL1 |= BIT7;                           // SOMI
    P1SEL0 &= ~BIT7;

    P2SEL1 |= BIT2;                           // SCLK
    P2SEL0 &= ~BIT2;

    P1DIR |= BIT5;                            // CS
    P1OUT |= BIT5;
}
