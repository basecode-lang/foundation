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

#include <basecode/core/types.h>

namespace basecode::binfmt::pdb {
    constexpr const s8* file_magic = "Microsoft C / C++ MSF 7.00\r\n\x1a\x44\x53\x00\x00\x00";

    struct super_block_t final {
        // file_magic
        u32                     block_size;             // 4k
        u32                     free_block_map_block;   // can only be 1 or 2
        u32                     num_blocks;
        u32                     num_dir_bytes;
        u32                     reserved;
        u32                     block_map_addr;
    };

    struct stream_dir_t final {
        u32                     num_streams;
//        u32                     sizes[num_streams];
//        u32                     blocks[num_streams][];
    };

    enum class pdb_stream_ver_t : u32 {
        vc2                     = 19941610,
        vc4                     = 19950623,
        vc41                    = 19950814,
        vc50                    = 19960307,
        vc98                    = 19970604,
        vc70dep                 = 19990604,
        vc70                    = 20000404,
        vc80                    = 20030901,
        vc110                   = 20091201,
        vc140                   = 20140508,
    };

    enum class pdb_feature_t : u32 {
        vc110                   = 20091201,
        vc140                   = 20140508,
        no_type_merge           = 0x4d544f4e,
        minimal_debug_info      = 0x494e494d,
    };

    struct pdb_stream_hdr_t final {
        u32                     version;
        u32                     signature;
        u32                     age;
        u8                      guid[16];
    };

    // on-disk format
    //
    // +-------------------+    0
    // |      Size         |
    // +-------------------+    4
    // |   Capacity        |
    // +-------------------+    8
    // |Present Bit Vector |
    // +-------------------+    N
    // |Deleted Bit Vector |
    // +-------------------+    M                   <----+
    // |       Key         |                             |
    // +-------------------+    M+4                      |
    // |      Value        |                             |
    // +-------------------+    M+4+sizeof(Value)        +--> buckets (Capacity)
    //        .....                                      |
    // +-------------------+                             |
    // |       Key         |                             |
    // +-------------------+                             |
    // |      Value        |                             |
    // +-------------------+                        <----+
    //
    //
    // present/deleted bit vectors:
    //
    // | u32  N | .... | u32 1 | u32 0 |
    //   N*32   N-1 * 32       32      0
    struct hash_entry_t final {
        u32                     key;
        u32                     value;
    };

    struct named_stream_map_t final {
    };

    // tpi & ipi streams
    // codeview type information
    enum simple_type_mode_t : u32 {
        direct                  = 0,
        near_pointer            = 1,
        far_pointer             = 2,
        huge_pointer            = 3,
        near_pointer_32         = 4,
        far_pointer_32          = 5,
        near_pointer_64         = 6,
        near_pointer_128        = 7
    };

    enum simple_type_kind_t : u32 {
        none                    = 0x0000,
        void_                   = 0x0003,
        not_translated          = 0x0007,
        hresult                 = 0x0008,
        signed_character        = 0x0010,
        unsigned_character      = 0x0020,
        narrow_character        = 0x0070,
        wide_character          = 0x0071,
        character_16            = 0x007a,
        character_32            = 0x007b,
        sbyte                   = 0x0068,
        byte                    = 0x0069,
        int16short              = 0x0011,
        uint16short             = 0x0021,
        int16                   = 0x0072,
        uint16                  = 0x0073,
        int32long               = 0x0012,
        uint32long              = 0x0022,
        int32                   = 0x0074,
        uint32                  = 0x0075,
        int64quad               = 0x0013,
        uint64quad              = 0x0023,
        int64                   = 0x0076,
        uint64                  = 0x0077,
        int128oct               = 0x0014,
        uint128oct              = 0x0024,
        int128                  = 0x0078,
        uint128                 = 0x0079,
        float16                 = 0x0046,
        float32                 = 0x0040,
        float32_pp              = 0x0045,
        float48                 = 0x0044,
        float64                 = 0x0041,
        float80                 = 0x0042,
        float128                = 0x0043,
        complex16               = 0x0056,
        complex32               = 0x0050,
        complex32_pp            = 0x0055,
        complex48               = 0x0054,
        complex64               = 0x0051,
        complex80               = 0x0052,
        complex128              = 0x0053,
        boolean8                = 0x0030,
        boolean16               = 0x0031,
        boolean32               = 0x0032,
        boolean64               = 0x0033,
        boolean128              = 0x0034,
    };

    struct type_index_t final {
        u32                     unused: 20;
        u32                     mode:   4;
        u32                     kind:   8;
    };

    enum class dbi_stream_ver_t : u32 {
        vc41                    = 930803,
        v50                     = 19960307,
        v60                     = 19970606,
        v70                     = 19990903,
        v110                    = 20091201
    };

    enum class tpi_stream_ver_t : u32 {
        v40                     = 19950410,
        v41                     = 19951122,
        v50                     = 19961031,
        v70                     = 19990903,
        v80                     = 20040203,
    };

    struct tpi_stream_hdr_t final {
        u32                     version;
        u32                     header_size;
        u32                     type_index_begin;
        u32                     type_index_end;
        u32                     type_record_bytes;
        u16                     hash_stream_index;
        u16                     hash_aux_stream_index;
        u32                     hash_key_size;
        u32                     num_hash_buckets;
        s32                     hash_value_buffer_offset;
        u32                     hash_value_buffer_length;
        s32                     index_offset_buffer_offset;
        u32                     index_offset_buffer_length;
        s32                     hash_adj_buffer_offset;
        u32                     hash_adj_buffer_length;
    };

    struct build_flags_t final {
        u16                     inc_link:               1;
        u16                     private_syms_stripped:  1;
        u16                     has_conflicting_types:  1;
        u16                     reserved:               13;
    };

    struct build_number_t final {
        u16                     minor_version:      8;
        u16                     major_version:      7;
        u16                     new_version_fmt:    1;
    };

    struct dbi_stream_hdr_t final {
        s32                     version_signature;
        u32                     version_header;
        u32                     age;
        u16                     global_stream_index;
        u16                     build_number;
        u16                     public_stream_index;
        u16                     pdb_dll_version;
        u16                     sym_record_stream;
        u16                     pdb_dll_rbld;
        s32                     mod_info_size;
        s32                     section_contribution_size;
        s32                     section_map_size;
        s32                     source_info_size;
        s32                     type_server_map_size;
        u32                     mfc_type_server_index;
        s32                     optional_dbg_header_size;
        s32                     ec_sub_stream_size;
        u16                     flags;
        u16                     machine;        // 0x8664?
        u32                     padding;
    };

    // module info substream
//    struct ModInfo {
//        uint32_t Unused1;
//        struct SectionContribEntry {
//            uint16_t Section;
//            char Padding1[2];
//            int32_t Offset;
//            int32_t Size;
//            uint32_t Characteristics;
//            uint16_t ModuleIndex;
//            char Padding2[2];
//            uint32_t DataCrc;
//            uint32_t RelocCrc;
//        } SectionContr;
//        uint16_t Flags;
//        uint16_t ModuleSymStream;
//        uint32_t SymByteSize;
//        uint32_t C11ByteSize;
//        uint32_t C13ByteSize;
//        uint16_t SourceFileCount;
//        char Padding[2];
//        uint32_t Unused2;
//        uint32_t SourceFileNameIndex;
//        uint32_t PdbFilePathNameIndex;
//        char ModuleName[];
//        char ObjFileName[];
//    };
// ``true`` if this ModInfo has been written since reading the PDB.  This is
// likely used to support incremental linking, so that the linker can decide
// if it needs to commit changes to disk.
//    uint16_t Dirty : 1;
// ``true`` if EC information is present for this module. EC is presumed to
// stand for "Edit & Continue", which LLVM does not support.  So this flag
// will always be be false.
//    uint16_t EC : 1;
//    uint16_t Unused : 6;
// Type Server Index for this module.  This is assumed to be related to /Zi,
// but as LLVM treats /Zi as /Z7, this field will always be invalid for LLVM
// generated PDBs.
//    uint16_t TSM : 8;

    // section contribution substream
    // version: u32
    //enum class SectionContrSubstreamVersion : uint32_t {
    //  Ver60 = 0xeffe0000 + 19970605,
    //  V2 = 0xeffe0000 + 20140516
    //};
    //
    // if Ver60 then array of SectionContribEntry (above)
    // else array of
    //struct SectionContribEntry2 {
    //  SectionContribEntry SC;
    //  uint32_t ISectCoff;
    //};

    // section map substream
    //struct SectionMapHeader {
    //  uint16_t Count;    // Number of segment descriptors
    //  uint16_t LogCount; // Number of logical segment descriptors
    //};
    //
    //struct SectionMapEntry {
    //  uint16_t Flags;         // See the SectionMapEntryFlags enum below.
    //  uint16_t Ovl;           // Logical overlay number
    //  uint16_t Group;         // Group index into descriptor array.
    //  uint16_t Frame;
    //  uint16_t SectionName;   // Byte index of segment / group name in string table, or 0xFFFF.
    //  uint16_t ClassName;     // Byte index of class in string table, or 0xFFFF.
    //  uint32_t Offset;        // Byte offset of the logical segment within physical segment.  If group is set in flags, this is the offset of the group.
    //  uint32_t SectionLength; // Byte count of the segment or group.
    //};
    //
    //enum class SectionMapEntryFlags : uint16_t {
    //  Read = 1 << 0,              // Segment is readable.
    //  Write = 1 << 1,             // Segment is writable.
    //  Execute = 1 << 2,           // Segment is executable.
    //  AddressIs32Bit = 1 << 3,    // Descriptor describes a 32-bit linear address.
    //  IsSelector = 1 << 8,        // Frame represents a selector.
    //  IsAbsoluteAddress = 1 << 9, // Frame represents an absolute address.
    //  IsGroup = 1 << 10           // If set, descriptor represents a group.
    //};

    // file info substream
    //struct FileInfoSubstream {
    //  uint16_t NumModules;
    //  uint16_t NumSourceFiles;
    //
    //  uint16_t ModIndices[NumModules];
    //  uint16_t ModFileCounts[NumModules];
    //  uint32_t FileNameOffsets[NumSourceFiles];
    //  char NamesBuffer[][NumSourceFiles];
    //};

    // type server map substream
    // M$ type server, won't support

    // optional debug header stream
    // array of u16 stream indices, -1 means empty
    // 0 = fpo data
    // 1 = exception data (.pdata from exe?)
    // 2 = fixup data
    // 3 = omap to src data
    // 4 = omap from src data
    // 5 = section header data
    // 6 = token/rid map
    // 7 = copy of .xdata from exe
    // 8 = copy of .pdata from exe
    // 9 = new fpo data
    // 10 = original section header data

    // ec substream
    // M$ edit & continue, won't support
}
