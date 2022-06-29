#include "move.h"

void moveDirectory(FILE* disk, uint32_t src_cluster, uint32_t dest_cluster, BPB_struct boot_sector, string name){

    uint32_t src_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (src_cluster-2) * boot_sector.SectorsPerCluster;
    src_location *= boot_sector.BytesPerSector;

    uint32_t dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                            + (dest_cluster-2) * boot_sector.SectorsPerCluster;
    dest_location *= boot_sector.BytesPerSector;

    uint32_t j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                 + (dest_cluster+1-2) * boot_sector.SectorsPerCluster;

    j_dest *= boot_sector.BytesPerSector;

    vector<string> dir;
    
    if(src_cluster != boot_sector.extended.RootCluster){
        updateModificationTime(disk,src_cluster,boot_sector);
    }

    if(dest_cluster != boot_sector.extended.RootCluster){
        updateModificationTime(disk,dest_cluster, boot_sector);
    }

    if(name.length() > 13){

        int index = 0;

        while(index < name.length()){

            string str;
            if(name.length() - index > 13){
                str = name.substr(index,13);
            }
            else{
                str = name.substr(index,name.length()-index);
            }

            dir.push_back(str);

            index+=13;
        }
    }

    else{
        dir.push_back(name);
    }

    int empty_count = 0;

    while(empty_count != dir.size()+1){
        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,dest_location,boot_sector);

        if(lfn->sequence_number == 0 || lfn->sequence_number == 0xE5){
            empty_count++;
        }

        else{
            empty_count = 0;
        }

        dest_location+=32;

        if(dest_location>=j_dest){
            uint32_t next_cluster = readFAT(disk,dest_cluster,boot_sector);

            if(next_cluster == 268435448){
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + dest_cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);

                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                           + (next_cluster-2) * boot_sector.SectorsPerCluster;

                dest_location *= boot_sector.BytesPerSector;

                delete lfn;

                break;
            }

            else{
                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                           + (next_cluster-2) * boot_sector.SectorsPerCluster;

                dest_location *= boot_sector.BytesPerSector;

                j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                    + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

                j_dest *= boot_sector.BytesPerSector;
            }
        }
        if(empty_count == dir.size()+1){
            dest_location -= 32*empty_count;
            delete lfn;
            break;
        }
    }

    uint32_t j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
        + (src_cluster+1-2) * boot_sector.SectorsPerCluster;

    j_src *= boot_sector.BytesPerSector;

    if (src_cluster != boot_sector.extended.RootCluster) {
        src_location += 64;
    }

    while(src_location < j_src){

        string str;

        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,src_location,boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        if(lfn->sequence_number == 0xE5) lfn_count = 0;

        vector<uint32_t> reset_locations;

        while (lfn_count > 0) {
            str = directory_name_converter(*lfn) + str;
            reset_locations.push_back(src_location);

            src_location += 32;

            readLFN(disk, lfn, src_location, boot_sector);
            lfn_count--;
        }

        if(name == str){

            for (uint32_t reset_location: reset_locations) {
                FatFileLFN* lfn1 = new FatFileLFN;
                FatFileLFN* e5 = new FatFileLFN;
                readLFN(disk,lfn1,reset_location,boot_sector);
                readLFN(disk,e5,reset_location,boot_sector);

                e5->sequence_number = 0xE5;
                fseek(disk, dest_location, SEEK_SET);
                fwrite(lfn1,32,1,disk);

                fseek(disk, reset_location,SEEK_SET);
                fwrite(e5,32,1,disk);

                delete e5;

                dest_location += 32;

                delete lfn1;
            }

            FatFile83* entry = new FatFile83;
            FatFileLFN* e5 = new FatFileLFN;
            readEntry(disk,entry,src_location,boot_sector);

            e5->sequence_number = 0xE5;
            fseek(disk,src_location,SEEK_SET);
            fwrite(e5,32,1,disk);

            delete e5;

            uint32_t two_dot_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                       + (entry->firstCluster-2) * boot_sector.SectorsPerCluster;

            two_dot_location *= boot_sector.BytesPerSector;

            FatFile83* entry1 = new FatFile83;
            readEntry(disk, entry1, two_dot_location+32, boot_sector);

            entry1->firstCluster = dest_cluster & 0xFFFF;
            entry1->eaIndex = (dest_cluster >> 16) & 0xFFFF;
            if(dest_cluster == boot_sector.extended.RootCluster) entry1->firstCluster = 0;

            fseek(disk,two_dot_location+32,SEEK_SET);
            fwrite(entry1, 32, 1 ,disk);

            delete entry1;

            fseek(disk,dest_location,SEEK_SET);
            fwrite(entry,32,1,disk);

            delete entry;
            delete lfn;

            return;
        }

        src_location += 32;

        if (src_location >= j_src) {
            uint32_t res = readFAT(disk, res, boot_sector);

            src_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                       + (res - 2) * boot_sector.SectorsPerCluster;
            src_location *= boot_sector.BytesPerSector;

            //bool found = false;
            j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j_src *= boot_sector.BytesPerSector;
        }
        delete lfn;
    }
}

void moveFile(FILE *disk, uint32_t src_cluster, uint32_t dest_cluster, BPB_struct boot_sector, string name) {
    uint32_t src_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                            + (src_cluster-2) * boot_sector.SectorsPerCluster;
    src_location *= boot_sector.BytesPerSector;

    uint32_t dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                             + (dest_cluster-2) * boot_sector.SectorsPerCluster;
    dest_location *= boot_sector.BytesPerSector;

    uint32_t j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                      + (dest_cluster+1-2) * boot_sector.SectorsPerCluster;

    j_dest *= boot_sector.BytesPerSector;

    vector<string> dir;

    if(src_cluster != boot_sector.extended.RootCluster){
        updateModificationTime(disk,src_cluster,boot_sector);
    }

    if(dest_cluster != boot_sector.extended.RootCluster){
        updateModificationTime(disk,dest_cluster, boot_sector);
    }

    if(name.length() > 13){

        int index = 0;

        while(index < name.length()){

            string str;
            if(name.length() - index > 13){
                str = name.substr(index,13);
            }
            else{
                str = name.substr(index,name.length()-index);
            }

            dir.push_back(str);

            index+=13;
        }
    }

    else{
        dir.push_back(name);
    }

    int empty_count = 0;

    while(empty_count != dir.size()+1){
        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,dest_location,boot_sector);

        if(lfn->sequence_number == 0 || lfn->sequence_number == 0xE5){
            empty_count++;
        }

        else{
            empty_count = 0;
        }

        dest_location+=32;

        if(dest_location>=j_dest){
            uint32_t next_cluster = readFAT(disk,dest_cluster,boot_sector);

            if(next_cluster == 268435448){
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + dest_cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);

                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                + (next_cluster-2) * boot_sector.SectorsPerCluster;

                dest_location *= boot_sector.BytesPerSector;

                delete lfn;

                break;
            }

            else{
                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                + (next_cluster-2) * boot_sector.SectorsPerCluster;

                dest_location *= boot_sector.BytesPerSector;

                j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                         + (next_cluster+1-2) * boot_sector.SectorsPerCluster;

                j_dest *= boot_sector.BytesPerSector;
            }
        }
        if(empty_count == dir.size()+1){
            dest_location -= 32*empty_count;
            delete lfn;
            break;
        }
    }

    uint32_t j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                     + (src_cluster+1-2) * boot_sector.SectorsPerCluster;

    j_src *= boot_sector.BytesPerSector;

    if (src_cluster != boot_sector.extended.RootCluster) {
        src_location += 64;
    }

    while(src_location < j_src){

        string str;

        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,src_location,boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        if(lfn->sequence_number == 0xE5) lfn_count = 0;

        vector<uint32_t> reset_locations;

        while (lfn_count > 0) {
            str = directory_name_converter(*lfn) + str;
            reset_locations.push_back(src_location);

            src_location += 32;

            readLFN(disk, lfn, src_location, boot_sector);
            lfn_count--;
        }

        if(name == str){

            for (uint32_t reset_location: reset_locations) {
                FatFileLFN* lfn1 = new FatFileLFN;
                FatFileLFN* e5 = new FatFileLFN;
                readLFN(disk,lfn1,reset_location,boot_sector);
                readLFN(disk,e5,reset_location,boot_sector);

                e5->sequence_number = 0xE5;
                fseek(disk, dest_location, SEEK_SET);
                fwrite(lfn1,32,1,disk);

                fseek(disk, reset_location,SEEK_SET);
                fwrite(e5,32,1,disk);

                delete e5;

                dest_location += 32;

                delete lfn1;
            }

            FatFile83* entry = new FatFile83;
            FatFileLFN* e5 = new FatFileLFN;
            readEntry(disk,entry,src_location,boot_sector);

            e5->sequence_number = 0xE5;
            fseek(disk,src_location,SEEK_SET);
            fwrite(e5,32,1,disk);

            delete e5;

            fseek(disk,dest_location,SEEK_SET);
            fwrite(entry,32,1,disk);

            delete entry;
            delete lfn;

            return;
        }

        src_location += 32;

        if (src_location >= j_src) {
            uint32_t res = readFAT(disk, res, boot_sector);

            src_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                           + (res - 2) * boot_sector.SectorsPerCluster;
            src_location *= boot_sector.BytesPerSector;

            //bool found = false;
            j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                    + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j_src *= boot_sector.BytesPerSector;
        }
        delete lfn;
    }
}
