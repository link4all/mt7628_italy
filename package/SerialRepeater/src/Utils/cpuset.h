#ifndef CPUSET_H
#define CPUSET_H

#include <sched.h>
#include <pthread.h>
#include <unistd.h>

bool set_process_affinity(int pid, int cpu_id);

bool unset_process_affinity(int pid);

bool set_thread_affinity(pthread_t tid, int cpu_id);

bool unset_thread_affinity(pthread_t tid);

#endif // CPUSET_H
