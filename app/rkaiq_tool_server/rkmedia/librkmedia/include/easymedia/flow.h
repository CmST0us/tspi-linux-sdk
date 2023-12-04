// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_FLOW_H_
#define EASYMEDIA_FLOW_H_

#include <stdarg.h>

#include <deque>
#include <thread>
#include <type_traits>
#include <vector>

#include "control.h"
#include "lock.h"
#include "message.h"
#include "reflector.h"
#include "utils.h"

namespace easymedia
{

    DECLARE_FACTORY(Flow)
    // usage: REFLECTOR(Flow)::Create<T>(flowname, param)
    // T must be the final class type exposed to user
    DECLARE_REFLECTOR(Flow)

#define DEFINE_FLOW_FACTORY(REAL_PRODUCT, FINAL_EXPOSE_PRODUCT)                                                        \
    DEFINE_MEDIA_CHILD_FACTORY(REAL_PRODUCT, REAL_PRODUCT::GetFlowName(), FINAL_EXPOSE_PRODUCT, Flow)                  \
    DEFINE_MEDIA_CHILD_FACTORY_EXTRA(REAL_PRODUCT)                                                                     \
    DEFINE_MEDIA_NEW_PRODUCT_BY(REAL_PRODUCT, FINAL_EXPOSE_PRODUCT, GetError() < 0)

    class MediaBuffer;
    enum class Model
    {
        NONE,
        ASYNCCOMMON,
        ASYNCATOMIC,
        SYNC
    };
    // PushMode
    enum class InputMode
    {
        NONE,
        BLOCKING,
        DROPFRONT,
        DROPCURRENT
    };
    enum class HoldInputMode
    {
        NONE,
        HOLD_INPUT,
        INHERIT_FORM_INPUT
    };
    using MediaBufferVector = std::vector<std::shared_ptr<MediaBuffer>>;
    // TODO: outputs ret, outslot index, outslot queue model
    using FunctionProcess = std::add_pointer<bool(Flow* f, MediaBufferVector& input_vector)>::type;
    template <int in_index, int out_index> bool void_transaction(Flow* f, MediaBufferVector& input_vector);
    using LinkVideoHandler = std::add_pointer<void(unsigned char* buffer, unsigned int buffer_size,
                                                   int64_t present_time, int nat_type)>::type;
    using LinkAudioHandler =
        std::add_pointer<void(unsigned char* buffer, unsigned int buffer_size, int64_t present_time)>::type;
    using LinkCaptureHandler =
        std::add_pointer<void(unsigned char* buffer, unsigned int buffer_size, int type, uint32_t sequence)>::type;
    using PlayVideoHandler = std::add_pointer<void(Flow* f)>::type;
    using PlayAudioHandler = std::add_pointer<void(Flow* f)>::type;
    using CallBackHandler = std::add_pointer<void>::type;
    using UserCallBack = std::add_pointer<void(void* handler, int type, void* ptr, int size)>::type;
    using OutputCallBack = std::add_pointer<void(void* handler, std::shared_ptr<MediaBuffer> mb)>::type;
    using EventCallBack = std::add_pointer<void(void* handler, void* data)>::type;

    class _API SlotMap
    {
      public:
        SlotMap() : thread_model(Model::SYNC), mode_when_full(InputMode::DROPFRONT), process(nullptr), interval(16.66f)
        {
        }
        std::vector<int> input_slots;
        Model thread_model;
        InputMode mode_when_full;
        std::vector<bool> fetch_block; // if ASYNCCOMMON
        std::vector<int> input_maxcachenum;
        std::vector<int> output_slots;
        // std::vector<DataSetModel> output_ds_model;
        std::vector<HoldInputMode> hold_input;
        FunctionProcess process;
        float interval;
    };

    class FlowCoroutine;
    class _API Flow
    {
      public:
        // We may need a flow which can be sync and async.
        // This make a side effect that sync flow contains superfluous variables
        // designed for async implementation.
        Flow();
        virtual ~Flow();
        static const char* GetFlowName()
        {
            return nullptr;
        }
        // The GetFlowName interface is occupied by the reflector,
        // so GetFlowTag is used to distinguish Flow.
        const char* GetFlowTag()
        {
            return flow_tag.c_str();
        }
        void SetFlowTag(std::string tag)
        {
            flow_tag = tag;
        }

        // TODO: Right now out_slot_index and in_slot_index is decided by exact
        //       subclass, automatically get these value or ignore them in future.
        bool AddDownFlow(std::shared_ptr<Flow> down, int out_slot_index, int in_slot_index_of_down);
        void RemoveDownFlow(std::shared_ptr<Flow> down);

        void SendInput(std::shared_ptr<MediaBuffer>& input, int in_slot_index);
        void SetDisable()
        {
            enable = false;
        }

        // The Control must be called in the same thread to that create flow
        virtual int Control(unsigned long int request _UNUSED, ...)
        {
            return -1;
        }
        virtual int SubControl(unsigned long int request, void* arg, int size = 0)
        {
            SubRequest subreq = {request, size, arg};
            return Control(S_SUB_REQUEST, &subreq);
        }

        // get input size for this flow
        virtual int GetInputSize()
        {
            return 0;
        }

        // The global event hander is the same thread to the born thread of this
        // object.
        void RegisterEventHandler(std::shared_ptr<Flow> flow, EventHook proc);
        void UnRegisterEventHandler();
        void EventHookWait();
        void NotifyToEventHandler(EventParamPtr param, int type = MESSAGE_TYPE_FIFO);
        void NotifyToEventHandler(int id, int type = MESSAGE_TYPE_FIFO);
        MessagePtr GetEventMessage();
        EventParamPtr GetEventParam(MessagePtr msg);

        // Add Link hander For app Link
        void SetVideoHandler(LinkVideoHandler hander)
        {
            link_video_handler_ = hander;
        }
        LinkVideoHandler GetVideoHandler()
        {
            return link_video_handler_;
        }
        void SetAudioHandler(LinkAudioHandler hander)
        {
            link_audio_handler_ = hander;
        }
        LinkAudioHandler GetAudioHandler()
        {
            return link_audio_handler_;
        }
        void SetCaptureHandler(LinkCaptureHandler hander)
        {
            link_capture_handler_ = hander;
        }
        LinkCaptureHandler GetCaptureHandler()
        {
            return link_capture_handler_;
        }

        // Add hander For rtsp flow
        void SetPlayVideoHandler(PlayVideoHandler handler)
        {
            play_video_handler_ = handler;
        }
        PlayVideoHandler GetPlayVideoHandler()
        {
            return play_video_handler_;
        }
        void SetPlayAudioHandler(PlayAudioHandler handler)
        {
            play_audio_handler_ = handler;
        }
        PlayAudioHandler GetPlayAudioHandler()
        {
            return play_audio_handler_;
        }

        // Add common hander for user
        void SetUserCallBack(CallBackHandler handler, UserCallBack callback)
        {
            user_handler_ = handler;
            user_callback_ = callback;
        }
        void SetOutputCallBack(CallBackHandler handler, OutputCallBack callback)
        {
            out_handler_ = handler;
            out_callback_ = callback;
        }
        void SetEventCallBack(CallBackHandler handler, EventCallBack callback)
        {
            event_handler2_ = handler;
            event_callback_ = callback;
        }
        CallBackHandler GetUserHandler()
        {
            return user_handler_;
        }
        UserCallBack GetUserCallBack()
        {
            return user_callback_;
        }

        // Control the number of executions of threads inside Flow
        // _run_times: -1, Endless loop; 0, skip process; > 0, do process cnt.
        int SetRunTimes(int _run_times);
        int GetRunTimesRemaining();

        bool IsAllBuffEmpty();
        void DumpBase(std::string& dump_info);
        virtual void Dump(std::string& dump_info)
        {
            DumpBase(dump_info);
        }

        void StartStream();

      protected:
        class FlowInputMap
        {
          public:
            FlowInputMap(std::shared_ptr<Flow>& f, int i) : flow(f), index_of_in(i)
            {
            }
            std::shared_ptr<Flow> flow; // weak_ptr?
            int index_of_in;
            bool operator==(const std::shared_ptr<easymedia::Flow> f)
            {
                return flow == f;
            }
        };
        class FlowMap
        {
          private:
            void SetOutputBehavior(const std::shared_ptr<MediaBuffer>& output);
            void SetOutputToQueueBehavior(const std::shared_ptr<MediaBuffer>& output);

          public:
            FlowMap() : valid(false), hold_input(HoldInputMode::NONE)
            {
                assert(list_mtx.valid);
            }
            FlowMap(FlowMap&&);
            void Init(Model m, HoldInputMode hold_in);
            bool valid;
            HoldInputMode hold_input;
            // down flow
            void AddFlow(std::shared_ptr<Flow> flow, int index);
            void RemoveFlow(std::shared_ptr<Flow> flow);
            std::list<FlowInputMap> flows;
            ReadWriteLockMutex list_mtx;
            std::deque<std::shared_ptr<MediaBuffer>> cached_buffers; // never drop
            std::shared_ptr<MediaBuffer> cached_buffer;
            decltype(&FlowMap::SetOutputBehavior) set_output_behavior;
        };
        class Input
        {
          private:
            void SyncSendInputBehavior(std::shared_ptr<MediaBuffer>& input);
            void ASyncSendInputCommonBehavior(std::shared_ptr<MediaBuffer>& input);
            void ASyncSendInputAtomicBehavior(std::shared_ptr<MediaBuffer>& input);
            // behavior when input list exceed max_cache_num
            bool ASyncFullBlockingBehavior(volatile bool& pred);
            bool ASyncFullDropFrontBehavior(volatile bool& pred);
            bool ASyncFullDropCurrentBehavior(volatile bool& pred);

          public:
            Input() : valid(false), flow(nullptr), fetch_block(true)
            {
            }
            Input(Input&&);
            void Init(Flow* f, Model m, int mcn, InputMode im, bool f_block, std::shared_ptr<FlowCoroutine> fc);
            bool valid;
            Flow* flow;
            Model thread_model;
            bool fetch_block;
            std::deque<std::shared_ptr<MediaBuffer>> cached_buffers;
            ConditionLockMutex mtx;
            int max_cache_num;
            InputMode mode_when_full;
            std::shared_ptr<MediaBuffer> cached_buffer;
            SpinLockMutex spin_mtx;
            decltype(&Input::SyncSendInputBehavior) send_input_behavior;
            decltype(&Input::ASyncFullBlockingBehavior) async_full_behavior;
            std::shared_ptr<FlowCoroutine> coroutine;
        };

        // Can not change the following values after initialize,
        // except AddFlow and RemoveFlow.
        int out_slot_num;
        std::vector<FlowMap> downflowmap;
        int input_slot_num;
        std::vector<Input> v_input;
        std::list<std::shared_ptr<FlowCoroutine>> coroutines;
        std::shared_ptr<ConditionLockMutex> source_start_cond_mtx;

        int down_flow_num;
        bool waite_down_flow;

        // source flow
        bool SetAsSource(const std::vector<int>& output_slots, FunctionProcess f, const std::string& mark);
        bool InstallSlotMap(SlotMap& map, const std::string& mark, int exp_process_time);
        bool SetOutput(const std::shared_ptr<MediaBuffer>& output, int out_slot_index);
        bool ParseWrapFlowParams(const char* param, std::map<std::string, std::string>& flow_params,
                                 std::list<std::string>& sub_param_list);
        // As sub threads may call the variable of child class,
        // we should define this for child class when it deconstruct.
        void StopAllThread();
        bool IsEnable()
        {
            return enable;
        }

        template <int in_index, int out_index> friend bool void_transaction(Flow* f, MediaBufferVector& input_vector)
        {
            return f->SetOutput(input_vector[in_index], out_index);
        }
        static const FunctionProcess void_transaction00;

        CallBackHandler event_handler2_;
        EventCallBack event_callback_;

      private:
        volatile bool enable;
        volatile bool quit;
        ConditionLockMutex cond_mtx;

        // event handler
        std::unique_ptr<EventHandler> event_handler_;

        friend class FlowCoroutine;

        LinkVideoHandler link_video_handler_;
        LinkAudioHandler link_audio_handler_;
        LinkCaptureHandler link_capture_handler_;

        PlayVideoHandler play_video_handler_;
        PlayAudioHandler play_audio_handler_;

        CallBackHandler user_handler_;
        UserCallBack user_callback_;

        CallBackHandler out_handler_;
        OutputCallBack out_callback_;

        // FlowTag is used to distinguish Flow.
        std::string flow_tag;

        // Control the number of executions of threads inside Flow
        int run_times;

        DEFINE_ERR_GETSET()
        DECLARE_PART_FINAL_EXPOSE_PRODUCT(Flow)
    };

    std::string gen_datatype_rule(std::map<std::string, std::string>& params);
    Model GetModelByString(const std::string& model);
    InputMode GetInputModelByString(const std::string& in_model);
    _API void ParseParamToSlotMap(std::map<std::string, std::string>& params, SlotMap& sm, int& input_maxcachenum);
    size_t FlowOutputHoldInput(std::shared_ptr<MediaBuffer>& out_buffer, const MediaBufferVector& input_vector);
    size_t FlowOutputInheritFromInput(std::shared_ptr<MediaBuffer>& out_buffer, const MediaBufferVector& input_vector);

// the separator of flow params and flow core element params
#define FLOW_PARAM_SEPARATE_CHAR ' '
    _API std::string JoinFlowParam(const std::string& flow_param, size_t num_elem, ...);
    _API std::list<std::string> ParseFlowParamToList(const char* param);

} // namespace easymedia

#endif // #ifndef EASYMEDIA_FLOW_H_
