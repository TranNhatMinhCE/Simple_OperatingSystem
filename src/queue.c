#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q->size >= 0 && q->size < MAX_QUEUE_SIZE) {
                q->proc[q->size++] = proc;
        }
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
	// return NULL;
        // if (empty(q)) {
        //         return NULL;
        // }
        // struct pcb_t * highest_priority_proc = q->proc[0];
        // int highest_priority_index = 0;
        // for (int i = 1; i < q->size; i++) {
        //         if (q->proc[i]->priority < highest_priority_proc->priority) {
        //         highest_priority_proc = q->proc[i];
        //         highest_priority_index = i;
        //         }
        // }
        // for (int i = highest_priority_index; i < q->size - 1; i++) {
        //         q->proc[i] = q->proc[i + 1];
        // }
        // q->size--;
        // return highest_priority_proc;

        if (q == NULL || q->size == 0) {
            return NULL;
        }
        if (q->size == 1) {
            struct pcb_t* return_proc = q->proc[0];
            q->proc[0] = NULL;
            q->size = 0;
            return return_proc;
        }
        struct pcb_t* return_proc = q->proc[0];
        uint32_t highest_priority_proc = q->proc[0]->priority;
        int highest_priority_index = 0;

        // Search for the process with the highest priority
        for (int i = 1; i < q->size; i++) {
            if (q->proc[i]->priority > highest_priority_proc) {
                highest_priority_proc = q->proc[i]->priority;
                highest_priority_index = i;
            }
        }
        return_proc = q->proc[highest_priority_index];

        if (highest_priority_index == q->size - 1) {
            q->proc[q->size - 1] = NULL;
            q->size--;
            return return_proc;
        }
        else {
            for (int i = highest_priority_index; i < q->size - 1; i++) {
            q->proc[i] = q->proc[i + 1];  // Shift the processes left to fill the gap
        }

        q->proc[q->size - 1] = NULL; // Set the last slot to NULL
        q->size--; // Decrease the size of the queue

        return return_proc; // Return the highest priority process
    }
}

