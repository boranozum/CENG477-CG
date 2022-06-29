#ifndef HW3_CREATE_H
#define HW3_CREATE_H

#include "read.h"
#include "utils.h"
#include "find.h"

// FOR CREATING A DIRECTORY
void createDirectory(FILE* disk,uint32_t next_cluster, uint32_t cluster, BPB_struct boot_sector, string dir_name, int create_count);

// FOR CREATING A FILE
void createFile(FILE* disk,int32_t next_cluster, int32_t cluster, BPB_struct boot_sector, string file_name, int create_count);

#endif //HW3_CREATE_H
