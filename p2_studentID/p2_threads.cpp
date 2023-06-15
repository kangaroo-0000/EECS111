#include "p2_threads.h"

#include <assert.h>

void woman_wants_to_enter(Person* p, Restroom& restroom) {
  std::vector<Person*>& current_people = restroom.getCurrentRestroomUsers();
  // monitor
  while (!current_people.empty() && !all_of_gender(current_people, WOMAN)) {
    pthread_cond_wait(&restroom_cond, &restroom_mutex);
  }
  restroom.setStatus(WOMEN_PRESENT);
  current_people.push_back(p);
  struct timeval current_thread_time;
  gettimeofday(&current_thread_time, NULL);
  std::cout
      << "[" << get_elasped_time(program_start_time, current_thread_time)
      << " ms] [Restroom Entry Manager] Send (Woman) into the restroom (Stay "
      << p->get_time() << "ms), State is" << restroom.getStatusString()
      << "Status: Total (number of people in queue): "
      << restroom.getQueueSize() << "(Men: " << restroom.getQueueMenCount()
      << ", Women: " << restroom.getQueueWomenCount() << ")" << std::endl;

  return;
}

void man_wants_to_enter(Person* p, Restroom& restroom) {
  std::vector<Person*>& current_people = restroom.getCurrentRestroomUsers();
  // monitor
  while (!current_people.empty() && !all_of_gender(current_people, MAN)) {
    pthread_cond_wait(&restroom_cond, &restroom_mutex);
  }
  restroom.setStatus(MEN_PRESENT);
  current_people.push_back(p);
  struct timeval current_thread_time;
  gettimeofday(&current_thread_time, NULL);
  std::cout
      << "[" << get_elasped_time(program_start_time, current_thread_time)
      << " ms] [Restroom Entry Manager] Send (Man) into the restroom (Stay "
      << p->get_time() << "ms), State is" << restroom.getStatusString()
      << "Status: Total (number of people in queue): "
      << restroom.getQueueSize() << "(Men: " << restroom.getQueueMenCount()
      << ", Women: " << restroom.getQueueWomenCount() << ")" << std::endl;

  return;
}

void woman_leaves(Person* p, Restroom& restroom) {
  struct timeval current_thread_time;
  gettimeofday(&current_thread_time, NULL);
  std::cout << "[" << get_elasped_time(program_start_time, current_thread_time)
            << " ms][Restroom Exit Manager] Woman leaves" << std::endl;
  return;
}

void man_leaves(Person* p, Restroom& restroom) {
  struct timeval current_thread_time;
  gettimeofday(&current_thread_time, NULL);
  std::cout << "[" << get_elasped_time(program_start_time, current_thread_time)
            << " ms][Restroom Exit Manager] Man leaves" << std::endl;
  return;
}

void* queue_manager(void* arg) {
  std::cout << "In queue manager" << std::endl;
  // argument parsing
  threadArgs* args = static_cast<threadArgs*>(arg);
  Restroom& restroom = *(args->restroom);
  int num_of_people_to_generate_per_gender =
      args->num_of_people_to_generate_per_gender;
  int num_of_people_generated_per_gender[2] = {0, 0};
  while (num_of_people_generated_per_gender[0] <
             num_of_people_to_generate_per_gender ||
         num_of_people_generated_per_gender[1] <
             num_of_people_to_generate_per_gender) {
    pthread_mutex_lock(&restroom_mutex);
    Gender gender = generate_random_gender();
    if (num_of_people_generated_per_gender[gender] >=
        num_of_people_to_generate_per_gender) {
      pthread_mutex_unlock(&restroom_mutex);
      continue;
    }
    pthread_mutex_unlock(&restroom_mutex);
    long time_to_stay_ms = (rand() % 8 + 3) * 1000;
    Person* person = new Person();
    person->set_gender(gender);
    person->set_time(time_to_stay_ms);
    pthread_mutex_lock(&restroom_mutex);
    restroom.addPersonToQueue(person);
    pthread_mutex_unlock(&restroom_mutex);
    struct timeval current_thread_time;
    gettimeofday(&current_thread_time, NULL);
    std::cout << "["
              << get_elasped_time(program_start_time, current_thread_time)
              << " ms]"
              << "[Queue Manager] "
              << "A person (" << (gender == MAN ? "Man" : "Woman")
              << ") goes into the queue." << std::endl;
    num_of_people_generated_per_gender[gender]++;
    usleep(1000 * (rand() % 5 + 1));
    pthread_mutex_unlock(&restroom_mutex);
  }
  isFinishedGeneratingPeople = true;
  pthread_exit(NULL);
}

void* restroom_manager(void* arg) {
  std::cout << "In restroom entry manager" << std::endl;
  threadArgs* args = static_cast<threadArgs*>(arg);
  Restroom& restroom = *(args->restroom);
  struct timeval current_thread_time;

  while (true) {
    pthread_mutex_lock(&restroom_mutex);
    // Check for entering
    if (!restroom.isEmpty()) {
      Person* p = restroom.getNextPersonFromQueue();
      if (p->get_gender() == MAN) {
        man_wants_to_enter(p, restroom);
      } else {
        woman_wants_to_enter(p, restroom);
      }
      pthread_cond_signal(&restroom_cond);
      usleep(p->get_time() * 1000);
    }
    pthread_mutex_unlock(&restroom_mutex);

    // Check for leaving
    pthread_mutex_lock(&restroom_mutex);
    std::vector<Person*>& current_people = restroom.getCurrentRestroomUsers();
    for (std::vector<Person*>::iterator it = current_people.begin();
         it != current_people.end();) {
      Person* person = *it;
      if (person->ready_to_leave()) {
        if (person->get_gender() == WOMAN) {
          woman_leaves(person, restroom);
        } else {
          man_leaves(person, restroom);
        }
        it = current_people.erase(it);  // erase returns the next iterator
        delete person;
        pthread_cond_signal(&restroom_cond);
      } else {
        ++it;
      }
    }
    pthread_mutex_unlock(&restroom_mutex);
    usleep(1);
    // check for empty queue
    pthread_mutex_lock(&restroom_mutex);
    if (restroom.isEmpty() && isFinishedGeneratingPeople) pthread_exit(NULL);
    pthread_mutex_unlock(&restroom_mutex);
  }
  return NULL;
}

void* restroom_status_recorder(void* arg) {
  std::cout << "In restroom status recorder" << std::endl;
  threadArgs* args = static_cast<threadArgs*>(arg);
  Restroom& restroom = *(args->restroom);
  struct timeval current_thread_time;

  while (true) {
    pthread_mutex_lock(&restroom_mutex);
    // block until there is a change in the restroom status, this is a debugging
    // thread
    pthread_cond_wait(&restroom_cond, &restroom_mutex);
    if (restroom.getCurrentRestroomUsers().empty()) {
      gettimeofday(&current_thread_time, NULL);
      std::cout << "["
                << get_elasped_time(program_start_time, current_thread_time)
                << " ms][Restroom Status Recorder] Last person has left the "
                   "restroom, "
                   "State is (Empty): Total (number of people in queue): "
                << restroom.getQueueSize()
                << "(Men: " << restroom.getQueueMenCount()
                << ", Women: " << restroom.getQueueWomenCount() << ")"
                << std::endl;
      if (restroom.isEmpty() && isFinishedGeneratingPeople) pthread_exit(NULL);
    } else if (all_of_gender(restroom.getCurrentRestroomUsers(), MAN)) {
      assert(restroom.getStatus() == MEN_PRESENT);
    } else {
      assert(restroom.getStatus() == WOMEN_PRESENT);
    }
    pthread_mutex_unlock(&restroom_mutex);
  }
  return NULL;
}
