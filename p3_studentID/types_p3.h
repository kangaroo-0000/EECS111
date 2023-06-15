#ifndef __TYPES_P3_H
#define __TYPES_P3_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

class ThreadCtrlBlk {
 private:
 public:
  int id;
  long period;
  long task_time;
  long deadline;
};

#endif
