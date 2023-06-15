#include "globals.h"

pthread_cond_t restroom_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t restroom_mutex = PTHREAD_MUTEX_INITIALIZER;
timeval program_start_time;
bool isFinishedGeneratingPeople = false;