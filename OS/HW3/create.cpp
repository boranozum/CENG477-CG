#include "create.h"

void createDirectory(FILE *disk,int32_t next_cluster ,int32_t cluster, BPB_struct boot_sector, string dir_name, int create_count) {

    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (next_cluster-2) * boot_sector.SectorsPerCluster;

    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                 + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    while(location < j){
        FatFileLFN *lfn = new FatFileLFN;
        readLFN(disk,lfn,location,boot_sector);

        if(lfn->attributes == 0){

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

            for (int i = dir.size()-1; i >= 0; i--) {

                FatFileLFN* lfn1 = new FatFileLFN;

                uint16_t temp1[13];
                for (int m = 0; m < 13; ++m) {
                    if(m < dir[i].length()){
                        temp1[m] = dir[i][m];
                    }
                    else if(m == dir[i].length()){
                        temp1[m] = 0;
                    }
                    else{
                        temp1[m] = -1;
                    }
                }

                for (int m = 0; m < 13; ++m) {
                    if(m < 5){
                        lfn1->name1[m] = temp1[m];
                    }

                    else if(m < 11){
                        lfn1->name2[m-5] = temp1[m];
                    }
                    else{
                        lfn1->name3[m-11] = temp1[m];
                    }
                }

                lfn1->attributes = 15;
                lfn1->reserved = 0;
                lfn1->firstCluster = 0;
                lfn1->checksum = lfn_checksum(dir_name.c_str());
                lfn1->sequence_number = 0;
                if(i == dir.size()-1) lfn1->sequence_number = lfn1->sequence_number | 0x40;
                lfn1->sequence_number += i+1;

                // overwrite
                fseek(disk,location,SEEK_SET);
                fwrite(lfn1,32,1,disk);
                location += 32;

                if(location >= j){
                    next_cluster = readFAT(disk,next_cluster,boot_sector);
                    if(next_cluster ==  268435448){
                        next_cluster = findEmptyCluster(disk,boot_sector);

                        uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + cluster*4;

                        fseek(disk,fat_location,SEEK_SET);
                        fwrite(&next_cluster,4,1,disk);
                    }

                    location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                        + (next_cluster-2) * boot_sector.SectorsPerCluster;

                    location *= boot_sector.BytesPerSector;

                    j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                 + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

                    j *= boot_sector.BytesPerSector;
                }
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

            entry->firstCluster = findEmptyCluster(disk,boot_sector);

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

            readEntry(disk,entry1,location1,boot_sector);

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
            entry1->eaIndex = 0;

            fseek(disk,location1,SEEK_SET);
            fwrite(entry1,32,1,disk);

            location1 += 32;

            readEntry(disk,entry1,location1,boot_sector);

            entry1->filename[0] = '.';
            entry1->filename[1] = '.';
            for(int i = 2; i < 8; i++){
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
            entry1->modifiedDate = entry->modifiedDate;
            entry1->modifiedTime = entry->modifiedDate;
            entry1->fileSize = 0;
            entry1->eaIndex = 0;

            if(cluster != boot_sector.extended.RootCluster) entry1->firstCluster = cluster;

            fseek(disk,location1,SEEK_SET);
            fwrite(entry1, 32,1,disk);

            delete entry1;

            // overwrite

            fseek(disk,location,SEEK_SET);
            fwrite(entry,32,1,disk);

            delete lfn;
            delete entry;

            return;
        }

        location += 32;

        if(location >= j){
            next_cluster = readFAT(disk,next_cluster,boot_sector);
            if(next_cluster ==  268435448){
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);
            }

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                       + (next_cluster-2) * boot_sector.SectorsPerCluster;

            location *= boot_sector.BytesPerSector;

            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;
        }

        delete lfn;
    }
}

void createFile(FILE *disk,int32_t next_cluster,int32_t cluster, BPB_struct boot_sector, string file_name,int create_count) {
    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (next_cluster-2) * boot_sector.SectorsPerCluster;

    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                 + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    while(location < j){
        FatFileLFN *lfn = new FatFileLFN;
        readLFN(disk,lfn,location,boot_sector);

        if(lfn->attributes == 0){

            vector<string> fn;

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

                    fn.push_back(str);

                    index+=13;
                }
            }
            else{
                fn.push_back(file_name);
            }

            for (int i = fn.size()-1; i >= 0; i--) {
                FatFileLFN* lfn1 = new FatFileLFN;

                uint16_t temp1[13];

                for (int m = 0; m < 13; ++m) {
                    if(m < fn[i].length()){
                        temp1[m] = fn[i][m];
                    }
                    else if(m == fn[i].length()){
                        temp1[m] = 0;
                    }
                    else{
                        temp1[m] = -1;
                    }
                }

                for (int m = 0; m < 13; ++m) {
                    if(m < 5){
                        lfn1->name1[m] = temp1[m];
                    }

                    else if(m < 11){
                        lfn1->name2[m-5] = temp1[m];
                    }
                    else{
                        lfn1->name3[m-11] = temp1[m];
                    }
                }

                lfn1->attributes = 15;
                lfn1->reserved = 0;
                lfn1->firstCluster = 0;
                lfn1->checksum = lfn_checksum(file_name.c_str());
                if(i == fn.size()-1) lfn1->sequence_number = lfn1->sequence_number | 0x40;
                lfn1->sequence_number += i+1;

                // overwrite
                fseek(disk,location,SEEK_SET);
                fwrite(lfn1,32,1,disk);
                location += 32;

                if(location >= j){
                    next_cluster = readFAT(disk,next_cluster,boot_sector);
                    if(next_cluster ==  268435448){
                        next_cluster = findEmptyCluster(disk,boot_sector);

                        uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + cluster*4;

                        fseek(disk,fat_location,SEEK_SET);
                        fwrite(&next_cluster,4,1,disk);
                    }

                    location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                               + (next_cluster-2) * boot_sector.SectorsPerCluster;

                    location *= boot_sector.BytesPerSector;

                    j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

                    j *= boot_sector.BytesPerSector;
                }
            }

            FatFile83* entry = new FatFile83;

            entry->attributes = 32;
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
            delete lfn;

            return;
        }

        location += 32;

        if(location >= j){
            next_cluster = readFAT(disk,next_cluster,boot_sector);
            if(next_cluster ==  268435448){
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);
            }

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                       + (next_cluster-2) * boot_sector.SectorsPerCluster;

            location *= boot_sector.BytesPerSector;

            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;
        }

        delete lfn;
    }
}
