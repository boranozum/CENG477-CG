#include "create.h"

void createDirectory(FILE *disk,uint32_t next_cluster ,uint32_t cluster, BPB_struct boot_sector, string dir_name, int create_count) {

    if(cluster != boot_sector.extended.RootCluster){
        updateModificationTime(disk,cluster,boot_sector);
    }

   

    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (next_cluster-2) * boot_sector.SectorsPerCluster;

    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                 + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    vector<string> dir;

    if(dir_name.length() > 13){

        int index = 0;

        while(index < dir_name.length()){

            string str; 
            if(dir_name.length() - index > 13){
                str = dir_name.substr(index,13);
            }
            else{
                str = dir_name.substr(index,dir_name.length()-index);
            }

            dir.push_back(str);

            index+=13;
        }
    }

    else{
        dir.push_back(dir_name);
    }

    int empty_count = 0;

    while (empty_count != dir.size()+1){
        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,location,boot_sector);

        if(lfn->sequence_number == 0 || lfn->sequence_number == 0xE5){
            empty_count++;
        }

        else{
            empty_count = 0;
        }

        location += 32;

        if(location >= j){
            next_cluster = readFAT(disk,next_cluster,boot_sector);
            if(next_cluster == 268435448){
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);

                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                           + (next_cluster-2) * boot_sector.SectorsPerCluster;

                location *= boot_sector.BytesPerSector;

                delete lfn;

                break;
            }

            else{
                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                           + (next_cluster-2) * boot_sector.SectorsPerCluster;

                location *= boot_sector.BytesPerSector;

                j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                             + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

                j *= boot_sector.BytesPerSector;
            }
        }

        if(empty_count == dir.size()+1){
            location -= 32*empty_count;
            delete lfn;
            break;
        }
    }

    for (int i = dir.size()-1; i >= 0; i--) {

        FatFileLFN *lfn1 = new FatFileLFN;

        uint16_t temp1[13];
        for (int m = 0; m < 13; ++m) {
            if (m < dir[i].length()) {
                temp1[m] = dir[i][m];
            } else if (m == dir[i].length()) {
                temp1[m] = 0;
            } else {
                temp1[m] = -1;
            }
        }

        for (int m = 0; m < 13; ++m) {
            if (m < 5) {
                lfn1->name1[m] = temp1[m];
            } else if (m < 11) {
                lfn1->name2[m - 5] = temp1[m];
            } else {
                lfn1->name3[m - 11] = temp1[m];
            }
        }

        lfn1->attributes = 15;
        lfn1->reserved = 0;
        lfn1->firstCluster = 0;
        lfn1->checksum = lfn_checksum(create_count);
        lfn1->sequence_number = 0;
        if (i == dir.size() - 1) lfn1->sequence_number = lfn1->sequence_number | 0x40;
        lfn1->sequence_number += i + 1;

        // overwrite
        fseek(disk, location, SEEK_SET);
        fwrite(lfn1, 32, 1, disk);
        location += 32;

        delete lfn1;
    }

    FatFile83* entry = new FatFile83;

    entry->attributes = 16;
    entry->filename[0] = '~';
    entry->filename[1] = create_count;

    struct timeval now{};
    gettimeofday(&now, nullptr);
    entry->creationTimeMs = now.tv_usec/1000;

    time_t t = time(0);
    tm* n = localtime(&t);

    string str = bitset<5>(n->tm_hour).to_string();
    str += bitset<6>(n->tm_min).to_string();
    str += bitset<5>(n->tm_sec).to_string();

    uint16_t creation = str_to_uint16(str);

    entry->creationTime = creation;
    entry->modifiedTime = creation;
    entry->lastAccessTime = creation;

    str = bitset<7>(n->tm_year-1980).to_string();
    str += bitset<4>(n->tm_mon).to_string();
    str += bitset<5>(n->tm_mday).to_string();

    creation = str_to_uint16(str);

    entry->creationDate = creation;
    entry->modifiedDate = creation;
    entry->fileSize = 0;
    
    uint32_t new_cluster = findEmptyCluster(disk,boot_sector);

    entry->firstCluster = new_cluster & 0xFFFF;
    entry->eaIndex = (new_cluster >> 16) & 0xFFFF;

    for(int i = 2; i < 8; i++){
        entry->filename[i] = 32;
    }

    for(int i = 0; i < 3; i++){
        entry->extension[i] = 32;
    }

    FatFile83* entry1 = new FatFile83;

    uint32_t location1 = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                         + (entry->firstCluster-2) * boot_sector.SectorsPerCluster;

    location1 *= boot_sector.BytesPerSector;

    entry1->filename[0] = '.';
    for(int i = 1; i < 8; i++){
        entry1->filename[i] = 32;
    }

    for(int i = 0; i < 3; i++){
        entry1->extension[i] = 32;
    }

    entry1->attributes = 16;
    entry1->reserved = 0;
    entry1->creationTimeMs = 0;
    entry1->creationDate = entry->creationDate;
    entry1->creationTime = entry->creationTime;
    entry1->lastAccessTime = entry->lastAccessTime;
    entry1->firstCluster = entry->firstCluster;
    entry1->modifiedDate = entry->modifiedDate;
    entry1->modifiedTime = entry->modifiedDate;
    entry1->fileSize = 0;
    entry1->eaIndex = entry->eaIndex;

    fseek(disk,location1,SEEK_SET);
    fwrite(entry1,32,1,disk);

    entry1->filename[0] = '.';
    entry1->filename[1] = '.';

    entry1->firstCluster = cluster & 0xFFFF;
    entry1->eaIndex = (cluster >> 16) & 0xFFFF;
    if(entry1->eaIndex == 0 && entry1->firstCluster == boot_sector.extended.RootCluster) entry1->firstCluster = 0;

    fseek(disk,location1+32,SEEK_SET);
    fwrite(entry1,32,1,disk);

    fseek(disk,location,SEEK_SET);
    fwrite(entry,32,1,disk);

    delete entry1;
    delete entry;
}

void createFile(FILE *disk,int32_t next_cluster,int32_t cluster, BPB_struct boot_sector, string file_name,int create_count) {

    if(cluster != boot_sector.extended.RootCluster){
        updateModificationTime(disk,cluster,boot_sector);
    }
    
    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (next_cluster-2) * boot_sector.SectorsPerCluster;

    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                 + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    vector<string> dir;

    if(file_name.length() > 13){

        int index = 0;

        while(index < file_name.length()){

            string str;
            if(file_name.length() - index > 13){
                str = file_name.substr(index,13);
            }
            else{
                str = file_name.substr(index,file_name.length()-index);
            }

            dir.push_back(str);

            index+=13;
        }
    }

    else{
        dir.push_back(file_name);
    }

    int empty_count = 0;

    while (empty_count != dir.size()+1){
        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,location,boot_sector);

        if(lfn->sequence_number == 0 || lfn->sequence_number == 0xE5){
            empty_count++;
        }

        else{
            empty_count = 0;
        }

        location += 32;

        if(location >= j){
            next_cluster = readFAT(disk,next_cluster,boot_sector);
            if(next_cluster == 268435448){
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);

                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                           + (next_cluster-2) * boot_sector.SectorsPerCluster;

                location *= boot_sector.BytesPerSector;

                delete lfn;

                break;
            }

            else{
                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                           + (next_cluster-2) * boot_sector.SectorsPerCluster;

                location *= boot_sector.BytesPerSector;

                j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                    + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

                j *= boot_sector.BytesPerSector;
            }
        }

        if(empty_count == dir.size()+1){
            location -= 32*empty_count;
            delete lfn;
            break;
        }
    }

    
    for (int i = dir.size()-1; i >= 0; i--) {

        FatFileLFN *lfn1 = new FatFileLFN;

        uint16_t temp1[13];
        for (int m = 0; m < 13; ++m) {
            if (m < dir[i].length()) {
                temp1[m] = dir[i][m];
            } else if (m == dir[i].length()) {
                temp1[m] = 0;
            } else {
                temp1[m] = -1;
            }
        }

        for (int m = 0; m < 13; ++m) {
            if (m < 5) {
                lfn1->name1[m] = temp1[m];
            } else if (m < 11) {
                lfn1->name2[m - 5] = temp1[m];
            } else {
                lfn1->name3[m - 11] = temp1[m];
            }
        }

        lfn1->attributes = 15;
        lfn1->reserved = 0;
        lfn1->firstCluster = 0;
        lfn1->checksum = lfn_checksum(create_count);
        lfn1->sequence_number = 0;
        if (i == dir.size() - 1) lfn1->sequence_number = lfn1->sequence_number | 0x40;
        lfn1->sequence_number += i + 1;

        
        // overwrite
        fseek(disk, location, SEEK_SET);
        fwrite(lfn1, 32, 1, disk);
        location += 32;

        

        delete lfn1;
    }

    FatFile83* entry = new FatFile83;

    entry->attributes = 32;
    entry->filename[0] = '~';
    entry->filename[1] = create_count;
    entry->fileSize = 0;

    struct timeval now{};
    gettimeofday(&now, nullptr);
    entry->creationTimeMs = now.tv_usec/1000;

    time_t t = time(0);
    tm* n = localtime(&t);

    string str = bitset<5>(n->tm_hour).to_string();
    str += bitset<6>(n->tm_min).to_string();
    str += bitset<5>(n->tm_sec).to_string();

    uint16_t creation = str_to_uint16(str);

    entry->creationTime = creation;
    entry->modifiedTime = creation;
    entry->lastAccessTime = creation;
    entry->eaIndex = 0;

    str = bitset<7>(n->tm_year-1980).to_string();
    str += bitset<4>(n->tm_mon).to_string();
    str += bitset<5>(n->tm_mday).to_string();

    creation = str_to_uint16(str);

    entry->creationDate = creation;
    entry->modifiedDate = creation;

    entry->firstCluster = 0;

    for(int i = 2; i < 8; i++){
        entry->filename[i] = 32;
    }

    for(int i = 0; i < 3; i++){
        entry->extension[i] = 32;
    }

    
    fseek(disk,location,SEEK_SET);
    fwrite(entry,32,1,disk);
    
    delete entry;
    
}
