/*
 * sim_os.c: simple test application
 *  Created on May 6, 2015
 *  Author:PHYLAS
 */

#include "platform.h"
#include "uart.h"
#include "aes.h"
#include "sim_os.h"
#include "fileSystem.h"

Xuint8 gsm_response[23] = {0x00,0x00,0x45,0x10,0x7F,0x20,0x02,0x00,0x00,0x00,0x00,0x00,0x0A,0xBB,0x00,0x0B,0x04,0x00,0x83,0x8A,0x83,0x8A,0x00};
Xuint8 mf_response[23] = {0x00,0x00,0x45,0x10,0x3F,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x0A,0xBB,0x01,0x02,0x04,0x00,0x83,0x8A,0x83,0x8A,0x00};
Xuint8 ef_response[15] = {0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0xFF,0x00,0x01,0x02,0x00, 0x00};

Xuint8 chv[2][4] = {{0x00,0x00,0x00,0x00},{0x00,0x00,0x00,0x00}};
Xuint8 chv_valid[2] = {0,0};

//Cipher key of the card for RUN_GSM_ALG, TODO: init_aes(K_i);
Xuint8 K_i[16] = { 0xF3, 0x54, 0x1F, 0xA3, 0x4B, 0x33, 0x9C, 0x0D,
                   0x80, 0x23, 0x7A, 0xF9, 0x7C, 0x21, 0xD7, 0x3B };

static void get_command(APDU_command *cmd, APDU_response *response, int *errnum) {
    int i = 0;
    Xuint8 in_buffer[6];

    //reading command from uart. cmd is 5 bytes long
    for(i=0; i<5; i++) {
        sim_uart_read_byte(&in_buffer[i]);
    }

    cmd->cla = in_buffer[0]; 
    cmd->ins = in_buffer[1]; 
    cmd->p1 = in_buffer[2]; 
    cmd->p2 = in_buffer[3]; 
    cmd->p3 = in_buffer[4]; 

    if (cmd->cla != CLA) {
        //CLA invalid and aborting
        response->sw1 = 0x6e;
        response->sw2 = 0x00;
        *errnum = ABORT_ERROR;
    }
}

static void execute_command(APDU_command *cmd, APDU_response *response, int *errnum) {
	int i;
	Xuint8 id_buff[cmd->p3];
	file_node *search;
    switch (cmd->ins) {
    case VERIFY_CHV:
    	//send ack
    	sim_uart_write_byte(&cmd->ins);
    	//reading pin
    	Xuint8 pin_buffer[4];
    	Xuint8 incorrect_pin = 0x00;
    	Xuint8 pin_num = cmd->p2 -1;

    	sim_uart_read(pin_buffer,cmd->p3);
    	for (i=0; i<4; i++) {
    		if(chv[pin_num][i] != (pin_buffer[i] & 0x0F) ) {
    			incorrect_pin = 0x01;
    			break;
    		}
    	}
    	if (incorrect_pin == 0x01) {
    		if (pin_num == 0) {
    			if ((mf_response[18] & 0x0F) > 1) { //chv1
            		mf_response[18] = mf_response[18] - 1;
            		gsm_response[18] = gsm_response[18] - 1;
            		response->sw1 = 0x98;
            		response->sw2 = 0x04;
            		*errnum = WARNING_ERROR;
    			} else {
    	    		response->sw1 = 0x98;
    	    		response->sw2 = 0x40;
    	    		*errnum = ABORT_ERROR;
    			}
    		} else if (pin_num == 1) { // chv2
    			if ((mf_response[20]  & 0x0F)> 1) {
            		mf_response[20] = mf_response[20] - 1;
            		gsm_response[20] = gsm_response[20] - 1;
            		response->sw1 = 0x98;
            		response->sw2 = 0x04;
            		*errnum = WARNING_ERROR;
    			} else { // no more
    	    		response->sw1 = 0x98;
    	    		response->sw2 = 0x40;
    	    		*errnum = ABORT_ERROR;
    			}
    		}
    	} else {
    		chv_valid[pin_num] = 1;
    		response->sw1 = 0x90;
    		response->sw2 = 0x00;
    	}
        break;
        /*case RUN_GSM_ALG:
                //Only accessible when CHV1 verification successful
                if(chv_valid[0] == 0){
                	response->sw1 = 0x98;
                	response->sw2 = 0x04;
                	*errnum = WARNING_ERROR;
                	break;
                }
                Xuint8 i;

                //send ack
            	sim_uart_write_byte(&cmd->ins);

            	//read RAND (16 bytes)
            	Xuint8 RAND_buffer[16];
            	sim_uart_read(RAND_buffer,0x10);

            	//call AES
            	//AES module should be initialized to use the key K_i of the card, TODO
            	aes_encrypt(RAND_buffer, K_i);

                //respond
                //SRES (output of A3) are the first 4 bytes
                //Kc (output of A8) are the following 8 bytes (4 to 11)
            	sim_uart_write(RAND_buffer,0x0C); //only the first 12 out of 16 bytes

            	id_buff[0] = EF_KC_ID >> 8;
            	id_buff[1] = EF_KC_ID & 0xFF;

            	search = find_node(current_file_ptr, id_buff);
        		if (search != NULL) {
        			for (i = 4; i <12; i++ ) {
        				search->data[i-4] = RAND_buffer[i];
        			}
        	    	response->sw1 = 0x90;
        			response->sw2 = 0x00;
        			*errnum = NO_ERROR;
        		} else { //file not found
        			response->sw1 = 0x94;
        			response->sw2 = 0x04;
        			*errnum = WARNING_ERROR;
        		}
                break;*/
    case STATUS:
    	//send ack
    	sim_uart_write_byte(&cmd->ins);
		if (current_file_ptr->id[0] == 0x7f && current_file_ptr->id[1] == 0x20 ) {
			sim_uart_write(gsm_response,cmd->p3);
			response->sw1 = 0x90;
			response->sw2 = 0x00;
			*errnum = NO_ERROR;
		} else if (current_file_ptr->id[0] == 0x3f && current_file_ptr->id[1] == 0x00) {
			sim_uart_write(mf_response,cmd->p3);
			response->sw1 = 0x90;
			response->sw2 = 0x00;
			*errnum = NO_ERROR;
		} else {
			response->sw1 = 0x6f;
			response->sw2 = 0x00;
			*errnum = ABORT_ERROR;
		}
        break;
    case READ_BINARY:
    	//sending ack
    	sim_uart_write_byte(&cmd->ins);
    	if (cmd->p3 <= current_node_ptr->data_size) {
        	//sending requested data to uart
        	sim_uart_write(current_node_ptr->data, cmd->p3);
    		response->sw1 = 0x90;
    		response->sw2 = 0x00;
    		*errnum = NO_ERROR;
    	} else {
    		//data requested is larger than stored
    		response->sw1 = 0x67;
    		response->sw2 = 0x00;
    		*errnum = WARNING_ERROR;
    	}

        break;
    case UPDATE_BINARY:
    	//sending ack
    	sim_uart_write_byte(&cmd->ins);

    	if (cmd->p3 <= current_node_ptr->data_size) {
    		sim_uart_read(current_node_ptr->data, cmd->p3);
    		response->sw1 = 0x90;
    		response->sw2 = 0x00;
    		*errnum = NO_ERROR;
    	} else {
    		response->sw1 = 0x67;
    		response->sw2 = 0x00;
    		*errnum = WARNING_ERROR;
    	}
        break;
    case SELECT:
		//sending ack
		sim_uart_write_byte(&cmd->ins);

		//reading file id
		sim_uart_read(id_buff,cmd->p3);

		//searching for file by id
		search = find_node(current_file_ptr, id_buff);
		if (search != NULL) {
			response->sw1 = 0x9F;
			current_node_ptr = search;
			if(current_node_ptr->data_size == 0) {
				current_file_ptr = current_node_ptr;
				response->sw2 = 0x17;
			} else {
				response->sw2 = 0x0F;
			}
		} else { //file not found
			response->sw1 = 0x94;
			response->sw2 = 0x04;
		}
        break;
	case GET_RESPONSE:
		//sending ack
		sim_uart_write_byte(&cmd->ins);

		if (current_node_ptr->id[0] == 0x7f && current_node_ptr->id[1] == 0x20 ) {
			sim_uart_write(gsm_response,cmd->p3);
			response->sw1 = 0x90;
			response->sw2 = 0x00;
		} else if (current_node_ptr->id[0] == 0x3f && current_node_ptr->id[1] == 0x00) {
			sim_uart_write(mf_response,cmd->p3);
			response->sw1 = 0x90;
			response->sw2 = 0x00;
		} else {
			//EF
			ef_response[2] = 0x00;
			ef_response[3] = current_node_ptr->data_size;
			ef_response[4] = current_node_ptr->id[0];
			ef_response[5] = current_node_ptr->id[1];
			ef_response[8] = current_node_ptr->rw_permission;
			ef_response[10] = current_node_ptr->IR_permission;
			sim_uart_write(ef_response,cmd->p3);
			response->sw1 = 0x90;
			response->sw2 = 0x00;
		}
		*errnum = NO_ERROR;
        break;
    default:
        //command not supported
        response->sw1 = 0x6d;
        response->sw2 = 0x00;
        *errnum = WARNING_ERROR;
    }
}

static void test_aes () {
	int i=0;
	unsigned char key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,0x0e, 0x0f};
	unsigned char text[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
	
	xil_printf("%s\n", "DEMO FOR AES API");
	xil_printf("%s\n","Text:");

	for(i=0;i<16;i++)  xil_printf("%02x, ", key[i]);
	xil_printf("\n%s\n" "Key:");
	for(i=0;i<16;i++)  xil_printf("%02x, ", text[i]);

	aes_encrypt(text, key);

	xil_printf("\n%s\n" "Cipher:");
	for(i=0;i<16;i++)  xil_printf("%02x, ", text[i]);
}

int main() {
    int errnum = 0;

    APDU_command cmd;
    APDU_response response;

    init_platform();
    create_sim_files();

    //main loop
    while(1){
        //getting command from uart
		get_command(&cmd, &response, &errnum);
		if (errnum == ABORT_ERROR) {
			sim_uart_write_byte(&response.sw1);
			sim_uart_write_byte(&response.sw2);
			break;
		}

		execute_command(&cmd, &response, &errnum);
		//wrting status bytes
		sim_uart_write_byte(&response.sw1);
		sim_uart_write_byte(&response.sw2);

		if (errnum == ABORT_ERROR) {
			break;
		}
    }
    return 0;
}

