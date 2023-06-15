#include "p1_process.h"

#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "p1_threads.h"

using namespace std;

// This file implements the multi-processing logic for the project

// This function should be called in each child process right after forking
// The input vector should be a subset of the original files vector
void process_classes(vector<string> classes, int num_threads) {
  printf("Child process is created. (pid: %d)\n", getpid());
  // Each process should use the sort function which you have defined
  // in the p1_threads.cpp for multithread sorting of the data.
  for (int i = 0; i < classes.size(); i++) {
    // get all the input/output file names here
    string class_name = classes[i];
    char buffer[40];
    sprintf(buffer, "input/%s.csv", class_name.c_str());
    string input_file_name(buffer);

    sprintf(buffer, "output/%s_sorted.csv", class_name.c_str());
    string output_sorted_file_name(buffer);

    sprintf(buffer, "output/%s_stats.csv", class_name.c_str());
    string output_stats_file_name(buffer);

    vector<student> students;

    // Your implementation goes here, you will need to implement:
    // File I/O
    //  - This means reading the input file, and creating a list of students,
    //  see p1_process.h for the definition of the student struct
    //
    //  - Also, once the sorting is done and the statistics are generated, this
    //  means creating the appropritate output files
    //
    // Multithreaded Sorting
    //  - See p1_thread.cpp and p1_thread.h
    //
    //  - The code to run the sorter has already been provided
    //
    // Generating Statistics
    //  - This can be done after sorting or during

    // Run multi threaded sort
    // read csv file
    FILE *fp = fopen(input_file_name.c_str(), "r");
    if (fp == NULL) {
      printf("Error: cannot open file %s\n", input_file_name.c_str());
      exit(1);
    }
    char line[100];
    fgets(line, 100, fp);
    while (fgets(line, 100, fp) != NULL) {
      char *token = strtok(line, ",");
      student *s;
      s->id = strtoul(token, NULL, 10);
      token = strtok(NULL, ",");
      s->grade = atof(token);
      students.push_back(*s);
    }
    ParallelMergeSorter sorter(students, num_threads);
    vector<student> sorted = sorter.run_sort();
    // print vector sorted to output csv
    FILE *fp_sorted = fopen(output_sorted_file_name.c_str(), "w");
    if (fp_sorted == NULL) {
      printf("Error: cannot open file %s\n", output_sorted_file_name.c_str());
      exit(1);
    }
    fprintf(fp_sorted, "Rank,Student ID,Grade\n");
    for (int i = 0; i < sorted.size(); i++) {
      fprintf(fp_sorted, "%d,%lu,%.10f\n", i + 1, sorted[i].id,
              sorted[i].grade);
    }
    fclose(fp_sorted);
    // print statistics to output csv
    FILE *fp_stats = fopen(output_stats_file_name.c_str(), "w");
    if (fp_stats == NULL) {
      printf("Error: cannot open file %s\n", output_stats_file_name.c_str());
      exit(1);
    }
    // calculate and print mean, median, and standard deviation
    double mean = 0;
    double median = 0;
    double std_dev = 0;
    for (int i = 0; i < sorted.size(); i++) {
      mean += sorted[i].grade;
    }
    mean /= sorted.size();
    for (int i = 0; i < sorted.size(); i++) {
      std_dev += pow(sorted[i].grade - mean, 2);
    }
    std_dev /= sorted.size();
    std_dev = sqrt(std_dev);
    if (sorted.size() % 2 == 0) {
      median = (sorted[sorted.size() / 2].grade +
                sorted[sorted.size() / 2 - 1].grade) /
               2;
    } else {
      median = sorted[sorted.size() / 2].grade;
    }
    fprintf(fp_stats, "Average,Median,Std. Dev\n");
    fprintf(fp_stats, "%2.3f,%2.3f,%2.3f\n", mean, median, std_dev);
    fclose(fp_stats);
  }

  // child process done, exit the program
  printf("Child process is terminated. (pid: %d)\n", getpid());
  exit(0);
}

void create_processes_and_sort(vector<string> class_names, int num_processes,
                               int num_threads) {
  vector<pid_t> child_pids;
  int classes_per_process = max(class_names.size() / num_processes, 1ul);

  // This code is provided to you to test your sorter, this code does not use
  // any child processes Remove this later on
  // process_classes(class_names, 1);

  // Your implementation goes here, you will need to implement:
  // Splitting up work
  //   - Split up the input vector of classes into num_processes sublists
  //
  //   - Make sure all classes are included, remember integer division rounds
  //   down
  //
  // Creating child processes
  //   - Each child process will handle one of the sublists from above via
  //   process_classes
  //
  // Waiting for all child processes to terminate

  for (int i = 0; i < class_names.size(); i++) {
    vector<string> classes;
    for (int j = i * classes_per_process;
         j < min((i + 1) * classes_per_process, (int)class_names.size()); j++) {
      classes.push_back(class_names[j]);
    }
    pid_t pid = fork();
    if (pid == 0) {  // child process
      process_classes(classes, num_threads);

    } else {
      child_pids.push_back(pid);
    }
  }
  // wait for all child processes to terminate
  for (int i = 0; i < child_pids.size(); i++) {
    int status;
    waitpid(child_pids[i], &status, 0);
    if (!WIFEXITED(status)) {
      // Child process exited with a non-zero exit status
      printf("Child process encountered an exception.\n");
    }
  }
}
