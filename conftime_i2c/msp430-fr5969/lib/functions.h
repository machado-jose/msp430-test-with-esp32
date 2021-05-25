/*
 * functions.h
 *
 *  Created on: 8 de mar de 2021
 *  Author: José Gustavo
 */

#ifndef LIBS_FUNCTIONS_H_
#define LIBS_FUNCTIONS_H_

typedef struct
{
  unsigned int sentence_decimal[8];
  unsigned int sentence_hex[8];
} time_info;

void getTimeInteger(char *data, time_info* time);
void resetTime(time_info* time);
void decToHex(time_info* time);
void hexToDec(time_info* time);

#endif /* LIBS_FUNCTIONS_H_ */
