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

    while(dest_location < j_dest){
        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,dest_location,boot_sector);

        if(lfn->attributes == 0) {
            delete lfn;
            break;
        }

        dest_location += 32;

        if(dest_location >= j_dest){
            delete lfn;

            uint32_t next_cluster = readFAT(disk,dest_cluster,boot_sector);

            if(next_cluster < 268435448){
                j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                    + (next_cluster+1-2) * boot_sector.SectorsPerCluster;
                j_dest *= boot_sector.BytesPerSector;
                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                + (next_cluster-2) * boot_sector.SectorsPerCluster;
                dest_location *= boot_sector.BytesPerSector;
            }

            else{
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + dest_cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);

                j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                         + (next_cluster+1-2) * boot_sector.SectorsPerCluster;
                j_dest *= boot_sector.BytesPerSector;

                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                + (next_cluster-2) * boot_sector.SectorsPerCluster;
                dest_location *= boot_sector.BytesPerSector;
            }
        }
    }

    uint32_t j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
        + (src_cluster+1-2) * boot_sector.SectorsPerCluster;

    j_src *= boot_sector.BytesPerSector;

    if (src_location != boot_sector.extended.RootCluster) {
        src_location += 64;
    }

    while(src_location < j_src){

        string str;

        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,src_location,boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        vector<uint32_t> reset_locations;

        while (lfn_count > 0) {
            str = directory_name_converter(*lfn) + str;
            reset_locations.push_back(src_location);

            src_location += 32;
            if (src_location >= j_src) {
                src_cluster = readFAT(disk, src_cluster, boot_sector);
                src_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                           + (src_cluster - 2) * boot_sector.SectorsPerCluster;
                src_location *= boot_sector.BytesPerSector;

                j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                    + (src_cluster + 1 - 2) * boot_sector.SectorsPerCluster;

                j_src *= boot_sector.BytesPerSector;
            }

            readLFN(disk, lfn, src_location, boot_sector);
            lfn_count--;
        }

        if(name == str){

            for (uint32_t reset_location: reset_locations) {
                FatFileLFN* lfn1 = new FatFileLFN;
                readLFN(disk,lfn1,reset_location,boot_sector);

                fseek(disk, dest_location, SEEK_SET);
                fwrite(lfn1,32,1,disk);

                uint32_t z = 0;

                fseek(disk, reset_location,SEEK_SET);
                fwrite(&z,32,1,disk);

                dest_location += 32;

                if(dest_location >= j_dest){
                    uint32_t next_cluster = findEmptyCluster(disk,boot_sector);

                    uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + dest_cluster*4;

                    fseek(disk,fat_location,SEEK_SET);
                    fwrite(&next_cluster,4,1,disk);

                    j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                             + (next_cluster+1-2) * boot_sector.SectorsPerCluster;
                    j_dest *= boot_sector.BytesPerSector;

                    dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                    + (next_cluster-2) * boot_sector.SectorsPerCluster;
                    dest_location *= boot_sector.BytesPerSector;
                }

                delete lfn1;
            }

            FatFile83* entry = new FatFile83;
            readEntry(disk,entry,src_location,boot_sector);

            uint32_t z = 0;
            fseek(disk,src_location,SEEK_SET);
            fwrite(&z,32,1,disk);

            uint32_t two_dot_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                       + (entry->firstCluster-2) * boot_sector.SectorsPerCluster;

            two_dot_location *= boot_sector.BytesPerSector;

            FatFile83* entry1 = new FatFile83;
            readEntry(disk, entry1, two_dot_location+32, boot_sector);

            entry1->firstCluster = dest_cluster;

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

    while(dest_location < j_dest){
        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,dest_location,boot_sector);

        if(lfn->attributes == 0) {
            delete lfn;
            break;
        }

        dest_location += 32;

        if(dest_location >= j_dest){
            delete lfn;

            uint32_t next_cluster = readFAT(disk,dest_cluster,boot_sector);

            if(next_cluster < 268435448){
                j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                         + (next_cluster+1-2) * boot_sector.SectorsPerCluster;
                j_dest *= boot_sector.BytesPerSector;
                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                + (next_cluster-2) * boot_sector.SectorsPerCluster;
                dest_location *= boot_sector.BytesPerSector;
            }

            else{
                next_cluster = findEmptyCluster(disk,boot_sector);

                uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + dest_cluster*4;

                fseek(disk,fat_location,SEEK_SET);
                fwrite(&next_cluster,4,1,disk);

                j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                         + (next_cluster+1-2) * boot_sector.SectorsPerCluster;
                j_dest *= boot_sector.BytesPerSector;

                dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                + (next_cluster-2) * boot_sector.SectorsPerCluster;
                dest_location *= boot_sector.BytesPerSector;
            }
        }
    }

    uint32_t j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                     + (src_cluster+1-2) * boot_sector.SectorsPerCluster;

    j_src *= boot_sector.BytesPerSector;

    if (src_location != boot_sector.extended.RootCluster) {
        src_location += 64;
    }

    while(src_location < j_src){

        string str;

        FatFileLFN* lfn = new FatFileLFN;
        readLFN(disk,lfn,src_location,boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        int i = 1;

        vector<uint32_t> reset_locations;

        while (lfn_count > 0) {
            str = directory_name_converter(*lfn) + str;
            reset_locations.push_back(src_location);

            src_location += 32;
            if (src_location >= j_src) {
                src_cluster = readFAT(disk, src_cluster, boot_sector);
                src_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                               + (src_cluster - 2) * boot_sector.SectorsPerCluster;
                src_location *= boot_sector.BytesPerSector;

                j_src = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (src_cluster + 1 - 2) * boot_sector.SectorsPerCluster;

                j_src *= boot_sector.BytesPerSector;
            }

            readLFN(disk, lfn, src_location, boot_sector);
            lfn_count--;
        }

        if(name == str){

            for (uint32_t reset_location: reset_locations) {
                FatFileLFN* lfn1 = new FatFileLFN;
                readLFN(disk,lfn1,reset_location,boot_sector);

                fseek(disk, dest_location, SEEK_SET);
                fwrite(lfn1,32,1,disk);

                uint32_t z = 0;

                fseek(disk, reset_location,SEEK_SET);
                fwrite(&z,32,1,disk);

                dest_location += 32;

                if(dest_location >= j_dest){
                    uint32_t next_cluster = findEmptyCluster(disk,boot_sector);

                    uint32_t fat_location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + dest_cluster*4;

                    fseek(disk,fat_location,SEEK_SET);
                    fwrite(&next_cluster,4,1,disk);

                    j_dest = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                             + (next_cluster+1-2) * boot_sector.SectorsPerCluster;
                    j_dest *= boot_sector.BytesPerSector;

                    dest_location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                                    + (next_cluster-2) * boot_sector.SectorsPerCluster;
                    dest_location *= boot_sector.BytesPerSector;
                }

                delete lfn1;
            }

            FatFile83* entry = new FatFile83;
            readEntry(disk,entry,src_location,boot_sector);

            uint32_t z = 0;
            fseek(disk,src_location,SEEK_SET);
            fwrite(&z,32,1,disk);

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
