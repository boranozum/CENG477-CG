#include "utils.h"
#include "routines.h"

/*  TODO:
 *      - Timing issue after got a continue order
 *      - SegFault somewhere
 *      - test
 */

int main(){

    hw2_init_notifier();

    vector<int> grid_size = parser();       // Taking grid size
    vector<vector<int>> grid;

    for (int i = 0; i < grid_size[0]; i++)      // Creating the grid
    {
        vector<int> row = parser();
        grid.push_back(row);
    }

    vector<proper_private> proper_privates;     // Proper private array
    int proper_private_count = parser()[0];      // How many proper privates?

    for (int i = 0; i < proper_private_count; i++)
    {
        proper_private p;           // Default constructor calls parser for each top-left corner
        proper_privates.push_back(p);
    }

    int order_count = parser()[0];      // How many orders?
    vector<pair<int,string>> orders;       // Order array

    for (int i = 0; i < order_count; ++i) {
        order_parser(orders);
    }

    int smoker_count = parser()[0];     // How many smokers?
    vector<sneaky_smoker> sneaky_smokers;       // Smoker array

    for (int i = 0; i < smoker_count; ++i) {
        sneaky_smoker s;
        sneaky_smokers.push_back(s);
    }

    int overlap_count = 0;
    int index = 0;

    // COMPARISON FOR OVERLAP BETWEEN EVERY AREA THAT PROPER PRIVATES WILL CLEAN
    for (int i = 0; i < proper_private_count-1; ++i) {
        for (int j = i+1; j < proper_private_count; ++j) {
            for (int k = 0; k < proper_privates[i].getAreaCount(); ++k) {
                for (int l = 0; l < proper_privates[j].getAreaCount(); ++l) {

                    if(find_overlap(proper_privates[i].getAreas()[k],proper_privates[i].getGatherArea(),
                                    proper_privates[j].getAreas()[l],proper_privates[j].getGatherArea())){
                        overlap_count++;

                        proper_privates[i].updateMutexMap(k,index);
                        proper_privates[j].updateMutexMap(l,index);
                        index++;
                    }
                }
            }
        }
    }
    // --------------------------------------------------------------------------

    int overlap_count1 = 0;
    index = 0;

    // COMPARISON FOR OVERLAP BETWEEN CLEANING AREAS AND 3X3 SMOKING AREAS
    for (int i = 0; i < proper_private_count; ++i) {
        for (int j = 0; j < smoker_count; ++j) {
            for (int k = 0; k < proper_privates[i].getAreaCount(); ++k) {
                for (int l = 0; l < sneaky_smokers[j].getCellCount(); ++l) {
                    if(find_overlap(proper_privates[i].getAreas()[k],proper_privates[i].getGatherArea(),
                                    {sneaky_smokers[j].getCells()[l].first-1,sneaky_smokers[j].getCells()[l].second-1},
                                    {3,3})){
                        overlap_count1++;

                        proper_privates[i].updateSmokerMap(k,index);
                        sneaky_smokers[j].updateMutexMap(l,index);

                        index++;
                    }
                }
            }
        }
    }
    // ---------------------------------------------------------------------

    pthread_t th[proper_private_count+1];   // Proper private threads + Order thread
    pthread_t s_th[smoker_count];           // Smoker Threads
    pthread_mutex_t mutexes[overlap_count];     // Lock out proper privates
    pthread_mutex_t smoke_mutexes[overlap_count1];      // Lock out smokers (vice versa)

    pthread_cond_t order_obeyed = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t obey_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t go_smokers = PTHREAD_COND_INITIALIZER;
    pthread_cond_t smoker_ok = PTHREAD_COND_INITIALIZER;

    auto** grid_mutexes = new pthread_mutex_t*[grid_size[0]];   // Each cell has own mutex for smokers consistency
    auto** grid_for_smokers = new pthread_mutex_t*[grid_size[0]];
    pthread_mutex_t condMutex;      // Temp mutex for condition variables

    pthread_cond_t condOrder[3];        // Condition variables (0 -> break/stop for proper privates, 1 -> continue, 2 stop for smokers)

    // INITIALIZATION OF EACH MUTEX AND CONDITIONAL VARIABLES
    pthread_mutex_init(&condMutex, nullptr);
    for (int i = 0; i < 3; ++i) {
        pthread_cond_init(&condOrder[i], nullptr);
    }
    for (int i = 0; i < overlap_count; ++i) {
        pthread_mutex_init(&mutexes[i], nullptr);
    }
    for (int i = 0; i < overlap_count1; ++i) {
        pthread_mutex_init(&smoke_mutexes[i], nullptr);
    }
    for (int i = 0; i < grid_size[0]; ++i) {
        grid_mutexes[i] = new pthread_mutex_t[grid_size[1]];
        grid_for_smokers[i] = new pthread_mutex_t[grid_size[1]];
        for (int j = 0; j < grid_size[1]; ++j) {
            pthread_mutex_init(&grid_mutexes[i][j], nullptr);
            pthread_mutex_init(&grid_for_smokers[i][j], nullptr);
        }
    }
    // -----------------------------------------------------

    auto *flag = new bool;          // Stop flag for proper privates
    *flag = false;

    auto *break_flag = new bool;       // Break flag for proper privates
    *break_flag = false;

    int* obeyed_privates = new int;
    *obeyed_privates = 0;

    auto * order = new bool;
    *order = false;

    int * initial = new int;
    *initial = 0;

    vector<mutex_status> actively_using_p (overlap_count,UNLOCKED);
    vector<mutex_status> actively_using_s (overlap_count1,UNLOCKED);

    // PROPER PRIVATES THREAD CREATION
    for (int i = 0; i < proper_private_count; ++i) {

        auto *temp_args = new proper_args;

        temp_args->p = &proper_privates[i];
        temp_args->smoke_mutexes = smoke_mutexes;
        temp_args->mutexes = mutexes;
        temp_args->grid = &grid;
        temp_args->condOrders = condOrder;
        temp_args->condMutex = &condMutex;
        temp_args->flag = flag;
        temp_args->break_flag = break_flag;
        temp_args->actively_using_p = &actively_using_p;
        temp_args->actively_using_s = &actively_using_s;
        temp_args->obeyed_privates = obeyed_privates;
        temp_args->private_count = proper_private_count;
        temp_args->obey = &order_obeyed;
        temp_args->obey_mutex = &obey_mutex;

        if(pthread_create(&th[i], nullptr,&pickUpCigButts,temp_args) != 0) {
            perror("Failed to create thread");
        }
    }
    // -------------------------------------------------

    // ORDER THREAD CREATION
    if(order_count != 0){

        auto* temp_order = new order_args;

        temp_order->orders = orders;
        temp_order->flag = flag;
        temp_order->condOrders = condOrder;
        temp_order->break_flag = break_flag;
        temp_order->obey = &order_obeyed;
        temp_order->mutexes = smoke_mutexes;
        temp_order->actively_using = &actively_using_s;
        temp_order->overlap_count1 = overlap_count1;
        temp_order->go_smokers = &go_smokers;
        temp_order->order_obeyed = order;
        temp_order->smoker_ok = &smoker_ok;

        if(pthread_create(&th[proper_private_count], nullptr,&giveOrder, temp_order) != 0){
            perror("Failed to create thread");
        }
    }
    // ----------------------

    // SMOKER THREADS CREATION
    for (int i = 0; i < smoker_count; ++i) {

        auto* temp_args = new smoker_args;

        temp_args->s = &sneaky_smokers[i];
        temp_args->mutexes = smoke_mutexes;
        temp_args->grid_mutexes = grid_mutexes;
        temp_args->grid_for_smokers = grid_for_smokers;
        temp_args->grid = &grid;
        temp_args->condOrders = condOrder;
        temp_args->condMutex = &condMutex;
        temp_args->flag = flag;
        temp_args->actively_using = &actively_using_s;
        temp_args->order_obeyed = order;
        temp_args->go_smokers = &go_smokers;
        temp_args->smoker_ok = &smoker_ok;
        temp_args->initial = initial;
        temp_args->smoker_count = smoker_count;

        if(pthread_create(&s_th[i], nullptr,&smokeAndLitter, temp_args) != 0){
            perror("Failed to create thread");
        }
    }
    // -----------------------------------------

    // JOINS
    for (int i = 0; i < proper_private_count; ++i) {
        if(pthread_join(th[i], nullptr) != 0){
            perror("Failed to join thread");
        }
    }
    if(order_count != 0){
        if(pthread_join(th[proper_private_count], nullptr) != 0){
            perror("Failed to join thread");
        }
    }
    for (int i = 0; i < smoker_count; ++i) {
        if(pthread_join(s_th[i], nullptr) != 0){
            perror("Failed to join thread");
        }
    }
    // -------------------------------------------------

    // DESTROYS
    for (int i = 0; i < overlap_count; ++i) {
        pthread_mutex_destroy(&mutexes[i]);
    }
    for (int i = 0; i < overlap_count1; ++i) {
        pthread_mutex_destroy(&smoke_mutexes[i]);
    }
    for (int i = 0; i < grid_size[0]; ++i) {
        for (int j = 0; j < grid_size[1]; ++j) {
            pthread_mutex_destroy(&grid_mutexes[i][j]);
            pthread_mutex_destroy(&grid_for_smokers[i][j]);
        }
        delete grid_mutexes[i];
        delete grid_for_smokers[i];
    }
    for (int i = 0; i < 3; ++i) {
        pthread_cond_destroy(&condOrder[i]);
    }
    pthread_mutex_destroy(&condMutex);
    pthread_cond_destroy(&order_obeyed);
    pthread_mutex_destroy(&obey_mutex);
    pthread_cond_destroy(&go_smokers);
    pthread_cond_destroy(&smoker_ok);
    // ------------------------------------------------

    delete [] grid_mutexes;
    delete [] grid_for_smokers;

    return 0;
}