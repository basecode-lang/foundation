cmake_minimum_required(VERSION 3.16)
project(basecode-gfx C CXX ASM)

# -----------------------------------------------------------------------------
#
# Project Configuration
#
# -----------------------------------------------------------------------------

set(VERSION_MAJOR   0)
set(VERSION_MINOR   1)
set(PRODUCT_NAME    "Basecode Foundation Graphics Library")
set(LIBRARY_NAME    "basecode-gfx")
set(LIBRARY_TARGET  "${PROJECT_NAME}-lib")
set(GFX_INC_DIR     "${INC_DIR_BASE}/basecode/gfx")

# -----------------------------------------------------------------------------
#
# Dependencies
#
# -----------------------------------------------------------------------------

# glfw
option(GLFW_BUILD_EXAMPLES  "" OFF)
option(GLFW_BUILD_TESTS     "" OFF)
option(GLFW_BUILD_DOCS      "" OFF)
option(GLFW_INSTALL         "" OFF)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

target_compile_options(
    glfw PRIVATE

    -Wno-documentation
    -Wno-reserved-id-macro
    -Wno-extra-semi-stmt
    -Wno-missing-prototypes
    -Wno-sign-conversion
    -Wno-undef
    -Wno-shadow
    -Wno-implicit-int-conversion
    -Wno-implicit-int-float-conversion
    -Wno-cast-qual
    -Wno-bad-function-cast
    -Wno-unused-parameter
    -Wno-missing-field-initializers
    -Wno-pedantic
    -Wno-format-nonliteral
    -Wno-double-promotion
    -Wno-unused-function
    -Wno-missing-variable-declarations
    -Wno-float-equal
)

# libpng
option(PNG_STATIC "" ON)
option(PNG_SHARED "" OFF)
option(PNG_TESTS  "" OFF)
add_subdirectory(libpng EXCLUDE_FROM_ALL)
set(PNG_PNG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libpng")
set(PNG_PNG_INCLUDE_DIRS "${PNG_PNG_INCLUDE_DIR}")
if (MSVC)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set(PNG_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libpng16_staticd.lib")
    else ()
        set(PNG_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libpng16_static.lib")
    endif ()
endif ()

# freetype
option(FT_WITH_ZLIB         "" ON)
option(FT_WITH_BZIP2        "" ON)
option(FT_WITH_PNG          "" ON)
#option(FT_WITH_HARFBUZZ     "" ON)
set(SKIP_INSTALL_ALL ON)
add_subdirectory(freetype EXCLUDE_FROM_ALL)

# -----------------------------------------------------------------------------
#
# Library Configuration Header
#
# -----------------------------------------------------------------------------

configure_file (
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/basecode/gfx/configure.h"
)

# -----------------------------------------------------------------------------
#
# libbasecode-gfx static library
#
# -----------------------------------------------------------------------------

if (APPLE)
    set(FRAMEWORKS

       "-framework OpenGL"
       "-framework Cocoa"
       "-framework IOKit"
       "-framework CoreVideo"
       "-framework Quartz"
       "-framework WebKit"
    )
elseif (WIN32)
    set(FRAMEWORKS

        shcore
    )
else()
    set(FRAMEWORKS "")
endif()

set(HARFBUZZ_SOURCES

    harfbuzz/src/harfbuzz.cc
    harfbuzz/src/hb-aat-layout.h
    harfbuzz/src/hb-aat.h
    harfbuzz/src/hb-blob.h
    harfbuzz/src/hb-buffer.h
    harfbuzz/src/hb-common.h
    harfbuzz/src/hb-deprecated.h
    harfbuzz/src/hb-face.h
    harfbuzz/src/hb-font.h
    harfbuzz/src/hb-map.h
    harfbuzz/src/hb-ot-color.h
    harfbuzz/src/hb-ot-deprecated.h
    harfbuzz/src/hb-ot-font.h
    harfbuzz/src/hb-ot-layout.h
    harfbuzz/src/hb-ot-math.h
    harfbuzz/src/hb-ot-meta.h
    harfbuzz/src/hb-ot-metrics.h
    harfbuzz/src/hb-ot-name.h
    harfbuzz/src/hb-ot-shape.h
    harfbuzz/src/hb-ot-var.h
    harfbuzz/src/hb-ot.h
    harfbuzz/src/hb-set.h
    harfbuzz/src/hb-shape-plan.h
    harfbuzz/src/hb-shape.h
    harfbuzz/src/hb-unicode.h
    harfbuzz/src/hb-version.h
    harfbuzz/src/hb.h
    harfbuzz/src/
)
add_library(harfbuzz ${HARFBUZZ_SOURCES})
target_include_directories(harfbuzz PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/freetype/include)
target_compile_definitions(harfbuzz PRIVATE -DHAVE_FREETYPE)

set(CLIP_SOURCES

   ${GFX_INC_DIR}/clip/clip.h
   ${GFX_INC_DIR}/clip/clip_common.h
   ${GFX_INC_DIR}/clip/clip_win_wic.h
   ${GFX_INC_DIR}/clip/clip_lock_impl.h

   clip/clip.cpp
   clip/image.cpp
   clip/clip_none.cpp
)
if (APPLE)
    set(CLIP_SOURCES ${CLIP_SOURCES} clip/clip_osx.mm)
elseif (WIN32)
    set(CLIP_SOURCES ${CLIP_SOURCES} clip/clip_win.cpp)
elseif (UNIX)
    set(CLIP_SOURCES

        ${CLIP_SOURCES}
        clip/clip_x11.cpp
        ${GFX_INC_DIR}/clip/clip_x11_png.h
    )
endif()

set(IMPLOT_SOURCES

    implot/implot.cpp
    implot/implot_items.cpp
    ${GFX_INC_DIR}/implot/implot.h
    ${GFX_INC_DIR}/implot/implot_internal.h
)

set(IMGUI_SOURCES

   ${GFX_INC_DIR}/imgui/imgui.h
   ${GFX_INC_DIR}/imgui/imconfig.h
   ${GFX_INC_DIR}/imgui/imgui_internal.h
   ${GFX_INC_DIR}/imgui/imstb_rectpack.h
   ${GFX_INC_DIR}/imgui/imstb_textedit.h
   ${GFX_INC_DIR}/imgui/imstb_truetype.h
   ${GFX_INC_DIR}/imgui/imgui_freetype.h
   ${GFX_INC_DIR}/imgui/imgui_impl_glfw.h
   ${GFX_INC_DIR}/imgui/imgui_impl_opengl3.h
   ${GFX_INC_DIR}/imgui/imgui_memory_editor.h

   imgui/imgui.cpp
   imgui/imgui_draw.cpp
   imgui/imgui_tables.cpp
   imgui/imgui_widgets.cpp
   imgui/imgui_freetype.cpp
   imgui/imgui_impl_glfw.cpp
   imgui/imgui_impl_opengl3.cpp
)

set(GL3W_SOURCES

   ${GFX_INC_DIR}/gl3w/GL/gl3w.h
   ${GFX_INC_DIR}/gl3w/GL/glcorearb.h
   ${GFX_INC_DIR}/gl3w/KHR/khrplatform.h

   gl3w/gl3w.c
)

set(GFX_SOURCES

    ${GFX_INC_DIR}/gfx.h

    app.cpp ${GFX_INC_DIR}/app.h

    tool/alloc.cpp ${GFX_INC_DIR}/tool/alloc.h
    tool/errors.cpp ${GFX_INC_DIR}/tool/errors.h
    tool/strings.cpp ${GFX_INC_DIR}/tool/strings.h

    font.cpp ${GFX_INC_DIR}/font.h

    gfx.cpp ${GFX_INC_DIR}/gfx.h

    ${GFX_INC_DIR}/fonts/IconsKenney.h
    ${GFX_INC_DIR}/fonts/IconsFontaudio.h
    ${GFX_INC_DIR}/fonts/IconsForkAwesome.h
    ${GFX_INC_DIR}/fonts/IconsFontAwesome4.h
    ${GFX_INC_DIR}/fonts/IconsFontAwesome5.h
    ${GFX_INC_DIR}/fonts/IconsFontAwesome5Brands.h
)

add_library(
    ${LIBRARY_TARGET}

    ${GFX_SOURCES}
    ${CLIP_SOURCES}
    ${GL3W_SOURCES}
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
)
target_include_directories(
    ${LIBRARY_TARGET}

    PRIVATE ${INC_DIR_BASE}
            ${GFX_INC_DIR}
            ${GFX_INC_DIR}/clip
            ${GFX_INC_DIR}/gl3w
            ${GFX_INC_DIR}/imgui
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_SOURCE_DIR}/harfbuzz/src
            ${PROJECT_SOURCE_DIR}/freetype/include
)
set_target_properties(
    ${LIBRARY_TARGET} PROPERTIES
    OUTPUT_NAME ${LIBRARY_NAME}
)
target_link_libraries(
    ${LIBRARY_TARGET}

    PRIVATE basecode-core
            glfw
            freetype
            zlibstatic
            png_static
            harfbuzz
            ${FRAMEWORKS}
)

# -----------------------------------------------------------------------------
#
# Interface Library
#
# -----------------------------------------------------------------------------

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
    ${PROJECT_NAME}

    INTERFACE   ${INC_DIR_BASE}
                ${GFX_INC_DIR}
                ${GFX_INC_DIR}/clip
                ${GFX_INC_DIR}/gl3w
                ${GFX_INC_DIR}/imgui
                ${CMAKE_BINARY_DIR}/include
                ${PROJECT_SOURCE_DIR}/harfbuzz/src
                ${PROJECT_SOURCE_DIR}/freetype/include
)
target_link_libraries(
    ${PROJECT_NAME}

    INTERFACE   ${LIBRARY_TARGET}
                basecode-core
                glfw
                freetype
                zlibstatic
                png_static
                harfbuzz
                ${FRAMEWORKS}
)
