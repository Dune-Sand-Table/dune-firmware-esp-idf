#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "storage.h"

static const char *TAG = "[points_provider/points_provider.c]";

static QueueHandle_t out_queue = NULL;
static SemaphoreHandle_t sync_semaphore = NULL;
static int queue_capacity;
static int queue_batch_size;
static int task_file_handle = -1;
static bool running = false;

typedef struct {
    float a;
    float r;
    bool end;
} pt__local;

static void points_provider_task(void *pvParameters) {
    while (1) {
        
        if (!running || task_file_handle == -1) {
            vTaskDelay(pdMS_TO_TICKS(100)); 
            continue;
        }

        if (xSemaphoreTake(sync_semaphore, pdMS_TO_TICKS(100)) != pdTRUE) {
            continue;  
        }

        int n = 0;
        bool eof = false;
        pt__local p;

        while (n < queue_batch_size) {
            if (!storage_read(task_file_handle, &p, sizeof(p))) {  
                eof = true;
                break;
            }
            p.end = false;          
            if (xQueueSend(out_queue, &p, pdMS_TO_TICKS(100)) != pdPASS) {
                break; // why?
            }
            n++;
        }

         if (n == 0 && eof) {
            xSemaphoreGive(sync_semaphore);  
            storage_close(task_file_handle);
            task_file_handle = -1;
            continue;
        }

        if (eof) {
            // последняя точка уже положена, но её нужно пометить end
            // вариант A: пометить в файле заранее (см. ниже)
            // вариант B: положить отдельный маркер (но у вас end — на реальной точке)
        }
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

    xTaskCreatePinnedToCore(points_provider_task, "points_provider_task", 4096, NULL, 14, NULL, 1);
}

void points_provider_starting() {
    ESP_LOGI(TAG, "points_provider_starting");
    running = true;
}

void points_provider_pausing() {
    ESP_LOGI(TAG, "points_provider_pausing");
    running = false;
}

void points_provider_resuming() {
    ESP_LOGI(TAG, "points_provider_resuming");
    running = true;
}

void points_provider_stopping() {
    ESP_LOGI(TAG, "points_provider_stopping");
    running = false;
}

void points_provider_set_task(const char* path) {
    ESP_LOGI(TAG, "points_provider_set_task %s", path);
    task_file_handle = storage_open(path, "rb");
}

bool points_provider_has_job() {
    return task_file_handle != -1;
}
