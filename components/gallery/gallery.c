#include "gallery.h"
#include "storage.h"
#include "stdio.h"
#include "esp_random.h" 
#include "string.h"
#include "esp_log.h"

//static const char *TAG = "[gallery/gallery.c]";

#define GALLERY_DIR "/gallery"

void gallery_get_file_path(char* out, size_t out_size, const char* id) {
    snprintf(out, out_size, "%s/%s.bin", GALLERY_DIR, id);
}

void gallery_init() {
    storage_ensure_directory(GALLERY_DIR);
}


// get gallery list
bool gallery_iterator_start(gallery_iterator_t* it) {
    it->dir = storage_dir_open(GALLERY_DIR);
    it->file = -1;
    it->bytes_was_read = 0;
    return true;
}
bool gallery_iterator_next(gallery_iterator_t* it, char *buffer, size_t max_len) {
    if (it == NULL || buffer == NULL || max_len == 0) 
        return false;

    if (it->file >= 0) {
        size_t read_bytes = storage_read(it->file, buffer, max_len - 1);
        if (read_bytes > 0) {
            buffer[read_bytes] = '\0'; 
            it->bytes_was_read = read_bytes;
            if (read_bytes != max_len - 1) {
                storage_close(it->file);
                it->file = -1;
            }
            return true; 
        }
        storage_close(it->file);
        it->file = -1;
        return true;
    }

    char filename[128];
    while (storage_dir_next(it->dir, filename, sizeof(filename))) {
        char *ext = strrchr(filename, '.');
        if (ext != NULL && strcmp(ext, ".json") == 0) {
            char file_path[140];
            snprintf(file_path, sizeof(file_path), "%s/%s", GALLERY_DIR, filename);
            int json_file = storage_open(file_path, "rb");
            if (json_file >= 0) {
                it->file = json_file;
                it->bytes_was_read = 0;
                return true; 
            }
        }
    }
    return false;
}

void gallery_iterator_close(gallery_iterator_t* it) {
    storage_dir_close(it->dir);
}

static void generate_random_id(char *output, size_t length) {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t charset_size = sizeof(charset) - 1;
    for (size_t i = 0; i < length; i++) {
        uint32_t random_num = esp_random();
        output[i] = charset[random_num % charset_size];
    }
    output[length] = '\0'; 
}

// upload
bool gallery_upload_start(gallery_upload_t* upload) {
    generate_random_id(upload->id, 8);
    char path[128];
    snprintf(path, sizeof(path), "%s/%s.bin", GALLERY_DIR, upload->id);
    upload->file = storage_open(path, "wb");
    return true;
}
bool gallery_upload_metadata_start(gallery_upload_t* upload) {
    char path[128];
    snprintf(path, sizeof(path), "%s/%s.json", GALLERY_DIR, upload->id);
    upload->file = storage_open(path, "wb");
    return true;
}
void gallery_upload_write(gallery_upload_t* upload, char* buffer, size_t received) {
    storage_write(upload->file, buffer, received);
}   
// void gallery_update_item_metadata(char* id, char* json) {
//     char path[128];
//     snprintf(path, sizeof(path), "%s/%s.json", GALLERY_DIR, id);
//     int descr = storage_open(path, "wb");
//     storage_write(descr, json, strlen(json));
//     storage_close(descr);
// }
void gallery_upload_finish(gallery_upload_t* upload) {
    storage_close(upload->file);
   // gallery_update_item_metadata(upload->id, "{}");
}

// get item chunked
bool gallery_item_open(gallery_item_t* item) {
    char path[128];
    snprintf(path, sizeof(path), "%s/%s.bin", GALLERY_DIR, item->id);
    item->file = storage_open(path, "rb");
    return true;
}
size_t gallery_item_read(gallery_item_t* item, char* buffer, size_t len) {
    return storage_read(item->file, buffer, len);
}
void gallery_item_close(gallery_item_t* item) {
    storage_close(item->file);
}
