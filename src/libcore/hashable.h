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

#pragma once

#include "types.h"

namespace basecode::hashing {

    template <typename K> u32 hash32(const K& value);
    template <typename K> u64 hash64(const K& value);

    inline u32 hash32(const u8& key)    { return key; }
    inline u64 hash64(const u8& key)    { return key; }

    inline u32 hash32(const s8& key)    { return key; }
    inline u64 hash64(const s8& key)    { return key; }

    inline u32 hash32(const u16& key)   { return key; }
    inline u64 hash64(const u16& key)   { return key; }

    inline u32 hash32(const s16& key)   { return key; }
    inline u64 hash64(const s16& key)   { return key; }

    inline u32 hash32(const u32& key)   { return key; }
    inline u64 hash64(const u32& key)   { return key; }

    inline u32 hash32(const s32& key)   { return key; }
    inline u64 hash64(const s32& key)   { return key; }

    inline u64 hash64(const s64& key)   { return key; }
    inline u64 hash64(const u64& key)   { return key; }
}
