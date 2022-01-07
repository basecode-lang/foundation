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

#include <basecode/core/types.h>

#define SQL_BEGIN(h) SAFE_SCOPE({                                           \
        s32 r;                                                              \
        r = sqlite3_exec((h), "BEGIN;", nullptr, nullptr, nullptr);         \
        if (r != SQLITE_OK) return status_t::sql_error;                     \
    });

#define SQL_COMMIT(h) SAFE_SCOPE({                                          \
        s32 r;                                                              \
        r = sqlite3_exec((h), "COMMIT;", nullptr, nullptr, nullptr);        \
        if (r != SQLITE_OK) return status_t::sql_error;                     \
    });

#define SQL_RELEASE(h, id) SAFE_SCOPE({                                     \
        auto sql = format::format("RELEASE {};", (id));                     \
        s32 r;                                                              \
        r = sqlite3_exec((h), str::c_str(sql), nullptr, nullptr, nullptr);  \
        if (r != SQLITE_OK) return status_t::sql_error;                     \
    });

#define SQL_SAVEPOINT(h, id) SAFE_SCOPE({                                   \
        auto sql = format::format("SAVEPOINT {};", (id));                   \
        s32 r;                                                              \
        r = sqlite3_exec((h), str::c_str(sql), nullptr, nullptr, nullptr);  \
        if (r != SQLITE_OK) return status_t::sql_error;                     \
    });

#define SQL_ROLLBACK(h) SAFE_SCOPE({                                        \
        s32 r;                                                              \
        r = sqlite3_exec((h), "ROLLBACK;", nullptr, nullptr, nullptr);      \
        if (r != SQLITE_OK) return status_t::sql_error;                     \
    });

#define SQL_ROLLBACK_TO(h, id) SAFE_SCOPE({                                 \
        auto sql = format::format("ROLLBACK TO {};", (id));                 \
        s32 r;                                                              \
        r = sqlite3_exec((h), str::c_str(sql), nullptr, nullptr, nullptr);  \
        if (r != SQLITE_OK) return status_t::sql_error;                     \
    });

#define SQL_FINALIZE_STMT(s) SAFE_SCOPE({                                   \
        if ((s)) sqlite3_finalize((s));                                     \
    });

#define SQL_PREPARE_STMT(h, s, p) SAFE_SCOPE({                              \
        auto rc = sqlite3_prepare_v2((h), (s), -1, &(p), 0);                \
        if (rc != SQLITE_OK) return status_t::sql_error;                    \
    })

#define SQL_PRAGMA(h, p, v) SAFE_SCOPE({                                    \
        auto sql = format::format("PRAGMA {} = {};", (p), (v));             \
        s32 r;                                                              \
        r = sqlite3_exec((h), str::c_str(sql), nullptr, nullptr, nullptr);  \
        if (r != SQLITE_OK) return status_t::sql_error;                     \
    })

