// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "utils.h"

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <sstream>

#ifdef RKMEDIA_SUPPORT_MINILOG
    #include "minilogger/log.h"
#endif

enum
{
    LOG_LEVEL_INFO,
    LOG_LEVEL_DBG,
};

enum
{
    LOG_METHOD_MINILOG,
    LOG_METHOD_PRINT
};

static int rkmedia_log_method = LOG_METHOD_PRINT;
static int rkmedia_log_level = LOG_LEVEL_INFO;

_API void LOG_INIT()
{
    char* ptr = NULL;

    rkmedia_log_method = LOG_METHOD_PRINT;
#ifdef RKMEDIA_SUPPORT_MINILOG
    ptr = getenv("RKMEDIA_LOG_METHOD");
    if (ptr && strstr(ptr, "MINILOG")) {
        fprintf(stderr, "##RKMEDIA Method: MINILOG\n");
        __minilog_log_init("RKMEDIA", NULL, false, false, "RKMEDIA", "0.1");
        rkmedia_log_method = LOG_METHOD_MINILOG;
    } else {
        fprintf(stderr, "##RKMEDIA Method: PRINT\n");
        rkmedia_log_method = LOG_METHOD_PRINT;
    }
#endif //#ifdef RKMEDIA_SUPPORT_MINILOG

    ptr = getenv("RKMEDIA_LOG_LEVEL");
    if (ptr && strstr(ptr, "DBG")) {
        fprintf(stderr, "##RKMEDIA Log level: DBG\n");
        rkmedia_log_level = LOG_LEVEL_DBG;
    } else {
        fprintf(stderr, "##RKMEDIA Log level: INFO\n");
        rkmedia_log_level = LOG_LEVEL_INFO;
    }
}

static void LogPrintf(const char* prefix, const char* fmt, va_list vl)
{
    char line[1024];
    if (rkmedia_log_level >= LOG_LEVEL_DBG) {
#ifdef RKMEDIA_SUPPORT_MINILOG
        if (rkmedia_log_method == LOG_METHOD_PRINT) {
#endif
            fprintf(stderr, "%s", (char*)prefix);
            vsnprintf(line, sizeof(line), fmt, vl);
            fprintf(stderr, "%s", line);
#ifdef RKMEDIA_SUPPORT_MINILOG
        } else {
            // minilog_debug("%s", (char *)prefix);
            vsnprintf(line, sizeof(line), fmt, vl);
            minilog_debug("%s%s", (char*)prefix, line);
        }
#endif
    }
}

void LOGD(const char* format, ...)
{
    va_list vl;
    va_start(vl, format);
    LogPrintf("Debug - ", format, vl);
    va_end(vl);
}

void LOG(const char* format, ...)
{
    do {
        va_list vl;
        va_start(vl, format);
        char line[1024];
        vsnprintf(line, sizeof(line), format, vl);
#ifdef RKMEDIA_SUPPORT_MINILOG
        if (rkmedia_log_method == LOG_METHOD_PRINT)
#endif
            fprintf(stderr, "%s", line);
#ifdef RKMEDIA_SUPPORT_MINILOG
        else {
            minilog_info("%s", line);
        }
#endif
        va_end(vl);
    } while (0);
}

namespace easymedia
{

    bool parse_media_param_map(const char* param, std::map<std::string, std::string>& map)
    {
        if (!param) {
            return false;
        }

        std::string token;
        std::istringstream tokenStream(param);
        while (std::getline(tokenStream, token)) {
            std::string key, value;
            std::istringstream subTokenStream(token);
            if (std::getline(subTokenStream, key, '=')) {
                std::getline(subTokenStream, value);
            }
            map[key] = value;
        }

        return true;
    }

    bool parse_media_param_list(const char* param, std::list<std::string>& list, const char delim)
    {
        if (!param) {
            return false;
        }

        std::string token;
        std::istringstream tokenStream(param);
        while (std::getline(tokenStream, token, delim)) {
            list.push_back(token);
        }

        return true;
    }

    int parse_media_param_match(const char* param, std::map<std::string, std::string>& map,
                                std::list<std::pair<const std::string, std::string&>>& list)
    {
        if (!easymedia::parse_media_param_map(param, map)) {
            return 0;
        }
        int match_num = 0;
        for (auto& key : list) {
            const std::string& req_key = key.first;
            for (auto& entry : map) {
                const std::string& entry_key = entry.first;
                if (req_key == entry_key) {
                    key.second = entry.second;
                    match_num++;
                    break;
                }
            }
        }

        return match_num;
    }

    bool has_intersection(const char* str, const char* expect, std::list<std::string>* expect_list)
    {
        std::list<std::string> request;
        // LOG("request: %s; expect: %s\n", str, expect);

        if (!expect || (strlen(expect) == 0)) {
            return true;
        }

        if (!parse_media_param_list(str, request, ',')) {
            return false;
        }
        if (expect_list->empty() && !parse_media_param_list(expect, *expect_list)) {
            return false;
        }
        for (auto& req : request) {
            if (std::find(expect_list->begin(), expect_list->end(), req) != expect_list->end()) {
                return true;
            }
        }
        return false;
    }

    std::string get_media_value_by_key(const char* param, const char* key)
    {
        std::map<std::string, std::string> param_map;
        if (!parse_media_param_map(param, param_map)) {
            return std::string();
        }
        return param_map[key];
    }

    bool string_start_withs(std::string const& fullString, std::string const& starting)
    {
        if (fullString.length() >= starting.length()) {
            return (0 == fullString.find(starting));
        } else {
            return false;
        }
    }

    bool string_end_withs(std::string const& fullString, std::string const& ending)
    {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

#ifndef NDEBUG

    #include <fcntl.h>
    #include <unistd.h>

    bool DumpToFile(std::string path, const char* ptr, size_t len)
    {
        int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_FSYNC);
        if (fd < 0) {
            LOG("Fail to open %s\n", path.c_str());
            return false;
        }
        LOGD("dump to %s, size %d\n", path.c_str(), (int)len);
        write(fd, ptr, len);
        close(fd);
        return true;
    }

#endif // #ifndef NDEBUG

} // namespace easymedia
