#ifndef HW3_FIND_H
#define HW3_FIND_H

#include "utils.h"
#include "read.h"

 // FOR FINDING THE DESIRED DIRECTORY WITHIN THE WORKING DIRECTORY (RELATIVE) OR IN THE FILE SYSTEM (ABSOLUTE)
 int32_t findDirectory(FILE* disk, int32_t cluster, BPB_struct & boot_sector, vector<string> dir);

int findEmptyCluster(FILE* disk,BPB_struct boot_sector);

int32_t findFile(FILE* disk, int32_t cluster, BPB_struct boot_sector, vector<string> path);

#endif //HW3_FIND_H
