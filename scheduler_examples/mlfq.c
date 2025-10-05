#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "queue.h"
#include "msg.h"

#define MLFQ_LEVELS 3
#define TICKS_MS 10
#define TIME_QUANTUM_LEVEL_0 20
#define TIME_QUANTUM_LEVEL_1 40
#define TIME_QUANTUM_LEVEL_2 80
#define PROCESS_REQUEST_DONE 1

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq[MLFQ_LEVELS], pcb_t **cpu_task) {
    static uint32_t time_in_queue = 0;
    static int current_level = 0;

    // Atualizar tarefa atual se existir
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        time_in_queue += TICKS_MS;

        // Verifica se terminou
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t))
                perror("write");

            free(*cpu_task);
            *cpu_task = NULL;
            time_in_queue = 0;
        } else {
            uint32_t quantum = (current_level == 0) ? TIME_QUANTUM_LEVEL_0 :
                               (current_level == 1) ? TIME_QUANTUM_LEVEL_1 :
                               TIME_QUANTUM_LEVEL_2;

            if (time_in_queue >= quantum) {
                int next_level = (current_level < MLFQ_LEVELS - 1) ? current_level + 1 : current_level;
                enqueue_pcb(rq[next_level], *cpu_task);
                *cpu_task = NULL;
                time_in_queue = 0;
            }
        }
    }

    if (*cpu_task == NULL) {
        for (int lvl = 0; lvl < MLFQ_LEVELS; lvl++) {
            if (rq[lvl]->head) {
                *cpu_task = dequeue_pcb(rq[lvl]);
                current_level = lvl;
                time_in_queue = 0;
                break;
            }
        }
    }
}