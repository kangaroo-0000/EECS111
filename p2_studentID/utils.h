#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "globals.h"
#include "types_p2.h"

#define MSEC(x) x * 1000

long get_elasped_time(const struct timeval& start, const struct timeval& end);
bool all_of_gender(std::vector<Person*>& current_people, Gender gender);
Gender generate_random_gender();

#endif
