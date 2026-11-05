/*
 * scheduler.c
 *
 *  Created on: Nov 5, 2026
 *      Author: user
 */


#include "main.h"
#include "scheduler.h"
#include <stddef.h>

typedef struct sTask {
    void (*pTask)(void);
    uint32_t Deadline;
    uint32_t Period;
    uint32_t TaskID;
    struct sTask *next;
    uint8_t running;
    uint8_t cancelled;
} sTask;

static sTask tasks[SCH_MAX_TASKS];
static sTask *head = NULL;
static volatile uint32_t tickCount = 0;
static uint32_t nextTaskID = 0;

static void SCH_Insert_Task(sTask *task) {
    sTask **current = &head;

    while (*current != NULL) {
        int32_t diff =
            (int32_t)(task->Deadline - (*current)->Deadline);

        if (diff < 0 ||
            (diff == 0 && task->TaskID < (*current)->TaskID)) {
            break;
        }

        current = &(*current)->next;
    }

    task->next = *current;
    *current = task;
}

static void SCH_Clear_Task(sTask *task) {
    task->pTask = NULL;
    task->Deadline = 0;
    task->Period = 0;
    task->TaskID = NO_TASK_ID;
    task->next = NULL;
    task->running = 0;
    task->cancelled = 0;
}

void SCH_Init(void) {
    head = NULL;
    tickCount = 0;
    nextTaskID = 0;

    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_Clear_Task(&tasks[i]);
    }
}

void SCH_Update(void) {
    tickCount++;
}

uint32_t SCH_Add_Task(void (*pFunction)(void),
                      uint32_t DELAY, uint32_t PERIOD) {
    if (pFunction == NULL ||
        DELAY > INT32_MAX || PERIOD > INT32_MAX) {
        return NO_TASK_ID;
    }

    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (tasks[i].pTask == NULL) {
            sTask *task = &tasks[i];

            nextTaskID++;
            if (nextTaskID == NO_TASK_ID) {
                nextTaskID++;
            }

            task->pTask = pFunction;
            task->Deadline = tickCount + DELAY;
            task->Period = PERIOD;
            task->TaskID = nextTaskID;
            task->running = 0;
            task->cancelled = 0;
            task->next = NULL;

            SCH_Insert_Task(task);
            return task->TaskID;
        }
    }

    return NO_TASK_ID;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID == NO_TASK_ID) return 0;

    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        sTask *task = &tasks[i];

        if (task->pTask == NULL ||
            task->TaskID != taskID ||
            task->cancelled) {
            continue;
        }

        if (task->running) {
            task->cancelled = 1;
            return 1;
        }

        sTask **current = &head;

        while (*current != NULL && *current != task) {
            current = &(*current)->next;
        }

        if (*current == task) {
            *current = task->next;
        }

        SCH_Clear_Task(task);
        return 1;
    }

    return 0;
}

void SCH_Dispatch_Tasks(void) {
    while (head != NULL &&
           (int32_t)(tickCount - head->Deadline) >= 0) {

        sTask *task = head;
        head = task->next;
        task->next = NULL;
        task->running = 1;

        uint32_t oldDeadline = task->Deadline;

        task->pTask();

        task->running = 0;

        if (task->cancelled || task->Period == 0) {
            SCH_Clear_Task(task);
            continue;
        }

        uint32_t nextDeadline = oldDeadline + task->Period;

        uint32_t now = tickCount;
        if ((int32_t)(now - nextDeadline) >= 0) {
            uint32_t missed = (now - nextDeadline) / task->Period + 1U;
            nextDeadline += missed * task->Period;
        }

        task->Deadline = nextDeadline;
        SCH_Insert_Task(task);
    }
}

uint32_t get_time(void) {
    return tickCount * SCH_TICK_MS;
}
