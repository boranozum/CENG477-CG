#include "utils.h"

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

unsigned char lfn_checksum(const char *pFCBName)
{
    int i;
    unsigned char sum = 0;

    for (i = 11; i; i--)
        sum = ((sum & 1) << 7) + (sum >> 1) + *pFCBName++;

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

    while (j<fat_size_in_bytes){

        uint32_t temp;

        fseek(disk,location,SEEK_SET);
        fread(&temp,4,1,disk);

        for (int i = 0; i < boot_sector.NumFATs-1; ++i) {
            uint32_t location1 = location + fat_size_in_bytes;
            fseek(disk,location1,SEEK_SET);
            fwrite(&temp,4,1,disk);
        }

        j+=4;
        location+=4;
    }
}
