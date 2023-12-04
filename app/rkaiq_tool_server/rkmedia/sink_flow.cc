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

    class _API SinkFlow : public Flow
    {
      public:
        SinkFlow(const char* param);
        virtual ~SinkFlow();
        static const char* GetFlowName()
        {
            return "sink_flow";
        }

      private:
        friend bool process_buffer(Flow* f, MediaBufferVector& input_vector);

      private:
        LinkType link_type_;
        std::string extra_data;
    };

    SinkFlow::SinkFlow(const char* param)
    {
        std::map<std::string, std::string> params;
        if (!parse_media_param_map(param, params)) {
            SetError(-EINVAL);
            return;
        }

        SetVideoHandler(nullptr);
        SetAudioHandler(nullptr);
        SetCaptureHandler(nullptr);

        SlotMap sm;
        sm.input_slots.push_back(0);
        if (sm.thread_model == Model::NONE)
            sm.thread_model = !params[KEY_FPS].empty() ? Model::ASYNCATOMIC : Model::ASYNCCOMMON;
        if (sm.mode_when_full == InputMode::NONE) {
            sm.mode_when_full = InputMode::DROPCURRENT;
        }

        sm.input_maxcachenum.push_back(0);
        sm.process = process_buffer;

        if (!InstallSlotMap(sm, "SinkFlow", 0)) {
            LOG("Fail to InstallSlotMap for SinkFlow\n");
            return;
        }
        SetFlowTag("SinkFlow");
        std::string& type = params[KEY_INPUTDATATYPE];

        link_type_ = LINK_VIDEO;
    }

    SinkFlow::~SinkFlow()
    {
        StopAllThread();
    }

    bool process_buffer(Flow* f, MediaBufferVector& input_vector)
    {
        SinkFlow* flow = static_cast<SinkFlow*>(f);
        auto& buffer = input_vector[0];
        if (!buffer && !flow) {
            return true;
        }

        LOG("SinkFlow process_buffer \n");

        if (flow->link_type_ == LINK_VIDEO) {
            auto link_handler = flow->GetVideoHandler();
            if (link_handler)
                link_handler((unsigned char*)buffer->GetPtr(), buffer->GetValidSize(), 0, 0);
        }

        return 0;
    }

    DEFINE_FLOW_FACTORY(SinkFlow, Flow)
    const char* FACTORY(SinkFlow)::ExpectedInputDataType()
    {
        return nullptr;
    }
    const char* FACTORY(SinkFlow)::OutPutDataType()
    {
        return "";
    }

} // namespace easymedia
