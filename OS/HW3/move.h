#ifndef HW3_MOVE_H
#define HW3_MOVE_H

#include "utils.h"
#include "find.h"
#include "read.h"

void moveDirectory(FILE* disk, uint32_t src_cluster, uint32_t dest_cluster, BPB_struct boot_sector, string name);

void moveFile(FILE* disk, uint32_t src_cluster, uint32_t dest_cluster, BPB_struct boot_sector, string name);


#endif //HW3_MOVE_H
