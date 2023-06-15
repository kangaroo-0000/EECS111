#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

class Restroom;
class Person;
struct threadArgs {
  Restroom* restroom;
  int num_of_people_to_generate_per_gender;
};
typedef enum { MAN, WOMAN } Gender;
typedef enum { EMPTY, WOMEN_PRESENT, MEN_PRESENT } RestroomStatus;
extern pthread_mutex_t restroom_mutex;
extern pthread_cond_t restroom_cond;
extern struct timeval program_start_time;
extern bool isFinishedGeneratingPeople;

#endif  // GLOBALS_H