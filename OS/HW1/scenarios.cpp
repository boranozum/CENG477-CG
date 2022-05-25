#include "scenarios.h"

void scenario_0(bundle b1){

    int process_count = b1.process_count;

    for (int i = 0; i < process_count; ++i) {

        // Fork call for every command in the bunde

        if(fork()==0){       // Child

            if(execvp(b1.processes[i][0], b1.processes[i]) == -1) perror("Couldn't execute execve!");    //execv call
        }
    }

    // Get rid of the zombies :S

    for (int i = 0; i < process_count; i++)
    {
        pid_t wpid = wait(NULL);
    }   
}

void scenario_1(bundle b1, char* input, char* output){

    int process_count = b1.process_count;

    int input_fd;
    int output_fd;

    for (int i = 0; i < process_count; ++i) {

        if(fork()==0){

            // Open calls 
            // if the file does not exist create one
            if(output != nullptr){

                output_fd = open(output, O_WRONLY | O_CREAT | O_APPEND, 0777); // Included append

                dup2(output_fd,1); // dup2 call to change stdout to the output file

                close(output_fd);
            }
            
            if(input != nullptr){

                input_fd = open(input, O_RDONLY | O_CREAT, 0777); 

                dup2(input_fd,0); // dup2 call to change stdout to the output file

                close(input_fd);
            }

            if(execvp(b1.processes[i][0], b1.processes[i]) == -1) perror("Couldn't execute execve!");
        }
    }    

    for (int i = 0; i < process_count; i++)
    {
        pid_t wpid = wait(NULL);
    }
}

void scenario_2(bundle b1, char* input, bundle b2, char* output){

    int fd[2];

    if(pipe(fd) == -1){
        cout << "An error occurred while creating the pipe!" << endl;
    }

    if(fork()){

        // Parent will wait, will not use fd's
        if(fork()){
            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
        }

        // Second child handles RHS
        else{

            if(output != nullptr){

                int output_fd = open(output, O_WRONLY | O_CREAT | O_APPEND, 0777); // Included append

                dup2(output_fd,1); // dup2 call to change stdout to the output file

                close(output_fd);
            }
            
            close(fd[1]);

            dup2(fd[0], 0);

            close(fd[0]);

            if(execvp(b2.processes[0][0], b2.processes[0]) == -1) perror("Couldn't execute execve!");
        }
    }

    // First child handles LHS
    else{

        if(input != nullptr){

            int input_fd = open(input, O_RDONLY | O_CREAT, 0777); 

            dup2(input_fd,0); 

            close(input_fd);
        }

        close(fd[0]);

        dup2(fd[1],1);

        close(fd[1]);

        if(execvp(b1.processes[0][0], b1.processes[0]) == -1) perror("Couldn't execute execve!");
    }
}

void scenario_3(vector<bundle> bundles, vector<int> bundles_to_be_executed, 
                        char* input, char* output){

    int pipe_count = bundles_to_be_executed.size()-1;

    int fd[pipe_count][2];

    for (int i = 0; i < pipe_count; i++)
    {
        pipe(fd[i]);
    }

    for (int i = 0; i < bundles_to_be_executed.size(); i++)
    {
        if(fork() == 0){

            if(i == 0){
                dup2(fd[i][1],1);

                if(input != nullptr){
                    int input_fd = open(input, O_RDONLY | O_CREAT, 0777); 

                    dup2(input_fd,0); 

                    close(input_fd);
                }
            }

            else if(i == pipe_count){
                dup2(fd[i-1][0],0);

                if(output != nullptr){
                    int output_fd = open(output, O_WRONLY | O_CREAT | O_APPEND, 0777); // Included append

                    dup2(output_fd,1); // dup2 call to change stdout to the output file

                    close(output_fd);
                }
            }

            else{
                dup2(fd[i-1][0],0);
                dup2(fd[i][1],1);
            }

            for(int j = 0; j < pipe_count; j++){
                close(fd[j][0]); close(fd[j][1]);
            }
            
            if(execvp(bundles[bundles_to_be_executed[i]].processes[0][0], 
                bundles[bundles_to_be_executed[i]].processes[0]) == -1) perror("Couldn't execute execve!");

        }
    }

    for(int j = 0; j < pipe_count; j++){
        close(fd[j][0]); close(fd[j][1]);
    }
    
    for(int i = 0; i < pipe_count+1; i++){
        wait(NULL);
    }
}

void scenario_4(vector<bundle> bundles, vector<int> bundles_to_be_executed, 
                        char* input, char* output){
    
    int connections = bundles_to_be_executed.size()-1;

    int pipe_count = 0;

    int child_count = 0;

    vector<int> w_indexes;

    for (int i = 0; i < connections; i++)
    {
        w_indexes.push_back(pipe_count);

        pipe_count += bundles[bundles_to_be_executed[i]].process_count + bundles[bundles_to_be_executed[i+1]].process_count;

        child_count += bundles[bundles_to_be_executed[i]].process_count;      
    }

    w_indexes.push_back(pipe_count);    

    child_count += bundles[bundles_to_be_executed.back()].process_count; 
    
    int fd[pipe_count][2];

    for (int i = 0; i < pipe_count; i++)
    {
        if(pipe(fd[i]) == -1) perror("Piping error");
    }

    connections++;

    for (int i = 0; i < connections; i++)
    {    

        string buf;

        //cout << i << endl;

        for (int j = 0; j < bundles[bundles_to_be_executed[i]].process_count; j++)
        {
            
            if(fork() == 0){

                //cout << "Child created" << endl;
                if(i == 0){
                    if(input != nullptr){
                        int input_fd = open(input, O_RDONLY | O_CREAT, 0777);

                        dup2(input_fd,0);

                        close(input_fd);
                    }

                    dup2(fd[j][1],1);

                    for(int k = 0; k < pipe_count; k++){
                        close(fd[k][0]); close(fd[k][1]);
                    }

                    if(execvp(bundles[bundles_to_be_executed[i]].processes[j][0], bundles[bundles_to_be_executed[i]].processes[j]) == -1)
                        perror("Couldn't execute execve!");
                }

                else if(i != connections-1){


                    dup2(fd[w_indexes[i]-bundles[bundles_to_be_executed[i]+j].process_count][0],0);
                    dup2(fd[w_indexes[i]+j][1],1);

                    for(int k = 0; k < pipe_count; k++){
                        close(fd[k][0]); close(fd[k][1]);
                    }

                    if(execvp(bundles[bundles_to_be_executed[i]].processes[j][0], bundles[bundles_to_be_executed[i]].processes[j]) == -1)
                        perror("Couldn't execute execve!");
                }

                else{


                    if(output != nullptr){
                        int output_fd = open(output, O_WRONLY | O_CREAT | O_APPEND, 0777); // Included append

                        dup2(output_fd,1); // dup2 call to change stdout to the output file

                        close(output_fd);
                    }

                    dup2(fd[w_indexes[i+1]-bundles[bundles_to_be_executed[i+1]+j].process_count][0],0);

                    for(int k = 0; k < pipe_count; k++){
                        close(fd[k][0]); close(fd[k][1]);
                    }

                    if(execvp(bundles[bundles_to_be_executed[i+1]].processes[j][0], bundles[bundles_to_be_executed[i+1]].processes[j]) == -1)
                        perror("Couldn't execute execve!");
                    
                }
            }

            

            char temp[512];

            ssize_t byte = read(fd[w_indexes[i]+j][0], temp, 511*sizeof(char));

            temp[byte] = '\0';

            string str(temp);

            buf += str;

            //cout << "READ: \n" << buf << endl;
        }

        if(i != connections-1){

            for(int j = 0; j < bundles[bundles_to_be_executed[i+1]].process_count; j++){
                write(fd[w_indexes[i]+j+bundles[bundles_to_be_executed[i]].process_count][1], buf.c_str(), 4095*sizeof(char));
            }
        }        
    }

    for(int j = 0; j < pipe_count; j++){
        close(fd[j][0]); close(fd[j][1]);
    }
    
    for (int i = 0; i < child_count; i++)
    {
        wait(NULL);
    }
}

void scenario_5(bundle b1, char* input, bundle b2, char* output){

    int pipe_count = b1.process_count + b2.process_count;
    int fd[pipe_count][2];

    for(int i = 0; i < pipe_count; i++){

        if(pipe(fd[i]) == -1) perror("Piping error");
    }

    pid_t pid;

    if((pid=fork()) == 0){

        string buf;

        pid_t pid;

        for(int i = 0; i < b1.process_count; i++){

            char temp[4096];

            if((pid=fork()) == 0){

                if(input != nullptr){
                    int input_fd = open(input, O_RDONLY | O_CREAT, 0777);

                    dup2(input_fd,0);

                    close(input_fd);
                }

                dup2(fd[i][1],1);

                for(int j = 0; j < pipe_count; j++){
                    close(fd[j][0]); close(fd[j][1]);
                }

                if(execvp(b1.processes[i][0], b1.processes[i]) == -1)
                        perror("Couldn't execute execve!"); 
            }

            int byte = read(fd[i][0], temp, 4095*sizeof(char));

            temp[byte] = '\0';

            string str(temp);

            buf += str;
        }

        for (int i = 0; i < b2.process_count; i++)
        {
            if((pid=fork()) == 0){

                if(output != nullptr){          // Handle output redirection

                    int output_fd = open(output, O_WRONLY | O_CREAT | O_APPEND, 0777); // Included append

                    dup2(output_fd,1); // dup2 call to change stdout to the output file

                    close(output_fd);
                }

                dup2(fd[i+b1.process_count][0],0);

                for(int j = 0; j < pipe_count; j++){
                    close(fd[j][0]); close(fd[j][1]);
                }

                if(execvp(b2.processes[i][0], b2.processes[i]) == -1)
                        perror("Couldn't execute execve!");
            }

            else{
                write(fd[i+b1.process_count][1], buf.c_str(), 4095*sizeof(char));
            }   
        }

        for(int j = 0; j < pipe_count; j++){
            close(fd[j][0]); close(fd[j][1]);
        }

        exit(0);
    }
    
    else{

        for(int j = 0; j < pipe_count; j++){
            close(fd[j][0]); close(fd[j][1]);
        }
        for(int i = 0; i < pipe_count+1; i++){
            pid_t pid = wait(NULL);
        }
    }

}

