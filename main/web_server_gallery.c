#include <server.h>
#include <server_api.h>
#include <gallery.h>
#include "esp_random.h" 
#include "esp_log.h"
#include "points_provider.h"
#include "broadcaster.h"

static const char *TAG = "[main/web_server_gallery.c]";

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define SCRATCH_BUFSIZE 1024

esp_err_t web_server__get_gallery_list(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send_chunk(req, "[\n", 2);
    gallery_iterator_t it;
    gallery_iterator_start(&it);
    char buf[SCRATCH_BUFSIZE];
    bool is_first_file = true;
    while (gallery_iterator_next(&it, buf, SCRATCH_BUFSIZE)) {
        if (it.bytes_was_read == 0) {
            if (!is_first_file) {
                httpd_resp_send_chunk(req, ",\n", 2);
            }
            continue;
        }
        httpd_resp_send_chunk(req, buf, it.bytes_was_read); 
        is_first_file = false;
    }
    gallery_iterator_close(&it);
    httpd_resp_send_chunk(req, "]\n", 2);
    httpd_resp_send_chunk(req, NULL, 0); 
    return ESP_OK;
}

esp_err_t web_server__gallery_upload(httpd_req_t *req) {
    char buf[SCRATCH_BUFSIZE];
    int received;
    size_t total_len = req->content_len;
    size_t remaining = total_len;
    gallery_upload_t upload;
    gallery_upload_start(&upload);
    while (remaining > 0) {
        received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE));
        if (received <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "{\"error\":\"Failed to receive post data\"}");
            return ESP_FAIL;
        }
        gallery_upload_write(&upload, buf, received);
        remaining -= received;
    }
    gallery_upload_finish(&upload);
    // response
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_status(req, "201 Created");
    char response[64];
    snprintf(response, sizeof(response), "{\"result\":\"Binary uploaded successfully\",\"id\":\"%s\"}", upload.id);
    httpd_resp_sendstr(req, response);
    return ESP_OK;
}

esp_err_t web_server__gallery_metadata(httpd_req_t *req) {
    char buf[SCRATCH_BUFSIZE];
    int received;
    size_t total_len = req->content_len;
    size_t remaining = total_len;
    gallery_upload_t upload;
    if (httpd_req_get_hdr_value_str(req, "X-Id", upload.id, 9) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "{\"error\":\"Missing X-Id header\"}");
        return ESP_ERR_NOT_FOUND;
    } 
    ESP_LOGI(TAG, "metadata upload id: %s", upload.id);
    gallery_upload_metadata_start(&upload);
    while (remaining > 0) {
        received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE));
        if (received <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive post data");
            return ESP_FAIL;
        }
        gallery_upload_write(&upload, buf, received);
        remaining -= received;
    }
    gallery_upload_finish(&upload);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_status(req, "201 Created");
    httpd_resp_sendstr(req, "{\"result\":\"Metadata uploaded successfully\"}");
    return ESP_OK;
}


esp_err_t web_server__get_gallery_item(httpd_req_t *req) {
    gallery_item_t item;
    if (httpd_req_get_hdr_value_str(req, "X-Id", item.id, 9) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "{\"error\":\"Missing X-Id header\"}");
        return ESP_ERR_NOT_FOUND;
    } 
    ESP_LOGI(TAG, "get item id: %s", item.id);
    if (!gallery_item_open(&item)) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Gallery item not found");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/octet-stream"); 
    char buffer[SCRATCH_BUFSIZE];
    int read_bytes;
    esp_err_t res = ESP_OK;
    while ((read_bytes = gallery_item_read(&item, buffer, sizeof(buffer))) > 0) {
        res = httpd_resp_send_chunk(req, buffer, read_bytes);
        if (res != ESP_OK) {
            break; 
        }
    }
    gallery_item_close(&item);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


esp_err_t web_server__gallery_activate(httpd_req_t *req) {
    char id[9];
    if (httpd_req_get_hdr_value_str(req, "X-Id", id, 9) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "{\"error\":\"Missing or corrupted X-Id header\"}");
        return ESP_ERR_NOT_FOUND;
    } 
    char path[128];
    gallery_get_file_path(path, sizeof(path), id);
    points_provider_set_task(path);
    broadcaster_publish("currentTask", id);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_status(req, "200");
    httpd_resp_sendstr(req, "{\"result\":\"Task loaded successfully\"}");
    return ESP_OK;
}