#ifndef HW3_UTILS_H
#define HW3_UTILS_H

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <fcntl.h>
#include "unistd.h"
#include "fat.h"
#include <ctime>
#include <sys/time.h>
#include <bitset>
#include <cmath>

using namespace std;

// COMMAND ENUMERATION
typedef enum {QUIT, CD, LS, MKDIR, MV, CAT, TOUCH} command_type;

// OVERALL SHELL COMMAND STRUCTURE INCLUDING PARAMETERS
typedef struct command{
    command_type cmd;
    vector<string> params;
} command;

// FOR PARSING INPUT
void parser(command &cmd);

// FOR PRINTING THE CURRENT WORKING DIRECTORY LIKE SHELL DURING EXECUTION
void cur_dir_prompt(vector<string> dir);

// FOR SPLITTING THE GIVEN ABSOLUTE/RELATIVE PATH WITH THE DELIMITER '/'
void tokenizer(string str, vector<string> & cur_directory);

// FOR EXTRACTING THE LONG FILENAME FROM DIRECTORY ENTRY
string directory_name_converter(FatFileLFN dir);

unsigned char lfn_checksum(int create_count);

uint16_t str_to_uint16(string str);

void copyFat(FILE* disk, BPB_struct boot_sector);

void updateModificationTime(FILE* disk, uint32_t cluster, BPB_struct boot_sector);

#endif //HW3_UTILS_H
