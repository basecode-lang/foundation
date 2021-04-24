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

#include <basecode/core/utf.h>
#include <basecode/binfmt/cv.h>
#include <basecode/core/bits.h>
#include <basecode/core/string.h>
#include <basecode/core/numbers.h>
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/slice_utils.h>

#include "ar.cpp"
#include "coff.cpp"
#include "elf.cpp"
#include "macho.cpp"
#include "pe.cpp"
