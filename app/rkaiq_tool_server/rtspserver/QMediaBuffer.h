#ifndef QMEDIA_BUFFER_H_
#define QMEDIA_BUFFER_H_

class QMediaBuffer
{
  public:
    QMediaBuffer();
    QMediaBuffer(void* data, int size);
    QMediaBuffer(void* data, int size, int fd);

    void baseInit();

    ~QMediaBuffer()
    {
    }

    void* getData();
    int getSize();
    int getFd();
    int getBufferID();
    void* getPrivateData();

    void setData(void* data, int size);
    void setFd(int fd);
    void setBufferID(int id);
    void setPrivateData(void* data);

  private:
    void* mData;
    int mSize;
    int mFd;
    int mBufferID;

    void* mPrivateData;
};

#endif // QMEDIA_BUFFER_H_
