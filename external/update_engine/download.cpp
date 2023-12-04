/*************************************************************************
	> File Name: download.cpp
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Thu 07 Mar 2019 10:30:31 AM CST
 ************************************************************************/

#include <stdio.h>
#include <curl/curl.h>
#include "log.h"
size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

extern double processvalue;

int my_progress_func(char *progress_data,
                     double t, /* dltotal */
                     double d, /* dlnow */
                     double ultotal,
                     double ulnow)
{
    processvalue = ulnow / ultotal * 100 / 110;
    return 0;
}

int download_file(char *url, char *output_filename)
{
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    char *progress_data = "* ";

    curl = curl_easy_init();
    if(curl)
    {
        outfile = fopen(output_filename, "wb");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write_func);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, my_progress_func);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            LOGE("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        fclose(outfile);
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    if(res != CURLE_OK){
        LOGE("download Error.\n");
        return -1;
    }
    return 0;
}

