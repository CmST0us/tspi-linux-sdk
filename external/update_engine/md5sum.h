/*************************************************************************
	> File Name: md5sum.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Thu 07 Mar 2019 03:14:08 PM CST
 ************************************************************************/

#ifndef _MD5SUM_H
#define _MD5SUM_H
bool comparefile(const char *dest_path, const char *source_path, long long dest_offset, long long source_offset, long long checkSize);
bool compareMd5sum(const char *dest_path, unsigned char *source_md5sum, long long offset, long long checkSize);
#endif
