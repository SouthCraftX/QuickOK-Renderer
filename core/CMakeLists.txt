cmake_minimum_required(VERSION 3.16)
project(QuickOK-Engine)

if(VULKAN_SDK_PATH)
    find_package(Vulkan REQUIRED)
else()
    add_library(Vulkan::Vulkan INTERFACE IMPORTED)
    target_include_directories(Vulkan::Vulkan INTERFACE "${VULKAN_SDK_PATH}/include")
    target_link_libraries(Vulkan::Vulkan INTERFACE "${VULKAN_SDK_PATH}/lib/vulkan-1.lib")
endif()

set(INSTALL_DIR ${CMAKE_PREFIX_PATH})


add_library(ZLIB::ZLIB STATIC IMPORTED GLOBAL)
set_target_properties(ZLIB::ZLIB PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libzlibstatic.a")

add_library(PNG::PNG STATIC IMPORTED GLOBAL)
set_target_properties(PNG::PNG PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libpng.a")

add_library(Brotli::common STATIC IMPORTED GLOBAL)
set_target_properties(Brotli::common PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libbrotlicommon.a")
add_library(Brotli::dec STATIC IMPORTED GLOBAL)
set_target_properties(Brotli::dec PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libbrotlidec.a")
add_library(Brotli::enc STATIC IMPORTED GLOBAL)
set_target_properties(Brotli::enc PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libbrotlienc.a")

add_library(Freetype::Freetype STATIC IMPORTED GLOBAL)
set_target_properties(Freetype::Freetype PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libfreetype.a")

add_library(HarfBuzz::HarfBuzz STATIC IMPORTED GLOBAL)
set_target_properties(HarfBuzz::HarfBuzz PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libharfbuzz.a")

add_library(xxHash::xxhash STATIC IMPORTED GLOBAL)
set_target_properties(xxHash::xxhash PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libxxhash.a")

add_library(LibLZMA::LibLZMA STATIC IMPORTED GLOBAL)
set_target_properties(LibLZMA::LibLZMA PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/liblzma.a")

add_library(ZSTD::ZSTD STATIC IMPORTED GLOBAL)
set_target_properties(ZSTD::ZSTD PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libzstd.a")

add_library(Archive::Archive STATIC IMPORTED GLOBAL)
set_target_properties(Archive::Archive PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libarchive.a")

add_library(cglm::cglm SHARED IMPORTED GLOBAL)
set_target_properties(cglm::cglm PROPERTIES IMPORTED_IMPLIB "${INSTALL_DIR}/lib/libcglm.a")

add_library(yyjson::yyjson STATIC IMPORTED GLOBAL)
set_target_properties(yyjson::yyjson PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libyyjson.a")

add_library(ICU::data STATIC IMPORTED GLOBAL)
set_target_properties(ICU::data PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libsicudt.a")

add_library(ICU::uc STATIC IMPORTED GLOBAL)
set_target_properties(ICU::uc PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libsicuuc.a")
target_link_libraries(ICU::uc INTERFACE ICU::data)

add_library(ICU::i18n STATIC IMPORTED GLOBAL)
set_target_properties(ICU::i18n PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/libsicuin.a")
target_link_libraries(ICU::i18n INTERFACE ICU::uc)

file(GLOB_RESCURSE ENGINE_SOURCES "source/*.c")
file(GLOB_RESCURSE ENGINE_HEADERS "include/*.h")

add_library(libQuickOK_Engine SHARED
    ${ENGINE_SOURCES}
)

target_include_directories(libQuickOK_Engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    libQuickOK_Engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(
    libQuickOK_Engine PRIVATE
    Freetype::Freetype
    HarfBuzz::HarfBuzz
    Brotli::dec
    Brotli::enc
    Brotli::common
    PNG::PNG
    ZLIB::ZLIB
    xxHash::xxhash
    cglm::cglm
    yyjson::yyjson
    ICU::i18n
    ICU::uc
    ICU::data
)

include(GNUInstallDirs)
install(TARGETS libQuickOK_Engine
    EXPORT libQuickOK_Engine-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
include(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
