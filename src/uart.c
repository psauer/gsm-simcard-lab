/*

 * uart.c
 *
 *  Created on: May 28, 2015
 *      Author: Paul Sauer
 */

#include "uart.h"
#include "error_handling.h"
#include <unistd.h>
#include <stdio.h>
#include "xil_io.h"

void sim_uart_write(Xuint8 *InputBufferPtr, Xuint8 NumBytes) {
	int i;

    for(i = 0; i < NumBytes; i++) {
    	sim_uart_write_byte(&InputBufferPtr[i]);
    }
}

void sim_uart_write_byte(Xuint8 *InputBufferPtr) {
	int dummy = 0;//for uart delay
	Xuint32 i;

	//send on phylas uart
	Xil_Out32(UART_BASE_ADDRESS, (Xuint32) InputBufferPtr[0]);
    for (i = 0 ; i < 14000 ; i++) {
		dummy ++;
    }
    xil_printf("%s %02x\n","W:", InputBufferPtr[0]);
}

void sim_uart_read_byte(Xuint8 *OutputBufferPtr) {
	int parity = 0;
	int i;
	
	Xuint32 read = Xil_In32(UART_BASE_ADDRESS);
	//blocking read
	while (read == UART_NO_DATA_AVAILABLE) {
		read = Xil_In32(UART_BASE_ADDRESS);
	}
	OutputBufferPtr[0] = (Xuint8)(0xFF & read);//take 8 least significant bits.


	for(i = 0; i < 8; i++){
		parity ^= 0x01 & (read>>i);
	}
	//debug print
	xil_printf("%s %s %02x %s %02x %s %02x %s %01x %s %01x \n","R:",
		"reading*:", (0x7F & read>>24),
		"written*:", (0x7F & read>>16),
		"received-value:", (0xFF & read>>0), "parity-rec:", (0x01 & read>>8), "parity-calc:", parity
		);
}

void sim_uart_read(Xuint8 *OutputBufferPtr, Xuint8 NumBytes) {
	int i;

	for (i = 0; i < NumBytes; i++) {
		sim_uart_read_byte(&OutputBufferPtr[i]);
	}
}
