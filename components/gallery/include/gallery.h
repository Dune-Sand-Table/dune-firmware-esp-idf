#ifndef GALLERY_H
#define GALLERY_H

#include <stdio.h>

void gallery_init();

// get list chunked

typedef struct {
    void *dir;
    int file; 
    size_t bytes_was_read;
} gallery_iterator_t;

bool gallery_iterator_start(gallery_iterator_t*);
bool gallery_iterator_next(gallery_iterator_t*, char*, size_t);
void gallery_iterator_close(gallery_iterator_t*);


// upload item chunked

typedef struct {
    char* name;
    int file;
    char id[9];
} gallery_upload_t ;

bool gallery_upload_start(gallery_upload_t*);
bool gallery_upload_metadata_start(gallery_upload_t*);
void gallery_upload_write(gallery_upload_t*, char*, size_t);
void gallery_upload_finish(gallery_upload_t*);


// get item chunked

typedef struct {
    char id[9];
    int file;
} gallery_item_t;

bool   gallery_item_open(gallery_item_t*);
size_t gallery_item_read(gallery_item_t*, char*, size_t);
void   gallery_item_close(gallery_item_t*);


void gallery_get_file_path(char* out, size_t out_size, const char* id);

#endif


