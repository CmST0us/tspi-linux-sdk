// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_LIVE555_MEDIA_INPUT_HH_
#define EASYMEDIA_LIVE555_MEDIA_INPUT_HH_

#include <functional>
#include <list>
#include <memory>
#include <type_traits>

#include <liveMedia/MediaSink.hh>

#include "lock.h"
#include "media_type.h"

namespace easymedia {

class MediaBuffer;
class VideoFramedSource;
class CommonFramedSource;
using ListReductionPtr = std::add_pointer<void(
    void *userdata, std::list<std::shared_ptr<MediaBuffer>> &mb_list)>::type;

// using StartStreamCallback = std::add_pointer<void(void)>::type;
typedef std::function<void()> StartStreamCallback;

class Source {
public:
  Source();
  ~Source();
  bool Init(ListReductionPtr func = nullptr);
  void Push(std::shared_ptr<easymedia::MediaBuffer> &);
  std::shared_ptr<MediaBuffer> Pop();
  int GetReadFd() { return wakeFds[0]; }
  int GetWriteFd() { return wakeFds[1]; }
  Boolean GetReadFdStatus() { return m_read_fd_status; }
  void CloseReadFd();
  unsigned GetCachedBufSize() { return m_cached_buffers_size; }
  void SetCachedBufSize(size_t one_buf_size);

private:
  std::list<std::shared_ptr<MediaBuffer>> cached_buffers;
  ConditionLockMutex mtx;
  ListReductionPtr reduction;
  int wakeFds[2]; // Live555's EventTrigger is poor for multithread, use fds
  unsigned m_cached_buffers_size;
  Boolean m_read_fd_status;
};

class Live555MediaInput : public Medium {
public:
  static Live555MediaInput *createNew(UsageEnvironment &env);
  FramedSource *videoSource(CodecType c_type);
  FramedSource *audioSource();
  FramedSource *muxerSource();
  void Start(UsageEnvironment &env);
  void Stop(UsageEnvironment &env);

  void PushNewVideo(std::shared_ptr<MediaBuffer> &buffer);
  void PushNewAudio(std::shared_ptr<MediaBuffer> &buffer);
  void PushNewMuxer(std::shared_ptr<MediaBuffer> &buffer);

  void SetStartVideoStreamCallback(const StartStreamCallback &cb);
  StartStreamCallback GetStartVideoStreamCallback();
  void SetStartAudioStreamCallback(const StartStreamCallback &cb);
  StartStreamCallback GetStartAudioStreamCallback();

  unsigned getMaxIdrSize();

protected:
  virtual ~Live555MediaInput();

private:
  Live555MediaInput(UsageEnvironment &env);

  std::list<Source *> video_list;
  std::list<Source *> audio_list;
  std::list<Source *> muxer_list;
  volatile bool connecting;

  StartStreamCallback video_callback;
  StartStreamCallback audio_callback;

  ConditionLockMutex video_callback_mtx;
  ConditionLockMutex audio_callback_mtx;

  friend class VideoFramedSource;
  friend class CommonFramedSource;
  unsigned m_max_idr_size;
};

class ListSource : public FramedSource {
protected:
  ListSource(UsageEnvironment &env, Source &source)
      : FramedSource(env), fSource(source) {}
  virtual ~ListSource() {
    if (fSource.GetReadFd() >= 0)
      envir().taskScheduler().turnOffBackgroundReadHandling(
          fSource.GetReadFd());
    fSource.CloseReadFd();
  }

  virtual bool readFromList(bool flush = false) = 0;
  virtual void flush();

  Source &fSource;

private: // redefined virtual functions:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();

  static void incomingDataHandler(ListSource *source, int mask);
  void incomingDataHandler1();
};

class VideoFramedSource : public ListSource {
public:
  VideoFramedSource(UsageEnvironment &env, Source &source);
  virtual ~VideoFramedSource();

  void SetCodecType(CodecType type) { codec_type = type; }
  CodecType GetCodecType() { return codec_type; }

protected: // redefined virtual functions:
  virtual bool readFromList(bool flush = false);
  bool got_iframe;
  CodecType codec_type;
};

class CommonFramedSource : public ListSource {
public:
  CommonFramedSource(UsageEnvironment &env, Source &source);
  virtual ~CommonFramedSource();

protected: // redefined virtual functions:
  virtual bool readFromList(bool flush = false);
};

// Functions to set the optimal buffer size for RTP sink objects.
// These should be called before each RTPSink is created.
#define AUDIO_MAX_FRAME_SIZE 204800
#define VIDEO_MAX_FRAME_SIZE (1920 * 1080 * 2)
inline void setAudioRTPSinkBufferSize() {
  OutPacketBuffer::maxSize = AUDIO_MAX_FRAME_SIZE;
}
inline void setVideoRTPSinkBufferSize() {
  OutPacketBuffer::maxSize = VIDEO_MAX_FRAME_SIZE;
}

} // namespace easymedia

#endif // #ifndef EASYMEDIA_LIVE555_MEDIA_INPUT_HH_
