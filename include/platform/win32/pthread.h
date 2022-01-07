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

#pragma once

#ifdef _MSC_VER
#include <basecode/core/types.h>
#include <windows.h>

typedef CRITICAL_SECTION    pthread_mutex_t;
typedef void                pthread_attr_t;
typedef void                pthread_mutexattr_t;
typedef void                pthread_condattr_t;
typedef void                pthread_rwlockattr_t;
typedef HANDLE              pthread_t;
typedef CONDITION_VARIABLE  pthread_cond_t;

typedef struct {
    SRWLOCK                 lock;
    bool                    exclusive;
} pthread_rwlock_t;

pthread_t pthread_self();

int pthread_getthreadid_np();

int pthread_detach(pthread_t);

int pthread_create(pthread_t *thread,
                   pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);

void pthread_exit(void* retval);

int pthread_cancel(pthread_t thread);

int pthread_equal(pthread_t t1, pthread_t t2);

int pthread_cond_signal(pthread_cond_t *cond);

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_cond_destroy(pthread_cond_t *cond);

int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_mutex_trylock(pthread_mutex_t *mutex);

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

int pthread_join(pthread_t thread, void **value_ptr);

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);

int pthread_rwlock_trywrlock(pthread_rwlock_t  *rwlock);

int pthread_rwlock_init(pthread_rwlock_t *rwlock,
                        const pthread_rwlockattr_t *attr);

void ms_to_timespec(struct timespec *ts, unsigned int ms);

int pthread_setname_np(pthread_t thread, const char* name);

int pthread_cond_timedwait(pthread_cond_t *cond,
                           pthread_mutex_t *mutex,
                           const struct timespec *abstime);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);

int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
#else
#   include_next <pthread.h>
#endif
