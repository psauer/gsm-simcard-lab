/*
 * aes.c
 *
 *  Created on: May 28, 2015
 *      Author: ga57zum, ga24hel
 */

#include "aes.h"
//#include <string.h>

#define AES_BASE_REGISTER 0x77A00000
void aes_encrypt(Xuint8 *text, Xuint8 *key) {
	Xuint32 key_int;
	Xuint32 text_int;
	Xuint32 cipher_int[4];
	Xuint8* temp;
	int i;
	//sending key
	for(i = 0; i < 4; i++) {
		key_int =  (key[i*4+3] << 24) + (key[i*4+2] << 16) + (key[i*4+1] << 8) + key[i*4];
		Xil_Out32(AES_BASE_REGISTER + i,key_int);
	}
	
	//sending text to be encrypted
	for(i = 0; i < 4; i++) {
		text_int =  (text[i*4+3] << 24) + (text[i*4+2] << 16) + (text[i*4+1] << 8) + text[i*4];
		Xil_Out32(AES_BASE_REGISTER + (4 + i),text_int);
	}	
	
	//geting back encrypted text
	while (!(Xil_In32(AES_BASE_REGISTER + 12) == AES_BUSY)){}
	cipher_int[0] = Xil_In32(AES_BASE_REGISTER + 8);
	cipher_int[1] = Xil_In32(AES_BASE_REGISTER + 9);
	cipher_int[2] = Xil_In32(AES_BASE_REGISTER + 10);
	cipher_int[3] = Xil_In32(AES_BASE_REGISTER + 11);

	temp = (Xuint8*)cipher_int;
	for(i = 0; i < 16; i++)  text[i] = temp[i];
}

