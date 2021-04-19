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

#include <dirent.h>

int alphasort(const struct dirent** a, const struct dirent** b) {
    return 0;
}

int versionsort(const struct dirent** a, const struct dirent** b) {
    return 0;
}

struct DIR* opendir(const char* path) {
    return nullptr;
}

struct dirent* readdir(struct DIR* dir) {
    return nullptr;
}

int readdir_r(struct DIR* dir, struct dirent* entry, struct dirent** result) {
    return 0;
}

int closedir(struct DIR* dir) {
    return 0;
}

void rewinddir(struct DIR* dir) {
}

long telldir(struct DIR* dir) {
    return 0;
}

void seekdir(struct DIR* dir, long tell) {
}

int scandir(const char* buf, struct dirent*** namelist, scandir_f sf, scandir_alphasort af) {
    return 0;
}


