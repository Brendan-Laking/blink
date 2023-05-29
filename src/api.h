#ifndef API_H_
#define API_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stddef.h>
#include "api_key.h"

struct Buffer {
    char *data;
    size_t size;
};

char *get_stop_routes(char *stop_id, const char *url_start);
size_t write_callback(void *contents, size_t size, size_t nmemb, struct Buffer *buffer);

#endif // API_KEY

