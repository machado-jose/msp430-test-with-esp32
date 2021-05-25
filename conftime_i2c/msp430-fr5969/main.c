/*
 * main.c
 *
 *  Created on: 8 de mar de 2021
 *  Author: José Gustavo
 */

#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./lib/functions.h"

void SetupCLK(void);

void SetupLedPins(void);
void SetupI2CPins(void);

void SetupI2CTX(void);
void SetupI2CRX(void);
void TransmitI2C(int size_data);
void ReceiveI2C(int size_data);

void SetupUART(void);
void TransmitUART(time_info *time);

void ToggleLed(void);

char input[100];
unsigned int input_index = 0;

time_info time;
unsigned int time_index = 0;

int type_command = 0;

int main(void)
{
    /* Start Configuration */

    WDTCTL = WDTPW | WDTHOLD;                       // Stop Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                           // Enable Config. GPIO

    SetupLedPins();
    SetupI2CPins();

    SetupCLK();
    SetupUART();

    __enable_interrupt();                           // Enable interrupt

    while(1);
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_UART_ISR(void){
    // 0x0D -> Enter
    if(UCA0RXBUF != 'z'){
        input[input_index] = UCA0RXBUF;
        input_index++;
    }else{
        if(strstr(input, "-t") != NULL){
            const char ch = 't';
            char *ret;

            SetupI2CTX();
            resetTime(&time);

            ret = memchr(input, ch, strlen(input));
            memmove(ret, ret + 1, strlen(ret));

            getTimeInteger(ret, &time);

            input_index = 0;
            memset(input, 0, sizeof(input));
            time_index = 0;
            type_command = 1;                               // Set Time Command

            ToggleLed();

            TransmitI2C(sizeof(time.sentence_hex));

            while (UCB0CTL1 & UCTXSTP);
            ToggleLed();
        }else if(strstr(input, "-g") != NULL){

            memset(input, 0, sizeof(input));
            input_index = 0;
            time_index = 0;
            type_command = 2;                               // Get Time Without to Save Command

            ToggleLed();

            SetupI2CTX();
            TransmitI2C(1);
            while (UCB0CTL1 & UCTXSTP);
            SetupI2CRX();
            ReceiveI2C(7);                                  // Clear Stop Flag
            TransmitUART(&time);

            ToggleLed();
        }
    }
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
    {
        case USCI_NONE:          break;         // Vector 0: No interrupts
        case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
        case USCI_I2C_UCNACKIFG: break;
        case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
        case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
        case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
        case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
        case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
        case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
        case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
        case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
        case USCI_I2C_UCRXIFG0:                 // Vector 22: RXIFG0
            if(type_command == 2){
                time.sentence_hex[time_index] = UCB0RXBUF;
                time_index++;
                if(time_index == 7){
                    type_command = 0;
                    time_index = 0;
                    hexToDec(&time);
                    __bic_SR_register_on_exit(LPM0_bits);
                }
            }
            break;
        case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
            if(type_command == 1){
                /* Verify the information byte last */
                if(time_index == (sizeof(time.sentence_hex) / 2 - 1)){
                    UCB0TXBUF = time.sentence_hex[time_index];
                    time_index = 0;
                    type_command = 0;

                    __delay_cycles(20000);
                    UCB0CTLW0 |= UCTXSTP;                       // I2C stop condition
                    UCB0IFG &= ~UCTXIFG;                        // Clear USCI_B0 TX int flag
                    __bic_SR_register_on_exit(LPM0_bits);       // Exit LPM0
                }else{
                    UCB0TXBUF = time.sentence_hex[time_index];  // Data Transmission for buffer
                    time_index++;
                }
            }else if(type_command == 2){
                UCB0TXBUF = 0;                              // Send Register Address
                __delay_cycles(20000);
                __bic_SR_register_on_exit(LPM0_bits);
            }
            break;
        case USCI_I2C_UCBCNTIFG: break;         // Vector 26: BCNTIFG
        case USCI_I2C_UCCLTOIFG: break;         // Vector 28: clock low timeout
        case USCI_I2C_UCBIT9IFG: break;         // Vector 30: 9th bit
        default: break;
    }
}

void SetupLedPins(void){
    /* Set Leds */
    P1DIR |= BIT0;                                  // Green Led
    P1OUT |= BIT0;                                  // Led on
    P4DIR |= BIT6;                                  // Red Led
    P4OUT &= ~BIT6;                                 // Led off
}

void SetupI2CPins(void){
    /* Set I2C Pins */
    P1SEL1 |= BIT6;
    P1SEL0 &= ~BIT6;                                // SDA Config
    P1SEL1 |= BIT7;
    P1SEL0 &= ~BIT7;                                // SCL Config
}

void ToggleLed(void){
    P1OUT ^= BIT0;                                  // Toggle Green Led
    __delay_cycles(2000);
    P4OUT ^= BIT6;                                  // Toggle Red Led
    __delay_cycles(2000);
}

void SetupCLK(void){
    /* Setup SMCLK to 8MHz */
    CSCTL0_H = CSKEY >> 8;                          // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;                   // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;           // Set all dividers
    CSCTL0_H = 0;                                   // Lock CS registers
}

void SetupUART(void){
    /* Set UART Protocol */

    UCA0CTLW0 |= UCSWRST;                           // Setup the UART Mode; Enable SW reset;
    UCA0CTLW0 |= UCSSEL__SMCLK;                     // Use SMCLK
    UCA0BRW = 4;                                    // Low bit of UCBRx is 4
    UCA0MCTLW = 0x5551;                             // Second modulation stage select is 1; Baud Rate = 115200

    P2SEL1 |= BIT1;                                 // USCI_A0 UART RX
    P2SEL0 &= ~BIT1;

    P2SEL1 |= BIT0;                                 // USCI_A0 UART TX
    P2SEL0 &= ~BIT0;

    UCA0CTLW0 &= ~UCSWRST;                          // Initialize eUSCI

    UCA0IE |= UCRXIE;                               // Enable USCI_A0 RX interrupt
}

void TransmitUART(time_info *time){
    char str[7];
    unsigned int i, second, minute, hour, weekday, day, month, year;
    second = time->sentence_decimal[0];
    minute = time->sentence_decimal[1];
    hour = time->sentence_decimal[2];
    weekday = time->sentence_decimal[3];
    day = time->sentence_decimal[4];
    month = time->sentence_decimal[5];
    year = time->sentence_decimal[6];

    sprintf(str, "%d%s%d%s%d%s%d%s%d%s%d%s%d", second, ",", minute, ",", hour, ",", weekday, ",", day, ",", month, ",", year);
    for(i = 0; i < strlen(str); i++){
        UCA0TXBUF = str[i];
        __delay_cycles(1009);
    }
}

void SetupI2CTX(void){
    __disable_interrupt();
    while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent

    UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC;   // I2C mode, Master mode, sync
    UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated
                                              // after UCB0TBCNT is reached
    UCB0BRW = 0x0020;                         // baudrate = SMCLK / 20
    UCB0I2CSA = 0x0068;                       // Slave address
    UCB0CTLW0 &= ~UCSWRST;

    UCB0IE |= UCTXIE;
}

void SetupI2CRX(void){
    __disable_interrupt();
    while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent

    UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC;   // I2C mode, Master mode, sync
    UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated
                                              // after UCB0TBCNT is reached
    UCB0BRW = 0x0020;                         // baudrate = SMCLK / 20
    UCB0I2CSA = 0x0068;                       // Slave address
    UCB0CTLW0 &= ~UCSWRST;

    UCB0IE |= UCRXIE;
}

void ReceiveI2C(int size_data){
    while (UCB0CTL1 & UCTXSTP);
    UCB0TBCNT = size_data;
    UCB0CTLW0 &= ~UCTR;
    UCB0CTL1 |= UCTXSTT;
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}

void TransmitI2C(int size_data){
    while (UCB0CTL1 & UCTXSTP);
    UCB0TBCNT = size_data;
    UCB0CTL1 |= UCTR + UCTXSTT;
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}
