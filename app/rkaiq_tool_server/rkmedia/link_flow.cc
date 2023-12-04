// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "buffer.h"
#include "flow.h"
#include "link_config.h"
#include "stream.h"
#include "utils.h"

namespace easymedia
{

    static bool process_buffer(Flow* f, MediaBufferVector& input_vector);

    class _API LinkFlow : public Flow
    {
      public:
        LinkFlow(const char* param);
        virtual ~LinkFlow();
        static const char* GetFlowName()
        {
            return "link_flow";
        }
        int Control(unsigned long int request, ...);

      private:
        friend bool process_buffer(Flow* f, MediaBufferVector& input_vector);

      private:
        int enable;
        int socket_fd;
    };

    LinkFlow::LinkFlow(const char* param)
    {
        std::map<std::string, std::string> params;
        if (!parse_media_param_map(param, params)) {
            SetError(-EINVAL);
            return;
        }

        SetVideoHandler(nullptr);
        SetAudioHandler(nullptr);
        SetCaptureHandler(nullptr);
        SetUserCallBack(nullptr, nullptr);

        SlotMap sm;
        sm.input_slots.push_back(0);
        sm.thread_model = Model::ASYNCCOMMON;
        sm.mode_when_full = InputMode::DROPCURRENT;
        sm.input_maxcachenum.push_back(0);
        sm.process = process_buffer;

        if (!InstallSlotMap(sm, "LinkFLow", 0)) {
            LOG("Fail to InstallSlotMap for LinkFLow\n");
            return;
        }
        SetFlowTag("LinkFLow");
    }

    LinkFlow::~LinkFlow()
    {
        StopAllThread();
    }

    bool process_buffer(Flow* f, MediaBufferVector& input_vector)
    {
        LinkFlow* flow = static_cast<LinkFlow*>(f);
        auto& buffer = input_vector[0];
        if (!buffer || !flow) {
            return false;
        }

        if (flow->enable > 0) {
            auto link_handler = flow->GetCaptureHandler();
            if (link_handler) {
                link_handler((unsigned char*)buffer->GetPtr(), buffer->GetValidSize(), flow->socket_fd,
                             buffer->GetFrameSequenceNumber());
                flow->enable--;
            }
        }
        return false;
    }

    static const uint32_t kSocket_fd = (1 << 0);
    static const uint32_t kEnable_Link = (1 << 1);

    int LinkFlow::Control(unsigned long int request, ...)
    {
        va_list ap;
        va_start(ap, request);
        auto value = va_arg(ap, int);
        va_end(ap);
        assert(value);

        switch (request) {
            case kSocket_fd:
                socket_fd = value;
                break;
            case kEnable_Link:
                enable = value;
                break;
        }

        return 0;
    }

    DEFINE_FLOW_FACTORY(LinkFlow, Flow)
    const char* FACTORY(LinkFlow)::ExpectedInputDataType()
    {
        return nullptr;
    }
    const char* FACTORY(LinkFlow)::OutPutDataType()
    {
        return "";
    }

} // namespace easymedia
