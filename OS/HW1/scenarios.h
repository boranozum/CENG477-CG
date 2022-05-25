#ifndef SCENARIOS_H
#define SCENARIOS_H

// Required includes

#include <iostream>
#include <vector>
#include <cstring>
#include <sys/wait.h>
#include "unistd.h"
#include "fcntl.h"

using namespace std;

typedef struct bundle{  // Bundle representation, includes its processes

    string name; // Name of the bundle

    int process_count; // Count of how many commands that the bundle contains.

    // Struct constructor
    bundle(string name){
        this->name = name;
        this->process_count = 0;
    }

    vector<char**> processes; // process vector

} bundle;

void scenario_0(bundle b1); // One bundle, no file redirection or pipeline

void scenario_1(bundle b1, char* input_file, char* output_file); // One bundle, includes file direction

void scenario_2(bundle b1, char* input, bundle b2, char* output); // Two bundles single process pipe with file redirection

void scenario_3(vector<bundle> bundles, vector<int> bundles_to_be_executed, 
                        char* input_file, char* output_file);   // Multiple bundles single pipe

void scenario_4(vector<bundle> bundles, vector<int> bundles_to_be_executed, 
                        char* input_file, char* output_file);   // Repeater  

void scenario_5(bundle b1, char* input, bundle b2, char* output); // Two bundles multiple process pipe           

#endif