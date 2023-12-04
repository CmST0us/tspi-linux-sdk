// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MJPEG_VIDEO_SOURCE_HH_
#define EASYMEDIA_MJPEG_VIDEO_SOURCE_HH_

#include "live555_media_input.hh"
#include <liveMedia/JPEGVideoSource.hh>
#include <liveMedia/MediaSink.hh>

namespace easymedia {
class MJPEGVideoSource : public JPEGVideoSource {
public:
  static MJPEGVideoSource *createNew(UsageEnvironment &env,
                                     FramedSource *source);

  virtual unsigned char type();
  virtual unsigned char qFactor();
  virtual unsigned char width();
  virtual unsigned char height();

protected:
  MJPEGVideoSource(UsageEnvironment &env, FramedSource *source);
  virtual ~MJPEGVideoSource();
  static void afterGettingFrame(void *clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds);

private:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();

  virtual void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                 struct timeval presentationTime,
                                 unsigned durationInMicroseconds);
  virtual u_int8_t const *quantizationTables(u_int8_t &precision,
                                             u_int16_t &length);

  FramedSource *m_inputSource;
  unsigned char m_width;
  unsigned char m_height;
  unsigned char m_qTable[128];
  bool m_qTable0Init;
  bool m_qTable1Init;
};
} // namespace easymedia

#endif // #ifndef EASYMEDIA_MJPEG_VIDEO_SOURCE_HH_
