#ifndef __TYPES_P2_H
#define __TYPES_P2_H

#include <queue>
#include <string>
#include <vector>

#include "globals.h"
#include "utils.h"

class Person {
  Gender gender;
  struct timeval t_create;
  struct timeval t_start;
  struct timeval t_end;
  long time_to_stay_ms;

 public:
  Person();

  void set_gender(Gender gender);
  Gender get_gender(void);

  void set_time(long data);
  long get_time(void);
  int ready_to_leave(void);
};

class Restroom {
 private:
  int menCount;
  int womenCount;
  std::queue<Person*> restroom_queue;
  RestroomStatus status;
  std::vector<Person*> current_restroom_users;

 public:
  Restroom();
  void addPersonToQueue(Person* person);
  Person* getNextPersonFromQueue();
  int getQueueMenCount();
  int getQueueWomenCount();
  RestroomStatus getStatus();
  void setStatus(RestroomStatus status);
  std::string getStatusString();
  bool isEmpty();
  int getQueueSize();
  std::vector<Person*>& getCurrentRestroomUsers();
  void printQueue();
};

#endif
