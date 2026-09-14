#ifndef STEPPERS_H
#define STEPPERS_H

void steppers_starting();

void steppers_running();

void steppers_pausing();

void steppers_resuming();

void steppers_stopping();


typedef struct {
    float angle;
    float radius;
    uint32_t speed;
} polar_point_t;

void steppers_init(
    int capacity,
    int batch_size,
    QueueHandle_t in, 
    SemaphoreHandle_t sync,
    void (*publish)(const char*, const char*)
);

#endif