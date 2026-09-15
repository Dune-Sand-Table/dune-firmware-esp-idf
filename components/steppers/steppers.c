
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "steppers.h"
#include "esp_log.h"

static const char *TAG = "[stepprs/stepprs.c]";

static int queue_capacity;
static int queue_batch_size;
static QueueHandle_t queue_in; 
static SemaphoreHandle_t sync_semaphore;
static int processed_counter;
static void (*broadcast)(const char*, const char*);

static void run_motors(polar_point_t* target) {

    vTaskDelay(pdMS_TO_TICKS(100)); 
}

static void steppers_task(void *pvParameters) {
    polar_point_t target;
    while (1) {
        
        if (xQueueReceive(queue_in, &target, portMAX_DELAY) == pdTRUE) {
            run_motors(&target); 

            processed_counter++;
            if (target.end) {
                // end of task
                processed_counter = 0;
                // if (broadcast) broadcast("processed", "tail batch");
                xSemaphoreGive(sync_semaphore);  
                continue;
            }
          
            if ((processed_counter >= queue_batch_size)) {
                processed_counter = 0; 
                xSemaphoreGive(sync_semaphore); 
            }
        }
    }
}

void steppers_init(
    int capacity,
    int batch_size,
    QueueHandle_t in, 
    SemaphoreHandle_t sync,
    void (*publish)(const char*, const char*)
) {
    broadcast = publish;
    queue_capacity = capacity;
    queue_batch_size = batch_size;
    queue_in = in;
    sync_semaphore = sync;

    xTaskCreatePinnedToCore(steppers_task, "steppers_task", 4096, NULL, 15, NULL, 1);
}

void steppers_starting() {
    ESP_LOGI(TAG, "steppers_starting");
}

void steppers_running() {
    ESP_LOGI(TAG, "steppers_running");
}

void steppers_pausing() {
    ESP_LOGI(TAG, "steppers_pausing");
}

void steppers_resuming() {
    ESP_LOGI(TAG, "steppers_resuming");
}

void steppers_stopping() {
    ESP_LOGI(TAG, "steppers_stopping");
}
