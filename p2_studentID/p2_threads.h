#ifndef __P2_THREADS_H
#define __P2_THREADS_H

#include <algorithm>
#include <vector>

#include "globals.h"
#include "types_p2.h"
#include "utils.h"

void *queue_manager(void *arg);
void *restroom_manager(void *arg);
void *restroom_status_recorder(void *arg);
void woman_wants_to_enter(Person *p, Restroom &restroom);
void man_wants_to_enter(Person *p, Restroom &restroom);
void woman_leaves(Person *p, Restroom &restroom);
void man_leaves(Person *p, Restroom &restroom);

#endif
