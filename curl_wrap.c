#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>
#include <glib.h>

struct mem_block_t {
    char * memory;
    size_t size;
};

    static size_t
WriteMemoryCallback(void * contents, size_t size, size_t nmemb, void * userp)
{
    size_t realsize; 
    struct mem_block_t * mem;

    realsize = size * nmemb;
    mem = (struct mem_block_t *) userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        exit(-1);
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    g_debug("Memory chunking: %zu bytes.", mem->size);
    if (mem->size > 16*1024) {
        g_debug("Passed chunking limit\n");
        return -1;
    }
    return realsize;
}

char * curl_perform(char * url)
{
    CURL * curl;
    struct mem_block_t chunk;
    char * url_content = NULL;

    memset(&chunk, 0, sizeof (struct mem_block_t));

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "neuro_irc_bot");
    //curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    url_content = chunk.memory;

    return url_content;
}

