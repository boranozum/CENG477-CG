#ifndef HW3_READ_H
#define HW3_READ_H

#include "utils.h"

// FOR READING THE BOOT SECTOR INTO "boot_sector" VARIABLE
void readBootSector(FILE* disk, BPB_struct& boot_sector);

void readLFN(FILE* disk, void* dir_entry, uint32_t location, BPB_struct boot_sector);

// FOR READING A 32 BYTE DIRECTORY ENTRY INTO "dir_entry" VARIABLE AT THE "location"
void readEntry(FILE* disk, void* dir_entry, uint32_t location, BPB_struct & boot_sector);

// FOR PRINTING THE CONTENTS OF THE DESIRED DIRECTORY -- arg==true -> -l
void readDirectory(FILE * disk, uint32_t location, BPB_struct & boot_sector, bool arg, int & file_count);

void readFile(FILE* disk, uint32_t cluster, BPB_struct boot_sector, string file_name);

uint32_t readFAT(FILE* disk, uint32_t cluster, BPB_struct boot_sector);

#endif //HW3_READ_H
