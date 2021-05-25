/*
 * functions.c
 *
 *  Created on: 8 de mar de 2021
 *  Author: José Gustavo
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./functions.h"

void getTimeInteger(char *data, time_info* time){

    char *token;
    char res[8][3];
    unsigned int i = 0;
    const char *s;

    token = strtok(data, " ");

    while( token != NULL ) {
        strcpy(res[i], token);
        i++;
        token = strtok(NULL, " ");
    }

    for(i = 0; i < 8; i++){
        s = res[i];
        time->sentence_decimal[i] = atoi(s);
    }

    decToHex(time);

    memset(res, 0, sizeof(res));

}

void resetTime(time_info* time){
    unsigned int i;
    for(i = 0; i < sizeof(time->sentence_decimal); i++){
        time->sentence_decimal[i] = 0;
        time->sentence_hex[i] = 0;
    }
}

void decToHex(time_info* time){
    unsigned int i;
    for(i = 0; i < sizeof(time->sentence_decimal) / 2; i++){
        time->sentence_hex[i] = (time->sentence_decimal[i] / 10) * 16 + (time->sentence_decimal[i] % 10);
    }
}

void hexToDec(time_info* time){
    unsigned int i;
    for(i = 0; i < sizeof(time->sentence_hex) / 2; i++){
        time->sentence_decimal[i] = (time->sentence_hex[i] / 16) * 10 + (time->sentence_hex[i] % 16);
    }
}
