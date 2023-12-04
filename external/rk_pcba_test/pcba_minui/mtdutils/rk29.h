/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RECOVERY_RK29_H_
#define RECOVERY_RK29_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define READ_SIZE 16384
#define READ_MASK (READ_SIZE - 1)

#define WRITE_SIZE 16384
#define WRITE_MASK (WRITE_SIZE - 1)

int run(const char *filename, char *const argv[]);
int rk_make_ext3fs(const char *filename);
int rk_check_and_resizefs(const char *filename);
int rk_check_and_resizefs_f2fs(const char *filename);
int rk_make_ext4fs(const char *filename, long long len, const char *mountpoint);
size_t rk29_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t rk29_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int make_vfat(const char *filename,const char* volumelabel);
int make_ntfs(const char *filename,const char* volumelabel);

#ifdef __cplusplus
}
#endif

#endif  // RECOVERY_RK29_H_
