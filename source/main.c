// Just test

#include <stdio.h>
#include <libavformat/avformat.h>
#include <ft2build.h>
#include FT_FREETYPE_H

int main() {
    // Get FFmpeg version
    unsigned int ffmpeg_version = avformat_version();
    printf("FFmpeg version: %d.%d.%d\n", ffmpeg_version >> 16, (ffmpeg_version & 0xFF00) >> 8, ffmpeg_version & 0xFF);

    // Get Freetype version
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        printf("Could not initialize FreeType library\n");
        return -1;
    }
    FT_Int major, minor, patch;
    FT_Library_Version(library, &major, &minor, &patch);
    printf("Freetype version: %d.%d.%d\n", major, minor, patch);

    // Clean up
    FT_Done_Library(library);

    return 0;
}
