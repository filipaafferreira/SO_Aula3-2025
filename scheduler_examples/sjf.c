#include "fifo.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed (this is FIFO after all)
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }
    if (*cpu_task == NULL) {            // If CPU is idle
        pcb_t *shortest = NULL;
        queue_elem_t *cur = rq->head;
        queue_elem_t *best_elem = NULL;

        while (cur) {
            if (shortest == NULL || cur->pcb->time_ms < shortest->time_ms) {
                shortest = cur->pcb;
                best_elem = cur;
            }
            cur = cur->next;
        }

        if (best_elem) {
            queue_elem_t *removed = remove_queue_elem(rq, best_elem);
            if (removed) {
                *cpu_task = removed->pcb;
                free(removed);
            }
        }
    }
}