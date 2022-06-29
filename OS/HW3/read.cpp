#include "read.h"

void readBootSector(FILE* disk, BPB_struct& boot_sector)
{
    fread(&boot_sector,sizeof(boot_sector), 1, disk);
}

void readEntry(FILE* disk, void* dir_entry, uint32_t location, BPB_struct & boot_sector)
{
    fseek(disk, location, SEEK_SET);
    fread(dir_entry, 32, 1, disk);
}

void readDirectory(FILE *disk, uint32_t cluster, BPB_struct &boot_sector, bool arg, int & file_count) {

    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (cluster-2) * boot_sector.SectorsPerCluster;
    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                 + (cluster+1-2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    if (cluster != boot_sector.extended.RootCluster) {
        location += 64;
    }

    while (location < j){

        string str;

        FatFileLFN* lfn = new FatFileLFN;

        readLFN(disk,lfn,location,boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        if(lfn_count == 0 || lfn->sequence_number == 0xE5) {
            location += 32;
            lfn_count = 0;
        }


        while (lfn_count > 0){
            str = directory_name_converter(*lfn) + str;
            location += 32;
            lfn_count--;

            if(lfn_count == 0){

                if (location >= j) {
                    cluster = readFAT(disk, cluster, boot_sector);
                    location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                               + (cluster - 2) * boot_sector.SectorsPerCluster;
                    location *= boot_sector.BytesPerSector;

                    j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (cluster + 1 - 2) * boot_sector.SectorsPerCluster;

                    j *= boot_sector.BytesPerSector;
                }

                file_count++;

                if(!arg){
                    cout << str << " ";
                }

                else{
                    FatFile83* entry = new FatFile83;
                    readEntry(disk, entry,location,boot_sector);

                    vector<string> months = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October",
                                             "November", "December"};
                    int day = entry->modifiedDate & 0x1F;
                    string m = months[(entry->modifiedDate >> 5) & 0xF];
                    uint16_t hour = (entry->modifiedTime >> 11) & 0x1F;
                    uint16_t min = (entry->modifiedTime >> 5) & 0x3F;

                    string min_str;
                    stringstream ss(min_str);

                    string hour_str;
                    stringstream ss1(hour_str);

                    ss << min;
                    ss >> min_str;

                    ss1 << hour;
                    ss1 >> hour_str;

                    if(min < 10) min_str = "0" + min_str;
                    if(hour < 10) hour_str = "0" + hour_str;

                    if(entry->attributes == 16){
                        cout << "drwx------ 1 root root 0" << " " << m << " " << day
                             << " " << hour_str << ":" << min_str << " " << str << endl;
                    }

                    else {
                        cout << "-rwx------ 1 root root " << entry->fileSize << " " << m << " " << day
                             << " " << hour_str << ":" << min_str << " " << str << endl;
                    }

                    delete entry;
                }

                location += 32;
            }

            if (location >= j) {
                cluster = readFAT(disk, cluster, boot_sector);

                if(cluster == 268435448){
                    delete lfn;
                    return;
                }

                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                           + (cluster - 2) * boot_sector.SectorsPerCluster;
                location *= boot_sector.BytesPerSector;

                j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                    + (cluster + 1 - 2) * boot_sector.SectorsPerCluster;

                j *= boot_sector.BytesPerSector;
            }

            readLFN(disk,lfn,location,boot_sector);
        }

        delete lfn;
    }
}

void readLFN(FILE *disk, void* dir_entry, uint32_t location, BPB_struct boot_sector) {
    fseek(disk, location, SEEK_SET);
    fread(dir_entry, sizeof(FatFileLFN), 1, disk);
}

void readFile(FILE *disk, uint32_t cluster, BPB_struct boot_sector, string file_name) {

    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
                        + (cluster-2) * boot_sector.SectorsPerCluster;
    location *= boot_sector.BytesPerSector;


    uint32_t first_cluster = cluster;

    if(first_cluster == 0) cout << endl;

    else{
        location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
        + (first_cluster-2) * boot_sector.SectorsPerCluster;
        location *= boot_sector.BytesPerSector;

        while(first_cluster < 101590){
            char buf[1024];
            fseek(disk,location,SEEK_SET);
            fread(buf,1023,1,disk);
            buf[1023] = '\0';
            cout << buf;

            first_cluster = readFAT(disk,first_cluster,boot_sector);
            location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs*boot_sector.extended.FATSize)
              + (first_cluster-2) * boot_sector.SectorsPerCluster;
            location *= boot_sector.BytesPerSector;
        }

        cout << endl;
    }
}

uint32_t readFAT(FILE *disk, uint32_t cluster, BPB_struct boot_sector) {

    uint32_t location = boot_sector.ReservedSectorCount * boot_sector.BytesPerSector + cluster*4;

    uint32_t* val = new uint32_t;

    fseek(disk,location,SEEK_SET);
    fread(val,4,1,disk);

    *val &= 0x0FFFFFFF;

    uint32_t res = *val;
    delete val;

    return res;
}
