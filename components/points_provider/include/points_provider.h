#ifndef POINTS_PROVIDER_H
#define POINTS_PROVIDER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void points_provider_starting();

void points_provider_pausing();

void points_provider_resuming();

void points_provider_stopping();

void points_provider_set_task(const char * id);

bool points_provider_has_job();

void points_provider_init(
    int capacity, 
    int batch_size, 
    QueueHandle_t queue, 
    SemaphoreHandle_t sync
);



#endif