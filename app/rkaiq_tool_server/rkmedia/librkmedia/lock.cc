// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lock.h"

#include <stdio.h>

namespace easymedia
{

    LockMutex::LockMutex()
#ifndef NDEBUG
        : lock_times(0)
#endif
    {
    }
    LockMutex::~LockMutex()
    {
#ifndef NDEBUG
        assert(lock_times == 0 && "mutex lock/unlock mismatch");
#endif
    }

    void LockMutex::locktimeinc()
    {
#ifndef NDEBUG
        lock_times++;
#endif
    }
    void LockMutex::locktimedec()
    {
#ifndef NDEBUG
        lock_times--;
#endif
    }

    void ConditionLockMutex::lock()
    {
        mtx.lock();
        locktimeinc();
    }
    void ConditionLockMutex::unlock()
    {
        locktimedec();
        mtx.unlock();
    }
    void ConditionLockMutex::wait()
    {
        cond.wait(mtx);
    }
    void ConditionLockMutex::notify()
    {
        cond.notify_all();
    }

    ReadWriteLockMutex::ReadWriteLockMutex() : valid(true)
    {
        int ret = pthread_rwlock_init(&rwlock, NULL);
        if (ret) {
            fprintf(stderr, "Fail to pthread_rwlock_init\n");
            valid = false;
        }
    }
    ReadWriteLockMutex::~ReadWriteLockMutex()
    {
        if (valid) {
            pthread_rwlock_destroy(&rwlock);
        }
    }
    void ReadWriteLockMutex::lock()
    {
        // write lock
        if (valid) {
            pthread_rwlock_wrlock(&rwlock);
        }
        locktimeinc();
    }
    void ReadWriteLockMutex::unlock()
    {
        locktimedec();
        if (valid) {
            pthread_rwlock_unlock(&rwlock);
        }
    }
    void ReadWriteLockMutex::read_lock()
    {
        if (valid) {
            pthread_rwlock_rdlock(&rwlock);
        }
        locktimeinc();
    }

    SpinLockMutex::SpinLockMutex() : flag(ATOMIC_FLAG_INIT)
    {
    }
    void SpinLockMutex::lock()
    {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
        locktimeinc();
    }
    void SpinLockMutex::unlock()
    {
        locktimedec();
        flag.clear(std::memory_order_release);
    }

} // namespace easymedia
