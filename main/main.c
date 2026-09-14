#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h> 

#include <nvs_manager.h>
#include <storage.h>
#include <network.h>
#include <network_settings.h>
#include <broadcaster.h>
#include <ws.h>
#include <fsm.h>
#include <gallery.h>
#include <points_provider.h>
#include <steppers.h>
#include "stdio.h"


void delay() {
    vTaskDelay(pdMS_TO_TICKS(50));
}

void state_machine_init();
void web_server_init();

void app_main(void) {
    printf("\n\n");
    printf("     ___  __  ___  ______  \n");
    printf("    / _ \\/ / / / |/ / __/ \n");
    printf("   / // / /_/ /    / _/    \n");
    printf("  /____/\\____/_/|_/___/   \n");
    printf("   @ strangerinteq.art     \n\n\n\n");

    nvs_manager_init();

    network_init(
        nvs_manager_get_str, 
        nvs_manager_set_str
    );

    storage_init();

    web_server_init();

    broadcaster_init(
        ws_send
    );

    ws_set_client_connected_callback(
        broadcaster_on_client_connected_callback
    );

    gallery_init();

    const int capacity = 40;
    const int batch_size = 20;
    const size_t point_size = sizeof(polar_point_t);
    QueueHandle_t queue = xQueueCreate(capacity, point_size);
    SemaphoreHandle_t sync = xSemaphoreCreateBinary();

    points_provider_init(
        capacity, 
        batch_size,
        queue, 
        sync
    );

    steppers_init(
        capacity, 
        batch_size,
        queue, 
        sync, 
        broadcaster_publish
    );

    state_machine_init();

   
}
