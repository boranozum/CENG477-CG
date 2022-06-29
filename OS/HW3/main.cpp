#include "utils.h"
#include "read.h"
#include "find.h"
#include "create.h"
#include "move.h"

int main(int argc, char** argv)
{
    BPB_struct boot_sector;         // Struct variable that holds the boot sector of the disk
    int file_count;                 // Flag: if set => Don't print extra end line (directory is empty)
    int create_count = 0;           // Counter for uniquely name newly created files/folders

    vector<string> cur_directory;       // String vector. Each member is printed iteratively to show the working directory prompt
    vector<string> temp_vec;            // Util vector that mimics the cur_directory vector in different cases

    int32_t temp_dir;                // Util cluster indicator
    int32_t current_cluster;         // Working directory cluster indicator

    FILE* disk = fopen(argv[1],"r+");        // Actual image file, opened with read/write mode

    readBootSector(disk,boot_sector);                   // First things first, read boot sector

    current_cluster = boot_sector.extended.RootCluster;         // Program will land on root directory at the beginning

    while(true)
    {
        cur_dir_prompt(cur_directory);          // Always show the prompt

        command cmd = command();               // Construct a command structure
        parser(cmd);                        // Parse the input and assign it to cmd

        switch (cmd.cmd)            // Switch case for each operation
        {
            case CD:
                if(cmd.params[0][0] != '/')             // Relative path given
                {
                    tokenizer(cmd.params[0], temp_vec);
                    temp_dir = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                    if(temp_dir != -1)      // If found...
                    {
                        for (string s: temp_vec)        // Vector manipulation for the prompt, handle ".."
                        {
                            if(s == "..") cur_directory.pop_back();
                            else if(s == ".") continue;
                            else cur_directory.push_back(s);
                        }
                        if(temp_dir == 0) current_cluster = boot_sector.extended.RootCluster;
                        else current_cluster = temp_dir;
                    }
                }
                else            // Absolute path given
                {
                    tokenizer(cmd.params[0].substr(1,cmd.params[0].size()), temp_vec);
                    if(temp_vec.size() == 1 && temp_vec[0] == "")           // Absolute path of root is given
                    {
                        current_cluster = boot_sector.extended.RootCluster;
                        cur_directory.clear();
                    }
                    else
                    {
                        temp_dir = findDirectory(disk, boot_sector.extended.RootCluster, boot_sector, temp_vec);

                        if (temp_dir != -1)
                        {
                            cur_directory = temp_vec;
                            current_cluster = temp_dir;
                        }
                    }
                }

                temp_vec.clear();
                break;

            case LS:

                file_count = 0;

                if(cmd.params.size() == 0)          // ls is used without argument and path
                {
                    readDirectory(disk,current_cluster,boot_sector, false, file_count);
                    if(file_count != 0) cout << endl;
                }

                else if(cmd.params[0] != "-l")          // Only a path is given
                {
                    if(cmd.params[0][0] != '/')         // Relative
                    {
                        tokenizer(cmd.params[0], temp_vec);
                        int32_t temp_cluster = findDirectory(disk, current_cluster, boot_sector, temp_vec);
                        if (temp_cluster != -1)
                        {
                            readDirectory(disk, temp_cluster, boot_sector, false, file_count);
                            if(file_count != 0) cout << endl;
                        }
                    }

                    else            // Absolute
                    {
                        tokenizer(cmd.params[0].substr(1,cmd.params[0].size()), temp_vec);
                        if(temp_vec.size() == 1 && temp_vec[0] == "")       // Root
                        {
                            readDirectory(disk, boot_sector.extended.RootCluster, boot_sector, false, file_count);
                            if(file_count != 0) cout << endl;
                        }
                        else
                        {
                            int32_t temp_cluster = findDirectory(disk, boot_sector.extended.RootCluster, boot_sector,temp_vec);
                            if (temp_cluster != -1)
                            {
                                readDirectory(disk, temp_cluster, boot_sector, false, file_count);
                                if (file_count != 0) cout << endl;
                            }
                        }
                    }
                }

                else            // -l
                {
                    if(cmd.params.size() == 1)          // No path
                    {
                        readDirectory(disk,current_cluster,boot_sector, true, file_count);
                    }

                    else{
                        if(cmd.params[1][0] != '/')     // Relative
                        {
                            tokenizer(cmd.params[1], temp_vec);
                            int32_t temp_cluster = findDirectory(disk, current_cluster, boot_sector, temp_vec);
                            if (temp_cluster != -1)
                            {
                                readDirectory(disk, temp_cluster, boot_sector, true, file_count);
                            }
                        }
                        else{
                            tokenizer(cmd.params[1].substr(1,cmd.params[1].size()), temp_vec);
                            if(temp_vec.size() == 1 && temp_vec[0] == "")
                            {
                                readDirectory(disk, boot_sector.extended.RootCluster, boot_sector, true, file_count);
                            }
                            else
                            {
                                int32_t temp_cluster = findDirectory(disk, boot_sector.extended.RootCluster, boot_sector,temp_vec);
                                if (temp_cluster != -1) {
                                    readDirectory(disk, temp_cluster, boot_sector, true, file_count);
                                }
                            }
                        }
                    }
                }

                temp_vec.clear();

                break;

            case MKDIR:

                create_count++;

                if(cmd.params[0][0] != '/')             // Relative
                {
                    tokenizer(cmd.params[0], temp_vec);
                    int32_t temp_cluster = findDirectory(disk, current_cluster, boot_sector, temp_vec);
                    if(temp_cluster == -1)
                    {
                        string name = temp_vec.back();
                        temp_vec.pop_back();
                        if(temp_vec.empty()) createDirectory(disk,current_cluster,current_cluster,boot_sector,name,create_count);   // Only the new folder name
                        else
                        {
                            temp_cluster = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                            createDirectory(disk,temp_cluster,temp_cluster,boot_sector,name,create_count);
                        }
                    }

                }
                else{
                    tokenizer(cmd.params[0].substr(1,cmd.params[0].size()), temp_vec);
                    int32_t temp_cluster = findDirectory(disk, boot_sector.extended.RootCluster, boot_sector, temp_vec);
                    if(temp_cluster == -1){
                        string name = temp_vec.back();
                        temp_vec.pop_back();
                        if(temp_vec.empty()) createDirectory(disk,boot_sector.extended.RootCluster,boot_sector.extended.RootCluster,boot_sector,name,create_count);
                        else{
                            temp_cluster = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                            createDirectory(disk,temp_cluster,temp_cluster,boot_sector,name,create_count);
                        }
                    }
                }

                temp_vec.clear();

                break;

            case MV:

                if(cmd.params[1][0] != '/')         // First argument is relative
                {
                    tokenizer(cmd.params[1], temp_vec);
                    int32_t temp_cluster = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                    temp_vec.clear();

                    if(temp_cluster != -1){

                        if(cmd.params[0][0] != '/')         // Second argument is relative
                        {
                            tokenizer(cmd.params[0],temp_vec);
                            int32_t temp_cluster1 = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                            if(temp_cluster1 != -1){
                                string name = temp_vec.back();
                                temp_vec.pop_back();
                                if(temp_vec.empty()){
                                    moveDirectory(disk,current_cluster,temp_cluster,boot_sector,name);
                                }
                                else{
                                    temp_cluster1 = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                                    moveDirectory(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                }
                            }

                            else{
                                temp_cluster1 = findFile(disk,current_cluster,boot_sector,temp_vec);

                                if(temp_cluster1 != -1){

                                    string name = temp_vec.back();
                                    temp_vec.pop_back();
                                    if(temp_vec.empty()){
                                        moveFile(disk,current_cluster,temp_cluster,boot_sector,name);
                                    }
                                    else{
                                        temp_cluster1 = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                                        moveFile(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                    }

                                }
                            }
                        }

                        else{
                            tokenizer(cmd.params[0].substr(1,cmd.params[0].size()), temp_vec);
                            int32_t temp_cluster1 = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                            if(temp_cluster1 != -1){
                                string name =temp_vec.back();
                                temp_vec.pop_back();
                                if(temp_vec.empty()){
                                    moveDirectory(disk,boot_sector.extended.RootCluster,temp_cluster,boot_sector,name);
                                }
                                else{
                                    temp_cluster1 = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                                    moveDirectory(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                }
                            }

                            else{
                                temp_cluster1 = findFile(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);

                                if(temp_cluster1 != -1){

                                    string name = temp_vec.back();
                                    temp_vec.pop_back();
                                    if(temp_vec.empty()){
                                        moveFile(disk,boot_sector.extended.RootCluster,temp_cluster,boot_sector,name);
                                    }
                                    else{
                                        temp_cluster1 = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                                        moveFile(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                    }

                                }
                            }
                        }
                    }
                }

                else{
                    tokenizer(cmd.params[1].substr(1,cmd.params[1].size()), temp_vec);

                    int32_t temp_cluster = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);

                    if(temp_vec.size() == 1 && temp_vec[0] == ""){
                        temp_cluster = boot_sector.extended.RootCluster;
                    }

                    temp_vec.clear();

                    if(temp_cluster != -1){
                        if(cmd.params[0][0] != '/'){
                            tokenizer(cmd.params[0],temp_vec);
                            int32_t temp_cluster1 = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                            if(temp_cluster1 != -1){
                                string name =temp_vec.back();
                                temp_vec.pop_back();
                                if(temp_vec.empty()){
                                    moveDirectory(disk,current_cluster,temp_cluster,boot_sector,name);
                                }
                                else{
                                    temp_cluster1 = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                                    moveDirectory(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                }
                            }

                            else{
                                temp_cluster1 = findFile(disk,current_cluster,boot_sector,temp_vec);

                                if(temp_cluster1 != -1){

                                    string name = temp_vec.back();
                                    temp_vec.pop_back();
                                    if(temp_vec.empty()){
                                        moveFile(disk,current_cluster,temp_cluster,boot_sector,name);
                                    }
                                    else{
                                        temp_cluster1 = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                                        moveFile(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                    }

                                }
                            }
                        }

                        else{
                            tokenizer(cmd.params[0].substr(1,cmd.params[0].size()), temp_vec);
                            int32_t temp_cluster1 = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                            if(temp_cluster1 != -1){
                                string name =temp_vec.back();
                                temp_vec.pop_back();
                                if(temp_vec.empty()){
                                    moveDirectory(disk,boot_sector.extended.RootCluster,temp_cluster,boot_sector,name);
                                }
                                else{
                                    temp_cluster1 = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                                    moveDirectory(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                }
                            }

                            else{
                                temp_cluster1 = findFile(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);

                                if(temp_cluster1 != -1){

                                    string name = temp_vec.back();
                                    temp_vec.pop_back();
                                    if(temp_vec.empty()){
                                        moveFile(disk,boot_sector.extended.RootCluster,temp_cluster,boot_sector,name);
                                    }
                                    else{
                                        temp_cluster1 = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                                        moveFile(disk,temp_cluster1,temp_cluster,boot_sector,name);
                                    }

                                }
                            }
                        }
                    }
                }


                temp_vec.clear();
                break;
            case TOUCH:

                create_count++;

                if(cmd.params[0][0] != '/') {
                    tokenizer(cmd.params[0], temp_vec);
                    int32_t temp_cluster = findFile(disk,current_cluster,boot_sector,temp_vec);
                    if(temp_cluster == -1){
                        string name = temp_vec.back();
                        temp_vec.pop_back();
                        if(temp_vec.empty()) createFile(disk,current_cluster,current_cluster,boot_sector,name,create_count);
                        else{
                            temp_cluster = findDirectory(disk,current_cluster,boot_sector,temp_vec);
                            createFile(disk,temp_cluster,temp_cluster,boot_sector,name,create_count);
                        }
                    }
                }
                else{
                    tokenizer(cmd.params[0].substr(1,cmd.params[0].size()), temp_vec);
                    int32_t temp_cluster = findFile(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                    if(temp_cluster == -1){
                        string name = temp_vec.back();
                        temp_vec.pop_back();
                        if(temp_vec.empty()) createFile(disk,boot_sector.extended.RootCluster,boot_sector.extended.RootCluster,boot_sector,name,create_count);
                        else{
                            temp_cluster = findDirectory(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                            createFile(disk,temp_cluster,temp_cluster,boot_sector,name,create_count);
                        }
                    }
                }

                temp_vec.clear();

                break;
            case CAT:

                if(cmd.params[0][0] != '/') {
                    tokenizer(cmd.params[0], temp_vec);
                    int32_t temp_cluster = findFile(disk,current_cluster,boot_sector,temp_vec);
                    if(temp_cluster != -1){
                        readFile(disk,temp_cluster,boot_sector,temp_vec.back());
                    }
                }
                else{
                    tokenizer(cmd.params[0].substr(1,cmd.params[0].size()), temp_vec);
                    int32_t temp_cluster = findFile(disk,boot_sector.extended.RootCluster,boot_sector,temp_vec);
                    if(temp_cluster != -1){
                        readFile(disk,temp_cluster,boot_sector,temp_vec.back());
                    }
                }

                temp_vec.clear();

                break;
            case QUIT:
                goto end;
        }

        copyFat(disk,boot_sector);
    }

    end:

    return 0;
}