#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "p3_threads.h"
#include "types_p3.h"
#include "utils.h"

// pthread conditional variable to start/resume the thread
pthread_cond_t resume[4];

// pthread conditional variable to wait for threads to finish init
pthread_cond_t init[4];

// pthread conditional variable to signal when the thread's task is done
pthread_cond_t a_task_is_done;

// pthread conditional variable to allow the scheduler to wait for a thread to
// preempt
pthread_cond_t preempt_task;

// tcbs for each thread
ThreadCtrlBlk tcb[4];

// ready queue of threads
std::vector<int> ready_queue;

// number of tasks that did not miss deadline
int num_of_alive_tasks = 4;

// -1 = no thread working, <number> = thread <number> currently working
int running_thread = -1;

// 0 = don't preempt, 1 = preempt current running thread
int preempt = 0;

// mutex used to protect variables defined in this file
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// mutex used for the task done pthread conditional variable
pthread_mutex_t taskDoneMutex = PTHREAD_MUTEX_INITIALIZER;

// marks the "start time"
struct timeval t_global_start;

// used to tell threads when to stop working (after 240 iterations)
int global_work = 0;

void fifo_schedule(void);
void edf_schedule(void);
void rm_schedule(void);
bool compare_by_deadline(int a, int b);
bool compare_by_period(int a, int b);
void schedule_next_task(void);
void wait_for_task_completion(void);

int main(int argc, char** argv) {
  if (argc != 2 || atoi(argv[1]) < 0 || atoi(argv[1]) > 2) {
    std::cout << "[ERROR] Expecting 1 argument, but got " << argc - 1
              << std::endl;
    std::cout << "[USAGE] p3_exec <0, 1, or 2>" << std::endl;
    return 0;
  }
  int schedule = atoi(argv[1]);

  // pthreads we are creating
  pthread_t tid[4];

  // This is to set the global start time
  gettimeofday(&t_global_start, NULL);

  // initialize all tcbs
  tcb[0].id = 0;
  tcb[0].task_time = 200;
  tcb[0].period = 1000;
  tcb[0].deadline = 1000;

  tcb[1].id = 1;
  tcb[1].task_time = 500;
  tcb[1].period = 2000;
  tcb[1].deadline = 2000;

  tcb[2].id = 2;
  tcb[2].task_time = 1000;
  tcb[2].period = 4000;
  tcb[2].deadline = 4000;

  tcb[3].id = 3;
  tcb[3].task_time = 1000;
  tcb[3].period = 6000;
  tcb[3].deadline = 6000;

  // initialize all pthread conditional variables
  for (int i = 0; i < 4; i++) {
    pthread_cond_init(&resume[i], NULL);
    pthread_cond_init(&init[i], NULL);
  }
  pthread_cond_init(&a_task_is_done, NULL);
  pthread_cond_init(&preempt_task, NULL);

  // allow all threads to work
  global_work = 1;
  printf("[Main] Create worker threads\n");

  pthread_mutex_lock(&mutex);
  for (int i = 0; i < 4; i++) {
    if (pthread_create(&tid[i], NULL, threadfunc, &tcb[i])) {
      fprintf(stderr, "Error creating thread\n");
    }
    // Wait until the threads are "ready" and in the ready queue
    pthread_cond_wait(&init[i], &mutex);
  }
  pthread_mutex_unlock(&mutex);

  // Reset the global time and skip the initial wait
  gettimeofday(&t_global_start, NULL);

  for (int i = 0; i < 20; i++) {
    // Select scheduler based on argv[1]
    switch (schedule) {
      case 0:
        fifo_schedule();
        break;
      case 1:
        edf_schedule();
        break;
      case 2:
        rm_schedule();
        break;
    }

    // Wait until the next 100ms interval or until a task is done
    int sleep = 100 - (get_time_stamp() % 100);
    if (num_of_alive_tasks > 0) {
      timed_wait_for_task_complition(sleep);
    } else {
      printf("All the tasks missed the deadline\n");
      break;
    }
  }

  // after 240 iterations, finish off all threads
  printf("[Main] It's time to finish the threads\n");

  printf("[Main] Locks\n");
  pthread_mutex_lock(&mutex);
  global_work = 0;

  // signal all the processes in the ready queue so they finish
  usleep(MSEC(3000));
  while (ready_queue.size() > 0) {
    pthread_cond_signal(&resume[ready_queue[0]]);
    ready_queue.erase(ready_queue.begin());
  }

  printf("[Main] Unlocks\n");
  pthread_mutex_unlock(&mutex);

  /* wait for the threads to finish */
  for (int i = 0; i < 4; i++) {
    if (pthread_join(tid[i], NULL)) {
      fprintf(stderr, "Error joining thread\n");
    }
  }

  return 0;
}

void schedule_next_task() {
  while (true) {
    // If there is a thread in the ready queue, schedule it
    if (ready_queue.size() > 0) {
      running_thread = ready_queue[0];
      pthread_cond_signal(&(resume[ready_queue[0]]));
      ready_queue.erase(ready_queue.begin());
      break;
    } else {
      // wait until a thread is ready
      pthread_mutex_unlock(&mutex);
      usleep(30);
    }
  }
}

void wait_for_task_completion() {
  pthread_mutex_lock(&taskDoneMutex);
  pthread_cond_wait(&a_task_is_done, &taskDoneMutex);
  pthread_mutex_unlock(&taskDoneMutex);
}

void fifo_schedule() {
  pthread_mutex_lock(&mutex);
  if (running_thread == -1) schedule_next_task();
  pthread_mutex_unlock(&mutex);
  wait_for_task_completion();
}

void edf_schedule(void) {
  pthread_mutex_lock(&mutex);
  if (running_thread == -1) schedule_next_task();
  std::sort(ready_queue.begin(), ready_queue.end(), compare_by_deadline);

  // If there is no running thread, or if the next task has an earlier
  // deadline
  if (running_thread == -1 ||
      tcb[ready_queue[0]].deadline < tcb[running_thread].deadline) {
    // If there is a running thread, preempt it
    if (running_thread != -1) {
      preempt = 1;
      pthread_cond_wait(&preempt_task, &mutex);
    }
    schedule_next_task();
  }
  pthread_mutex_unlock(&mutex);
  wait_for_task_completion();
}

void rm_schedule(void) {
  pthread_mutex_lock(&mutex);
  // If the task queue is not empty
  if (!ready_queue.empty()) {
    std::sort(ready_queue.begin(), ready_queue.end(), compare_by_period);
    if (running_thread != -1 &&
        tcb[running_thread].period > tcb[ready_queue[0]].period) {
      // Preempt the running task
      preempt = 1;
      pthread_cond_wait(&preempt_task, &mutex);
    }

    // Schedule the first task in the ready queue if the queue is not empty
    if (!ready_queue.empty()) {
      running_thread = ready_queue[0];
      pthread_cond_signal(&resume[running_thread]);
      ready_queue.erase(ready_queue.begin());
    }
  }
  pthread_mutex_unlock(&mutex);
  if (running_thread != -1) wait_for_task_completion();
}

bool compare_by_deadline(int a, int b) {
  return tcb[a].deadline < tcb[b].deadline;
}

bool compare_by_period(int a, int b) { return tcb[a].period < tcb[b].period; }
