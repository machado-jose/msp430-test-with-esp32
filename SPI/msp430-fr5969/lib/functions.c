/*
 * functions.c
 *
 *  Created on: 18 de abr de 2021
 *      Author: José Gustavo
 */

#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./functions.h"

int* getDataFromTerminal(char *data){

    char *token, res[30][3];
    int i = 0, j = 0, *response;
    const char *s;

    token = strtok(data, " ");

    while( token != NULL ) {
        strcpy(res[i], token);
        i++;
        token = strtok(NULL, " ");
    }

    response = (int *)malloc(i * sizeof(int));

    for(j = 0; j < i; j++){
        s = res[j];
        response[j] = atoi(s);
    }

    return response;

}

void TransmitDataSPI(int *data){
    unsigned int position;
    volatile unsigned char RXData = 0;

    UCB0CTL1 &= ~UCSWRST;                   //Start USCI
    P1OUT &= ~BIT5;                         //Pull CSB line low
    for(position = 0; position < sizeof(data); position++)
    {
        while (!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = data[position];
        while(UCB0STAT & UCBUSY);

        while (!(UCB0IFG & UCRXIFG));
        RXData = UCB0RXBUF;
        while(UCB0STAT & UCBUSY);
    }
    P1OUT |= BIT5;                          //Pull CSB line high
    UCB0CTL1 |= UCSWRST;                    //Stop USCI

}

void ReceiveDataPassingAddressSPI(int address, int size_data){
    volatile unsigned char RXData = 0;
    int i = 0, *data;

    data = (int *)malloc(size_data * sizeof(int));

    UCB0CTL1 &= ~UCSWRST;                   //Start USCI
    P1OUT &= ~BIT5;                         //Pull CSB line low

    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = address;
    while(UCB0STAT & UCBUSY);
    while (!(UCB0IFG & UCRXIFG));
    RXData = UCB0RXBUF;
    while(UCB0STAT & UCBUSY);

    for(i = 0; i < size_data; i++){
        while (!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = 0;
        while(UCB0STAT & UCBUSY);
        while (!(UCB0IFG & UCRXIFG));
        data[i] = UCB0RXBUF;
        while(UCB0STAT & UCBUSY);
    }

    P1OUT |= BIT5;                          //Pull CSB line high
    UCB0CTL1 |= UCSWRST;                    //Stop USCI

    TransmitDataUART(data, size_data);

    free(data);
}

void ReceiveDataSPI(int size_data){
    int i = 0, *data;

    data = (int *)malloc(size_data * sizeof(int));

    UCB0CTL1 &= ~UCSWRST;                   //Start USCI
    P1OUT &= ~BIT5;                         //Pull CSB line low

    for(i = 0; i < size_data; i++){
        while (!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = 0;
        while(UCB0STAT & UCBUSY);
        while (!(UCB0IFG & UCRXIFG));
        data[i] = UCB0RXBUF;
        while(UCB0STAT & UCBUSY);
    }

    P1OUT |= BIT5;                          //Pull CSB line high
    UCB0CTL1 |= UCSWRST;                    //Stop USCI

    TransmitDataUART(data, size_data);

    free(data);
}

void TransmitDataUART(int *data, int size_data){
    char *data_string, *buff;
    unsigned int i;

    data_string = (char *)malloc(size_data * sizeof(char));
    buff = malloc(1);

    for(i = 0; i < size_data; i++){
        sprintf(buff, "%d", data[i]);
        data_string[i] = buff[0];
    }

    for(i = size_data; i > 0; --i){
        UCA0TXBUF = data_string[size_data - i];
        __delay_cycles(5000);
        if(i > 1){
            UCA0TXBUF = ',';
            __delay_cycles(5000);
        }
    }

    free(buff);
    free(data_string);
}
