#include "utils.h"

long get_elasped_time(const struct timeval& start, const struct timeval& end) {
  long mtime = 0;
  long seconds = end.tv_sec - start.tv_sec;
  long useconds = end.tv_usec - start.tv_usec;

  mtime = ((seconds)*1000 + useconds / 1000.0) + 0.5;
  return mtime;
}
Gender generate_random_gender() { return static_cast<Gender>(rand() % 2); }

bool all_of_gender(std::vector<Person*>& current_people, Gender gender) {
  for (std::vector<Person*>::iterator it = current_people.begin();
       it != current_people.end(); ++it) {
    if ((*it)->get_gender() != gender) {
      return false;
    }
  }
  return true;
}
