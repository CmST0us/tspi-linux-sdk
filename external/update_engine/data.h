/*************************************************************************
	> File Name: data.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Fri 26 Oct 2018 04:49:58 PM CST
 ************************************************************************/

#ifndef _DATA_H
#define _DATA_H

#define SLOT_A 0
#define SLOT_B 1
#define SLOT_UNKNOW 2

using namespace std;


#define READ_SIZE 512000
struct ImageData{
    unsigned char md5sum[16];
    char *data;
    unsigned int offset;
    unsigned int size; //默认为4096, 至少传递512
};

#endif
