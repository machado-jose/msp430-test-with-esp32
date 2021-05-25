/*
 * functions.h
 *
 *  Created on: 18 de abr de 2021
 *      Author: José Gustavo
 */

#ifndef LIB_FUNCTIONS_H_
#define LIB_FUNCTIONS_H_

int* getDataFromTerminal(char *data);
void TransmitDataSPI(int *data);
void ReceiveDataSPI(int size_data);
void TransmitDataUART(int *data, int size_data);
void ReceiveDataPassingAddressSPI(int address, int size_data);

#endif /* LIB_FUNCTIONS_H_ */
