// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow.h"
#include "stream.h"

namespace easymedia
{

    static bool send_buffer(Flow* f, MediaBufferVector& input_vector);

    class OutPutStreamFlow : public Flow
    {
      public:
        OutPutStreamFlow(const char* param);
        virtual ~OutPutStreamFlow()
        {
            AutoPrintLine apl(__func__);
            StopAllThread();
        };
        static const char* GetFlowName()
        {
            return "output_stream";
        }
        virtual int Control(unsigned long int request, ...) final
        {
            if (!out_stream) {
                return -1;
            }
            va_list vl;
            va_start(vl, request);
            void* arg = va_arg(vl, void*);
            va_end(vl);
            return out_stream->IoCtrl(request, arg);
        }

      private:
        std::shared_ptr<Stream> out_stream;
        friend bool send_buffer(Flow* f, MediaBufferVector& input_vector);
    };

    OutPutStreamFlow::OutPutStreamFlow(const char* param)
    {
        std::list<std::string> separate_list;
        std::map<std::string, std::string> params;
        if (!ParseWrapFlowParams(param, params, separate_list)) {
            SetError(-EINVAL);
            return;
        }
        std::string& name = params[KEY_NAME];
        const char* stream_name = name.c_str();
        SlotMap sm;
        int input_maxcachenum = 10;
        ParseParamToSlotMap(params, sm, input_maxcachenum);
        if (sm.thread_model == Model::NONE)
            sm.thread_model = !params[KEY_FPS].empty() ? Model::ASYNCATOMIC : Model::ASYNCCOMMON;
        if (sm.mode_when_full == InputMode::NONE) {
            sm.mode_when_full = InputMode::DROPCURRENT;
        }
        const std::string& stream_param = separate_list.back();
        auto stream = REFLECTOR(Stream)::Create<Stream>(stream_name, stream_param.c_str());
        if (!stream) {
            LOG("Fail to create stream %s\n", stream_name);
            SetError(-EINVAL);
            return;
        }
        sm.input_slots.push_back(0);
        sm.input_maxcachenum.push_back(input_maxcachenum);
        sm.process = send_buffer;
        std::string tag = "OutputStreamFlow:";
        tag.append(stream_name);
        if (!InstallSlotMap(sm, tag, -1)) {
            LOG("Fail to InstallSlotMap for %s\n", tag.c_str());
            SetError(-EINVAL);
            return;
        }
        out_stream = stream;
        SetFlowTag(tag);
    }

    bool send_buffer(Flow* f, MediaBufferVector& input_vector)
    {
        OutPutStreamFlow* flow = static_cast<OutPutStreamFlow*>(f);
        auto& buffer = input_vector[0];
        if (!buffer) {
            return true;
        }
        return flow->out_stream->Write(buffer);
    }

    DEFINE_FLOW_FACTORY(OutPutStreamFlow, Flow)
    // TODO!
    const char* FACTORY(OutPutStreamFlow)::ExpectedInputDataType()
    {
        return "";
    }
    const char* FACTORY(OutPutStreamFlow)::OutPutDataType()
    {
        return nullptr;
    }

} // namespace easymedia
