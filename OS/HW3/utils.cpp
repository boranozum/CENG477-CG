#include "utils.h"
#include "read.h"

void parser(command &cmd){

    vector<string> res;
    string str;
    getline(cin,str);

    stringstream ss(str);

    string temp;
    int i = 0;

    while(ss >> temp){
        if(i == 0) {
            if(temp == "cd") cmd.cmd = CD;
            if(temp == "ls") cmd.cmd = LS;
            if(temp == "mkdir") cmd.cmd = MKDIR;
            if(temp == "touch") cmd.cmd = TOUCH;
            if(temp == "mv") cmd.cmd = MV;
            if(temp == "cat") cmd.cmd = CAT;
            if(temp == "quit") cmd.cmd = QUIT;
        }
        else{
            cmd.params.push_back(temp);
        }

        i++;
    }
}

void tokenizer(string str, vector<string> & cur_directory){

    int j = 0;
    int i = 0;

    for (; i < str.length(); ++i) {
        if(str[i] == '/'){

            string temp = str.substr(j,i-j);

            cur_directory.push_back(str.substr(j,i-j));
            j = i+1;
        }
    }
    string temp = str.substr(j,i-j);
    cur_directory.push_back(temp);
}


void cur_dir_prompt(vector<string> dir){
    string s = "/";

    for (int i = 0; i < dir.size(); ++i) {
        if(i == dir.size()-1){
            s += dir[i];
        }
        else{
            s+= dir[i] + "/";
        }
    }

    s += "> ";

    cout << s;
}


string directory_name_converter(FatFileLFN dir) {
    string s;
    bool end = false;

    for (int i = 0; i < 5; ++i) {
        if(dir.name1[i] > 1000 || dir.name1[i] == 0){
            end = true;
            break;
        }
        char temp = dir.name1[i];
        s += temp;
    }

    if(!end){
        for (int i = 0; i < 6; ++i) {
            if(dir.name2[i] > 1000 || dir.name2[i] == 0){
                end = true;
                break;
            }
            char temp = dir.name2[i];
            s += temp;
        }

        if(!end){
            for (int i = 0; i < 3; ++i) {
                if(dir.name3[i] > 1000 || dir.name3[i] == 0){
                    end = true;
                    break;
                }
                char temp = dir.name3[i];
                s += temp;
            }
        }
    }

    return s;
}

unsigned char lfn_checksum(int create_count)
{
    int i,j = 0;
    unsigned char sum = 0;
    unsigned char pFCBName[11] = {'~',create_count,32,32,32,32,32,32,32,32,32};

    for (i = 11; i; i--)
        sum = ((sum & 1) << 7) + (sum >> 1) + pFCBName[j++];

    return sum; 
}

uint16_t str_to_uint16(string str) {

    int i = 0;
    uint16_t sum = 0;

    for (int j = str.length()-1; j >= 0; j--) {
        if(str[j] == '1'){
            sum += pow(2,i);
        }
        i++;
    }

    return sum;
}

void copyFat(FILE *disk, BPB_struct boot_sector) {

    uint32_t fat_size_in_bytes = boot_sector.extended.FATSize*boot_sector.BytesPerSector;

    uint32_t location = boot_sector.ReservedSectorCount*boot_sector.BytesPerSector;

    uint32_t j = 0;

    char buffer[406528];

    fseek(disk,location,SEEK_SET);
    fread(buffer,406528,1,disk);

    for (int i = 0; i < boot_sector.NumFATs-1; ++i) {
        uint32_t location1 = location + fat_size_in_bytes;
        fseek(disk,location1,SEEK_SET);
        fwrite(buffer,406528,1,disk);
    }
}

void updateModificationTime(FILE* disk, uint32_t cluster, BPB_struct boot_sector){
    
    uint32_t location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (cluster - 2) * boot_sector.SectorsPerCluster;
    location *= boot_sector.BytesPerSector;

    FatFile83* entry = new FatFile83;

    readEntry(disk,entry,location+32,boot_sector);

    uint32_t parent_cluster = entry->firstCluster + ((entry->eaIndex << 16) & 0xFFFF0000);

    if(parent_cluster == 0) parent_cluster = boot_sector.extended.RootCluster;

    time_t t = time(0);
    tm* n = localtime(&t);

    string str = bitset<5>(n->tm_hour).to_string();
    str += bitset<6>(n->tm_min).to_string();
    str += bitset<5>(n->tm_sec).to_string();

    uint16_t creation = str_to_uint16(str);

    entry->modifiedTime = creation;
    //entry->lastAccessTime = creation;

    str = bitset<7>(n->tm_year-1980).to_string();
    str += bitset<4>(n->tm_mon).to_string();
    str += bitset<5>(n->tm_mday).to_string();

    creation = str_to_uint16(str);

    entry->modifiedDate = creation;

    fseek(disk,location+32,SEEK_SET);
    fwrite(entry,32,1,disk);

    location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (parent_cluster - 2) * boot_sector.SectorsPerCluster;
    location *= boot_sector.BytesPerSector;

    uint32_t j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                 + (parent_cluster + 1 - 2) * boot_sector.SectorsPerCluster;

    j *= boot_sector.BytesPerSector;

    delete entry;

    while(location < j){

        FatFileLFN* lfn = new FatFileLFN;
        readEntry(disk, lfn, location, boot_sector);

        uint8_t lfn_count = lfn->sequence_number & 0x0F;

        if(lfn_count == 0 || lfn->sequence_number == 0xE5) {
            location += 32;

            if (location >= j) {
                uint32_t res = readFAT(disk, res, boot_sector);

                location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (res - 2) * boot_sector.SectorsPerCluster;
                location *= boot_sector.BytesPerSector;

                //bool found = false;
                j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                    + (res + 1 - 2) * boot_sector.SectorsPerCluster;

                j *= boot_sector.BytesPerSector;
            }
        }

        else{
            location += 32*lfn_count;

            FatFile83* entry = new FatFile83;

            readEntry(disk, entry, location, boot_sector);

            if(entry->firstCluster == cluster){
                time_t t = time(0);
                tm* n = localtime(&t);

                string str = bitset<5>(n->tm_hour).to_string();
                str += bitset<6>(n->tm_min).to_string();
                str += bitset<5>(n->tm_sec).to_string();

                uint16_t creation = str_to_uint16(str);

                entry->modifiedTime = creation;
                //entry->lastAccessTime = creation;

                str = bitset<7>(n->tm_year-1980).to_string();
                str += bitset<4>(n->tm_mon).to_string();
                str += bitset<5>(n->tm_mday).to_string();

                creation = str_to_uint16(str);

                entry->modifiedDate = creation;

                fseek(disk,location,SEEK_SET);
                fwrite(entry,32,1,disk);

                delete entry;
                delete lfn;

                return; 
            }

            else{
                location += 32;

                if (location >= j) {
                    uint32_t res = readFAT(disk, res, boot_sector);

                    location = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                            + (res - 2) * boot_sector.SectorsPerCluster;
                    location *= boot_sector.BytesPerSector;

                    //bool found = false;
                    j = (boot_sector.ReservedSectorCount + boot_sector.NumFATs * boot_sector.extended.FATSize)
                        + (res + 1 - 2) * boot_sector.SectorsPerCluster;

                    j *= boot_sector.BytesPerSector;
                }
            }

            delete entry;
        }

        delete lfn;
    }
}

