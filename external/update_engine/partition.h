/*************************************************************************
	> File Name: partition.h
	> Author: jkand.huang
	> Mail: jkand.huang@rock-chips.com
	> Created Time: Fri 26 Oct 2018 11:20:44 AM CST
 ************************************************************************/

#ifndef _PARTITION_H
#define _PARTITION_H
int writeDataToPartition(struct ImageData *data);
bool writeImageToPartition(const char* path);
int getNowSlot();
#endif
