#include "cpuset.h"

bool set_process_affinity(int pid, int cpu_id)
{
    int cpu_count = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_id < 0 || cpu_id >= cpu_count) return false;

    cpu_set_t cpus_mask;
    CPU_ZERO(&cpus_mask);
    CPU_SET(cpu_id, &cpus_mask);
    return (sched_setaffinity(pid, sizeof(cpus_mask), &cpus_mask) >= 0);
}

bool unset_process_affinity(int pid)
{
    cpu_set_t cpus_mask;
    CPU_ZERO(&cpus_mask);
    return (sched_setaffinity(pid, sizeof(cpus_mask), &cpus_mask) >= 0);
}

bool set_thread_affinity(pthread_t tid, int cpu_id)
{
    int cpu_count = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_id < 0 || cpu_id >= cpu_count) return false;

    cpu_set_t cpus_mask;
    CPU_ZERO(&cpus_mask);
    CPU_SET(cpu_id, &cpus_mask);
    return (pthread_setaffinity_np(tid, sizeof(cpus_mask), &cpus_mask) >= 0);
}

bool unset_thread_affinity(pthread_t tid)
{
    cpu_set_t cpus_mask;
    CPU_ZERO(&cpus_mask);
    return (pthread_setaffinity_np(tid, sizeof(cpus_mask), &cpus_mask) >= 0);
}
