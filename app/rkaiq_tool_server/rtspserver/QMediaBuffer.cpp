#include <stdlib.h>
#include "QMediaBuffer.h"

QMediaBuffer::QMediaBuffer()
{
    baseInit();
}

QMediaBuffer::QMediaBuffer(void* data, int size)
{
    baseInit();
    setData(data, size);
}

QMediaBuffer::QMediaBuffer(void* data, int size, int fd)
{
    baseInit();
    mFd = fd;
    setData(data, size);
}

void QMediaBuffer::baseInit()
{
    mData = NULL;
    mSize = 0;
    mFd = -1;
    mBufferID = -1;
    mPrivateData = NULL;
}

void QMediaBuffer::setData(void* data, int size)
{
    mData = data;
    mSize = size;
}

void QMediaBuffer::setFd(int fd)
{
    mFd = fd;
}

void QMediaBuffer::setBufferID(int id)
{
    mBufferID = id;
}

void QMediaBuffer::setPrivateData(void* data)
{
    mPrivateData = data;
}

void* QMediaBuffer::getData()
{
    return mData;
}

int QMediaBuffer::getSize()
{
    return mSize;
}

int QMediaBuffer::getFd()
{
    return mFd;
}

int QMediaBuffer::getBufferID()
{
    return mBufferID;
}

void* QMediaBuffer::getPrivateData()
{
    return mPrivateData;
}
