#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "globals.h"
#include "p2_threads.h"
#include "types_p2.h"
#include "utils.h"

int main(int argc, char** argv) {
  int num_of_people = atoi(argv[1]);
  if (argc > 2) {
    std::cout << "Error: Expected one argument, but got " << argc
              << ". \nUsage: " << argv[0] << " [number of people]" << std::endl;
    exit(1);
  }
  srand(time(NULL));  // Seed the random number generator
  Restroom restroom;
  threadArgs args;
  args.restroom = &restroom;
  args.num_of_people_to_generate_per_gender = num_of_people;
  pthread_t queue_manager_thread, restroom_manager_thread,
      restroom_recorder_thread;

  // This is to set the global start time
  gettimeofday(&program_start_time, NULL);

  pthread_create(&queue_manager_thread, NULL, queue_manager, &args);
  pthread_create(&restroom_manager_thread, NULL, restroom_manager, &args);
  pthread_create(&restroom_recorder_thread, NULL, restroom_status_recorder,
                 &args);

  pthread_join(queue_manager_thread, NULL);
  pthread_join(restroom_manager_thread, NULL);
  pthread_join(restroom_recorder_thread, NULL);

  return 0;
}
