/*
 * main.c
 *
 *  Created on: 27 de jan de 2021
 *  Author: Jose Gustavo
 */

#include <msp430.h>
#include <string.h>

#include "lib/gps/gps.h"
#include "lib/gps/nmea/sentence.h"

void SetupUartTerminal(void);
void SetupUartGPS(void);
void SetupLedPins(void);
void ToggleLed(void);

unsigned short int sentence_ctl = 0;

/* Commands to Send */
char *command = "PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
char *command_2 = "PMTK220,1000";

/* Received Command Buffer */
char received_command_buff[80];
unsigned int rx_byte_cnt = 0;

nmeaGPRMC pack;

int main(void)
{
    nmea_zero_GPRMC(&pack);

    WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog
    PM5CTL0 &= ~LOCKLPM5;

    SetupLedPins();
    SetupUartTerminal();
    SetupUartGPS();

    __enable_interrupt();

    while(1){}

}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    switch(UCA0RXBUF){
        case('y'):
            ToggleLed();
            send_command(command);
            ToggleLed();
            break;
        case('t'):
            ToggleLed();
            send_command(command_2);
            ToggleLed();
            break;
        case('b'):
            ToggleLed();
            received_command(received_command_buff);
            update_information(received_command_buff, &pack);
            ToggleLed();
            break;
        case('u'):
            ToggleLed();
            get_position(&pack);
            ToggleLed();
            break;
        default:
            break;
    }
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
    /* Line Feed (\n): 0xA */
    if(UCA1RXBUF == 0xA){
        sentence_ctl = 0;
    }else if(UCA1RXBUF == 0x24){
        sentence_ctl = 1;
    }

    if(sentence_ctl == 1){
        /* Hex Code of '$' is 0x24. Restart received_command_buff, because is a new command. */
        if(UCA1RXBUF == 0x24){
            memset(received_command_buff, 0, sizeof(received_command_buff));
            rx_byte_cnt = 0;
        }
        received_command_buff[rx_byte_cnt] = UCA1RXBUF;
        rx_byte_cnt++;
    }
}

void SetupLedPins(void){
    /* Set Led */
    P1DIR |= BIT0;                            // Green Led
    P1OUT |= BIT0;                            // Led on
    P4DIR |= BIT6;                            // Red Led
    P4OUT &= ~BIT6;                           // Led off
}

void ToggleLed(void){
    P1OUT ^= BIT0;                            // Toggle Green Led
    __delay_cycles(2000);
    P4OUT ^= BIT6;                            // Toggle Red Led
    __delay_cycles(2000);
}

void SetupUartTerminal(void){
    /* Setup UART A0 - 115200 baud */
    UCA0CTLW0 |= UCSWRST;                     // SW Reset

    UCA0CTLW0 |= UCSSEL__SMCLK;               // BRCLK = SMCLK (1MHz)
    UCA0BRW = 8;                              // Divisor = 8
    UCA0MCTLW = 0xD600;                       // Modulation = 0xD600; UCBRS = 0xD6; UCBRF = 0; UCOS16 = 0

    P2SEL1 |= BIT0;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT0;                          // To transmit test result from computer terminal (/dev/ttyACM*)

    P2SEL1 |= BIT1;                           // USCI_A0 UART operation
    P2SEL0 &= ~BIT1;                          // To receive test command from terminal

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void SetupUartGPS(void){
    /* Setup UART A1 - 9600 baud */
    UCA1CTLW0 |= UCSWRST;                     // SW Reset

    UCA1CTLW0 |= UCSSEL__SMCLK;               // BRCLK = SMCLK (1MHz)
    UCA1BRW = 6;                              // Divisor = 6
    UCA1MCTLW = 0x2081;                       // Modulation = 0x2081; UCBRS = 0x20; UCBRF = 8; UCOS16 = 1

    P2SEL1 |= BIT5;                           // USCI_A1 UART operation
    P2SEL0 &= ~BIT5;                          // To transmit GPS command

    P2SEL1 |= BIT6;                           // USCI_A1 UART operation
    P2SEL0 &= ~BIT6;                          // To received GPS command

    UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI

    UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
}
