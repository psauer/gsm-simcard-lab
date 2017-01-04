/*
 * fileSystem.c: Where file system handeling fuction are
 *  Created on June 12, 2015
 *  Author:PHYLAS
 */

#include "fileSystem.h"
file_node *mf_ptr = NULL;
file_node *gsm_ptr = NULL;
file_node *current_file_ptr = NULL;
file_node *current_node_ptr = NULL;

//This function creates the DF and updates the parent's child counter as well as it's child_ptr array
static file_node * create_file(Xuint8 *id, Xuint8 rw_permission, Xuint8 ir_permission,file_node *parent_ptr, int data_size, Xuint8 *data_ptr) {
	//allocating memory for structure
    file_node *ptr = malloc(sizeof(file_node));

    //assigning all values of file (except children)
    ptr->id[0] = id[0];
    ptr->id[1] = id[1];
    ptr->rw_permission = rw_permission;
    ptr->IR_permission = ir_permission;
    ptr->parent_ptr = parent_ptr;
    ptr->data_size = data_size;
    ptr->data = data_ptr;
    //DF always has no children
    ptr->num_children = 0;

    //updating parent's child counter and child_ptr array
    parent_ptr->child_ptr[parent_ptr->num_children] = ptr;
    parent_ptr->num_children = parent_ptr->num_children +1;

    return ptr;
}

static file_node * create_folder(Xuint8 *id, file_node *parent_ptr, unsigned int num_children){
    //allocating memory for structure
    file_node *ptr = malloc(sizeof(file_node));

    //assigning all values of file (except children)
    ptr->id[0] = id[0];
    ptr->id[1] = id[1];
    ptr->rw_permission = R_ALWAYS & W_ALWAYS;
    ptr->IR_permission = INVALIDATE_ALWAYS & REHABILITATE_ALWAYS;
    ptr->parent_ptr = parent_ptr;
    ptr->num_children = 0;
    ptr->data_size = 0; //folders always has no data

    //allocating space for child_ptr array
    ptr->child_ptr[0] = malloc(num_children * sizeof(file_node *));

    //updating parent's child counter and child_ptr array
    parent_ptr->child_ptr[parent_ptr->num_children] = ptr;
    parent_ptr->num_children = parent_ptr->num_children +1;

    return ptr;
}

int count_children(file_node * node, Xint8 filter) {
	int counter = 0;
	int i = 0;

	for (i=0; i < node->num_children; i++) {
		if (filter == DF) {
			if (node->child_ptr[i]->data_size == 0) {
				counter++;
			}

		} else if (filter == EF) {
			if (node->child_ptr[i]->data_size != 0) {
				counter++;
			}
		}
	}
	return counter;
}

Xint8 list_children (file_node * node, Xuint8 *output, Xint8 filter){
	int counter = 0;
	int i = 0;
	for (i=0; i < node->num_children; i++) {
	    switch (filter) {
	    case DF:
	    	if (node->child_ptr[i]->data_size == 0){
				output[counter] = node->child_ptr[i]->id[0];
				output[counter+1] = node->child_ptr[i]->id[1];
				counter = counter+2;
	    	}
	        break;
	    case EF:
	    	if (node->child_ptr[i]->data_size != 0){
				output[counter] = node->child_ptr[i]->id[0];
				output[counter+1] = node->child_ptr[i]->id[1];
				counter = counter+2;
	    	}
	        break;
	    case ALL:
			output[counter] = node->child_ptr[i]->id[0];
			output[counter+1] = node->child_ptr[i]->id[1];
			counter = counter+2;
	    	break;
	    default://should never be here
			output[counter] = node->child_ptr[i]->id[0];
			output[counter+1] = node->child_ptr[i]->id[1];
			counter = counter+2;
	    }
	}
	return counter/2;
}
void print_node_data(Xuint8 *data_ptr, int size) {
	int i;

	for (i=0; i< size; i++){
		xil_printf("%02x", data_ptr[i]);
	}
	xil_printf("%s", "\n");
}

file_node * find_node (file_node * current_node, Xuint8 *file_id) {
	file_node *found_node = NULL;
	int i;
	int counter = 0;
	int num_found = 0;

	Xuint8 in_buffer[((current_node->num_children)*2)];//each node id is 2 bytes
	Xuint8 in_buffer2[((current_node->parent_ptr->num_children)*2)];//each node id is 2 bytes

	if (current_node->parent_ptr->id[0] == file_id[0] && current_node->parent_ptr->id[1] == file_id[1]) { //checking parent
		found_node = current_node->parent_ptr;
	} else if (current_node->id[0] == file_id[0] && current_node->id[1] == file_id[1]) { //checking current node
		found_node = current_node;
	} else if (mf_ptr->id[0] == file_id[0] && mf_ptr->id[1] == file_id[1]) { //checking master node
		found_node = mf_ptr;
	} else {
		//checking immediate children
		list_children(current_node, in_buffer, ALL);
		for(i=0; i < current_node->num_children; i++) {
			if (in_buffer[counter] == file_id[0] && in_buffer[counter+1] == file_id[1]) {
				found_node = current_node->child_ptr[i];
				break;
			}
			counter = counter +2;
		}
		//checking any DF which is an immediate child of the parent of the current DF
		if (found_node == NULL) {
			counter = 0;
			num_found = list_children(current_node->parent_ptr, in_buffer2, DF);
			for(i=0; i < num_found; i++) {
				if (in_buffer2[counter] == file_id[0] && in_buffer2[counter+1] == file_id[1]) {
					found_node = current_node->parent_ptr->child_ptr[i];
					break;
				}
				counter = counter +2;
			}
		}
	}
	return found_node;
}

void test_file_system() {

	file_node *found_node = NULL;
	Xuint8 file_id[2];
	int i,counter;

	//printing total number of children in MF
	xil_printf("%s%d\n", "Number of Children in MF = ", mf_ptr->num_children);

	//printing number of EF children
	xil_printf("%s%d\n", "Num Master EF Children = ", count_children(mf_ptr, EF));

	//printing number of DF children
	xil_printf("%s%d\n", "Num Master DF Children = ", count_children(mf_ptr, DF));

	//Listing all children nodes and searching for an ID
	xil_printf("%s\n", "Listing MF Children:");
	Xuint8 in_buffer[((mf_ptr->num_children)*2)];//each node id is 2 bytes
	list_children(mf_ptr, in_buffer, ALL);
	counter=0;
	for(i=0; i < mf_ptr->num_children; i++) {
		xil_printf("%s%02x%02x\n", "Node ID = ",in_buffer[counter],in_buffer[counter+1] );
		counter = counter +2;
	}
	xil_printf("%s\n", "Looking for EF_ELP_ID node in mf");
	file_id[0] = EF_ELP_ID >> 8;
	file_id[1] = EF_ELP_ID & 0xFF;
	found_node = find_node(mf_ptr, file_id);

	//printing out info of found node
	if (found_node != NULL) {
		xil_printf("%s\n", "Found EF_ELP_ID. Printing node info");
		xil_printf("%s%02x%02x\n", "Node ID = ",found_node->id[0],found_node->id[1] );
		xil_printf("%s%d\n", "Data Size = ",found_node->data_size );
		xil_printf("%s", "Data = ");
		print_node_data(found_node->data, found_node->data_size);
		xil_printf("%s%02x%02x\n", "Parent node ID = ",found_node->parent_ptr->id[0],found_node->parent_ptr->id[1] );
		xil_printf("%s%02x\n", "RW Permission = ",found_node->rw_permission );
	}
	//printing total number of children in gsm
	xil_printf("%s%d\n", "Number of Children in gsm = ", gsm_ptr->num_children);

	//printing number of EF children
	xil_printf("%s%d\n", "Num gsm EF Children = ", count_children(gsm_ptr, EF));

	//printing number of DF children
	xil_printf("%s%d\n", "Num gsm DF Children = ", count_children(gsm_ptr, DF));

	//Listing all children nodes and searching for an ID
	xil_printf("%s\n", "Listing gsm Children:");
	Xuint8 in_buffer_gsm[((gsm_ptr->num_children)*2)];//each node id is 2 bytes
	list_children(gsm_ptr, in_buffer_gsm, ALL);
	counter=0;
	for(i=0; i < gsm_ptr->num_children; i++) {
		xil_printf("%s%02x%02x\n", "Node ID = ",in_buffer_gsm[counter],in_buffer_gsm[counter+1] );
		counter = counter +2;
	}
}

void create_sim_files() {
    Xuint8 id[FILE_ID_LENGTH];
    Xuint8 *data_ptr = NULL;
    unsigned int data_size;

    //creating MF
    mf_ptr = malloc(sizeof(file_node));
    //assigning all values of file (except children)

    mf_ptr->id[0] = MF_ID >> 8;
    mf_ptr->id[1] = MF_ID & 0xFF;
    mf_ptr->rw_permission = R_ALWAYS & W_ALWAYS;
    mf_ptr->IR_permission = INVALIDATE_ALWAYS & REHABILITATE_ALWAYS;
    mf_ptr->parent_ptr = mf_ptr;
    mf_ptr->num_children = 0;
    mf_ptr->data_size = 0;
    //allocating space for child_ptr array
    mf_ptr->child_ptr[0] = malloc(MF_CHILDREN * sizeof(file_node *));
    current_node_ptr = mf_ptr;
    current_file_ptr = mf_ptr;
    //creating EF_ICCID. contains unique id. I just made this one up
    data_size = 10;
    id[0] = EF_ICCID_ID >> 8;
    id[1] = EF_ICCID_ID & 0xFF;
    data_ptr = malloc(data_size * sizeof(Xuint8));
    data_ptr[0] = 0x11;
    data_ptr[1] = 0x22;
    data_ptr[2] = 0x33;
    data_ptr[3] = 0x44;
    data_ptr[4] = 0x55;
    data_ptr[5] = 0x66;
    data_ptr[6] = 0x77;
    data_ptr[7] = 0x88;
    data_ptr[8] = 0x99;
    data_ptr[9] = 0x00;
    create_file(id, (R_ALWAYS & W_NEVER) ,(INVALIDATE_ADM & REHABILITATE_ADM), mf_ptr, data_size, data_ptr);

    // EF_ELP Setting main language to english, secondary to german
    data_size = 4;
    id[0] = EF_ELP_ID >> 8;
    id[1] = EF_ELP_ID & 0xFF;
    data_ptr = malloc(data_size * sizeof(Xuint8));
    data_ptr[0] = 0x65;
    data_ptr[1] = 0x6e;
    data_ptr[2] = 0x64;
    data_ptr[3] = 0x65;
    create_file(id, (R_ALWAYS & W_CHV1) , (INVALIDATE_ADM & REHABILITATE_ADM),mf_ptr, data_size, data_ptr);
    // DF_GSM
    id[0] = DF_GSM_ID >> 8;
    id[1] = DF_GSM_ID & 0xFF;
    gsm_ptr = create_folder(id, mf_ptr, DF_GSM_CHILDREN);


    //creating DF_GSM EF's

    // EF_LP. Setting main language english, secondary german
    data_size = 2;
    id[0] = EF_LP_ID >> 8;
    id[1] = EF_LP_ID & 0xFF;
    data_ptr = malloc(data_size * sizeof(Xuint8));
    data_ptr[0] = 0x01;
    data_ptr[1] = 0x00;
    create_file(id, (R_ALWAYS & W_CHV1) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_IMSI
	data_size = 9;
	id[0] = EF_IMSI_ID >> 8;
	id[1] = EF_IMSI_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x08;
	data_ptr[1] = 0x99;
	data_ptr[2] = 0x10;
	data_ptr[3] = 0x07;
	data_ptr[4] = 0x00;
	data_ptr[5] = 0x00;
	data_ptr[6] = 0x00;
	data_ptr[7] = 0x25;
	data_ptr[8] = 0x50;
	create_file(id, (R_CHV1 & W_ADM) , (INVALIDATE_ADM & REHABILITATE_CHV1),gsm_ptr, data_size, data_ptr);

	// EF_KC.
	data_size = 9;
	id[0] = EF_KC_ID >> 8;
	id[1] = EF_KC_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0xFF;
	data_ptr[1] = 0xFF;
	data_ptr[2] = 0xFF;
	data_ptr[3] = 0xFF;
	data_ptr[4] = 0xFF;
	data_ptr[5] = 0xFF;
	data_ptr[6] = 0xFF;
	data_ptr[7] = 0xFF;
	data_ptr[8] = 0x07;
	create_file(id, (R_CHV1 & W_CHV1) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_HPPLMN
	data_size = 1;
	id[0] = EF_HPPLMN_ID >> 8;
	id[1] = EF_HPPLMN_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x50;
	create_file(id, (R_CHV1 & W_ADM) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_BCCH.
	data_size = 16;
	id[0] = EF_BCCH_ID >> 8;
	id[1] = EF_BCCH_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x00;
	data_ptr[1] = 0x00;
	data_ptr[2] = 0x00;
	data_ptr[3] = 0x00;
	data_ptr[4] = 0x00;
	data_ptr[5] = 0x00;
	data_ptr[6] = 0x00;
	data_ptr[7] = 0x00;
	data_ptr[8] = 0x00;
	data_ptr[9] = 0x00;
	data_ptr[10] = 0x00;
	data_ptr[11] = 0x00;
	data_ptr[12] = 0x00;
	data_ptr[13] = 0x00;
	data_ptr[14] = 0x00;
	data_ptr[15] = 0x00;
	create_file(id, (R_CHV1 & W_CHV1) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_ACC.
	data_size = 2;
	id[0] = EF_ACC_ID >> 8;
	id[1] = EF_ACC_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x00;
	data_ptr[1] = 0x10;
	create_file(id, (R_CHV1 & W_ADM) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_FPLMN.
	data_size = 12;
	id[0] = EF_FPLMN_ID >> 8;
	id[1] = EF_FPLMN_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x62;
	data_ptr[1] = 0xF2;
	data_ptr[2] = 0x20;
	data_ptr[3] = 0x62;
	data_ptr[4] = 0xF2;
	data_ptr[5] = 0x30;
	data_ptr[6] = 0x62;
	data_ptr[7] = 0xF2;
	data_ptr[8] = 0x70;
	data_ptr[9] = 0x62;
	data_ptr[10] = 0xF2;
	data_ptr[11] = 0x10;
	create_file(id, (R_CHV1 & W_CHV1) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_LOCI_ID.
	data_size = 11;
	id[0] = EF_LOCI_ID >> 8;
	id[1] = EF_LOCI_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0xFF;
	data_ptr[1] = 0xFF;
	data_ptr[2] = 0xFF;
	data_ptr[3] = 0xFF;
	data_ptr[4] = 0x64;
	data_ptr[5] = 0xF0;
	data_ptr[6] = 0x00;
	data_ptr[7] = 0xFF;
	data_ptr[8] = 0xFE;
	data_ptr[9] = 0x00;
	data_ptr[10] = 0x03;
	create_file(id, (R_CHV1 & W_CHV1) , (INVALIDATE_ADM & REHABILITATE_CHV1),gsm_ptr, data_size, data_ptr);

	// EF_AD.
	data_size = 3;
	id[0] = EF_AD_ID >> 8;
	id[1] = EF_AD_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x00;
	data_ptr[1] = 0x00;
	data_ptr[2] = 0x00;
	create_file(id, (R_ALWAYS & W_ADM) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_PHASE.
	data_size = 1;
	id[0] = EF_PHASE_ID >> 8;
	id[1] = EF_PHASE_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x03;
	create_file(id, (R_ALWAYS & W_ADM) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

	// EF_SST.
	data_size = 2;
	id[0] = EF_SST_ID >> 8;
	id[1] = EF_SST_ID & 0xFF;
	data_ptr = malloc(data_size * sizeof(Xuint8));
	data_ptr[0] = 0x00;
	data_ptr[1] = 0x00;
	create_file(id, (R_CHV1 & W_ADM) , (INVALIDATE_ADM & REHABILITATE_ADM),gsm_ptr, data_size, data_ptr);

    //updating mf get response data
    mf_response[14] = count_children(mf_ptr, DF);
    mf_response[15] = count_children(mf_ptr, EF);
    //updating df_gsm get response data
    gsm_response[14] = count_children(gsm_ptr, DF);
    gsm_response[15] = count_children(gsm_ptr, EF);

}
