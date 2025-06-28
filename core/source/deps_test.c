#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unicode/urename.h>
#include <unicode/uversion.h>
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#include "vk_mem_alloc.h"
#include "sqlite3.h"
#include "yyjson.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz/hb.h>
#include <xxhash.h>
#define U_DISABLE_RENAMING 1
#include <unicode/utypes.h>
#include <unicode/ustring.h>
#include <unicode/uclean.h>
#include <unicode/udata.h>
#include <unicode/unorm2.h>
#include <unicode/ubrk.h>
#if defined (_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <libgen.h>
#endif

char const *
get_executable_directory() {
    char path_buf[1024] = {0};
#ifdef _WIN32
    GetModuleFileNameA(NULL, path_buf, sizeof(path_buf));
    char* last_slash = strrchr(path_buf, '\\');
    if (last_slash) {
        *last_slash = '\0';
    }
#else
    readlink("/proc/self/exe", path_buf, sizeof(path_buf));
    char* dir = dirname(path_buf);
    return dir;
#endif
    return strdup(path_buf);
}

int 
main()
{
    puts("--- Verifying library integrations ---");

    uint32_t instance_version = 0;
    vkEnumerateInstanceVersion(&instance_version);
    printf(" Vulkan header works! API Version: %u.%u.%u\n", VK_API_VERSION_MAJOR(instance_version), VK_API_VERSION_MINOR(instance_version), VK_API_VERSION_PATCH(instance_version));

    vec4 v = {1.0f, 2.0f, 3.0f, 4.0f};
    if (glm_vec4_norm(v) > 0) {
        puts("cglm header works!");
    }

    puts("VMA header works! (header-only)");
    printf("SQLite header and library work! Version: %s\n", sqlite3_libversion());

    yyjson_doc * doc = yyjson_read("{\"hello\":\"world\"}", 17, 0);
    if (doc)
    {
        puts("yyjson header and library work! Read a simple JSON");
        yyjson_doc_free(doc);
    }

    FT_Library ft_library;
    if (!FT_Init_FreeType(&ft_library))
    {
        FT_Int major , minor , patch;
        FT_Library_Version(ft_library, &major, &minor, &patch);
        printf("FreeType header and library work! Version: %d.%d.%d\n", major, minor, patch);
        FT_Done_FreeType(ft_library);
    }

    hb_buffer_t * hb_buffer = hb_buffer_create();
    if (hb_buffer)
    {
        printf("HarfBuzz header and library work! Version: %s\n", hb_version());
        hb_buffer_destroy(hb_buffer);
    }

    char const * some_data = "Hello, world!";
    XXH64_hash_t hash = XXH3_64bits(some_data, strlen(some_data));
    if (hash)
    {
        printf("XXHash header and library work! Version: %s\n", XXH_VERSION);
    }

    char const * version_str = archive_version_string();
    if (version_str)
    {
        printf("libarchive header and library work! Version: %s\n", version_str);
    }

    puts("--- Verifying ICU ---");

    char const * exec_dir = get_executable_directory();
    u_setDataDirectory(exec_dir);

    UVersionInfo icu_version_array;
    u_getVersion(icu_version_array);
    char icu_version_string[U_MAX_VERSION_STRING_LENGTH];
    u_versionToString(icu_version_array , icu_version_string);
    printf("ICU version: %s\n", icu_version_string);

    
}