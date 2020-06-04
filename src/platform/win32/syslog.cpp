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
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <cstdio>
#include <fcntl.h>
#include <syslog.h>
#include <process.h>

static s8*     log_header{};
static HANDLE  log_source = INVALID_HANDLE_VALUE;

u0 closelog() {
    if (log_source != INVALID_HANDLE_VALUE) {
        DeregisterEventSource(log_source);
        log_source = INVALID_HANDLE_VALUE;
    }
    if (log_header) {
        free(log_header);
        log_header = nullptr;
    }
}

s32 setlogmask(s32 mask) {
    UNUSED(mask);
    return 0;
}

u0 syslog(s32 priority, const s8* message, ...) {
    va_list args;
    va_start(args, message);
    vsyslog(priority, message, args);
    va_end(args);
}

u0 vsyslog(s32 priority, const s8* message, va_list args) {
    LPCSTR log_strings[2];
    WORD   event_type;
    DWORD  event_id{};

    if (log_source == INVALID_HANDLE_VALUE)
        openlog("basecode", LOG_PID, LOG_SYSLOG);

    switch (priority) {
        case LOG_ALERT:
            event_type = EVENTLOG_ERROR_TYPE;
            break;
        case LOG_INFO:
            event_type = EVENTLOG_INFORMATION_TYPE;
            break;
        default:
            event_type = EVENTLOG_WARNING_TYPE;
    }

    s8 temp[256];
    vsnprintf(temp, 255, message, args);
    log_strings[0] = log_header;
    log_strings[1] = temp;

    ReportEventA(log_source, event_type, (WORD) priority, event_id, nullptr, 2, 0, log_strings, nullptr);
}

u0 openlog(const s8* ident, s32 logopt, s32 facility) {
    size_t header_len;
    closelog();
    log_source = RegisterEventSource(nullptr, "basecode");
    header_len = strlen(ident) + 2 + 11;
    log_header = (s8*) malloc(header_len*sizeof(char));
    sprintf_s(log_header, header_len, logopt & LOG_PID ? "%s[%d]" : "%s", ident, getpid());
}
