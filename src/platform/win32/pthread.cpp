// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//      F O U N D A T I O N   P R O J E C T
//
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <ctime>
#include <string>
#include <pthread.h>

static DWORD timespec_to_ms(const timespec* abstime) {
    DWORD t;
    if (!abstime)
        return INFINITE;
    t = ((abstime->tv_sec - time(nullptr)) * 1000) + (abstime->tv_nsec / 1000000);
    if (t < 0)
        t = 1;
    return t;
}

pthread_t pthread_self() {
    pthread_t self;
    auto proc = GetCurrentProcess();
    if (!DuplicateHandle(proc,
                         GetCurrentThread(),
                         proc,
                         &self,
                         0,
                         FALSE,
                         DUPLICATE_SAME_ACCESS)) {
        return nullptr;
    }
    return self;
}

int pthread_getthreadid_np() {
    return GetCurrentThreadId();
}

int pthread_setname_np(pthread_t thread, const char* name) {
    if (!name) return EINVAL;
    const auto name_len = strlen(name);
    const auto new_size = MultiByteToWideChar(CP_UTF8,
                                              0,
                                              name,
                                              name_len,
                                              nullptr,
                                              0);
    std::wstring temp(new_size, 0);
    MultiByteToWideChar(CP_UTF8, 0, name, name_len, temp.data(), new_size);
    SetThreadDescription(thread, temp.c_str());
    return 0;
}

int pthread_create(pthread_t* thread,
                   pthread_attr_t* attr,
                   void* (* start_routine)(void*),
                   void* arg) {
    UNUSED(attr);
    if (!thread || !start_routine)
        return EINVAL;
    *thread = CreateThread(
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE) start_routine,
        arg,
        0,
        nullptr);
    return *thread ? 0 : 1;
}

int pthread_join(pthread_t thread, void** value_ptr) {
    UNUSED(value_ptr);
    WaitForSingleObject(thread, INFINITE);
    return CloseHandle(thread) ? 0 : 1;
}

void pthread_exit(void* retval) {
    ExitThread(retval ? *((DWORD*)retval) : 0);
}

int pthread_cancel(pthread_t thread) {
    UNUSED(thread);
    return 0;
}

int pthread_detach(pthread_t thread) {
    return CloseHandle(thread) ? 0 : 1;
}

int pthread_equal(pthread_t t1, pthread_t t2) {
    return t1 == t2;
}

int pthread_mutex_init(pthread_mutex_t* mutex, pthread_mutexattr_t* attr) {
    UNUSED(attr);
    if (!mutex)
        return EINVAL;
    InitializeCriticalSection(mutex);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex) {
    if (!mutex) return EINVAL;
    DeleteCriticalSection(mutex);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t* mutex) {
    if (!mutex) return EINVAL;
    return TryEnterCriticalSection(mutex) ? 0 : 1;
}

int pthread_mutex_lock(pthread_mutex_t* mutex) {
    if (!mutex) return EINVAL;
    EnterCriticalSection(mutex);
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t* mutex) {
    if (!mutex) return EINVAL;
    LeaveCriticalSection(mutex);
    return 0;
}

int pthread_cond_init(pthread_cond_t* cond, pthread_condattr_t* attr) {
    UNUSED(attr);
    if (!cond) return EINVAL;
    InitializeConditionVariable(cond);
    return 0;
}

int pthread_cond_destroy(pthread_cond_t* cond) {
    UNUSED(cond);
    return 0;
}

int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    if (!cond || !mutex) return EINVAL;
    return pthread_cond_timedwait(cond, mutex, nullptr);
}

int pthread_cond_timedwait(pthread_cond_t* cond,
                           pthread_mutex_t* mutex,
                           const struct timespec* abstime) {
    if (!cond || !mutex) return EINVAL;
    const auto sleep_ms = timespec_to_ms(abstime);
    if (!SleepConditionVariableCS(cond, mutex, sleep_ms))
        return 1;
    return 0;
}

int pthread_cond_signal(pthread_cond_t* cond) {
    if (!cond) return EINVAL;
    WakeConditionVariable(cond);
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t* cond) {
    if (!cond) return EINVAL;
    WakeAllConditionVariable(cond);
    return 0;
}

int pthread_rwlock_init(pthread_rwlock_t* rwlock,
                        const pthread_rwlockattr_t* attr) {
    UNUSED(attr);
    if (!rwlock) return EINVAL;
    InitializeSRWLock(&(rwlock->lock));
    rwlock->exclusive = false;
    return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t* rwlock) {
    UNUSED(rwlock);
    return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock) {
    if (!rwlock) return EINVAL;
    AcquireSRWLockShared(&rwlock->lock);
    return 0;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t* rwlock) {
    if (!rwlock) return EINVAL;
    return !TryAcquireSRWLockShared(&rwlock->lock);
}

int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock) {
    if (!rwlock) return EINVAL;
    AcquireSRWLockExclusive(&rwlock->lock);
    rwlock->exclusive = true;
    return 0;
}

int pthread_rwlock_trywrlock(pthread_rwlock_t* rwlock) {
    BOOLEAN ret;
    if (!rwlock)
        return EINVAL;
    ret = TryAcquireSRWLockExclusive(&(rwlock->lock));
    if (ret)
        rwlock->exclusive = true;
    return ret;
}

int pthread_rwlock_unlock(pthread_rwlock_t* rwlock) {
    if (!rwlock)
        return EINVAL;
    if (rwlock->exclusive) {
        rwlock->exclusive = false;
        ReleaseSRWLockExclusive(&(rwlock->lock));
    } else {
        ReleaseSRWLockShared(&(rwlock->lock));
    }
    return 0;
}

void ms_to_timespec(struct timespec* ts, unsigned int ms) {
    if (!ts) return;
    ts->tv_sec = (ms / 1000) + time(nullptr);
    ts->tv_nsec = ms % 1000 * 1000000;
}
