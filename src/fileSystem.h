/*
 * fileSystem.h
 *
 *  Created on: Jun 12, 2015
 *      Author: ga57zum
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#define DF 0x00
#define EF 0x01
#define ALL 0x02

#include "sim_os.h"

typedef struct file_node_t file_node;

struct file_node_t{
    Xuint8 id[2];           //file ID
    Xuint8 rw_permission; // stores RW permision
    Xuint8 IR_permission; // stores INVALIDATE and REHABILITATE permision
    file_node *parent_ptr;     //pointer to parent node;
    Xuint8 data_size; //size of data in bytes
    Xuint8 *data; // pointer to data
    unsigned int num_children; //number of children file_nodes
    file_node *child_ptr[]; // array of pointers to children nodes
};

void create_sim_files();
void test_file_system();
file_node * find_node (file_node * current_node, Xuint8 *file_id);

extern file_node *mf_ptr;
extern file_node *current_node_ptr;
extern file_node *current_file_ptr;
#endif /* FILESYSTEM_H_ */
