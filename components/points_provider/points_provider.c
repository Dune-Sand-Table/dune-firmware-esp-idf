#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "[points_provider/points_provider.c]";

static QueueHandle_t out_queue = NULL;
static SemaphoreHandle_t sync_semaphore = NULL;
static int queue_capacity;
static int queue_batch_size;

static void points_provider_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void points_provider_init(
    int capacity,
    int batch_size,
    QueueHandle_t queue, 
    SemaphoreHandle_t sync
) {
    out_queue = queue;
    sync_semaphore = sync;
    queue_capacity = capacity;
    queue_batch_size = batch_size;

    xTaskCreatePinnedToCore(points_provider_task, "points_provider_task", 4096, NULL, 15, NULL, 1);
}

void points_provider_starting() {
    ESP_LOGI(TAG, "points_provider_starting");
}

void points_provider_pausing() {
    ESP_LOGI(TAG, "points_provider_pausing");
}

void points_provider_resuming() {
    ESP_LOGI(TAG, "points_provider_resume");
}

void points_provider_stopping() {
    ESP_LOGI(TAG, "points_provider_stopping");
}

void points_provider_set_task(const char* id) {
    ESP_LOGI(TAG, "points_provider_set_task %s", id);
}