/*
 * aes.h
 *
 *  Created on: May 28, 2015
 *      Author: ga57zum
 */

#ifndef AES_H_
#define AES_H_

#include "xbasic_types.h"		//Let's hope, that it includes "Xuint8"
#include "xil_io.h"

#define AES_BUSY 0xF0F0F0F0


//Executes encryption in module, writes back result in buf
void aes_encrypt(Xuint8 *key, Xuint8 *text);

//C replacement AES
//void aes128_encrypt(Xuint8 *buf, const Xuint8 *key);

#endif /* AES_H_ */
