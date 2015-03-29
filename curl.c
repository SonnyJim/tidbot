/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2013, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* Example source code to show how the callback function can be used to
 * download data into a chunk of memory instead of storing it in a file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include "tidbot.h"

struct MemoryStruct {
  char *memory;
  size_t size;
};

char string[1024];

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

char* get_url (const char* msg)
{
    char *start, *end;
    int len = 0;

    start = strstr (msg, MAGIC_HTTP);
    end = strstr (start, " ");
    if (end == NULL)
        len = strlen (start);
    else
        len = end - start;

    strncpy (string, start, len);
    string[len] = '\0';
    return string;
}

char* get_title (const char *url)
{
  CURL *curl_handle;
  CURLcode res;

  struct MemoryStruct chunk;
  
  char *p1, *p2;
  char title_open[] = "<title>";
  char title_close[] = "</title>";
  int len;

  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* Don't verify https links */
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

  /* Follow redirects */
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
  /* get it! */
  res = curl_easy_perform(curl_handle);

  /* check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    
    p1 = strcasestr (chunk.memory, title_open);
    p2 = strcasestr (chunk.memory, title_close);
    p1 += strlen (title_open);
    len = p2 - p1;
    if (len > 1024)
    {
        fprintf (stderr, "Error, title was bigger than 1024 bytes\n");
        return NULL;
    }
    if (len < 0)
    {
        fprintf (stderr, "Error, title len was less than zero?\n");
        fprintf (stderr, "%s\n", chunk.memory);
        return NULL;
    }
    strncpy (string, p1, len);
    //Add NULL terminator!!!
    string[len] = '\0';
    return string;
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  if(chunk.memory)
    free(chunk.memory);

  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();

  return 0;
}
