/*
 * uart.h
 *
 *  Created on: May 28, 2015
 *      Author: ga57zum
 */

#ifndef UART_H_
#define UART_H_
#include "xbasic_types.h"

#define UART_BASE_ADDRESS 0x7BE00000
#define UART_NO_DATA_AVAILABLE 0x11000000

void init_sim_uart(int *errnum);
void sim_uart_write(Xuint8 *InputBufferPtr, Xuint8 NumBytes);
void sim_uart_write_byte(Xuint8 *InputBufferPtr);
void sim_uart_read_byte(Xuint8 *OutputBufferPtr);
void sim_uart_read(Xuint8 *OutputBufferPtr, Xuint8 NumBytes);

#endif /* UART_H_ */
