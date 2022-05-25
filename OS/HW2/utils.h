#ifndef UTILS_H
#define UTILS_H

// INCLUDES

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include "hw2_output.h"
#include <unistd.h>
#include <sys/time.h>

using namespace std;

// PARSER FUNCTION PROTOTYPE
vector<int> parser();

// ORDER PARSER PROTOTYPE
void order_parser(vector<pair<int,string>> & orders);

// PROPER PRIVATE CLASS DECLARATION
class proper_private
{
    private:
        int gid;
        int delay_time;
        int area_count;
        pair<int,int> gather_area;
        vector<pair<int,int>> areas;
        vector<vector<int>> mutex_map;
        vector<vector<int>> smoker_map;

    public:
        proper_private();
        ~proper_private();
        int getGid() const;
        int getDelay() const;
        int getAreaCount() const;
        pair<int,int> getGatherArea();
        vector<pair<int,int>> getAreas();
        vector<vector<int>> getMutexMap();
        vector<vector<int>> getSmokeMap();
        void updateMutexMap(int k, int index);
        void updateSmokerMap(int k, int index);
};

// SNEAKY SMOKER CLASS DECLARATION
class sneaky_smoker
{
    private:
        int sid;
        int smoke_time;
        int cell_count;
        vector<pair<int,int>> cells;
        vector<int> smoke_count;
        vector<vector<int>> mutex_map;

    public:
        sneaky_smoker();
        ~sneaky_smoker();
        int getSid() const;
        int getSmokeTime() const;
        int getCellCount() const;
        vector<pair<int,int>> getCells();
        int getSmokeCount(int index);
        vector<vector<int>> getMutexMap();
        void updateMutexMap(int k,int index);
};

// OVERLAP FINDER PROTOTYPE
bool find_overlap(pair<int,int> p1_tl, pair<int,int> p1_a, pair<int,int> p2_tl, pair<int,int> p2_a);

enum mutex_status {UNLOCKED,PENDING,ACTIVE,HELD_BY_COMMANDER};

// ROUTINE ARGUMENT STRUCTURE FOR PROPER PRIVATE THREADS
typedef struct proper_args{
    proper_private* p;
    pthread_mutex_t* mutexes;
    pthread_mutex_t* smoke_mutexes;
    vector<mutex_status>* actively_using_p;
    vector<mutex_status>* actively_using_s;
    vector<vector<int>>* grid;
    pthread_cond_t* condOrders;
    pthread_mutex_t* condMutex;
    bool * flag;
    bool * break_flag;
    int * obeyed_privates;
    int private_count;
    pthread_cond_t * obey;
    pthread_mutex_t * obey_mutex;
} proper_args;

// ORDER ROUTINE ARGUMENT STRUCTURE FOR ORDER THREADS
typedef struct order_args{
    vector<pair<int,string>> orders;
    pthread_cond_t* condOrders;
    bool * flag;
    bool * break_flag;
    int overlap_count1;
    bool * order_obeyed;
    pthread_cond_t * go_smokers;
    pthread_cond_t * obey;
    pthread_mutex_t* mutexes;
    vector<mutex_status>* actively_using;
    pthread_cond_t * smoker_ok;
} order_args;

// ROUTINE ARGUMENT STRUCTURE FOR SNEAKY SMOKER THREADS
typedef struct smoker_args{
    sneaky_smoker* s;
    pthread_mutex_t* mutexes;
    pthread_mutex_t** grid_mutexes;
    pthread_mutex_t** grid_for_smokers;
    vector<mutex_status>* actively_using;
    vector<vector<int>>* grid;
    pthread_cond_t* condOrders;
    pthread_mutex_t* condMutex;
    pthread_cond_t * go_smokers;
    pthread_cond_t * smoker_ok;
    int * initial;
    int smoker_count;
    bool * flag;
    bool * order_obeyed;
}smoker_args;

#endif