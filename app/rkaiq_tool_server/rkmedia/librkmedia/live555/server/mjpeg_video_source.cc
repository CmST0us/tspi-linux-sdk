// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mjpeg_video_source.hh"
#include "utils.h"

namespace easymedia
{

    MJPEGVideoSource* MJPEGVideoSource::createNew(UsageEnvironment& env, FramedSource* source)
    {
        return new MJPEGVideoSource(env, source);
    }

    MJPEGVideoSource::MJPEGVideoSource(UsageEnvironment& env, FramedSource* source)
        : JPEGVideoSource(env), m_inputSource(source), m_width(0), m_height(0), m_qTable0Init(false),
          m_qTable1Init(false)
    {
        memset(&m_qTable, 0, sizeof(m_qTable));
    }
    MJPEGVideoSource::~MJPEGVideoSource()
    {
        Medium::close(m_inputSource);
    }
    void MJPEGVideoSource::doGetNextFrame()
    {
        if (m_inputSource) {
            m_inputSource->getNextFrame(fTo, fMaxSize, afterGettingFrame, this, FramedSource::handleClosure, this);
        }
    }

    void MJPEGVideoSource::doStopGettingFrames()
    {
        FramedSource::doStopGettingFrames();
        if (m_inputSource) {
            m_inputSource->stopGettingFrames();
        }
    }
    void MJPEGVideoSource::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                             struct timeval presentationTime, unsigned durationInMicroseconds)
    {
        MJPEGVideoSource* source = (MJPEGVideoSource*)clientData;
        source->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
    }
    void MJPEGVideoSource::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                             struct timeval presentationTime, unsigned durationInMicroseconds)
    {
        int headerSize = 0;
        bool headerOk = false;
        fFrameSize = 0;

        for (unsigned int i = 0; i < frameSize; ++i) {
            // SOF
            if ((i + 8) < frameSize && fTo[i] == 0xFF && fTo[i + 1] == 0xC0) {
                m_height = (fTo[i + 5] << 5) | (fTo[i + 6] >> 3);
                m_width = (fTo[i + 7] << 5) | (fTo[i + 8] >> 3);
            }
            // DQT
            if ((i + 5 + 64) < frameSize && fTo[i] == 0xFF && fTo[i + 1] == 0xDB) {
                if (fTo[i + 4] == 0) {
                    memcpy(m_qTable, fTo + i + 5, 64);
                    m_qTable0Init = true;
                } else if (fTo[i + 4] == 1) {
                    memcpy(m_qTable + 64, fTo + i + 5, 64);
                    m_qTable1Init = true;
                }
            }
            // End of header
            if ((i + 1) < frameSize && fTo[i] == 0x3F && fTo[i + 1] == 0x00) {
                headerOk = true;
                headerSize = i + 2;
                break;
            }
        }

        if (headerOk) {
            fFrameSize = frameSize - headerSize;
            memmove(fTo, fTo + headerSize, fFrameSize);
        }

        fNumTruncatedBytes = numTruncatedBytes;
        fPresentationTime = presentationTime;
        fDurationInMicroseconds = durationInMicroseconds;
        afterGetting(this);
    }

    u_int8_t const* MJPEGVideoSource::quantizationTables(u_int8_t& precision, u_int16_t& length)
    {
        length = 0;
        precision = 0;
        if (m_qTable0Init && m_qTable1Init) {
            precision = 8;
            length = sizeof(m_qTable);
        }
        return m_qTable;
    }

    unsigned char MJPEGVideoSource::type()
    {
        return 1;
    };
    unsigned char MJPEGVideoSource::qFactor()
    {
        return 128;
    };
    unsigned char MJPEGVideoSource::width()
    {
        return m_width;
    };
    unsigned char MJPEGVideoSource::height()
    {
        return m_height;
    };

} // namespace easymedia
