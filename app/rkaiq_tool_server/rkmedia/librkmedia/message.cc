// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message.h"

#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "key_string.h"
#include "utils.h"

namespace easymedia
{

    void EventHandler::RegisterEventHook(std::shared_ptr<easymedia::Flow> flow, EventHook proc)
    {
        process_ = proc;
        event_thread_loop_ = true;
        event_thread_.reset(new std::thread(process_, flow, std::ref(event_thread_loop_)));
    }

    void EventHandler::UnRegisterEventHook()
    {
        if (event_thread_) {
            event_thread_loop_ = false;
            event_cond_mtx_.lock();
            event_cond_mtx_.notify();
            event_cond_mtx_.unlock();
            event_thread_->join();
            event_thread_.reset(nullptr);
        }
    }

    void EventHandler::EventHookWait()
    {
        AutoLockMutex _signal_mtx(event_cond_mtx_);
        event_cond_mtx_.wait();
    }

    void EventHandler::SignalEventHook()
    {
        AutoLockMutex _signal_mtx(event_cond_mtx_);
        event_cond_mtx_.notify();
    }

    MessagePtr EventHandler::GetEventMessage()
    {
        AutoLockMutex _rw_mtx(event_queue_mtx_);
        if (process_) {
            if (event_msgs_.empty()) {
                return nullptr;
            }
            auto msg = event_msgs_.front();
            event_msgs_.erase(event_msgs_.begin());
            return msg;
        }
        return nullptr;
    }

    void EventHandler::CleanRepeatMessage(MessagePtr msg)
    {
        for (auto iter = event_msgs_.cbegin(); iter != event_msgs_.cend();) {
            auto tmp = *iter;
            auto param = tmp->GetEventParam();
            if (param->GetId() == msg->GetEventParam()->GetId()) {
                iter = event_msgs_.erase(iter);
            } else {
                iter++;
            }
        }
    }

    void EventHandler::InsertMessage(MessagePtr msg, bool front)
    {
        if (front) {
            auto iter = event_msgs_.begin();
            iter = event_msgs_.insert(iter, msg);
        } else {
            event_msgs_.push_back(msg);
        }
    }

    void EventHandler::NotifyToEventHandler(MessagePtr msg)
    {
        bool inser_front = false;
        AutoLockMutex _rw_mtx(event_queue_mtx_);
        if (process_) {
            if (msg->GetType() == MESSAGE_TYPE_UNIQUE) {
                CleanRepeatMessage(msg);
            } else if (msg->GetType() == MESSAGE_TYPE_LIFO) {
                inser_front = true;
                ;
            }
            InsertMessage(msg, inser_front);
        }
    }

} // namespace easymedia
