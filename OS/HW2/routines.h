#ifndef ROUTINES_H
#define ROUTINES_H

    // PROPER PRIVATE THREAD ROUTINE
    void* pickUpCigButts(void* args);

    // ORDERING THREAD ROUTINE
    void* giveOrder(void* args);

    // SMOKER THREAD ROUTINE
    void* smokeAndLitter(void* args);

#endif
