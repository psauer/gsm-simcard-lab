/*
 * sim_os.h
 *
 *  Created pm: June 5, 2015
 *      Author: ga57zum
 *  
 *  This file decodes the command name to instruction n
 */
#ifndef SIMOS_H_
#define SIMOS_H_

#include <stdlib.h>
#include "xbasic_types.h"
#include "error_handling.h"
#include "xil_printf.h"

//#define DEBUG 1

/* This part contains the inst number
 *
 */
//security commands
#define CHANGE_CHV 0x24
#define DISABLE_CHV 0x26
#define ENABLE_CHV 0x28
#define RUN_GSM_ALG 0x88
#define UNBLOCK_CHV 0x2c
#define VERIFY_CHV 0x20

//FILE OPERATIONS
#define INCREASE 0x32
#define INVALIDATE 0x04
#define READ_BINARY 0xb0
#define READ_RECORD 0xb2
#define REHABILITATE 0x44
#define SEEK 0xa2
#define SELECT 0xa4
#define STATUS 0xf2
#define UPDATE_BINARY 0xd6
#define UPDATE_RECORD 0xdc

//SIM APPLICATION TOOLKIT COMMANDS
#define ENVELOPE 0xc2
#define FETCH 0x12
#define TERMINAL_PROFILE 0x10
#define TERMINAL_RESPONSE 0x14

//MISCELLANEOUS COMMANDS
#define GET_RESPONSE 0xc0
#define SLEEP 0xfa

//CLA for GSM is 0xa0
#define CLA 0xa0

//File r_permission
#define R_ALWAYS 0x00
#define R_CHV1 0x10
#define R_CHV2 0x20
#define R_RFU 0x30
#define R_ADM 0x50
#define R_NEVER 0xf0
//File w_permission
#define W_ALWAYS 0x00
#define W_CHV1 0x01
#define W_CHV2 0x02
#define W_RFU 0x03
#define W_ADM 0x05
#define W_NEVER 0x0f

//File INVALIDATE_permission
#define INVALIDATE_ALWAYS 0x00
#define INVALIDATE_CHV1 0x01
#define INVALIDATE_CHV2 0x02
#define INVALIDATE_RFU 0x03
#define INVALIDATE_ADM 0x05
#define INVALIDATE_NEVER 0x0f
//File REHABILITATE_permission
#define REHABILITATE_ALWAYS 0x00
#define REHABILITATE_CHV1 0x10
#define REHABILITATE_CHV2 0x20
#define REHABILITATE_RFU 0x30
#define REHABILITATE_ADM 0x50
#define REHABILITATE_NEVER 0xf0


//File ID
#define MF_ID 0x3f00
//DF under MF
#define DF_GSM_ID 0x7f20
#define DF_TELECOM_ID 0x7f10
//EF under MF
#define EF_ICCID_ID 0x2fe2
#define EF_ELP_ID 0x2f05
//EF under DF_GSM
#define EF_LP_ID 0x6f05
#define EF_IMSI_ID 0x6f07
#define EF_KC_ID 0x6f20
#define EF_HPPLMN_ID 0x6f31
#define EF_SST_ID 0x6f38
#define EF_BCCH_ID 0x6f74
#define EF_ACC_ID 0x6f78
#define EF_FPLMN_ID 0x6f7b
#define EF_LOCI_ID 0x6f7e
#define EF_AD_ID 0x6fad
#define EF_PHASE_ID 0x6fae

//number of children for each folder
#define MF_CHILDREN 4 // DF_GSM, DF_TELECOM, EF_ICCID, EF_ELP
#define DF_GSM_CHILDREN 11 // see 
#define DF_TELECOM_CHILDREN 0

#define FILE_ID_LENGTH 2
#define MAX_FILE_BUFFER_SIZE 20

typedef struct APDU_command_t{
    Xuint8 cla;
    Xuint8 ins;
    Xuint8 p1;
    Xuint8 p2;
    Xuint8 p3;
    Xuint8 *data;
} APDU_command;

typedef struct APDU_response_t{
    Xuint8 sw1;
    Xuint8 sw2;
    Xuint8 ack;
    Xuint8 *data;
    unsigned int  data_length;
} APDU_response;

extern Xuint8 gsm_response[23];
extern Xuint8 mf_response[23];

void print(char *str);
#endif
