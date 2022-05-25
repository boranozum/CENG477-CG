#include "utils.h"
#include "routines.h"

// THREAD ROUTINE IMPLEMENTATION OF PROPER PRIVATES
void* pickUpCigButts(void* args) {

    auto *arguments = (proper_args *) args;      // Casting
    int gid = arguments->p->getGid();

    // DELAY ADJUSTMENTS
    struct timespec wait_time{};
    clock_gettime(CLOCK_REALTIME, &wait_time);

    long delay = arguments->p->getDelay();
    wait_time.tv_sec += time_t(delay / 1000);

    if (wait_time.tv_nsec + (delay % 1000) * 1000000 > 999999999) {
        wait_time.tv_sec++;
        wait_time.tv_nsec = wait_time.tv_nsec + (delay % 1000) * 1000000 - 999999999;
    } else {
        wait_time.tv_nsec += (delay % 1000) * 1000000;
    }
    // -----------------------------------------------

    hw2_notify(PROPER_PRIVATE_CREATED, gid, 0, 0);

    for (int i = 0; i < arguments->p->getAreaCount(); ++i) {

        // LOCK OUT OTHER PROPER PRIVATES IN THE AREA
        if (!arguments->p->getMutexMap()[i].empty()) {
            for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                int index = arguments->p->getMutexMap()[i][j];
                if ((*arguments->actively_using_p)[index] != PENDING) {
                    pthread_mutex_lock(&arguments->mutexes[index]);
                    (*arguments->actively_using_p)[index] = PENDING;
                }
            }
        }

        if (*arguments->flag) {
            if (!arguments->p->getMutexMap()[i].empty()) {
                for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                    int index = arguments->p->getMutexMap()[i][j];
                    (*arguments->actively_using_p)[index] = UNLOCKED;
                    pthread_mutex_unlock(&arguments->mutexes[index]);
                }
            }

            hw2_notify(PROPER_PRIVATE_STOPPED, gid, 0, 0);
            pthread_exit(nullptr);
        }

        // LOCK OUT SMOKERS IN THE AREA
        if (!arguments->p->getSmokeMap()[i].empty()) {
            for (int j = 0; j < arguments->p->getSmokeMap()[i].size(); ++j) {
                int index = arguments->p->getSmokeMap()[i][j];
                if ((*arguments->actively_using_s)[index] != PENDING) {
                    pthread_mutex_lock(&arguments->smoke_mutexes[index]);
                    (*arguments->actively_using_s)[index] = PENDING;
                }
            }
        }

        if (!arguments->p->getMutexMap()[i].empty()) {
            for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                int index = arguments->p->getMutexMap()[i][j];
                if ((*arguments->actively_using_p)[index] == ACTIVE) {
                    pthread_mutex_lock(&arguments->mutexes[index]);
                }
                (*arguments->actively_using_p)[index] = ACTIVE;
            }

            if (!arguments->p->getSmokeMap()[i].empty()) {
                for (int j = 0; j < arguments->p->getSmokeMap()[i].size(); ++j) {
                    int index = arguments->p->getSmokeMap()[i][j];
                    if ((*arguments->actively_using_s)[index] == ACTIVE) {
                        pthread_mutex_lock(&arguments->smoke_mutexes[index]);
                    }
                    (*arguments->actively_using_s)[index] = ACTIVE;
                }
            }
        }
        if (*arguments->flag) {
            if (!arguments->p->getMutexMap()[i].empty()) {
                for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                    int index = arguments->p->getMutexMap()[i][j];
                    (*arguments->actively_using_p)[index] = UNLOCKED;
                    pthread_mutex_unlock(&arguments->mutexes[index]);
                }
            }

            if (!arguments->p->getSmokeMap()[i].empty()) {
                for (int j = 0; j < arguments->p->getSmokeMap()[i].size(); ++j) {
                    int index = arguments->p->getSmokeMap()[i][j];
                    (*arguments->actively_using_s)[index] = UNLOCKED;
                    pthread_mutex_unlock(&arguments->smoke_mutexes[index]);
                }
            }

            hw2_notify(PROPER_PRIVATE_STOPPED, gid, 0, 0);
            pthread_exit(nullptr);
        }

        int i_index = arguments->p->getAreas()[i].first;
        int j_index = arguments->p->getAreas()[i].second;

        int j = i_index;
        int k = j_index;

        if (!*arguments->break_flag && !*arguments->flag) {
            hw2_notify(PROPER_PRIVATE_ARRIVED, gid, arguments->p->getAreas()[i].first,
                       arguments->p->getAreas()[i].second);
        }

        // CLEANING
        while (j < i_index + arguments->p->getGatherArea().first) {
            while (k < j_index + arguments->p->getGatherArea().second) {
                while ((*arguments->grid)[j][k] > 0) {

                    bool arrive = false;

                    // WAIT BETWEEN PICKING UP CIGBUTTS (IF AN ORDER COMES, HANDLE)
                    pthread_mutex_lock(arguments->condMutex);
                    if (*arguments->flag || *arguments->break_flag ||
                        !pthread_cond_timedwait(&arguments->condOrders[0], arguments->condMutex, &wait_time)) {

                        pthread_mutex_unlock(arguments->condMutex);
                        //*arguments->break_flag = true;


                        if (!arguments->p->getMutexMap()[i].empty()) {
                            for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                                int index = arguments->p->getMutexMap()[i][j];
                                (*arguments->actively_using_p)[index] = UNLOCKED;
                                pthread_mutex_unlock(&arguments->mutexes[index]);
                            }
                        }

                        if (!arguments->p->getSmokeMap()[i].empty()) {
                            for (int j = 0; j < arguments->p->getSmokeMap()[i].size(); ++j) {
                                int index = arguments->p->getSmokeMap()[i][j];
                                (*arguments->actively_using_s)[index] = UNLOCKED;
                                pthread_mutex_unlock(&arguments->smoke_mutexes[index]);
                            }
                        }

                        if (*arguments->flag) {
                            hw2_notify(PROPER_PRIVATE_STOPPED, gid, 0, 0);
                            pthread_exit(nullptr);
                        }

                        if (*arguments->break_flag) {
                            hw2_notify(PROPER_PRIVATE_TOOK_BREAK, gid, 0, 0);
                        }

                        pthread_mutex_lock(arguments->obey_mutex);
                        if (++(*arguments->obeyed_privates) == arguments->private_count) {
                            pthread_cond_signal(arguments->obey);
                            *arguments->obeyed_privates = 0;
                        }
                        pthread_mutex_unlock(arguments->obey_mutex);

                        pthread_mutex_lock(arguments->condMutex);
                        pthread_cond_wait(&arguments->condOrders[1], arguments->condMutex);
                        pthread_mutex_unlock(arguments->condMutex);

                        if (*arguments->flag) {
                            hw2_notify(PROPER_PRIVATE_STOPPED, gid, 0, 0);
                            pthread_exit(nullptr);
                        }

                        hw2_notify(PROPER_PRIVATE_CONTINUED, gid, 0, 0);

                        //*arguments->break_flag = false;

                        // LOCK OUT OTHER PROPER PRIVATES IN THE AREA
                        if (!arguments->p->getMutexMap()[i].empty()) {
                            for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                                int index = arguments->p->getMutexMap()[i][j];
                                if ((*arguments->actively_using_p)[index] != PENDING) {
                                    pthread_mutex_lock(&arguments->mutexes[index]);
                                    (*arguments->actively_using_p)[index] = PENDING;
                                }
                            }
                        }

                        // LOCK OUT SMOKERS IN THE AREA
                        if (!arguments->p->getSmokeMap()[i].empty()) {
                            for (int j = 0; j < arguments->p->getSmokeMap()[i].size(); ++j) {
                                int index = arguments->p->getSmokeMap()[i][j];
                                if ((*arguments->actively_using_s)[index] != PENDING) {
                                    pthread_mutex_lock(&arguments->smoke_mutexes[index]);
                                    (*arguments->actively_using_s)[index] = PENDING;
                                }
                            }
                        }

                        if (!arguments->p->getMutexMap()[i].empty()) {
                            for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                                int index = arguments->p->getMutexMap()[i][j];
                                if ((*arguments->actively_using_p)[index] == ACTIVE) {
                                    pthread_mutex_lock(&arguments->mutexes[index]);
                                }
                                (*arguments->actively_using_p)[index] = ACTIVE;
                            }
                        }

                        if (!arguments->p->getSmokeMap()[i].empty()) {
                            for (int j = 0; j < arguments->p->getSmokeMap()[i].size(); ++j) {
                                int index = arguments->p->getSmokeMap()[i][j];
                                if ((*arguments->actively_using_s)[index] == ACTIVE) {
                                    pthread_mutex_lock(&arguments->smoke_mutexes[index]);
                                }
                                (*arguments->actively_using_s)[index] = ACTIVE;
                            }
                        }

                        j = i_index;
                        k = j_index;

                        arrive = true;
                    }
                    pthread_mutex_unlock(arguments->condMutex);

                    if (!*arguments->break_flag && !*arguments->flag && arrive) {
                        arrive = false;
                        hw2_notify(PROPER_PRIVATE_ARRIVED, gid, arguments->p->getAreas()[i].first,
                                   arguments->p->getAreas()[i].second);
                    }

                    if ((*arguments->grid)[j][k] > 0 && !(*arguments->flag)) {
                        (*arguments->grid)[j][k]--;
                        hw2_notify(PROPER_PRIVATE_GATHERED, gid, j, k);
                    }

                    clock_gettime(CLOCK_REALTIME, &wait_time);
                    wait_time.tv_sec += time_t(delay / 1000);

                    if (wait_time.tv_nsec + (delay % 1000) * 1000000 > 999999999) {
                        wait_time.tv_sec++;
                        wait_time.tv_nsec = wait_time.tv_nsec + (delay % 1000) * 1000000 - 999999999;
                    } else {
                        wait_time.tv_nsec += (delay % 1000) * 1000000;
                    }
                }
                k++;
            }
            j++;
            k = j_index;
        }

        // FINISHED THE CURRENT AREA
        hw2_notify(PROPER_PRIVATE_CLEARED, gid, 0, 0);

        // UNLOCK THE AREA FOR PROPER PRIVATES AND SMOKERS
        if (!arguments->p->getMutexMap()[i].empty()) {
            for (int j = 0; j < arguments->p->getMutexMap()[i].size(); ++j) {
                int index = arguments->p->getMutexMap()[i][j];
                (*arguments->actively_using_p)[index] = UNLOCKED;
                pthread_mutex_unlock(&arguments->mutexes[index]);
            }
        }

        if (!arguments->p->getSmokeMap()[i].empty()) {
            for (int j = 0; j < arguments->p->getSmokeMap()[i].size(); ++j) {
                int index = arguments->p->getSmokeMap()[i][j];
                (*arguments->actively_using_s)[index] = UNLOCKED;
                pthread_mutex_unlock(&arguments->smoke_mutexes[index]);
            }
        }
        // -----------------------------------------------
    }

    // DONE
    hw2_notify(PROPER_PRIVATE_EXITED, gid, 0, 0);

    delete arguments;

}

// THREAD ROUTINE FOR COMMANDER THAT GIVES ORDERS
void *giveOrder(void* args){

    auto* orders = (order_args*) args;

    unsigned int timeSinceLastBreak = 0;    // For measuring time between orders
    string lastOrder = "nothing";

    for (const auto& i: orders->orders) {

        unsigned int sleep_time = i.first - timeSinceLastBreak;
        timeSinceLastBreak = i.first;
        usleep(sleep_time*1000);    // Wait for each order

        if (i.second == "break"){

            hw2_notify(ORDER_BREAK,0,0,0);

            if(lastOrder != "break" && lastOrder != "stop") {

                *orders->break_flag = true;
                for (int j = 0; j < orders->overlap_count1; ++j) {

                    (*orders->actively_using)[j] = HELD_BY_COMMANDER;
                    pthread_mutex_unlock(&orders->mutexes[j]);
                }


                pthread_cond_broadcast(&orders->condOrders[0]);
                pthread_mutex_t temp_mutex = PTHREAD_MUTEX_INITIALIZER;
                pthread_mutex_lock(&temp_mutex);
                pthread_cond_wait(orders->obey, &temp_mutex);
                pthread_mutex_unlock(&temp_mutex);

                for (int j = 0; j < orders->overlap_count1; ++j) {

                    (*orders->actively_using)[j] = UNLOCKED;
                }

                *orders->order_obeyed = true;

                if(orders->overlap_count1 != 0){
                    pthread_cond_broadcast(orders->go_smokers);

                    pthread_mutex_lock(&temp_mutex);
                    pthread_cond_wait(orders->smoker_ok, &temp_mutex);
                    pthread_mutex_unlock(&temp_mutex);
                }
                *orders->order_obeyed = false;

                pthread_mutex_destroy(&temp_mutex);
            }

            if(lastOrder != "stop") lastOrder = "break";
        }

        else if (i.second == "continue"){
            hw2_notify(ORDER_CONTINUE,0,0,0);
            *orders->break_flag = false;
            pthread_cond_broadcast(&orders->condOrders[1]);

            if(lastOrder != "stop") lastOrder = "continue";
        }

        else{   // Stop
            hw2_notify(ORDER_STOP,0,0,0);
            *orders->flag = true;
            pthread_cond_broadcast(&orders->condOrders[0]);
            pthread_cond_broadcast(&orders->condOrders[1]);
            pthread_cond_broadcast(&orders->condOrders[2]);

            lastOrder = "stop";
        }


    }

    delete orders;
}

void* smokeAndLitter(void* args){

    auto* arguments = (smoker_args*) args;
    int sid = arguments->s->getSid();

    // DELAY ADJUSTMENTS
    struct timespec wait_time{};
    long delay = arguments->s->getSmokeTime();
    clock_gettime(CLOCK_REALTIME,&wait_time);
    wait_time.tv_sec += time_t(delay/1000);

    if(wait_time.tv_nsec + (delay%1000)*1000000 > 999999999){
        wait_time.tv_sec++;
        wait_time.tv_nsec = wait_time.tv_nsec + (delay%1000)*1000000 - 999999999;
    }
    else{
        wait_time.tv_nsec += (delay%1000)*1000000;
    }
    // ----------------------------------------------------

    hw2_notify(SNEAKY_SMOKER_CREATED,sid,0,0);

    for (int i = 0; i < arguments->s->getCellCount(); ++i) {
        int current_cell_i = arguments->s->getCells()[i].first;
        int current_cell_j = arguments->s->getCells()[i].second;

        pthread_mutex_lock(&arguments->grid_for_smokers[current_cell_i][current_cell_j]);

        if (*arguments->flag){

            pthread_mutex_unlock(arguments->condMutex);

            if (!arguments->s->getMutexMap()[i].empty()){
                for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                    int index = arguments->s->getMutexMap()[i][j];
                    (*arguments->actively_using)[index] = UNLOCKED;
                    pthread_mutex_unlock(&arguments->mutexes[index]);
                }
            }

            pthread_mutex_unlock(&arguments->grid_for_smokers[current_cell_i][current_cell_j]);

            hw2_notify(SNEAKY_SMOKER_STOPPED,sid,0,0);
            pthread_exit(nullptr);
        }

        // LOCK OUT PROPER PRIVATES
        if (!arguments->s->getMutexMap()[i].empty()){
            for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                int index = arguments->s->getMutexMap()[i][j];
                if((*arguments->actively_using)[index] == HELD_BY_COMMANDER){
                    //cerr << arguments->s->getSid() << "----" << "HELD" << "----" << index << endl;
                    pthread_mutex_t temp = PTHREAD_MUTEX_INITIALIZER;
                    pthread_mutex_lock(&temp);
                    pthread_cond_wait(arguments->go_smokers,&temp);
                    pthread_mutex_unlock(&temp);
                    pthread_mutex_destroy(&temp);
                }

                if((*arguments->actively_using)[index] != PENDING) {
                    //cerr << arguments->s->getSid() << "----" << "NOT PENDING" << "----" << index << endl;
                    pthread_mutex_lock(&arguments->mutexes[index]);
                    (*arguments->actively_using)[index] = PENDING;
                }
            }
        }

        if (!arguments->s->getMutexMap()[i].empty()){
            for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                int index = arguments->s->getMutexMap()[i][j];

                /*
                if((*arguments->actively_using)[index] == ACTIVE){
                    pthread_mutex_lock(&arguments->mutexes[index]);
                }
                 */

                if((*arguments->actively_using)[index] == UNLOCKED){
                    pthread_mutex_lock(&arguments->mutexes[index]);
                }

                (*arguments->actively_using)[index] = ACTIVE;
            }
        }



        if(!*arguments->flag) {
            hw2_notify(SNEAKY_SMOKER_ARRIVED, sid, current_cell_i, current_cell_j);
        }

        // WAIT FOR SOME TIME BEFORE SMOKING (IF A STOP ORDER COMES, HANDLE..)
        pthread_mutex_lock(arguments->condMutex);
        if (*arguments->flag || !pthread_cond_timedwait(&arguments->condOrders[2],arguments->condMutex,&wait_time)){

            pthread_mutex_unlock(arguments->condMutex);

            if (!arguments->s->getMutexMap()[i].empty()){
                for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                    int index = arguments->s->getMutexMap()[i][j];
                    (*arguments->actively_using)[index] = UNLOCKED;
                    pthread_mutex_unlock(&arguments->mutexes[index]);
                }
            }

            pthread_mutex_unlock(&arguments->grid_for_smokers[current_cell_i][current_cell_j]);

            hw2_notify(SNEAKY_SMOKER_STOPPED,sid,0,0);
            pthread_exit(nullptr);
        }
        pthread_mutex_unlock(arguments->condMutex);
        // ------------------------------------------------------------------------------------------------------------

        int smoke_count = arguments->s->getSmokeCount(i);
        enum flick_direction{TOP_LEFT,TOP,TOP_RIGHT,RIGHT,BOTTOM_RIGHT,BOTTOM,BOTTOM_LEFT,LEFT};
        flick_direction cur = TOP_LEFT;     // Enumeration for clear switch-case implementation

        // SMOKE
        while(smoke_count != 0){
            switch (cur) {
                case TOP_LEFT:
                    cur = TOP;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i-1][current_cell_j-1]);
                    (*arguments->grid)[current_cell_i-1][current_cell_j-1]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i-1][current_cell_j-1]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i-1,current_cell_j-1);
                    break;

                case TOP:
                    cur = TOP_RIGHT;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i-1][current_cell_j]);
                    (*arguments->grid)[current_cell_i-1][current_cell_j]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i-1][current_cell_j]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i-1,current_cell_j);
                     break;

                case TOP_RIGHT:
                    cur = RIGHT;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i-1][current_cell_j+1]);
                    (*arguments->grid)[current_cell_i-1][current_cell_j+1]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i-1][current_cell_j+1]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i-1,current_cell_j+1);
                    break;

                case RIGHT:
                    cur = BOTTOM_RIGHT;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i][current_cell_j+1]);
                    (*arguments->grid)[current_cell_i][current_cell_j+1]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i][current_cell_j+1]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i,current_cell_j+1);
                    break;

                case BOTTOM_RIGHT:
                    cur = BOTTOM;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i+1][current_cell_j+1]);
                    (*arguments->grid)[current_cell_i+1][current_cell_j+1]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i+1][current_cell_j+1]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i+1,current_cell_j+1);
                    break;

                case BOTTOM:
                    cur = BOTTOM_LEFT;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i+1][current_cell_j]);
                    (*arguments->grid)[current_cell_i+1][current_cell_j]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i+1][current_cell_j]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i+1,current_cell_j);
                    break;

                case BOTTOM_LEFT:
                    cur = LEFT;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i+1][current_cell_j-1]);
                    (*arguments->grid)[current_cell_i+1][current_cell_j-1]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i+1][current_cell_j-1]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i+1,current_cell_j-1);
                    break;

                case LEFT:
                    cur = TOP_LEFT;

                    // CRITICAL!
                    pthread_mutex_lock(&arguments->grid_mutexes[current_cell_i][current_cell_j-1]);
                    (*arguments->grid)[current_cell_i][current_cell_j-1]++;
                    pthread_mutex_unlock(&arguments->grid_mutexes[current_cell_i][current_cell_j-1]);
                    // CRITICAL!

                    hw2_notify(SNEAKY_SMOKER_FLICKED,sid,current_cell_i,current_cell_j-1);
                    break;
            }

            smoke_count--;

            clock_gettime(CLOCK_REALTIME,&wait_time);

            wait_time.tv_sec += time_t(delay/1000);

            if(wait_time.tv_nsec + (delay%1000)*1000000 > 999999999){
                wait_time.tv_sec++;
                wait_time.tv_nsec = wait_time.tv_nsec + (delay%1000)*1000000 - 999999999;
            }
            else{
                wait_time.tv_nsec += (delay%1000)*1000000;
            }

            if(*arguments->order_obeyed){

                if (!arguments->s->getMutexMap()[i].empty()){
                    for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                        int index = arguments->s->getMutexMap()[i][j];
                        if((*arguments->actively_using)[index] == UNLOCKED) {
                            pthread_mutex_lock(&arguments->mutexes[index]);
                            (*arguments->actively_using)[index] = ACTIVE;
                        }
                    }
                }

                /*
                if (!arguments->s->getMutexMap()[i].empty()){
                    for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                        int index = arguments->s->getMutexMap()[i][j];
                        if((*arguments->actively_using)[index] == ACTIVE){
                            pthread_mutex_lock(&arguments->mutexes[index]);
                        }
                        (*arguments->actively_using)[index] = ACTIVE;
                    }
                }
                 */

                if(++(*arguments->initial) == arguments->smoker_count){
                    *arguments->initial = 0;
                    pthread_cond_signal(arguments->smoker_ok);
                }
            }

            pthread_mutex_lock(arguments->condMutex);
            if (*arguments->flag ||!pthread_cond_timedwait(&arguments->condOrders[2],arguments->condMutex,&wait_time)){

                pthread_mutex_unlock(arguments->condMutex);

                if (!arguments->s->getMutexMap()[i].empty()){
                    for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                        int index = arguments->s->getMutexMap()[i][j];
                        (*arguments->actively_using)[index] = UNLOCKED;
                        pthread_mutex_unlock(&arguments->mutexes[index]);
                    }
                }

                pthread_mutex_unlock(&arguments->grid_for_smokers[current_cell_i][current_cell_j]);

                hw2_notify(SNEAKY_SMOKER_STOPPED,sid,0,0);
                pthread_exit(nullptr);
            }
            pthread_mutex_unlock(arguments->condMutex);
        }

        // SMOKING IS FINISHED AT THE CURRENT AREA
        hw2_notify(SNEAKY_SMOKER_LEFT,sid,0,0);

        // UNLOCK
        if (!arguments->s->getMutexMap()[i].empty()){
            for (int j = 0; j < arguments->s->getMutexMap()[i].size(); ++j) {
                int index = arguments->s->getMutexMap()[i][j];
                (*arguments->actively_using)[index] = UNLOCKED;
                pthread_mutex_unlock(&arguments->mutexes[index]);
            }
        }

        pthread_mutex_unlock(&arguments->grid_for_smokers[current_cell_i][current_cell_j]);
    }
    hw2_notify(SNEAKY_SMOKER_EXITED,sid,0,0);

    delete arguments;
}
