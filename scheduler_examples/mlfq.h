#ifndef SCHEDULING_MLFQ_H
#define SCHEDULING_MLFQ_H

#include "queue.h"
#include <stdint.h>

#define MLFQ_LEVELS 3

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq[MLFQ_LEVELS], pcb_t **cpu_task);

#endif //SCHEDULING_MLFQ_H