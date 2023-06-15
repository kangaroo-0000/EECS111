#include "types_p2.h"

/* Person class functions*/
void Person::set_gender(Gender gender) { this->gender = gender; }
Gender Person::get_gender(void) { return gender; }

void Person::set_time(long data) { this->time_to_stay_ms = data; }
long Person::get_time(void) { return time_to_stay_ms / 1000; }
int Person::ready_to_leave(void) {
  struct timeval t_curr;
  gettimeofday(&t_curr, NULL);

  if (get_elasped_time(t_start, t_curr) >= time_to_stay_ms) {
    return 1;
  } else {
    return 0;
  }
}

Person::Person() {
  time_to_stay_ms = 0;
  gender = MAN;
  gettimeofday(&t_create, NULL);
}

/*Restroom class functions*/
Restroom::Restroom() {
  status = EMPTY;
  menCount = 0;
  womenCount = 0;
}

void Restroom::addPersonToQueue(Person* person) {
  restroom_queue.push(person);
  if (person->get_gender() == MAN)
    menCount++;
  else
    womenCount++;
}

Person* Restroom::getNextPersonFromQueue() {
  if (!restroom_queue.empty()) {
    Person* person = restroom_queue.front();
    restroom_queue.pop();
    if (person->get_gender() == MAN)
      menCount--;
    else
      womenCount--;
    return person;
  }
  return NULL;
}

int Restroom::getQueueMenCount() { return menCount; }
int Restroom::getQueueWomenCount() { return womenCount; }

int Restroom::getQueueSize() { return restroom_queue.size(); }

RestroomStatus Restroom::getStatus() { return status; }

std::string Restroom::getStatusString() {
  if (status == MEN_PRESENT) return "MenPresent";
  if (status == WOMEN_PRESENT) return "WomenPresent";
  if (status == EMPTY) return "Empty";
  return "Unknown";
}

void Restroom::setStatus(RestroomStatus status) { this->status = status; }

bool Restroom::isEmpty() { return restroom_queue.empty(); }

std::vector<Person*>& Restroom::getCurrentRestroomUsers() {
  return current_restroom_users;
}

void Restroom::printQueue() {
  // print the restroom queue. debug use.
  std::queue<Person*> temp = restroom_queue;
  while (!temp.empty()) {
    Person* p = temp.front();
    temp.pop();
    std::cout << "Person gender: " << p->get_gender() << std::endl;
  }
}
