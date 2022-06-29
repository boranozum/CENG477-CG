#include "find.h"

int32_t findDirectory(FILE* disk, int32_t cluster, BPB_struct & boot_sector, vector<string> vec) {

    int32_t res = cluster;
    int vec_index = 0;

    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (res - 2) * boot_sector.SectorsPerCluster;
    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                 + (res + 1 - 2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    bool sub_flag = false;

    while (vec_index < vec.size()) {

        string dir = vec[vec_index];

        if(dir == "."){
            vec_index++;
            continue;
        }

        if (dir == "..") {
            FatFile83 *entry = new FatFile83;

            readEntry(disk, entry, location + 32, boot_sector);

            if (entry->filename[1] == '.') {
                res = entry->firstCluster;
                res += (entry->eaIndex << 16) & 0xFFFF0000;
                if (res == 0) res = boot_sector.extended.RootCluster;
            }

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                       + (res - 2) * boot_sector.SectorsPerCluster;
            location *= boot_sector.BytesPerSector;

            //bool found = false;
            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;

            vec_index++;
            delete entry;
            continue;
        }

        string str;

        if (res != boot_sector.extended.RootCluster && !sub_flag) {
            location += 64;
            sub_flag = true;
        }

        FatFileLFN *lfn = new FatFileLFN;

        readLFN(disk, lfn, location, boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        if(lfn->sequence_number == 0xE5) lfn_count = 0;

        while (lfn_count > 0) {
            str = directory_name_converter(*lfn) + str;
            location += 32;
            if (location >= j) {
                res = readFAT(disk, res, boot_sector);
                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                           + (res - 2) * boot_sector.SectorsPerCluster;
                location *= boot_sector.BytesPerSector;

                j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                    + (res + 1 - 2) * boot_sector.SectorsPerCluster;

                j *= boot_sector.BytesPerSector;
            }

            readLFN(disk, lfn, location, boot_sector);
            lfn_count--;
        }

        if (dir == str) {
            sub_flag = false;

            FatFile83 *entry = new FatFile83;
            readEntry(disk, entry, location, boot_sector);

            res = entry->firstCluster;
            res += (entry->eaIndex << 16) & 0xFFFF0000;
            vec_index++;

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                       + (res - 2) * boot_sector.SectorsPerCluster;
            location *= boot_sector.BytesPerSector;

            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;

            delete lfn;
            delete entry;

            continue;
        }

        location += 32;

        if (location >= j) {
            res = readFAT(disk, res, boot_sector);
            if (res == 268435448) {
                delete lfn;
                return -1;
            }

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                       + (res - 2) * boot_sector.SectorsPerCluster;
            location *= boot_sector.BytesPerSector;

            //bool found = false;
            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;
        }
    }

    return res;
}

int findEmptyCluster(FILE *disk, BPB_struct boot_sector) {

    uint32_t location = boot_sector.ReservedSectorCount*boot_sector.BytesPerSector;
    fseek(disk,location,SEEK_SET);

    uint32_t entry;

    int res = 0;

    while(res <= 101590){
        fread(&entry,4,1,disk);

        if(entry == 0){

            entry = 268435448;

            fseek(disk,location+res*4,SEEK_SET);
            fwrite(&entry,4,1,disk);

        
            return res;
        }
        res++;
    }

    
    return 0;
}

int32_t findFile(FILE *disk, int32_t cluster, BPB_struct boot_sector, vector<string> path) {
    int32_t res = cluster;
    int vec_index = 0;

    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (res - 2) * boot_sector.SectorsPerCluster;
    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                 + (res + 1 - 2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    bool sub_flag = false;

    while (vec_index < path.size()) {

        string dir = path[vec_index];

        if(dir == "."){
            vec_index++;
            continue;
        }

        if (dir == "..") {
            FatFile83 *entry = new FatFile83;

            readEntry(disk, entry, location + 32, boot_sector);

            if (entry->filename[1] == '.') {
                res = entry->firstCluster;
                res += (entry->eaIndex << 16) & 0xFFFF0000;
                if (res == 0) res = boot_sector.extended.RootCluster;
            }

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                       + (res - 2) * boot_sector.SectorsPerCluster;
            location *= boot_sector.BytesPerSector;

            //bool found = false;
            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;

            vec_index++;
            delete entry;
            continue;
        }

        string str;

        if (res != boot_sector.extended.RootCluster && !sub_flag) {
            location += 64;
            sub_flag = true;
        }

        FatFileLFN *lfn = new FatFileLFN;

        readLFN(disk, lfn, location, boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        if(lfn->sequence_number == 0xE5) {
            lfn_count = 0;
        }

        while (lfn_count > 0) {
            str = directory_name_converter(*lfn) + str;
            location += 32;
            if (location >= j) {
                res = readFAT(disk, res, boot_sector);
                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                           + (res - 2) * boot_sector.SectorsPerCluster;
                location *= boot_sector.BytesPerSector;

                j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                    + (res + 1 - 2) * boot_sector.SectorsPerCluster;

                j *= boot_sector.BytesPerSector;
            }

            readLFN(disk, lfn, location, boot_sector);
            lfn_count--;
        }

        if (dir == str) {
            sub_flag = false;

            FatFile83 *entry = new FatFile83;
            readEntry(disk, entry, location, boot_sector);

            res = entry->firstCluster;
            res += (entry->eaIndex << 16) & 0xFFFF0000;
            vec_index++;

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                       + (res - 2) * boot_sector.SectorsPerCluster;
            location *= boot_sector.BytesPerSector;

            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;

            delete lfn;
            delete entry;

            continue;
        }

        location += 32;

        if (location >= j) {
            res = readFAT(disk, res, boot_sector);
            if (res == 268435448) {
                delete lfn;
                return -1;
            }

            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                       + (res - 2) * boot_sector.SectorsPerCluster;
            location *= boot_sector.BytesPerSector;

            //bool found = false;
            j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                + (res + 1 - 2) * boot_sector.SectorsPerCluster;

            j *= boot_sector.BytesPerSector;
        }
    }

    return res;
}
