#include "parser.h"
#include "scenarios.h"
#include <iostream>
#include <vector>

// TODO:
//  - Infinite loop structure (DONE)
//  - Quit functionality (DONE)
//  - PBC, PBS templates (DONE)
//  - Introduce parse() (DONE)
//  - Implement bundle structure (DONE)
//  - Introduce bundle processes to bundles (DONE)
//  - Test bundle structure (DONE)
//  - Single process no pipe - no file redirection (DONE)
//      - Fork() (DONE)
//      - execve() (DONE)
//  - Bundle deletion (DONE)
//  - Single process with file redirection (DONE)
//  - Two bundles with single process with pipe (DONE)
//  - Reaping (DONE)
//  - Single process multiple bundles pipe (DONE)
//  - Two bundles multiple process pipe (DONE)
//  - Multiple bundles, multiple processes
//  - Handle whitspace issue (DONE)

using namespace std;

int main(){

    vector<bundle> bundles;

    int index = 0;
    int creation_is_underway = 0; // is_bundle_creation

    // Infinite loop until the quit call
    while (true) {

        parsed_input *parsedInput = new parsed_input;
        char *line = new char[256];
        cin.getline(line, 256);

        char whitespace[] = " ";            

        strcat(line, whitespace);   // Appending a whitespace at the end of the input line for
                                    // the parse function to run properly.

        parse(line, creation_is_underway, parsedInput);

        // Quit case
        if (parsedInput->command.type == QUIT) {
            break;
        }

        else if(creation_is_underway == 1){
            if(parsedInput->command.type == PROCESS_BUNDLE_STOP){ // pbs encountered
                creation_is_underway = 0;
            }

            else{

                bundles.back().processes.push_back(parsedInput->argv); // Add the full command to the back of the bundles vector,
                                                                        // since current lines is related to the last bundle in the vector
                bundles.back().process_count++;
            }
        }

        else {
            if (parsedInput->command.type == PROCESS_BUNDLE_CREATE) {  // pbc encountered
                creation_is_underway = 1;

                string name(parsedInput->command.bundle_name);

                bundle new_bundle(name); // Create a new bundle

                bundles.push_back(new_bundle);
            }
            else if (parsedInput->command.type == PROCESS_BUNDLE_EXECUTION) {

                // Find out which bundles in bundle array that the current command includes

                vector<int> bundles_to_be_executed;

                bool each_have_single = true;

                for (int i = 0; i < bundles.size(); ++i) {
                    for (int j = 0; j < parsedInput->command.bundle_count; ++j) {
                        string temp(parsedInput->command.bundles[j].name);
                        if(bundles[i].name.compare(temp) == 0){
                            bundles_to_be_executed.push_back(i);
                            if(bundles[i].process_count > 1) each_have_single = false;
                            break;
                        }
                    }
                }

                if (parsedInput->command.bundle_count == 1) { // Command includes one bundle

                    if (parsedInput->command.bundles[0].input == nullptr &&
                        parsedInput->command.bundles[0].output == nullptr) { // No file redirection
                        scenario_0(bundles[bundles_to_be_executed[0]]);
                    }
                    else{
                        scenario_1(bundles[bundles_to_be_executed[0]], 
                        parsedInput->command.bundles[bundles_to_be_executed[0]].input, 
                        parsedInput->command.bundles[bundles_to_be_executed[0]].output);
                    }
                }

                else{

                    // Two bundles pipe, each have single process, with file redirection

                    if(bundles_to_be_executed.size() == 2 && bundles[bundles_to_be_executed[0]].process_count == 1 && 
                            bundles[bundles_to_be_executed[1]].process_count == 1){

                        scenario_2(bundles[bundles_to_be_executed[0]], 
                                parsedInput->command.bundles[0].input,bundles[bundles_to_be_executed[1]],
                                parsedInput->command.bundles[bundles_to_be_executed[1]].output);
                    }

                    // More than two bundles

                    else if(bundles_to_be_executed.size() == 2){
                        scenario_5(bundles[bundles_to_be_executed[0]], 
                                parsedInput->command.bundles[0].input,bundles[bundles_to_be_executed[1]],
                                parsedInput->command.bundles[bundles_to_be_executed[1]].output);
                    }

                    else{

                        if(each_have_single){
                            scenario_3(bundles,bundles_to_be_executed,
                                parsedInput->command.bundles[bundles_to_be_executed[0]].input,
                                parsedInput->command.bundles[bundles_to_be_executed.back()].output);
                        }
                        else{
                            scenario_4(bundles,bundles_to_be_executed,
                                parsedInput->command.bundles[bundles_to_be_executed[0]].input,
                                parsedInput->command.bundles[bundles_to_be_executed.back()].output);
                        }
                    }
                }

                // End of the execution, bundles that are used are removed for reuse
                for (int i = 0; i < bundles_to_be_executed.size(); ++i) {
                    bundles.erase(bundles.begin()+bundles_to_be_executed[i]);
                }
            }
        }

        delete parsedInput;
        delete line;
    }

    return 0;
}

