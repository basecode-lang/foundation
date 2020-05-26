cmake_minimum_required(VERSION 3.15)
project(core)

set(VERSION_MAJOR   0)
set(VERSION_MINOR   1)
set(PRODUCT_NAME    "Basecode Foundation Core Library")
set(LIBRARY_NAME    "basecode-core")

set(SCM_INC_DIR     "${INC_DIR_BASE}/chibi")
set(CORE_INC_DIR    "${INC_DIR_BASE}/basecode/core")
set(MOD_LIB         "${CMAKE_BINARY_DIR}/lib/lib")
set(MOD_SHARE       "${CMAKE_BINARY_DIR}/share/chibi")
set(MOD_SUFFIX      "${CMAKE_SHARED_LIBRARY_SUFFIX}")

function(add_module)
    cmake_parse_arguments(MOD "" "PATH;TARGET;LINK_TARGET" "MODULES" ${ARGN})
    set(MOD_SOURCES "")
    foreach(arg IN LISTS MOD_MODULES)
        list(APPEND MOD_SOURCES chibi/lib/${MOD_PATH}/${arg}.c)
    endforeach()
    add_library(${MOD_TARGET} SHARED ${MOD_SOURCES})
    target_link_libraries(${MOD_TARGET} ${PROJECT_NAME})
#    set_target_properties(${MODE_TARGET} PROPERTIES LINK_FLAGS "-ftls-model=global-dynamic")
    add_custom_command(
            TARGET ${MOD_TARGET} POST_BUILD
            COMMENT "Symlink ${MOD_TARGET}"
            ${LN} ${MOD_LIB}${MOD_TARGET}${MOD_SUFFIX} ${MOD_SHARE}/${MOD_PATH}/${MOD_LINK_TARGET}${MOD_SUFFIX}
    )
endfunction()

function(add_modules)
    cmake_parse_arguments(MOD "" "PATH" "MODULES" ${ARGN})
    foreach(arg IN LISTS MOD_MODULES)
        add_module(PATH ${MOD_PATH} TARGET ${arg} LINK_TARGET ${arg} MODULES ${arg})
    endforeach()
endfunction()

configure_file (
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/basecode/core/configure.h"
)

set(
        SCM_SOURCES

        ${SCM_INC_DIR}/install.h
        ${SCM_INC_DIR}/features.h
        ${SCM_INC_DIR}/eval.h chibi/eval.c
        ${SCM_INC_DIR}/bignum.h chibi/bignum.c
        ${SCM_INC_DIR}/gc_heap.h chibi/gc_heap.c chibi/gc.c
        ${SCM_INC_DIR}/sexp-huff.h ${SCM_INC_DIR}/sexp-unhuff.h
        ${SCM_INC_DIR}/sexp-hufftabdefs.h ${SCM_INC_DIR}/sexp-hufftabs.h
        ${SCM_INC_DIR}/sexp.h chibi/sexp.c chibi/simplify.c chibi/vm.c chibi/opcodes.c
)

set(
        CORE_SOURCES

        ${CORE_INC_DIR}/assoc_array.h

        ${CORE_INC_DIR}/defer.h
        ${CORE_INC_DIR}/types.h

        ${CORE_INC_DIR}/array.h

        base64.cpp ${CORE_INC_DIR}/base64.h

        bass.cpp ${CORE_INC_DIR}/bass.h

        ${CORE_INC_DIR}/bintree.h

        bits.cpp ${CORE_INC_DIR}/bits.h

        bitset.cpp ${CORE_INC_DIR}/bitset.h

        buf.cpp ${CORE_INC_DIR}/buf.h

        context.cpp ${CORE_INC_DIR}/context.h

        cxx/scope.cpp
        cxx/module.cpp
        cxx/program.cpp
        cxx/serializer.cpp
        cxx/cxx.cpp ${CORE_INC_DIR}/cxx/cxx.h

        ${CORE_INC_DIR}/digraph.h

        error.cpp ${CORE_INC_DIR}/error.h

        filesys.cpp ${CORE_INC_DIR}/filesys.h

        ffi.cpp ${CORE_INC_DIR}/ffi.h

        format.cpp ${CORE_INC_DIR}/format.h ${CORE_INC_DIR}/format_types.h

        gap_buf.cpp ${CORE_INC_DIR}/gap_buf.h

        ${CORE_INC_DIR}/gfx.h

        ${CORE_INC_DIR}/hashable.h

        hash/crc.cpp ${CORE_INC_DIR}/hash/crc.h
        hash/adler.cpp ${CORE_INC_DIR}/hash/adler.h
        hash/murmur.cpp ${CORE_INC_DIR}/hash/murmur.h
        hash/blake3.cpp ${CORE_INC_DIR}/hash/blake3.h
        hash/symbol.cpp ${CORE_INC_DIR}/hash/symbol.h

        ${CORE_INC_DIR}/hashtab.h

        intern.cpp ${CORE_INC_DIR}/intern.h

        ${CORE_INC_DIR}/iterator.h

        ${CORE_INC_DIR}/list.h

        log.cpp ${CORE_INC_DIR}/log.h

        log/system/syslog.cpp ${CORE_INC_DIR}/log/system/syslog.h
        log/system/spdlog.cpp ${CORE_INC_DIR}/log/system/spdlog.h
        log/system/default.cpp ${CORE_INC_DIR}/log/system/default.h

        memory.cpp ${CORE_INC_DIR}/memory.h

        memory/dyn_alloc.cpp memory/scm_alloc.cpp
        memory/meta.cpp ${CORE_INC_DIR}/memory/meta.h
        memory/std_alloc.cpp ${CORE_INC_DIR}/memory/std_alloc.h
        memory/system/dl.cpp ${CORE_INC_DIR}/memory/system/dl.h
        memory/system/page.cpp ${CORE_INC_DIR}/memory/system/page.h
        memory/system/slab.cpp ${CORE_INC_DIR}/memory/system/slab.h
        memory/system/bump.cpp ${CORE_INC_DIR}/memory/system/bump.h
        memory/system/trace.cpp ${CORE_INC_DIR}/memory/system/trace.h
        memory/system/proxy.cpp ${CORE_INC_DIR}/memory/system/proxy.h
        memory/system/stack.cpp ${CORE_INC_DIR}/memory/system/stack.h
        memory/system/default.cpp ${CORE_INC_DIR}/memory/system/default.h
        memory/system/dlmalloc.c ${CORE_INC_DIR}/memory/system/dlmalloc.h ${CORE_INC_DIR}/memory/system/dlmalloc_config.h

        network.cpp ${CORE_INC_DIR}/network.h

        numbers.cpp ${CORE_INC_DIR}/numbers.h

        path.cpp ${CORE_INC_DIR}/path.h

        plot.cpp ${CORE_INC_DIR}/plot.h

        profiler.cpp ${CORE_INC_DIR}/profiler.h

        ${CORE_INC_DIR}/queue.h

        ${CORE_INC_DIR}/set.h

        ${CORE_INC_DIR}/slice.h
        slice_utils.cpp ${CORE_INC_DIR}/slice_utils.h

        scm.cpp ${CORE_INC_DIR}/scm.h

        ${CORE_INC_DIR}/stack.h

        ${CORE_INC_DIR}/stable_array.h

        stopwatch.cpp ${CORE_INC_DIR}/stopwatch.h

        str.cpp ${CORE_INC_DIR}/str.h
        str_array.cpp ${CORE_INC_DIR}/str_array.h

        ${CORE_INC_DIR}/symtab.h

        timer.cpp ${CORE_INC_DIR}/timer.h
)
add_library(${PROJECT_NAME} ${CORE_SOURCES} ${SCM_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-march=native")
#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-ftls-model=global-dynamic")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${LIBRARY_NAME})
target_link_libraries(${PROJECT_NAME} utf8proc blake3 fmt-header-only spdlog::spdlog dyncall_s dynload_s ${DL_LIBRARY})
target_include_directories(${PROJECT_NAME} PUBLIC ${INC_DIR_BASE})
add_custom_command(
        TARGET      ${PROJECT_NAME} POST_BUILD
        COMMENT     "Create share/chibi directory; copy lib"
        ${MKDIR}    ${CMAKE_BINARY_DIR}/share/chibi
        ${CPR}      ${CMAKE_CURRENT_SOURCE_DIR}/chibi/lib ${CMAKE_BINARY_DIR}/share/chibi
)

add_modules(
        PATH chibi
        MODULES ast filesystem weak disasm json heap-stats emscripten process time system stty pty net
)
add_modules(PATH chibi/crypto MODULES crypto)
add_modules(PATH chibi/io MODULES io)
add_modules(PATH chibi/optimize MODULES rest profile)
add_modules(PATH srfi/18 MODULES threads)
add_modules(PATH srfi/27 MODULES rand)
add_modules(PATH srfi/151 MODULES bit)
add_modules(PATH srfi/39 MODULES param)
add_modules(PATH srfi/69 MODULES hash)
add_modules(PATH srfi/95 MODULES qsort)
add_modules(PATH srfi/98 MODULES env)
add_modules(PATH srfi/144 MODULES math)
add_modules(PATH srfi/160 MODULES uvprims)
add_module(PATH scheme TARGET scheme-time LINK_TARGET time MODULES time)

#add_library(disasm SHARED chibi/lib/chibi/disasm.cpp)
#target_link_libraries(disasm ${PROJECT_NAME})
#set_target_properties(disasm PROPERTIES LINK_FLAGS "-ftls-model=global-dynamic")
#add_custom_command(
#        TARGET disasm POST_BUILD
#        COMMENT "Symlink disasm"
#        ${LN} ${MOD_LIB}disasm${MOD_SUFFIX} ${MOD_SHARE}/chibi/disasm${MOD_SUFFIX}
#)
