#include "api.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "api_key.h"
#include <stddef.h>
#include <curl/curl.h>


// Custom callback function to write the response data into a dynamic string
size_t write_callback(void *contents, size_t size, size_t nmemb, struct Buffer *buffer)
{
    size_t total_size = size * nmemb;

    // Resize the buffer to accommodate the new data
    buffer->data = realloc(buffer->data, buffer->size + total_size + 1);
    if (buffer->data == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer.\n");
        return 0;
    }

    // Copy the new data into the buffer
    memcpy(buffer->data + buffer->size, contents, total_size);
    buffer->size += total_size;
    buffer->data[buffer->size] = '\0';

    return total_size;
}

char *get_stop_routes(char *stop_id, const char *url_start)
{
    CURLcode ret;
    CURL* hnd;
    struct curl_slist* slist1;
    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "accept: application/json");
    slist1 = curl_slist_append(slist1, "x-api-key: " API_KEY);
    hnd = curl_easy_init();

    // Create a buffer struct to store the response
    struct Buffer buffer;
    buffer.data = NULL;
    buffer.size = 0;

    // Set the write callback function to capture the response
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &buffer);

    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 1024L);

    char *url = malloc(strlen(url_start) + strlen(stop_id));
    if (url == NULL) {
        fprintf(stderr, "error: alloc");
        exit(1);
    }
    strcpy(url, url_start);
    strcat(url, stop_id);

    curl_easy_setopt(hnd, CURLOPT_URL, url);
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.85.0");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    ret = curl_easy_perform(hnd);
    
    free(url);

    if (ret != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
    }


    curl_easy_cleanup(hnd);

    curl_slist_free_all(slist1);

    return buffer.data;
}
