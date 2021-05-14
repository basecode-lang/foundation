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

#define FOOTERS             1
#define MSPACES             1
#define HAVE_MMAP           1
#define USE_LOCKS           0
#define HAVE_MORECORE       0
#define USE_DL_PREFIX       1
#define MALLOC_INSPECT_ALL  1

#ifndef DLMALLOC_IMPL_UNIT
#include <basecode/core/memory/system/dlmalloc.h>
#endif
